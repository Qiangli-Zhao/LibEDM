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

//functions needed by all ensemble pruning algorithms
#include <vector>
#include <ctime>
#include <fstream>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "Ensemble.h"
#include "EnsemblePruner.h"
using namespace libedm;

static const char MyName[MAX_OBJECT_NAME_LENGTH]="EnsemblePruner";
const string CEnsemblePruner::StaticName=MyName;

CEnsemblePruner::~CEnsemblePruner()
{
}

//user defined pruned ensemble
CEnsemblePruner::CEnsemblePruner(const CEnsemble &UEnsemble,const vector<double> &UWeights)
:Ensemble(UEnsemble),Weights(UWeights)
{
	Name=MyName;
	if(Ensemble.GetSize()!=(int)Weights.size())
		throw(CError("EnsemblePruner: wrong size for user-defined weights!",100,0));

	double Sum=0;
	for(int i=0;i<(int)Weights.size();i++)
		Sum+=Weights[i];
	for(int i=0;i<(int)Weights.size();i++)
		Weights[i]/=Sum;

	CreatingTime=0;
}

//classifier is saved into files, and can be reconstructed from it
int CEnsemblePruner::Save(const string &Path,const string &FileName) const
{
	ofstream OutFile;
	OutFile.open((Path+FileName+"."+Name).c_str(),ios_base::out|ios_base::trunc|ios_base::binary);
	if(OutFile.fail())
	{
		return 1;
	}

	//size of weights
	int TotalModel=(int)Weights.size();
	OutFile.write((char*)&TotalModel,sizeof(TotalModel));
	//
	for(int i=0;i<TotalModel;i++)
	{
		double Weight=Weights[i];
		OutFile.write((char*)&Weight,sizeof(Weight));
	}

	OutFile.close();
	return 0;
}

CEnsemblePruner::CEnsemblePruner(const string &Path,const string &FileName,const CEnsemble &UEnsemble)
:Ensemble(UEnsemble)
{
	Name=MyName;
	//start time for training
	clock_t start=clock();

	ifstream InFile;
	InFile.open((Path+FileName+"."+Name).c_str(),ios_base::in|ios_base::binary);
	if(InFile.fail())
	{
		throw(CError("Ensemble pruner: fail open saving file!",101,0));
	}

	//number of total classifier
	int TotalModel;
	InFile.read((char*)&TotalModel,sizeof(TotalModel));
	if(InFile.fail()) throw(CError("Ensemble pruner: read parameter error!",102,0));

	for(int i=0;i<TotalModel;i++)
	{
		double Weight;
		InFile.read((char*)&Weight,sizeof(Weight));
		if(InFile.fail()) throw(CError("Ensemble pruner: read parameter error!",103,0));
		Weights.push_back(Weight);
	}
	
	InFile.close();

	if(Ensemble.GetSize()!=(int)Weights.size())
		throw(CError("EnsemblePruner: wrong size for user-defined weights!",104,0));

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

