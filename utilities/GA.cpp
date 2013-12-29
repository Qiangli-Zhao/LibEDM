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

#include <cmath>
using namespace std;
#include "Obj.h"
#include "GA.h"
using namespace libep;


//int TribeSize=30;
//double Pc=0.5;
//double Pm=0.05;
//int MaxGen=100;
//int MaxNoInc=100;


CGA::CGA(int UTribeSize,double UPc,double UPm,int UMaxGen,int UMaxNoInc)
:TribeSize(UTribeSize),Pc(UPc),Pm(UPm),MaxGen(UMaxGen),MaxNoInc(UMaxNoInc)
{
}

//individual is defined as a vector of double
//use the fitness function and corresponding parameters to evolve a best individual
double CGA::Evolve(void *Params,FitFunc Fit,vector<double> &Best)
{
	//tribe: individuals
	DoubleArray2d Tribe;
	//fitness for each individual
	vector<double> Fitnesses;
	//size of individual
	int IndivSize=(int)Best.size();
	//generate initial individuals
	for(int j=0;j<TribeSize;j++)
	{
		Fitnesses.push_back(0);
		//initialize: random weights
		vector<double> Individual;
		for(int i=0;i<IndivSize;i++)
			Individual.push_back((double)rand()/RAND_MAX);
		Tribe.push_back(Individual);
	}
	
	//best individual in all generations
	double BestAccuracy=0;
	//number of successive generations in which we can't find a better individual
	int ConstPtTime=0;
	//number of generation
	int Gen=0;
	while(true)
	{
		//normalize weight for each individual
		for(int j=0;j<(int)Tribe.size();j++)
		{
			double Wt=0;
			for(int i=0;i<IndivSize;i++)
				Wt+=Tribe[j][i];
			for(int i=0;i<IndivSize;i++)
				Tribe[j][i]/=Wt;
		}

		//fitness of all individual
		Fit(Params,Tribe,Fitnesses);
		//sum of all fitness		
		double TotalFt=0;
		//best individual in this tribe
		vector<double> BestInTribe;
		double BestAccuracyInTribe=0;
		for(int j=0;j<(int)Tribe.size();j++)
		{
			TotalFt+=Fitnesses[j];
			//fittest individual
			if(Fitnesses[j]>BestAccuracyInTribe)
			{
				BestAccuracyInTribe=Fitnesses[j];
				BestInTribe=Tribe[j];
			}
		}
		//best individual of all generations
		if(BestAccuracyInTribe>BestAccuracy)
		{
			Best=BestInTribe;
			ConstPtTime=0;
		}
		else//how many generations since the last change of the best?
			ConstPtTime++;
		if(ConstPtTime>=MaxNoInc || Gen++>=MaxGen)
			break;
//		if(Tribe.Ft>=1.0 && ConstPtTime>=MaxNoInc/2)
//			break;

		//GA: individuals selecting base on roulette
		CRoulette Roulette(Fitnesses);
		//form the tribe for the next generation
		DoubleArray2d NextTribe;
		for(;(int)NextTribe.size()<TribeSize;)
		{
			int j=Roulette.Poll();
			NextTribe.push_back(Tribe[j]);
		}
		Tribe.clear();
		Tribe=NextTribe;

		//individual pool for crossover
		//if we need to do crossover
		int MateNo=0;
		vector<int> WillMate;
		for(int j=0;j<(int)Tribe.size();j++)
		{
			double r=(double)rand()/RAND_MAX;
			if(r<Pc)
				WillMate.push_back(++MateNo);
			else
				WillMate.push_back(0);
		}
		//single-float crossover
		for(int j=0;MateNo>1 && j<TribeSize;j++)
			while(WillMate[0]>0)
			{
				//no. of partner to do crossover
				int Partner=rand()%MateNo+1;
				//must not be itself
				if(Partner==WillMate[j])
					continue;
				//find the partner
				int k;
				for(k=0;k<(int)Tribe.size();k++)
					if(Partner==WillMate[k])
						break;

				//cross pot
				int CrosePot=rand()%IndivSize;
				//do crossover
				vector<double> Indivj=Tribe[j],Indivk=Tribe[k];
				for(int i=CrosePot;i<IndivSize;i++)
				{
					Indivj[i]=Tribe[k][i];
					Indivk[i]=Tribe[j][i];
				}
				//add new individuals into the tribe
				Tribe.push_back(Indivj);
				Tribe.push_back(Indivk);
				break;
			}

		//mutation
		for(int j=0;j<(int)Tribe.size();j++)
		{
			double r=(double)rand()/RAND_MAX;
			if(r<Pm)
			{
				vector<double> Indiv=Tribe[j];
				//pot of mutation
				int MutaPot=rand()%IndivSize;
				Indiv[MutaPot]=(double)rand()/RAND_MAX;
				//add new individual into the tribe
				Tribe.push_back(Indiv);
			}
		}
	}//while

	return BestAccuracy;
}

