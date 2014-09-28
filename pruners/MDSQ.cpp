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
#include <cmath>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "Ensemble.h"
#include "Prediction.h"
#include "EnsemblePruner.h"
#include "MDSQ.h"
using namespace libedm;

const char MyName[MAX_OBJECT_NAME_LENGTH]="MDSQ";

//SignVect-signature vector, EnsVect-ensemble vector, NextSize-size of target ensemble
double CMDSQ::Distance(const vector<double> &SignVect,const vector<double> &EnsVect,int EnsembleSize,int NextSize)
{
//	double u=0.075;
	//using a moving reference point instead of a fixed one
	double u=sqrt((double)2*NextSize)*2/EnsembleSize;

	double D=0;
	for(int i=0;i<(int)SignVect.size();i++)
	{
		double tmp=(EnsVect[i]+SignVect[i])/NextSize-u;
		D+=(tmp*tmp);
	}
	D=sqrt(D);

	return D;
}

//MDSQ- sort classifiers by signature
void CMDSQ::Prune(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions,int TargetSize)
{
	Name=MyName;
	if(Ensemble.GetSize()!=(int)Predictions.size())
		throw(CError("EnsemblePruner: wrong size for user-defined weights!",100,0));
	//Info
	int CaseNum=ValidatingSet.GetInfo().Height;
	int EnsembleSize=Ensemble.GetSize();
	if(TargetSize<=0)
		TargetSize=(int)(EnsembleSize*0.21);
	else if(TargetSize>=EnsembleSize)
	{
		for(int i=0;i<EnsembleSize;i++)
			Weights.push_back(1.0);
		return;
	}

	//ensemble vector(average of all signature vector)
	vector<double> EnsVect(CaseNum,0);
	//signature vectors for all classifiers (ensemble vector is temporally used to initialize it)
	DoubleArray2d SignVect(EnsembleSize,EnsVect);
	//fill all signature vectors
	for(int i=0;i<EnsembleSize;i++)
	{
		const BoolArray &Correctness=Predictions[i]->GetCorrectness();
		for(int j=0;j<CaseNum;j++)
		{
			//does the i-th classifier predict the j-th instance correctly?
			if(Correctness[j])
				SignVect[i][j]=1;
			else
				SignVect[i][j]=-1;
		}
	}
	//initialize pruner weights
	for(int i=0;i<EnsembleSize;i++)
		Weights.push_back(0);

	//each time a classifier is selected
	for(int i=1;i<=TargetSize;i++)
	{
		//select the classifier whose signature vector is closest to the ensemble vector
		int Best=-1;
		double BestDist=sqrt((double)9*CaseNum)+1;
		for(int j=0;j<EnsembleSize;j++)
		{
			//Can a classifier be selected multi times?
			if(Weights[j]>0) continue;
			//the Euclidean distance
			double D=Distance(SignVect[j],EnsVect,EnsembleSize,i);
			if(D<BestDist)
			{
				Best=j;
				BestDist=D;
			}
		}
		//the signature vector of selected classifier is add to the ensemble vector
		for(int j=0;j<CaseNum;j++)
			EnsVect[j]+=SignVect[Best][j];
		//selected
		Weights[Best]=1;
	}

}

//MDSQ- sort classifiers by signature
CMDSQ::CMDSQ(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions,int TargetSize)
:CEnsemblePruner(UEnsemble)
{
	//start time for training
	clock_t start=clock();

	Prune(UEnsemble,ValidatingSet,Predictions,TargetSize);

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

//MDSQ- sort classifiers by signature
CMDSQ::CMDSQ(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,int TargetSize)
:CEnsemblePruner(UEnsemble)
{
	//start time for training
	clock_t start=clock();

	//get predictions
	vector<CPrediction*> *Predictions=Ensemble.AllClassify(ValidatingSet);

	Prune(UEnsemble,ValidatingSet,(*Predictions),TargetSize);

	//release prediction
	for(int i=0;i<Ensemble.GetSize();i++)
		delete ((*Predictions)[i]);
	delete Predictions;

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

