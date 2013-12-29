/*
Copyright (c) 2014, Qiangli Zhao and Yanhuang Jiang
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

* Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <vector>
#include <ctime>
#include <set>
#include <cmath>
#include <iostream>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Prediction.h"
#include "Classifier.h"
#include "IncrementalClassifier.h"
#include "Ensemble.h"
#include "IncrementalEnsemble.h"
#include "GaussNaiveBayes.h"
#include "C45.h"
#include "Statistic.h"
#include "ACE.h"
using namespace libep;


const char	MyName[MAX_OBJECT_NAME_LENGTH]="ACE_ENSEMBLE";

CACE::CACE(int uMaxSize,double uAlpha,int uSa,int uSc,double uu,
	IncermentalCreateFunc *uOnline,const void *uOnlineParams,
	CreateFunc *uBatch,const void *uBtachParams)
:MaxSize(uMaxSize),Memory(NULL),OnlineJustCreated(false),Alpha(uAlpha),u(uu),Sa(uSa),Sc(uSc),Za2(CStat::rGauss(1-Alpha/2.0))
{
	if(Sa>=Sc)
		throw(CError("ACE: Sa>=Sc!",100,0));
	Name=MyName;
	CreatingTime=0;
	Times=0;

	OnlineLearner.Creator=uOnline;
	OnlineLearner.Params=uOnlineParams;
	BatchLearner.Creator=uBatch;
	BatchLearner.Params=uBtachParams;
}

CACE::~CACE()
{
	if(Memory!=NULL)
		delete Memory;
}

void CACE::Train(const CDataset &Dataset)
{	
	//start time for training
	clock_t start=clock();
	//data
	const MATRIX &TrainData=Dataset.GetData();
	const CASE_INFO &CaseInfo=Dataset.GetInfo();

	//we treat this data set as a data stream
	for(int i=0;i<CaseInfo.Height;i++)
	{
		//clock pulse
		Times++;
		//if concept drifted? (just reseted?) 
		bool AcqFlag=false;

		//each time processing a single instance
		MATRIX Instances;
		Instances.push_back(TrainData[i]);
		//prepare this instance as a data set
		CDataset Inst(CaseInfo,Instances);

		//save the new instance
		if(Memory==NULL)
			Memory=new CDataset(CaseInfo,Instances);
		else
		{
			//remove the oldest instance if buffer is full (because the buffer will be cleared periodically, so this isn't gonna to happen)
			if(Memory->GetInfo().Height>=Sc)
			{
				throw(CError("ACE: memory size>=Sc is not supposed to be happened!",200,0));
				Memory->Remove(0);
			}
			//add
			Memory->Insert(TrainData[i]);
		}

		//create C0 if there isn't one
		if(GetSize()<=0)
		{
			CIncrementalClassifier *Online=OnlineLearner.Creator(Inst,OnlineLearner.Params);
			ClassifierInfoStr ClassifierInfo;
			ClassifierInfo.CreateTime=Times;
			Infos.insert(Infos.begin(),ClassifierInfo);
			Classifiers.push_back(Online);
			Weights.push_back(1);
			OnlineJustCreated=true;
		}
		else
			OnlineJustCreated=false;

		//Best classifier in this round
		int Best=-1;
		double BestA=0;
		//all classifiers predict it and update their information
		vector<CPrediction*> *Predictions=AllClassify(Inst);
		for(int j=0;j<(int)Predictions->size();j++)
		{
			//save each classifier's prediction
			const BoolArray &Correctness=(*Predictions)[j]->GetCorrectness();
			//remove the oldest instance if short term buffer is full
			if((int)Infos[j].CR.size()>=Sa)
			{
				Infos[j].CR.erase(Infos[j].CR.begin());
				Infos[j].As.erase(Infos[j].As.begin());
				Infos[j].ALowers.erase(Infos[j].ALowers.begin());
				Infos[j].AUppers.erase(Infos[j].AUppers.begin());
			}
			//add new instance's prediction result
			if(Correctness[0])
				Infos[j].CR.push_back(1);
			else
				Infos[j].CR.push_back(0);


			//suitability
			int ShortTerm=(int)Infos[j].CR.size();
			if(j>0 && ShortTerm!=Sa)
				throw(CError("ACE: classifier's buffer error!",201,0));
			//get A(j,n)
			double A=0;
			for(int k=0;k<ShortTerm;k++)
				A+=((double)Infos[j].CR[k]/ShortTerm);
			Infos[j].As.push_back(A);
			//sum of all A(j,n)
			double SumA=0;
			for(int k=0;k<ShortTerm;k++)
				SumA+=Infos[j].As[k];
			//lower and upper endpoints
			{
				double sum=Za2*sqrt(A*(1-A)/ShortTerm+Za2*Za2/(4.0*ShortTerm*ShortTerm));
				double ALower=ShortTerm/(ShortTerm+Za2*Za2)*(A+Za2*Za2/(2*ShortTerm)-sum);
				double AUpper=ShortTerm/(ShortTerm+Za2*Za2)*(A+Za2*Za2/(2*ShortTerm)+sum);

				Infos[j].ALowers.push_back(ALower);
				Infos[j].AUppers.push_back(AUpper);
				Weights[j]=pow(1.0/(1-A+0.001),u);
			}
			//find best classifier
			if(j>0 && SumA>BestA)
			{
				BestA=SumA;
				Best=j;
			}

			delete (*Predictions)[j];
		}//update A(j,n)
		delete Predictions;

		//Update on-line classifier (don't do it when it has just created)
		if(!OnlineJustCreated)
		{
			CIncrementalClassifier *Online=dynamic_cast<CIncrementalClassifier*> (Classifiers[0]);
			Online->Train(Inst);
		}

		//concept drifted?
		if(Best>=0)
		{
			if(Infos[Best].As.size()!=Sa||Infos[Best].ALowers.size()!=Sa||Infos[Best].AUppers.size()!=Sa)
				throw(CError("ACE: classifier's buffer error!",202,0));
			if(Infos[Best].As.back()<Infos[Best].ALowers[0]||Infos[Best].As.back()>Infos[Best].AUppers[0])
			{
				if(Times-Infos[0].CreateTime>=Sa && Times-Infos[Best].CreateTime>=2*Sa)
					AcqFlag=true;
			}
		}
		if(Times-Infos[0].CreateTime>=Sc-1)
			AcqFlag=true;
		if(!AcqFlag)
			continue;

		//new classifier
		CClassifier *Cj1=BatchLearner.Creator(*Memory,BatchLearner.Params);
		CPrediction *Prediction=Cj1->Classify(*Memory);
		double Accuracy=Prediction->GetAccuracy();
		delete Prediction;
		//not a good classifier, discarded
		if(Accuracy<=(1.0/CaseInfo.ClassNum))
		{
// 			cout<<Times<<": ACE abandoned"<<endl;
			delete Cj1;
		}
		else//add to ensemble
		{
			//remove the worst classifier at present(other than C0)
			if(MaxSize>0)
			{
				while(GetSize()>=MaxSize)
				{
					int Worst=1;
					double WorstA=Infos[1].As.back();
					for(int j=2;j<GetSize();j++)
						if(Infos[j].As.back()<WorstA)
						{
							WorstA=Infos[j].As.back();
							Worst=j;
						}

					delete (Classifiers[Worst]);
					Classifiers.erase(Classifiers.begin()+Worst);
					Weights.erase(Weights.begin()+Worst);
					Infos.erase(Infos.begin()+Worst);
				}
			}

			//add
			ClassifierInfoStr ClassifierInfo;
			ClassifierInfo.CreateTime=Times;
			Infos.push_back(ClassifierInfo);
			Infos.back().CR.assign(Infos[0].CR.begin(),Infos[0].CR.end());
			Infos.back().As.assign(Infos[0].As.begin(),Infos[0].As.end());
			Infos.back().ALowers.assign(Infos[0].ALowers.begin(),Infos[0].ALowers.end());
			Infos.back().AUppers.assign(Infos[0].AUppers.begin(),Infos[0].AUppers.end());
			Classifiers.push_back(Cj1);
			Weights.push_back(pow(1.0/(1-Infos[0].As.back()+0.001),u));
		}//adding a new classifier
		//reset data buffer
		Memory->ClearData();
		//reset C0
		{
			CIncrementalClassifier *Online=dynamic_cast<CIncrementalClassifier*> (Classifiers[0]);
			Online->Reset();

			Weights[0]=0;
			
			Infos[0].CR.clear();
			Infos[0].As.clear();
			Infos[0].ALowers.clear();
			Infos[0].AUppers.clear();
			Infos[0].CreateTime=Times;
		}
	}//end of an instance

	//time consumed
	CreatingTime +=((double)(clock() - start) / CLOCKS_PER_SEC);
}

/*
many thing unreasonable in the paper, so change it
void CACE::Train(const CDataset &Dataset)
{	
	//start time for training
	clock_t start=clock();
	//data
	const MATRIX &TrainData=Dataset.GetData();
	const CASE_INFO &CaseInfo=Dataset.GetInfo();

	//we treat this data set as a data stream
	for(int i=0;i<CaseInfo.Height;i++)
	{
		//clock pulse
		Times++;
		//if concept drifted? (just reseted?) 
		bool AcqFlag=false;

		//each time processing a single instance
		MATRIX Instances;
		Instances.push_back(TrainData[i]);
		//prepare this instance as a data set
		CDataset Inst(CaseInfo,Instances);

		//save the new instance
		if(Memory==NULL)
			Memory=new CDataset(CaseInfo,Instances);
		else
		{
			//remove the oldest instance if buffer is full (because the buffer will be cleared periodically, so this isn't gonna to happen)
			if(Memory->GetInfo().Height>=Sc)
			{
				throw(CError("ACE: memory size>=Sc is not supposed to be happened!",200,0));
				Memory->Remove(0);
			}
			//add
			Memory->Insert(TrainData[i]);
		}

		//create C0 if there isn't one
		if(GetSize()<=0)
		{
			CIncrementalClassifier *Online=OnlineLearner.Creator(Inst,OnlineLearner.Params);
			ClassifierInfoStr ClassifierInfo;
			ClassifierInfo.CreateTime=Times;
			Infos.insert(Infos.begin(),ClassifierInfo);
			Classifiers.push_back(Online);
			Weights.push_back(1);
			OnlineJustCreated=true;
		}
		else
			OnlineJustCreated=false;

		//Best classifier in this round
		int Best=-1;
		double BestA=0;
		//all classifiers predict it and update their information
		vector<CPrediction*> *Predictions=AllClassify(Inst);
		for(int j=0;j<(int)Predictions->size();j++)
		{
			//save each classifier's prediction
			const BoolArray &Correctness=(*Predictions)[j]->GetCorrectness();
			//remove the oldest instance if short term buffer is full
			if((int)Infos[j].CR.size()>=Sa)
			{
				Infos[j].CR.erase(Infos[j].CR.begin());
				Infos[j].A=0;
				Infos[j].ALower=0;
				Infos[j].AUpper=0;
			}
			//add new instance's prediction result
			if(Correctness[0])
				Infos[j].CR.push_back(1);
			else
				Infos[j].CR.push_back(0);


			//suitability
			int ShortTerm=(int)Infos[j].CR.size();
			if(j>0 && ShortTerm!=Sa)
				throw(CError("ACE: classifier's buffer error!",201,0));
			//get A(j,n)
			double A=0;
			for(int k=0;k<ShortTerm;k++)
				A+=((double)Infos[j].CR[k]/ShortTerm);
			Infos[j].A=A;
			//lower and upper endpoints
			{
				double sum=Za2*sqrt(A*(1-A)/ShortTerm+Za2*Za2/(4.0*ShortTerm*ShortTerm));
				double ALower=ShortTerm/(ShortTerm+Za2*Za2)*(A+Za2*Za2/(2*ShortTerm)-sum);
				double AUpper=ShortTerm/(ShortTerm+Za2*Za2)*(A+Za2*Za2/(2*ShortTerm)+sum);

				Infos[j].ALower=ALower;
				Infos[j].AUpper=AUpper;
				Weights[j]=pow(1.0/(1-A+0.001),u);
			}
			//find best classifier
			if(j>0 && A>BestA)
			{
				BestA=A;
				Best=j;
			}

			delete (*Predictions)[j];
		}//update A(j,n)
		delete Predictions;

		//Update on-line classifier (don't do it when it has just created)
		if(!OnlineJustCreated)
		{
			CIncrementalClassifier *Online=dynamic_cast<CIncrementalClassifier*> (Classifiers[0]);
			Online->Train(Inst);
		}

		//concept drifted?
		if(Best>=0)
		{
			// 			if(Infos[Best].As.size()!=Sa||Infos[Best].ALowers.size()!=Sa||Infos[Best].AUppers.size()!=Sa)
			// 				throw(CError("ACE: classifier's buffer error!",202,0));
			if(Infos[Best].A<Infos[Best].ALower||Infos[Best].A>Infos[Best].AUpper)
			{
				if(Times-Infos[0].CreateTime>=Sa && Times-Infos[Best].CreateTime>=2*Sa)
					AcqFlag=true;
			}
		}
		if(Times-Infos[0].CreateTime>=Sc-1)
			AcqFlag=true;
		if(!AcqFlag)
			continue;

		//new classifier
		CClassifier *Cj1=BatchLearner.Creator(*Memory,BatchLearner.Params);
		CPrediction *Prediction=Cj1->Classify(*Memory);
		double Accuracy=Prediction->GetAccuracy();
		cout<<"ACE accuracy: "<<Accuracy<<endl;
		//anyway, a classifier need be added to ensemble
		ClassifierInfoStr ClassifierInfo;
		ClassifierInfo.CreateTime=Times;
		Infos.push_back(ClassifierInfo);
		//new created is not a good classifier, discard it and clone C0
		if(Accuracy<=(1.0/CaseInfo.ClassNum))
		{
			cout<<Times<<": ACE abandoned"<<endl;
			delete Cj1;

			Cj1=Classifiers[0]->Clone();
			Infos.back().CR.assign(Infos[0].CR.begin(),Infos[0].CR.end());
			Infos.back().A=Infos[0].A;
			Infos.back().ALower=Infos[0].ALower;
			Infos.back().AUpper=Infos[0].AUpper;
		}
		else//add to ensemble
		{
			//remove the worst classifier at present(other than C0)
			if(MaxSize>0)
			{
				while(GetSize()>=MaxSize)
				{
					int Worst=1;
					double WorstA=Infos[1].A;
					for(int j=2;j<GetSize();j++)
						if(Infos[j].A<WorstA)
						{
							WorstA=Infos[j].A;
							Worst=j;
						}

						delete (Classifiers[Worst]);
						Classifiers.erase(Classifiers.begin()+Worst);
						Weights.erase(Weights.begin()+Worst);
						Infos.erase(Infos.begin()+Worst);
				}
			}

			//add
			int ShortTerm=(int)Infos[0].CR.size();
			const BoolArray &Correctness=Prediction->GetCorrectness();
			if(ShortTerm!=Sa || (int)Correctness.size()<Sa)
				throw(CError("ACE: classifier's buffer error!",201,0));

			//CR, A
			Infos.back().A=0;
			for(int j=Sa-1;j>=0;j--)
			{
				if(Correctness[j])
				{
					Infos.back().CR.push_back(1.0);
					Infos.back().A+=(1.0/Sa);
				}
				else
					Infos.back().CR.push_back(0);
			}
			//Au, Al
			double sum=Za2*sqrt(Accuracy*(1-Accuracy)/ShortTerm+Za2*Za2/(4.0*ShortTerm*ShortTerm));
			Infos.back().ALower=ShortTerm/(ShortTerm+Za2*Za2)*(Accuracy+Za2*Za2/(2*ShortTerm)-sum);
			Infos.back().AUpper=ShortTerm/(ShortTerm+Za2*Za2)*(Accuracy+Za2*Za2/(2*ShortTerm)+sum);
		}//adding a new classifier
		Classifiers.push_back(Cj1);
		Weights.push_back(pow(1.0/(1-Infos.back().A+0.001),u));
		delete Prediction;

		//reset data buffer
		Memory->ClearData();
		//reset C0
		{
			CIncrementalClassifier *Online=dynamic_cast<CIncrementalClassifier*> (Classifiers[0]);
			Online->Reset();

			Weights[0]=0;

			Infos[0].CR.clear();
			Infos[0].A=0;
			Infos[0].ALower=0;
			Infos[0].AUpper=0;
			Infos[0].CreateTime=Times;
		}
	}//end of an instance (i)

	//time consumed
	CreatingTime +=((double)(clock() - start) / CLOCKS_PER_SEC);
}*/

