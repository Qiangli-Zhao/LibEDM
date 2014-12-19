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
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Prediction.h"
#include "Classifier.h"
#include "IncrementalClassifier.h"
#include "Ensemble.h"
#include "IncrementalEnsemble.h"
#include "IncrementalTrunkEnsemble.h"
#include "CrossValidate.h"
#include "C45.h"
#include "AWE.h"
using namespace libedm;


const char	MyName[MAX_OBJECT_NAME_LENGTH]="AWE_ENSEMBLE";

CAWE::CAWE(int uMaxSize,CreateFunc *Func, const void *Params)
:MaxSize(uMaxSize)
{
	Name=MyName;
	CreatingTime=0;
	Times=0;

	Creator.Creator=Func;
	Creator.Params=Params;
}

class WeightI
{
public:
	WeightI(const CDataset &Data,const CPrediction *Prediction)
	{
		//data info
		const MATRIX &TrainData=Data.GetData();
		const CASE_INFO &CaseInfo=Data.GetInfo();
		const DoubleArray2d &Probs=Prediction->GetProbs();

		//MSEi
		double MSEi=0;
		vector<double> Pc(CaseInfo.ClassNum,0);
		for(int i=0;i<CaseInfo.Height;i++)
		{
			//correct label  for this instance
			int Label=TrainData[i][CaseInfo.ValidWidth-1].Discr;
			//distribution of label
			Pc[Label]+=1.0;
			//error of prediction
			double Err=1-Probs[i][Label];
			//
			MSEi+=(Err*Err);
		}
		MSEi/=CaseInfo.Height;

		//MSEr
		double MSEr=0;
		for(int i=0;i<CaseInfo.ClassNum;i++)
		{
			Pc[i]/=CaseInfo.Height;
			MSEr=+((Pc[i])*(1-Pc[i])*(1-Pc[i]));
		}

		//Wi
		Wi=1.0-MSEi;
	};

	double GetWi(){return Wi;};

private:
	double Wi;
};

void CAWE::Train(const CDataset &Dataset)
{
	CClassifier *Cls=Creator.Creator(Dataset,Creator.Params);
	Train(Dataset,Cls);
	delete Cls;
}

void CAWE::Train(const CDataset &Dataset, const CClassifier *Cls)
{	
	//start time for training
	clock_t start=clock();
	Times++;


	//build classifier Ci
	CClassifier *Ci=Cls->Clone();
	CreatingTime+=Cls->GetCreateTime();

	//Evaluate Ci through cross validation
	vector<WeightI> Wps;
	double Wp=0;
	int Cross=1;
	CrossValidate<WeightI>(Cross,Dataset,Ci,Wps);
	for(int i=0;i<Cross;i++)
		Wp+=Wps[i].GetWi();
	Wp/=Cross;
	//skip weight that <=0
 	if(Wp<=0)
	{
		cout<<"AWE: Wp<=0"<<endl;
 		return;
	}

	//Existed classifiers
	int Size=(int)Classifiers.size();
	//>=0 if ensemble is full but a worse classifier is removed
	int Worst=-1;
	double WorstWight=Wp;
	//Is ensemble full?
	if(Size>=MaxSize)
	{
		//evaluate all classifiers
		vector<CPrediction*> *Results=AllClassify(Dataset);
		//find the worst
		for(int i=0;i<Size;i++)
		{
			//get wi for this classifier
			WeightI Wi(Dataset,(*Results)[i]);
			//note: here change all weights
			Weights[i]=Wi.GetWi();
			//is this classifier worse than new one?
			if(Weights[i]<WorstWight)
			{
				WorstWight=Weights[i];
				Worst=i;
			}

			delete (*Results)[i];
		}
		delete Results;

		//remove the worst
		if(Worst>=0)
		{
			delete (Classifiers[Worst]);
			Classifiers.erase(Classifiers.begin()+Worst);
			Weights.erase(Weights.begin()+Worst);
		}
	}

	//not full? or found a worse classifier?
	if(Worst>=0 || Size<MaxSize)
	{
		Classifiers.push_back(Ci);
		//bagging: same weight for all classifiers
		Weights.push_back(Wp);
	}

	//time consumed
	CreatingTime +=((double)(clock() - start) / CLOCKS_PER_SEC);
}

