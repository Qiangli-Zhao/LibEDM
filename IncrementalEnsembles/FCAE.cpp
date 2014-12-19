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
#include <iostream>
#include <cmath>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Prediction.h"
#include "Classifier.h"
#include "IncrementalClassifier.h"
#include "Ensemble.h"
#include "IncrementalEnsemble.h"
#include "IncrementalTrunkEnsemble.h"
#include "EnsemblePruner.h"
#include "Cluster.h"
#include "MDSQ.h"
#include "FS.h"
#include "C45.h"
#include "FCAE.h"
#include "OrientOrder.h"
using namespace libedm;


const char	MyName[MAX_OBJECT_NAME_LENGTH]="FCAE_ENSEMBLE";

CFCAE::CFCAE(int uMaxSize,double uAlpha,int uTargetSize,CreateFunc *Func,const void *CreatorParams)
:MaxSize(uMaxSize),Alpha(uAlpha),TargetSize(uTargetSize)
{
	Name=MyName;
	CreatingTime=0;
	Times=0;
	if(TargetSize<=0)
		TargetSize=MaxSize/5;

	Creator.Creator=Func;
	Creator.Params=CreatorParams;
}

//remove pruned classifiers and replace the worst one
// void CFCAE::Train(const CDataset &Dataset)
// {	
// 	//start time for training
// 	clock_t start=clock();
// 	//number of training
// 	Times++;
// 	//initializing weight for a new classifier
// 	const double IniWeight=1.0;
// 	//if we need to remove a classifier before create a new one
// 	int Worst=-1;
// 	//change weights for the rest classifiers
// 	//and find the classifier with least weight
// 	double WorstWeight=IniWeight;
// 
// 	//pruning
// 	CCluster Pruner(*this,Dataset);
// 	const vector<double> &PrunedWeights=Pruner.GetWeights();
// 	for(int i=0;i<(int)Weights.size();i++)
// 	{
// 		//which classifier is kept by the pruner
// 		if(PrunedWeights[i]!=0)
// 		{
// 			Recalls[i]++;
// 			Taos[i]=Times;
// 		}
// 		else
// 		{
// 			Weights[i]=0;
// 			continue;
// 		}
// 
// 		//new weights for rest classifier of this ensemble
// 		double ForgetFactor=Alpha/Recalls[i];
// 		Weights[i]=exp(double(-1.0)*ForgetFactor*(Times-Taos[i]));
// 		//find the classifier with least weight
// 		if(Weights[i]<WorstWeight)
// 		{
// 			Worst=i;
// 			WorstWeight=Weights[i];
// 		}
// 	}
// 	if(Worst>=0)
// 		Weights[Worst]=0;
// 	//removed classifiers that is the worst or are pruned by the pruner
// 	for(int i=0;i<(int)Weights.size();i++)
// 		if(Weights[i]==0)
// 		{
// 			delete Classifiers[i];
// 			Classifiers.erase(Classifiers.begin()+i);
// 			Recalls.erase(Recalls.begin()+i);
// 			Taos.erase(Taos.begin()+i);
// 			Weights.erase(Weights.begin()+i);
// 		}
// 
// 	//not full? or found a worse classifier?
// 	if(Worst>=0 || GetSize()<MaxSize)
// 	{
// 		//build classifier Ci
// 		CClassifier *Ci=Creator.Creator(Dataset,Creator.Params);
// 		Classifiers.push_back(Ci);
// 		//use(recall) times
// 		Recalls.push_back(1);
// 		//time of first recall(use)
// 		Taos.push_back(Times);
// 		//real weights used to predict
// 		Weights.push_back(IniWeight);
// 	}
// 
// 	//time consumed
// 	CreatingTime +=((double)(clock() - start) / CLOCKS_PER_SEC);
// }

void CFCAE::Train(const CDataset &Dataset)
{
	CClassifier *Cls=Creator.Creator(Dataset,Creator.Params);
	Train(Dataset,Cls);
	delete Cls;
}

//no pruned classifier is actually removed (it is just inactivated) 
void CFCAE::Train(const CDataset &Dataset, const CClassifier *Cls)
{	
	//start time for training
	clock_t start=clock();
	//number of training
	Times++;
	//
//	const int DataSize=Dataset.GetInfo().Height;
	//initializing weight for a new classifier
	const double IniWeight=1.0;
	//if we need to remove a classifier before create a new one
	int Worst=-1;
	//find the classifier with least weight in them
	double WorstWeight=100.0;
	//
	int EnsembleSize=GetSize();
	if((int)Memorys.size()!=EnsembleSize || (int)Weights.size()!=EnsembleSize)
	{
		throw(CError("Error!",100,0));
	}

// 	//restore the ensemble to un-pruned states, because pruning is proceeded on Weights 
// 	Weights.assign(Memorys.begin(),Memorys.end());
	for(int i=0;i<EnsembleSize;i++)
		Weights[i]=IniWeight;
	//pruning base on current data set
	{
		CMDSQ Pruner=CMDSQ(*this,Dataset,TargetSize-1);
		//change weights for the un-pruned classifiers
		const vector<double> &PrunedWeights=Pruner.GetWeights();
		for(int i=0;i<EnsembleSize;i++)
		{
			//the pruned classifiers don't participate in succeeding prediction(weights equal zero)
			if(PrunedWeights[i]==0)
				Weights[i]=0;
// 			//give new ones at last ten chances before possibly removed
// 			if(Times-CreatingTimes[i]<10)
// 			{
// 				Memorys[i]=1.0;
// 				continue;
// 			}

			//classifier is kept by the pruner means it is recalled
			if(PrunedWeights[i]!=0)
			{
				Recalls[i]++;
				Taos[i]=Times;
			}
			//new weights for all classifiers of this ensemble
			double ForgetFactor=Alpha/Recalls[i];
			Memorys[i]=exp(double(-1.0)*ForgetFactor*(Times-Taos[i]));

			//find the classifier with least weight
			if(Memorys[i]<WorstWeight)
			{
				Worst=i;
				WorstWeight=Memorys[i];
			}

		}
	}

	//removed the worst classifiers (if we have set the max size)
	if(GetSize()>=MaxSize)
	{
		if(Worst<0)
		{
			throw(CError("Error!",100,0));
		}
		delete Classifiers[Worst];
		Classifiers.erase(Classifiers.begin()+Worst);
		Memorys.erase(Memorys.begin()+Worst);
		Recalls.erase(Recalls.begin()+Worst);
		Taos.erase(Taos.begin()+Worst);
		CreatingTimes.erase(CreatingTimes.begin()+Worst);
		Weights.erase(Weights.begin()+Worst);
	}

	//not full (or we have removed the worst classifier)
// 	if(Times<MaxSize)
// 	{
// 		for(int i=0;i<(int)(MaxSize*1.0/Times+0.9999);i++)
// 		{
// 			//bootstrap to create more classifier
// 			CDataset Data;
// 			Dataset.BootStrap((int)(DataSize*1.0*2/3),Data);
// 			CClassifier *Cl=Creator.Creator(Data,Creator.Params);
// 			Classifiers.push_back(Cl);
// 			//initialization weight for all classifiers
// 			Memorys.push_back(IniWeight);
// 			//use(recall) times
// 			Recalls.push_back(1);
// 			//time of first recall(use)
// 			Taos.push_back(Times);
// 			//real weights used to predict
// 			Weights.push_back(IniWeight);
// 		}
// 		//pruning base on current data set
// 		CMDSQ::ParamStr Param;
// 		Param.TargetSize=MaxSize;
// 		CEnsemblePruner *Pruner=PruneFunc(*this,Dataset,&Param);
// 		//change weights for the un-pruned classifiers
// 		const vector<double> &PrunedWeights=Pruner->GetWeights();
// 		for(int i=(int)Weights.size()-1;i>=0;i--)
// 		{
// 			if(PrunedWeights[i]==0)
// 			{
// 				delete Classifiers[i];
// 				Classifiers.erase(Classifiers.begin()+i);
// 				Memorys.erase(Memorys.begin()+i);
// 				Recalls.erase(Recalls.begin()+i);
// 				Taos.erase(Taos.begin()+i);
// 				Weights.erase(Weights.begin()+i);
// 			}
// 		}
// 		delete Pruner;
// 	}
	if(GetSize()<MaxSize)
	{
		//build classifier Ci
		CClassifier *Ci=Cls->Clone();
		CreatingTime+=Cls->GetCreateTime();
		Classifiers.push_back(Ci);
		//initialization weight for all classifiers
		Memorys.push_back(IniWeight);
		//use(recall) times
		Recalls.push_back(1);
		//time of first recall(use)
		Taos.push_back(Times);
		//Creating time
		CreatingTimes.push_back(Times);
		//real weights used to predict
		Weights.push_back(IniWeight);
	}

	//time consumed
	CreatingTime +=((double)(clock() - start) / CLOCKS_PER_SEC);
}

