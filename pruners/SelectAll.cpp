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
#include "SelectAll.h"
using namespace libedm;

const char MyName[MAX_OBJECT_NAME_LENGTH]="SelectAll";

//select all the base classifier
CSelectAll::CSelectAll(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	//start time for training
	clock_t start=clock();
	const vector<double> EnsembleWeights=Ensemble.GetWeights();
	for(int i=0;i<(int)Predictions.size();i++)
		Weights.push_back(EnsembleWeights[i]);
	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

//select all the base classifier
CSelectAll::CSelectAll(const CEnsemble &UEnsemble,const CDataset &ValidatingSet)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	//start time for training
	clock_t start=clock();
	const vector<double> EnsembleWeights=Ensemble.GetWeights();
	for(int i=0;i<(int)Ensemble.GetSize();i++)
		Weights.push_back(EnsembleWeights[i]);
	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

