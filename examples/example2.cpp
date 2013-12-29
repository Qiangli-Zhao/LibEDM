#include <string>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "UCIData.h"
#include "Classifier.h"
#include "IncrementalClassifier.h"
#include "bpnn.h"
#include "svm.h"
#include "b-svm.h"
#include "C45.h"
#include "GaussNaiveBayes.h"
#include "NaiveBayes.h"
#include "Prediction.h"
using namespace libep;

int main(int argc, char* argv[])
{
	//read training data from file
	try
	{
		CUCIData TrainSet;
		TrainSet.Load("zoo.names","zoo.data");
		//remove the instances whose label are unknown
		TrainSet.RemoveUnknownInstance();

		//training a BP neural network by the RPORP (fast) method
		try
		{
			CBpnn BP1(TrainSet,0.01,3000,0);
			//dump to check
			BP1.Dump("BP1.dmp");
			//save this BP neural network into a file under the current directory
			BP1.Save("","BP1.sav");

			//use the BPNN to predict the original dataset
			const CPrediction *Result=BP1.Classify(TrainSet);
			//predicting accuracy
			cout<<"predictive accuracy:"<<Result->GetAccuracy()<<endl;
			delete Result;

			//clone a new classifier to predict
			CClassifier *Cls=BP1.Clone();
			//use the BPNN to predict the original dataset
			Result=Cls->Classify(TrainSet);
			//predicting accuracy
			cout<<"Cloned predictive accuracy:"<<Result->GetAccuracy()<<endl;
			delete Cls;

			//extract information of the prediction
			const vector<int> &Predicted=Result->GetPredictedLabels();
			const vector<bool> &Correct=Result->GetCorrectness();
			for(int i=0;i<(int)Predicted.size();i++)
			{
				cout<<"Instance "<<i<<": predicted label="<<Predicted[i]<<
					", Is correct?"<<(Correct[i]?"Yes":"No")<<endl;
			}
			delete Result;

		}
		catch (CError &e)
		{
			cout<<e.Description<<endl;
			if(e.Level<=0)
			{
				return 0;
			}
		}
	}
	catch (CError &e)
	{
		cout<<e.Description<<endl;
		if(e.Level<=0)
			return 0;
	}


	return 0;
}

