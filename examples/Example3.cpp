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
#include "bpnn.h"
#include "C45.h"
#include "Prediction.h"
#include "Ensemble.h"
#include "Bagging.h"
using namespace libep;

int main(int argc, char* argv[])
{
	//read training data from file
	try
	{
		CUCIData Original;
		Original.Load("zoo.names","zoo.data");
		//remove the instances whose label are unknown
		Original.RemoveUnknownInstance();
		//Get information of the training set
		const CASE_INFO &Info=Original.GetInfo();

		//instances are group into training set and test set
		CDataset TrainSet,TestSet;
		//90% as train set
		int Sample_num=(int)(Info.Height*0.9);
		//the rest as testing
		Original.SplitData(Sample_num,TrainSet,TestSet);


		//register parameters for base classifier trainers 
		vector<CEnsemble::CreatorRegisterStr> Creators;
		//using two different training parameters for BPNN
		{
			//parameters for first BPNN trainer
			CBpnn::RpropParamStr Param;
			Param.HideNode=(Info.ValidWidth-1)*2;
			Param.MaxEpoch=3000;
			Param.MinMSE=0.01;
			//first trainer
			CEnsemble::CreatorRegisterStr CreatorRegister;
			CreatorRegister.Creator=CBpnn::RpropCreate;
			CreatorRegister.Params=(void*)&Param;
			CreatorRegister.Ratio=0.4;
			Creators.push_back(CreatorRegister);
			//second trainer
			//BPNN using default parameters
			CreatorRegister.Params=(void*)NULL;
			Creators.push_back(CreatorRegister);
			//third trainer: C4.5 decision tree with default parameters
			CreatorRegister.Creator=CC45::Create;
			CreatorRegister.Params=(void*)NULL;
			CreatorRegister.Ratio=0.2;
			Creators.push_back(CreatorRegister);
		}
		

		try
		{
			//bagging ensemble
			CBagging BaggingEnsemble(TrainSet,0.5,10,Creators);
			//Get individual base classifier from ensemble
			const vector<CClassifier*> &Classes=BaggingEnsemble.GetAllClassifiers();
			for(int i=0;i<(int)Classes.size();i++)
			{
				//information for base classifier
				cout<<i<<"("<<Classes[i]->GetName()<<"): Creating time="<<Classes[i]->GetCreateTime();
				//each base classifier's prediction accuracy
				const CPrediction *Result=Classes[i]->Classify(TestSet);
				cout<<", predictive accuracy="<<Result->GetAccuracy()<<endl;
				delete Result;
			}

			//use the ensemble to predict the test dataset
			const CPrediction *Result=BaggingEnsemble.Classify(TestSet);
			//predicting accuracy
			cout<<BaggingEnsemble.GetName()<<" ensemble: Creating Time="<<BaggingEnsemble.GetCreateTime()<<
				", predictive accuracy="<<Result->GetAccuracy()<<endl;
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

