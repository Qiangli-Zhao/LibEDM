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
#include "IncrementalTrunkEnsemble.h"
#include "SEA.h"
using namespace libep;


const char	MyName[MAX_OBJECT_NAME_LENGTH]="SEA_ENSEMBLE";

CSEA::CSEA(int uMaxSize,CreateFunc *Func, const void *Params)
:MaxSize(uMaxSize)
{
	Name=MyName;
	CreatingTime=0;
	Times=0;

	Creator.Creator=Func;
	Creator.Params=Params;
}

void CSEA::Train(const CDataset &Dataset)
{
	CClassifier *Cls=Creator.Creator(Dataset,Creator.Params);
	Train(Dataset,Cls);
	delete Cls;
}

void CSEA::Train(const CDataset &Dataset, const CClassifier *Cls)
{	
	const CASE_INFO &Info=Dataset.GetInfo();
	const int Width=Info.ValidWidth;
	const MATRIX &Data=Dataset.GetData();
	//start time for training
	clock_t start=clock();
	Times++;


	//size of ensemble
	int Size=(int)Classifiers.size();
	//build classifier Ci
	CClassifier *Ci=Cls->Clone();
	CreatingTime+=Cls->GetCreateTime();
	//Evaluate Ci and ensemble
	CPrediction *Result_E=Classify(Dataset);
	CPrediction *Result_Ci=Ci->Classify(Dataset);
	//probabilities for all class labels for all instances
	const DoubleArray2d &Probs_E=Result_E->GetProbs();
	const DoubleArray2d &Probs_Ci=Result_Ci->GetProbs();
	//predicted class
	const vector<int> &Predicts_Ci=Result_Ci->GetPredictedLabels();
	//correctness
	const BoolArray &Corrects_Ci=Result_Ci->GetCorrectness();
	const BoolArray &Corrects_E=Result_E->GetCorrectness();
	//get quality for Ci
	double Quality_Ci=0;
	//we will change all qualities based on new data
	vector<CPrediction*> *Predictions=AllClassify(Dataset);
	for(int j=0;j<Size;j++)
		Qualities[j]=0;
	for(int i=0;i<Info.Height;i++)
	{
		//statistics for ensemble
		double P1=0,P2=0;
		for(int j=0;j<Info.ClassNum;j++)
		{
			if(Probs_E[i][j]>P1)
			{
				P2=P1;
				P1=Probs_E[i][j];
			}
			else if(Probs_E[i][j]>P2)
			{
				P2=Probs_E[i][j];
			}
		}
		double PC=Probs_E[i][Data[i][Width-1].Discr];

		//for Ci's prediction
		{
			double PT=Probs_E[i][Predicts_Ci[i]];

			//quality for Ci
			if(Corrects_Ci[i])
			{
				if(Corrects_E[i])
					Quality_Ci+=(Info.Height-abs(P1-P2));
				else
					Quality_Ci+=(Info.Height-abs(P1-PC));
			}
			else
				Quality_Ci-=(Info.Height-abs(PC-PT));
		}

//  		//qualities for all classifiers of the ensemble
// 		for(int j=0;j<Size;j++)
// 		{
// 			const vector<int> &Predicts=(*Predictions)[j]->GetPredictedLabels();
// 			const BoolArray &Corrects=(*Predictions)[j]->GetCorrectness();
// 
// 			//the percentage for the prediction of the j-th base classifier to the i-th instance
// 			double PT=Probs_E[i][Predicts[i]];
// 
// 			//quality for this classifier
// 			if(Corrects[i])
// 			{
// 				if(Corrects_E[i])
// 					Qualities[j]+=(Info.Height-abs(P1-P2));
// 				else
// 					Qualities[j]+=(Info.Height-abs(P1-PC));
// 			}
// 			else
// 				Qualities[j]-=(Info.Height-abs(PC-PT));
// 		}
	}

	//
	delete Result_Ci;
	delete Result_E;
	for(int i=0;i<Size;i++)
		delete (*Predictions)[i];
	delete Predictions;


	//>=0 if ensemble is full but a worse classifier is removed
	int Worst=-1;
	//Is ensemble full?
	if(Size>=MaxSize)
	{
		double WorstQuality=Quality_Ci;
		//find the worst
		for(int i=0;i<Size;i++)
		{
			if(Qualities[i]<WorstQuality)
			{
				WorstQuality=Qualities[i];
				Worst=i;
			}
		}

		//remove the worst classifier
		if(Worst>=0)
		{
			delete (Classifiers[Worst]);
			Classifiers.erase(Classifiers.begin()+Worst);
			Weights.erase(Weights.begin()+Worst);
			Qualities.erase(Qualities.begin()+Worst);
		}
		else// Ci is not good enough
		{
			delete Ci;
// 			cout<<Times<<": SEA abandoned"<<endl;
		}
	}

	//add new classifier to ensemble
	if(Worst>=0 || Size<MaxSize)
	{
		//
		Classifiers.push_back(Ci);
		//same weight for all classifiers
		Weights.push_back(1.0);
		//
		Qualities.push_back(Quality_Ci);
	}

	//time consumed
	CreatingTime +=((double)(clock() - start) / CLOCKS_PER_SEC);
}

