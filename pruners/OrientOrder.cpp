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
#include <algorithm>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "Ensemble.h"
#include "EnsemblePruner.h"
#include "Prediction.h"
#include "OrientOrder.h"
using namespace libedm;

const char MyName[MAX_OBJECT_NAME_LENGTH]="OrientOrder";

//oriental order
COrientOrder::COrientOrder(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions)
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

	//ensemble vector(average of all signature vector)
	vector<double> EnsVect(CaseNum,0);
	//signature vectors for all classifiers (ensemble vector is temporally used to initialize it)
	DoubleArray2d SignVect(EnsembleSize,EnsVect);
	//get all signature vectors
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
	//get ensemble vector
	double Sum=0,QSum=0;
	for(int j=0;j<CaseNum;j++)
	{
		for(int i=0;i<EnsembleSize;i++)
			EnsVect[j]+=SignVect[i][j];
		EnsVect[j]/=EnsembleSize;
		Sum+=EnsVect[j];
		QSum+=(EnsVect[j]*EnsVect[j]);
	}
	//initialize pruner weights
	for(int i=0;i<EnsembleSize;i++)
		Weights.push_back(0);

	//get reference vector
	double Lamda=(-1)*Sum/QSum;
	//mold of reference vector
	double MRef=0;
	vector<double> RefVect(CaseNum,0);
	for(int j=0;j<CaseNum;j++)
	{
		RefVect[j]=1+Lamda*EnsVect[j];
		MRef+=(RefVect[j]*RefVect[j]);
	}
	MRef=sqrt(MRef);
	//order by cosine
	vector<OrderStr> OrderVect(EnsembleSize);
	for(int i=0;i<EnsembleSize;i++)
	{
		double ScaleProd=0,MSign=0;
		for(int j=0;j<CaseNum;j++)
		{
			ScaleProd+=(RefVect[j]*SignVect[i][j]);
			MSign+=(SignVect[i][j]*SignVect[i][j]);
		}
		MSign=sqrt(MSign);
		OrderVect[i].Cosine=ScaleProd/MRef/MSign;
		OrderVect[i].Angle=acos(OrderVect[i].Cosine);
		OrderVect[i].Cls=i;
	}
	sort(OrderVect.begin(),OrderVect.end(),CosDescOrder);

	//average of the angles less than 90
	//sum: average angle
	QSum=0;Sum=0;
	//cosine<=0 (means: angle>=90) are skipped
	for(int i=0;i<EnsembleSize;i++)
	{
		if(OrderVect[i].Cosine<=0)
			break;
		Sum+=OrderVect[i].Angle;
		QSum++;
	}
	Sum/=QSum;
	//angles no less than the average are skipped
	int Last=-1;
	for(int i=0;i<EnsembleSize;i++)
		if(OrderVect[i].Angle>=Sum)
		{
			Last=i;
			break;
		}

	//display angle of each classifiers being selected
	for(int i=0;i<Last;i++)
		Weights[OrderVect[i].Cls]=1;

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

COrientOrder::COrientOrder(const CEnsemble &UEnsemble,const CDataset &ValidatingSet)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	//Info
	int CaseNum=ValidatingSet.GetInfo().Height;
	int EnsembleSize=Ensemble.GetSize();
	//start time for training
	clock_t start=clock();

	//get predictions
	vector<CPrediction*> *Predictions=Ensemble.AllClassify(ValidatingSet);
	//ensemble vector(average of all signature vector)
	vector<double> EnsVect(CaseNum,0);
	//signature vectors for all classifiers (ensemble vector is temporally used to initialize it)
	DoubleArray2d SignVect(EnsembleSize,EnsVect);
	//get all signature vectors
	for(int i=0;i<EnsembleSize;i++)
	{
		const BoolArray &Correctness=(*Predictions)[i]->GetCorrectness();
		for(int j=0;j<CaseNum;j++)
		{
			//does the i-th classifier predict the j-th instance correctly?
			if(Correctness[j])
				SignVect[i][j]=1;
			else
				SignVect[i][j]=-1;
		}
	}
	//release prediction
	for(int i=0;i<EnsembleSize;i++)
		delete ((*Predictions)[i]);
	delete Predictions;
	//get ensemble vector
	double Sum=0,QSum=0;
	for(int j=0;j<CaseNum;j++)
	{
		for(int i=0;i<EnsembleSize;i++)
			EnsVect[j]+=SignVect[i][j];
		EnsVect[j]/=EnsembleSize;
		Sum+=EnsVect[j];
		QSum+=(EnsVect[j]*EnsVect[j]);
	}
	//initialize pruner weights
	for(int i=0;i<EnsembleSize;i++)
		Weights.push_back(0);

	//get reference vector
	double Lamda=(-1)*Sum/QSum;
	//mold of reference vector
	double MRef=0;
	vector<double> RefVect(CaseNum,0);
	for(int j=0;j<CaseNum;j++)
	{
		RefVect[j]=1+Lamda*EnsVect[j];
		MRef+=(RefVect[j]*RefVect[j]);
	}
	MRef=sqrt(MRef);
	//order by cosine
	vector<OrderStr> OrderVect(EnsembleSize);
	for(int i=0;i<EnsembleSize;i++)
	{
		double ScaleProd=0,MSign=0;
		for(int j=0;j<CaseNum;j++)
		{
			ScaleProd+=(RefVect[j]*SignVect[i][j]);
			MSign+=(SignVect[i][j]*SignVect[i][j]);
		}
		MSign=sqrt(MSign);
		OrderVect[i].Cosine=ScaleProd/MRef/MSign;
		OrderVect[i].Angle=acos(OrderVect[i].Cosine);
		OrderVect[i].Cls=i;
	}
	sort(OrderVect.begin(),OrderVect.end(),CosDescOrder);

	//average of the angles less than 90
	//sum: average angle
	QSum=0;Sum=0;
	//cosine<=0 (means: angle>=90) are skipped
	for(int i=0;i<EnsembleSize;i++)
	{
		if(OrderVect[i].Cosine<=0)
			break;
		Sum+=OrderVect[i].Angle;
		QSum++;
	}
	Sum/=QSum;
	//angles no less than the average are skipped
	int Last=-1;
	for(int i=0;i<EnsembleSize;i++)
		if(OrderVect[i].Angle>=Sum)
		{
			Last=i;
			break;
		}

	//display angle of each classifiers being selected
	for(int i=0;i<Last;i++)
		Weights[OrderVect[i].Cls]=1;

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

