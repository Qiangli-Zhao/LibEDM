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

#include <cstring>
#include <vector>
#include <fstream>
using namespace std;
#include "Obj.h"
#include "Classifier.h"
#include "zString.h"
#include "DataSet.h"
#include "Prediction.h"
#include "Ensemble.h"
using namespace libedm;

static const char MyName[MAX_OBJECT_NAME_LENGTH]="Ensemble";
const string CEnsemble::StaticName=MyName;

vector<CEnsemble::FileCreatorRegisterStr> CEnsemble::FileCreators;

CEnsemble::~CEnsemble()
{
	Flush();
}

//remove all generated base classifiers
bool CEnsemble::Flush()
{
	for(int i=0;i<(int)Classifiers.size();i++)
		delete Classifiers[i];
	Classifiers.clear();
	Weights.clear();
	FileCreators.clear();

	return true;
}

//save all classifiers into file, and can be reconstructed from it
int CEnsemble::Save(const string &Path,const string &FileName) const
{
	ofstream OutFile;
	OutFile.open((Path+FileName+"."+Name+".ini").c_str(),ios_base::trunc|ios_base::out|ios_base::binary);
	if(OutFile.fail())
		return 1;

	//size of ensemble
	int EnsembleSize=(int)Classifiers.size();
	OutFile.write((char*)&EnsembleSize,sizeof(EnsembleSize));
	//file name and weight for each base classifier
	for(int i=0;i<(int)Classifiers.size();i++)
	{
		char Type[32];
		strncpy(Type,Classifiers[i]->GetName().c_str(),sizeof(Type));
		OutFile.write(Type,sizeof(Type));
		double Weight=Weights[i];
		OutFile.write((char*)&Weight,sizeof(Weight));
		Classifiers[i]->Save(Path,FileName+CzString::IntToStr(i));
	}
	return 0;
}

CEnsemble::CEnsemble(const string &Path,const string &FileName)
{
	Name=MyName;
	//start time for training
	clock_t start=clock();

	//please register file function at first!
	if((int)FileCreators.size()<=0)
	{
		throw(CError("Ensemble: no creating function registered!",106,0));
	}

	ifstream InFile;
	InFile.open((Path+FileName+"."+Name).c_str(),ios_base::in|ios_base::binary);
	if(InFile.fail())
	{
		throw(CError("Ensemble: fail open saving file!",101,0));
	}

	//reading
 	int EnsembleSize;
	InFile.read((char*)&EnsembleSize,sizeof(EnsembleSize));
	if(InFile.fail()) throw(CError("Ensemble: read parameter error!",102,0));

	CClassifier *Classifier;
	for(int i=0;i<EnsembleSize;i++)
	{
		//get type of a classifier
		char Type[32];
		InFile.read(Type,sizeof(Type));
		if(InFile.fail()) throw(CError("Ensemble: read parameter error!",103,0));
		//find corresponding construction function entry
		int j;
		for(j=0;j<(int)FileCreators.size();j++)
		{
			if(FileCreators[j].ClassifierType==Type)
				break;
		}
		//not found
		if(j>=(int)FileCreators.size())
			throw(CError("Ensemble: Error in initializing file!",104,0));

		//load it
		Classifier=FileCreators[j].Creator(Path,FileName+CzString::IntToStr(i));
		Classifiers.push_back(Classifier);
		//weight for this classifier
		int Weight;
		InFile.read((char*)&Weight,sizeof(Weight));
		if(InFile.fail()) throw(CError("Ensemble: read parameter error!",105,0));
		Weights.push_back(Weight);
	}

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

//get predictions of each classifier for a data set
vector<CPrediction*> *CEnsemble::AllClassify(const CDataset &DataSet) const
{
	//for return
	vector<CPrediction*> *Predictions=new (vector<CPrediction*>);
	//each classifier do predicting
	for(int i=0;i<(int)Classifiers.size();i++)
	{
		CPrediction *Prediction=Classifiers[i]->Classify(DataSet);
		Predictions->push_back(Prediction);
	}

	return Predictions;
}

//classify a data set
CPrediction *CEnsemble::Classify(const CDataset &DataSet) const
{
	return Classify(DataSet,Weights);
}

//vote predicting, using user-defined weights vector
CPrediction *CEnsemble::Classify(const CDataset &DataSet,const vector<double> &UserWeights) const
{
	//start time for training
	clock_t Start=clock();

	//data info
	const MATRIX &TrainData=DataSet.GetData();
	const CASE_INFO &CaseInfo=DataSet.GetInfo();
	//malloc space for result
	vector<double> Line(CaseInfo.ClassNum,0);
	DoubleArray2d Result(CaseInfo.Height,Line);

	//for each base classifier
	for(int i=0;i<(int)Classifiers.size();i++)
	{
		//skip classifier whose weight is zero
		if(UserWeights[i]==0)
			continue;
		//get prediction of each classifier
		CPrediction *Prediction=Classifiers[i]->Classify(DataSet);
		const IntArray &PredictedLabels=Prediction->GetPredictedLabelIndices();
		//for each instance
		for(int j=0;j<CaseInfo.Height;j++)
		{
			int Class=PredictedLabels[j];
			Result[j][Class]+=UserWeights[i];
		}
		delete Prediction;
	}

	return new CPrediction(DataSet,Result,clock() - Start);
}

//classify a data set (we already have each classifier's prediction)
CPrediction *CEnsemble::Classify(const CDataset &DataSet,const vector<CPrediction*> &Predictions) const
{
	return Classify(DataSet,Predictions,Weights);
}

//vote predicting, using user-defined weights vector
CPrediction *CEnsemble::Classify(const CDataset &DataSet,const vector<CPrediction*> &Predictions,const vector<double> &UserWeights) const
{
	//start time for training
	clock_t Start=clock();

	//check
	if(UserWeights.size()!=Predictions.size())
		throw(CError("Ensemble: error in user-defined weights!",104,0));
	//data info
	const MATRIX &TrainData=DataSet.GetData();
	const CASE_INFO &CaseInfo=DataSet.GetInfo();
	//malloc space for result
	vector<double> Line(CaseInfo.ClassNum,0);
	DoubleArray2d Result(CaseInfo.Height,Line);

	//for each base classifier
	for(int i=0;i<(int)Predictions.size();i++)
	{
		//skip classifier whose weight is zero
		if(UserWeights[i]==0)
			continue;
		//get prediction of each classifier
		const IntArray &PredictedLabels=Predictions[i]->GetPredictedLabelIndices();
		//for each instance
		for(int j=0;j<CaseInfo.Height;j++)
		{
			int Class=PredictedLabels[j];
			Result[j][Class]+=UserWeights[i];
		}
	}

	return new CPrediction(DataSet,Result,clock() - Start);
}

