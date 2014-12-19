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
#include <cmath>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Prediction.h"
#include "Classifier.h"
#include "IncrementalClassifier.h"
#include "Ensemble.h"
#include "IncrementalEnsemble.h"
#include "IncrementalTrunkEnsemble.h"
#include "C45.h"
#include "Win.h"
using namespace libedm;


const char	MyName[MAX_OBJECT_NAME_LENGTH]="FCAE_ENSEMBLE";

CWin::CWin(int uMaxSize,CreateFunc *Func,const void *CreatorParams)
:MaxSize(uMaxSize)
{
	Name=MyName;
	CreatingTime=0;
	Times=0;

	Creator.Creator=Func;
	Creator.Params=CreatorParams;
}

void CWin::Train(const CDataset &Dataset)
{
	CClassifier *Cls=Creator.Creator(Dataset,Creator.Params);
	Train(Dataset,Cls);
	delete Cls;
}

void CWin::Train(const CDataset &Dataset, const CClassifier *Cls)
{	
	//start time for training
	clock_t start=clock();
	//number of training
	Times++;

	//removed classifiers that is the worst if we have max size
	if(GetSize()>=MaxSize)
	{
		delete Classifiers[0];
		Classifiers.erase(Classifiers.begin());
		Weights.erase(Weights.begin());
	}

	//not full (or we have removed the worst classifier)
	//build a new classifier
	CClassifier *Cl=Cls->Clone();
	CreatingTime+=Cls->GetCreateTime();
	Classifiers.push_back(Cl);
	//real weights used to predict
	Weights.push_back(1.0);

	//time consumed
	CreatingTime +=((double)(clock() - start) / CLOCKS_PER_SEC);
}

