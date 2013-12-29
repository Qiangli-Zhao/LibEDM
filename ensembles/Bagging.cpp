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
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "Ensemble.h"
#include "Bagging.h"
using namespace libep;


const char	MyName[MAX_OBJECT_NAME_LENGTH]="Bagging";

CBagging::CBagging(const CDataset &TrainSet,
	double DataPerc,int TotalModel,const vector<CreatorRegisterStr> &uCreators)
{
	Name=MyName;

	//save the parameters
	vector<CreatorRegisterStr> Creators;
	Creators.assign(uCreators.begin(),uCreators.end());

	//create a empty ensemble
	if(TotalModel<=0 || DataPerc<=0.0 || DataPerc>1.0 || Creators.size()<=0)
		throw(CError("Bagging: Invalid parameters!",100,0));
	//data info
	const MATRIX &TrainData=TrainSet.GetData();
	const CASE_INFO &CaseInfo=TrainSet.GetInfo();
	//number of instances used to train each classifier of ensemble
	int SampleNum=(int)(CaseInfo.Height*DataPerc);


	//start time for training
	clock_t start=clock();
	//normalize distribution of all modeler
	double RatioSum=0;
	for(int j=0;j<(int)Creators.size();j++)
		RatioSum+=Creators[j].Ratio;
	for(int j=0;j<(int)Creators.size();j++)
		Creators[j].Ratio/=RatioSum;

	//the first classifier type
	int ModelNo=0;
	//number of classifier that will be created for this type
	RatioSum=Creators[ModelNo].Ratio;
	int ModelNum=(int)(TotalModel*RatioSum+0.5);
	//get all base classifiers
	for(int j=0;j<TotalModel;)
	{

		CClassifier *Classifier;
		if(j<ModelNum)
		{
			//get training set of each base classifier
			CDataset NNTrainSet;
			TrainSet.BootStrap(SampleNum,NNTrainSet);
			//create a classifier
			Classifier=Creators[ModelNo].Creator(NNTrainSet,Creators[ModelNo].Params);
			Classifiers.push_back(Classifier);
			//bagging: same weight for all classifiers
			Weights.push_back((double)1.0/TotalModel);
			j++;
		}
		else
		{
			ModelNo++;
			RatioSum+=Creators[ModelNo].Ratio;
			ModelNum=(int)(TotalModel*RatioSum+0.5);
			continue;
		}

	}//for j classifier
	
	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

