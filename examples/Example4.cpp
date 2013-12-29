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
#include "C45.h"
#include "svm.h"
#include "b-svm.h"
#include "GaussNaiveBayes.h"
#include "NaiveBayes.h"
#include "Prediction.h"
#include "Ensemble.h"
#include "Bagging.h"
#include "EnsemblePruner.h"
#include "SelectBest.h"
#include "SelectAll.h"
#include "FS.h"
#include "MDSQ.h"
#include "OrientOrder.h"
#include "Gasen.h"
using namespace libep;

//entry of pruning function and its corresponding parameters
typedef struct PrunerParamStr
{
	PrunerCreator *Creator;
	void *Params;
}PrunerParamStr;

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
		CDataset TrainSet,TestSet,ValSet;
		//90% as train set
		int Sample_num=(int)(Info.Height*0.9);
		//the rest as testing
		Original.SplitData(Sample_num,TrainSet,TestSet);
		//the validation set for ensemble pruning
		TrainSet.BootStrap((int)(Sample_num*0.5),ValSet);


		//register parameters for base classifier trainers 
		vector<CEnsemble::CreatorRegisterStr> Creators;
		{
			//BPNN with default parameters
			CEnsemble::CreatorRegisterStr CreatorRegister;
			CreatorRegister.Creator=CBpnn::RpropCreate;
			CreatorRegister.Ratio=1.0;
			CreatorRegister.Params=(void*)NULL;
			Creators.push_back(CreatorRegister);
		}

		try
		{
			//bagging ensemble, size 50
			CBagging BaggingEnsemble(TrainSet,0.5,50,Creators);

			try
			{
				//pruning the ensemble by selecting best
				CMDSQ Pruned(BaggingEnsemble,ValSet);
				//show selected classifiers
				cout<<"Classifiers being selected are: ";
				const vector<double> &Weights=Pruned.GetWeights();
				for(int i=0;i<(int)Weights.size();i++)
					if(Weights[i]>0)
						cout<<i<<",";
				cout<<endl;

				//predicting
				CPrediction *Result=Pruned.Classify(TestSet);
				//prediction accuracy
				cout<<Pruned.GetName()<<" pruner: Pruning Time="<<Pruned.GetCreateTime()<<
					", predictive accuracy="<<Result->GetAccuracy()<<endl;
				delete Result;
			}
			catch (CError &e)
			{
				cout<<e.Description<<endl;
				if(e.Level<=0)
					return 0;
			}
		}
		catch (CError &e)
		{
			cout<<e.Description<<endl;
			if(e.Level<=0)
				return 0;
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

