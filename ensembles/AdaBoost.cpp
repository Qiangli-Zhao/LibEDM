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
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "Ensemble.h"
#include "Prediction.h"
#include "AdaBoost.h"
using namespace libep;


const char	MyName[MAX_OBJECT_NAME_LENGTH]="AdaBoost";

CAdaBoost::CAdaBoost(const CDataset &TrainSet,
			double DataPerc,int TotalModel,CreateFunc *Creator,const void *Params)
{
	//start time for training
	clock_t start=clock();

	Name=MyName;
	//create a empty ensemble
	if(TotalModel<=0 || DataPerc<=0.0 || DataPerc>1.0)
		throw(CError("Bagging: Invalid input parameters!",100,0));

	//data info
	const MATRIX &TrainData=TrainSet.GetData();
	const CASE_INFO &CaseInfo=TrainSet.GetInfo();
	//number of instances
	int CaseNum=CaseInfo.Height;
	//number of instances randomly select from TrainSet
	int SampleNum=(int)(CaseNum*DataPerc);

	//initialize weights of instances
	vector<double> CaseWeights(CaseNum,(double)1.0/CaseNum);
	//in case that err=0.0
	double MINERR=1.0;
	for(int i=0;i<=TotalModel;i++)
		MINERR*=((double)1.0/CaseNum);

	//repeated time for training a good classifier
	int RepNum=0;
	//get all base classifiers
	for(int i=0;i<TotalModel;)
	{
		//if in current distribution we can't find a good classifier, reset the distribution
		if(RepNum>0 && RepNum%50==0)
		{
			for(int j=0;j<CaseNum;j++)
				CaseWeights[j]=(double)1.0/CaseNum;
		}
		//in case no classifier can reach to the error criterion, and it will never stop
		if(RepNum>=100)
		{
			throw(CError("AdaBoost: can't create a classifier!",100,0));
		}
		RepNum++;

		//get training set of each base classifier
		CDataset NNTrainSet;
		vector<int> OrginalPostions;
		//weighted bootstrap
		//CaseWeights is normalized in CDataSet::BootStrap()
		TrainSet.BootStrap(CaseWeights,SampleNum,OrginalPostions,NNTrainSet);
		//training a classifier
		CClassifier *Classifier;
		Classifier=Creator(NNTrainSet,Params);
		//accuracy on training set
		CPrediction *Prediction=Classifier->Classify(TrainSet);
		const BoolArray &Correctness=Prediction->GetCorrectness();
		//sum of weighted error
		double ErrSum=0;
		for(int j=0;j<CaseInfo.Height;j++)
			if(!Correctness[j])
				ErrSum+=CaseWeights[j];
		//if error > 0.5, discard it
		if(ErrSum>=0.5)
		{
			delete Prediction;
			delete Classifier;
			continue;
		}

		RepNum=0;
		//normalize the error
		double Beta;
		if(ErrSum>0)
			Beta=ErrSum/(1-ErrSum);
		else
			Beta=MINERR/(1-MINERR);
		//update instances' weights(only for the instances used for training this classifier)
		for(int j=0;j<SampleNum;j++)
			if(Correctness[j])
				CaseWeights[OrginalPostions[j]]*=Beta;
		delete Prediction;

		//note that bagging use same weight for all classifiers
		//once the classifier enters a ensemble, it is managed by ensemble and should not be deleted outside the ensemble
		Classifiers.push_back(Classifier);
		Weights.push_back(log((double)1.0/Beta));

		//added
		i++;
	}//for i classifier

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

