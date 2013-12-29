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
#include <list>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "Ensemble.h"
#include "EnsemblePruner.h"
#include "Prediction.h"
#include "GA.h"
#include "Gasen.h"
using namespace libep;

const char MyName[MAX_OBJECT_NAME_LENGTH]="GASEN";
//
//tribe: all individuals of a generation
//individual: a array of numeric values (weights for all classifiers of the ensemble).
//fitness: 1.0/error

CGasen::CGasen(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions,
				int TribeNum,double Pc,
				double Pm,int MaxGen,int MaxNoInc,double Lamda)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	int EnsembleSize=Ensemble.GetSize();
	if(EnsembleSize!=(int)Predictions.size())
		throw(CError("EnsemblePruner: wrong size for user-defined weights!",100,0));
	//start time for training
	clock_t start=clock();

	//parameters for fitness function
	FitParamStr FitParam;
	FitParam.Ensemble=&Ensemble;
	FitParam.ValidatingSet=&ValidatingSet;
	FitParam.Predictions=&Predictions;
	FitParam.IndivCache=&IndivCache;
	//evolve
	vector<double> Best(EnsembleSize,0);
	CGA GA(TribeNum,Pc,Pm,MaxGen,MaxNoInc);
	GA.Evolve((void *)&FitParam,Fitness,Best);

	//threshold of weight
	if(Lamda<=0 || Lamda>=1.0)
		Lamda=1.0/EnsembleSize;
	//remove the base classifiers whose weights is less than lambda
	for(int i=0;i<(int)Best.size();i++)
		if(Best[i]<=Lamda)
			Weights.push_back(0);
		else
			Weights.push_back(1);

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

CGasen::CGasen(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,int TribeNum,double Pc,
			   double Pm,int MaxGen,int MaxNoInc,double Lamda)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	int EnsembleSize=Ensemble.GetSize();
	//start time for training
	clock_t start=clock();

	//parameters for fitness function
	FitParamStr FitParam;
	FitParam.Ensemble=&Ensemble;
	FitParam.ValidatingSet=&ValidatingSet;
	FitParam.IndivCache=&IndivCache;
	//get prediction
	FitParam.Predictions=Ensemble.AllClassify(ValidatingSet);

	//evolve
	vector<double> Best(EnsembleSize,0);
	CGA GA(TribeNum,Pc,Pm,MaxGen,MaxNoInc);
	GA.Evolve((void *)&FitParam,Fitness,Best);
	//release all predictions
	for(int i=0;i<EnsembleSize;i++)
		delete ((*(FitParam.Predictions))[i]);
	delete FitParam.Predictions;

	//threshold of weight
	if(Lamda<=0 || Lamda>=1.0)
		Lamda=1.0/EnsembleSize;
	//remove the base classifiers whose weights is less than lambda
	for(int i=0;i<(int)Best.size();i++)
		if(Best[i]<=Lamda)
			Weights.push_back(0);
		else
			Weights.push_back(1);

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

//calculate fitnesses of all individuals' of this tribe
void CGasen::Fitness(void *Params,const DoubleArray2d &Tribe,vector<double> &Fitnesses)
{
	//Parameters for calculate fitness
	FitParamStr *FitParam=(FitParamStr *)Params;
	//info
	const int IndivSize=FitParam->Ensemble->GetSize();
	const int TribeSize=(int)Tribe.size();
	//cache of fitnesses for recent individuals
	list<CacheItemStr> *IndivCache=FitParam->IndivCache;


	Fitnesses.clear();
	for(int i=0;i<TribeSize;i++)
	{
		//search it in cache
		list<CacheItemStr>::iterator j;
		for(j=IndivCache->begin();j!=IndivCache->end();j++)
		{
			//same individual?
			if(Tribe[i]==j->Individual)
				break;
		}
		//found a match
		if(j!=IndivCache->end())
		{
			Fitnesses.push_back(j->Fitness);
			//move the matched item at the end
			IndivCache->push_back(*j);
			IndivCache->erase(j);
			
			continue;
		}

		//an individual is the weights for all classifiers of the ensemble
		CPrediction *Prediction=FitParam->Ensemble->Classify(*(FitParam->ValidatingSet),*(FitParam->Predictions),Tribe[i]);
		double Accuracy=Prediction->GetAccuracy();
		Fitnesses.push_back(1.0/(1.0-Accuracy+0.000001));
		//add the prediction to cache
		//the item
		CacheItemStr CacheItem;
		CacheItem.Individual=Tribe[i];
		CacheItem.Fitness=Accuracy;
		//cache TribeSize maximum items
		if((int)IndivCache->size()>=TribeSize)
			IndivCache->pop_front();
		IndivCache->push_back(CacheItem);

		delete Prediction;
	}
}

