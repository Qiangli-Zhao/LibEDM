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
#include "EnsemblePruner.h"
#include "Prediction.h"
#include "FS.h"
using namespace libedm;

const char MyName[MAX_OBJECT_NAME_LENGTH]="ForwardSelect";

//select the base classifier with the highest accuracy on validation set
CForwardSelect::CForwardSelect(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	if(Ensemble.GetSize()!=(int)Predictions.size())
		throw(CError("EnsemblePruner: wrong size for user-defined weights!",100,0));
	//Info
	int CaseNum=ValidatingSet.GetInfo().Height;
	int EnsembleSize=Ensemble.GetSize();


	//start time for training
	clock_t start=clock();

	//initialize with no classifier selected
	for(int i=0;i<EnsembleSize;i++)
		Weights.push_back(0);
	//add classifier one by one
	double BestAccr=0;
	for(int i=0;i<EnsembleSize;i++)
	{
		//add the best in each round
		int Best=-1;
		for(int j=0;j<EnsembleSize;j++)
		{
			//skip the one has been selected
			if(Weights[j]>0)continue;
			//add this classifier temporarily
			Weights[j]=1;
			//predicting
			CPrediction *Prediction=Ensemble.Classify(ValidatingSet,Predictions,Weights);
			double Accuracy=Prediction->GetAccuracy();
			delete Prediction;
			//better accuracy?
			if(Accuracy>BestAccr)
			{
				Best=j;
				BestAccr=Accuracy;
				//if accuracy is 1.0, no better one can be found
				if(Accuracy>=1.0)
					break;
			}
			//recover to the initial state
			Weights[j]=0;
		}
		if(BestAccr>=1.0)
			break;
		//select the best one of this round
		if(Best!=-1)
			Weights[Best]=1;
	}

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

//select the base classifier with the highest accuracy on validation set
CForwardSelect::CForwardSelect(const CEnsemble &UEnsemble,const CDataset &ValidatingSet)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	//Info
	int CaseNum=ValidatingSet.GetInfo().Height;
	int EnsembleSize=Ensemble.GetSize();


	//start time for training
	clock_t start=clock();

	//get prediction
	vector<CPrediction*> *Predictions=Ensemble.AllClassify(ValidatingSet);

	//initialize with no classifier selected
	for(int i=0;i<EnsembleSize;i++)
		Weights.push_back(0);
	//add classifier one by one
	double BestAccr=0;
	for(int i=0;i<EnsembleSize;i++)
	{
		//add the best in each round
		int Best=-1;
		for(int j=0;j<EnsembleSize;j++)
		{
			//skip the one has been selected
			if(Weights[j]>0)continue;
			//add this classifier temporarily
			Weights[j]=1;
			//predicting
			CPrediction *Prediction=Ensemble.Classify(ValidatingSet,*Predictions,Weights);
			double Accuracy=Prediction->GetAccuracy();
			delete Prediction;
			//better accuracy?
			if(Accuracy>BestAccr)
			{
				Best=j;
				BestAccr=Accuracy;
				//if accuracy is 1.0, no better one can be found
				if(Accuracy>=1.0)
					break;
			}
			//recover to the initial state
			Weights[j]=0;
		}
		//if accuracy is 1.0, no better one can be found
		if(BestAccr>=1.0)
			break;
		//select the best one of this round
		if(Best!=-1)
			Weights[Best]=1;
	}

	for(int i=0;i<EnsembleSize;i++)
		delete ((*Predictions)[i]);
	delete Predictions;
	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

