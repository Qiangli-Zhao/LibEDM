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

#include <string>
#include <fstream>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Prediction.h"
using namespace libep;

// void operator +=(PRED_RESULT &a, const PRED_RESULT &b)
// {
// 	if(a.PredLabelIndices.size()==0)
// 	{
// 		a=b;
// 		return;
// 	}
// 
// 	_ASSERT(a.ClassNum==b.ClassNum);
// 
// 	a.CaseNum+=b.CaseNum;
// 	for(int i=0;i<(int)b.PredLabelIndices.size();i++)
// 		a.PredLabelIndices.push_back(b.PredLabelIndices[i]);
// 	for(int i=0;i<(int)b.IsCorrect.size();i++)
// 		a.IsCorrect.push_back(b.IsCorrect[i]);
// }
// 
// PRED_RESULT operator +(const PRED_RESULT &a, const PRED_RESULT &b)
// {
// 	_ASSERT(a.ClassNum==b.ClassNum);
// 
// 	PRED_RESULT c;
// 	c.CaseNum=a.CaseNum+b.CaseNum;
// 	c.ClassNum=a.ClassNum;
// 
// 	for(int i=0;i<(int)a.PredLabelIndices.size();i++)
// 		c.PredLabelIndices.push_back(a.PredLabelIndices[i]);
// 	for(int i=0;i<(int)b.PredLabelIndices.size();i++)
// 		c.PredLabelIndices.push_back(b.PredLabelIndices[i]);
// 
// 	for(int i=0;i<(int)a.IsCorrect.size();i++)
// 		c.IsCorrect.push_back(a.IsCorrect[i]);
// 	for(int i=0;i<(int)b.IsCorrect.size();i++)
// 		c.IsCorrect.push_back(b.IsCorrect[i]);
// 
// 	return c;
// }
 
CPrediction::~CPrediction()
{

}

//Dataset: data set be predicted
//Probabilities: Probabilities of each instance belong to each class label
//start: start time of predicting
CPrediction::CPrediction(const CDataset &Dataset,const DoubleArray2d &Probabilities,clock_t PredictTime)
{
	const MATRIX &Data=Dataset.GetData();
	const CASE_INFO &Info=Dataset.GetInfo();
	CaseNum=Info.Height;
	ClassNum=Info.ClassNum;
	Accuracy=0;
	Probs.assign(Probabilities.begin(),Probabilities.end());

	//the label of an instance is the class with the max probability
	for(int j=0;j<CaseNum;j++)
	{
		int Class=0;
		double MaxProb=0;
		for(int k=0;k<ClassNum;k++)
			if(Probs[j][k]>MaxProb)
			{
				Class=k;
				MaxProb=Probs[j][k];
			}
		PredLabelIndices.push_back(Class);

		//correct prediction count
		int IdealResult=Data[j][Info.ValidWidth-1].Discr;
		if(IdealResult==Class)
		{
			IsCorrect.push_back(true);
			Accuracy+=1;
		}
		else
			IsCorrect.push_back(false);
	}

	Accuracy/=CaseNum;

	//Total time consumed
	CreatingTime = (double)PredictTime/CLOCKS_PER_SEC;
}

const DoubleArray2d& CPrediction::GetProbs() const
{
	return Probs;
}

const IntArray& CPrediction::GetPredictedLabelIndices() const
{
	return PredLabelIndices;
}

const BoolArray& CPrediction::GetCorrectness() const
{
	return IsCorrect;
}

double CPrediction::GetAccuracy() const
{
	return Accuracy;
}
