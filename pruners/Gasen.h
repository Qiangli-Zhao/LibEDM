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

#ifndef GASEN_ENSPRUNER
#define GASEN_ENSPRUNER

#include <vector>
#include <list>

namespace libep
{
	class  CGasen : public CEnsemblePruner
	{
	public:
		typedef struct ParamStr
		{
			//parameters used for GA
			int TribeNum;//size of tribe
			double Pc;//probability of crossover
			double Pm;//probability of mutation
			int MaxGen;//max number of evolution
			int MaxNoInc;//Max number of evolution if no better individual is found

			double Lamda;//threshold for a weight
		}ParamStr;

	public:
		CGasen(const CEnsemble &UEnsemble,const CDataset &UValidatingSet,int TribeNum=30,double Pc=0.5,
			double Pm=0.05,int MaxGen=100,int MaxNoInc=100,double Lamda=0);
		static CEnsemblePruner *Create(const CEnsemble &UEnsemble,const CDataset &UValidatingSet,const void* UParams)
		{
			if(UParams==NULL)
				return new CGasen(UEnsemble,UValidatingSet);

			const ParamStr *Params=(const ParamStr*)UParams;
			return new CGasen(UEnsemble,UValidatingSet,Params->TribeNum,Params->Pc,
				Params->Pm,Params->MaxGen,Params->MaxNoInc,Params->Lamda);
		}
		//predict a dataset
		CGasen(const CEnsemble &UEnsemble,const CDataset &UValidatingSet,const vector<CPrediction*> &UPredictions,
			int TribeNum=30,double Pc=0.5,
			double Pm=0.05,int MaxGen=100,int MaxNoInc=100,double Lamda=0);
		static CEnsemblePruner *CreateOnPrediction(const CEnsemble &UEnsemble,const CDataset &UValidatingSet,const vector<CPrediction*> &UPredictions, const void* UParams)
		{
			if(UParams==NULL)
				return new CGasen(UEnsemble,UValidatingSet,UPredictions);

			const ParamStr *Params=(const ParamStr*)UParams;
			return new CGasen(UEnsemble,UValidatingSet,UPredictions,Params->TribeNum,Params->Pc,
				Params->Pm,Params->MaxGen,Params->MaxNoInc,Params->Lamda);
		}

	private:
		//cache recent individuals' fitnesses to save time used by GA
		typedef struct CacheItemStr
		{
			vector<double>	Individual;
			double			Fitness;
		}CacheItemStr;
		list<CacheItemStr> IndivCache;

		//parameters used for fitness function
		typedef struct FitParamStr
		{
			const CEnsemble *Ensemble;
			const CDataset *ValidatingSet;
			const vector<CPrediction*> *Predictions;
			list<CGasen::CacheItemStr> *IndivCache;
		}FitParamStr;
		//used for CGA (generic algorithm) to calculate fitnesses of all individuals in each generation
		static void Fitness(void *Params,const DoubleArray2d &Tribe,vector<double> &Fitnesses);
	};
}

#endif
