#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "ArffData.h"
#include "Prediction.h"
#include "Classifier.h"
#include "bpnn.h"
#include "IncrementalClassifier.h"
#include "Ensemble.h"
#include "IncrementalEnsemble.h"
#include "IncrementalTrunkEnsemble.h"
#include "C45.h"
#include "SEA.h"
#include "AWE.h"
using namespace libedm;


int main(int argc, char* argv[])
{
	try
	{
		//open data file
		ifstream DataFile;
		DataFile.open("zoo.arff");
		if(DataFile.fail())
		{
			throw(CError("open file failed!",100,0));
		}
		//reading header of the ARFF format file; then the file pointer moves to the beginning of data
		//here we only get the description of this data set
		//we will read the data block by block
		CASE_INFO Info;
		{
			CArffData DataInfo;
			DataInfo.LoadInfo(DataFile);
			Info=DataInfo.GetInfo();
		}

		//create and initializing a SEA incremental ensemble
 		CSEA Sea(25,CBpnn::RpropCreate,NULL);
		double SeaAvg=0;
		//size of each data block
		const int BlockSize=10;
		//number of block
		int i;
		for(i=0;!DataFile.eof();)
		{
			//ten instance as a data block
			CArffData DataSet;
			DataSet.Load(Info,DataFile,BlockSize);
			if(DataSet.GetInfo().Height<=0)
				break;

			//try predicting new data using old classifier
			CPrediction *Prediction=Sea.Classify(DataSet);
			double Acc=Prediction->GetAccuracy();
			delete Prediction;
			cout<<"accuracy for round "<<i+1<<" :"<<setprecision(4)<<setiosflags(ios::fixed)<<Acc<<endl;
			SeaAvg+=Acc;
			i++;

			//incremental training
			Sea.Train(DataSet);
		}

		//finish this file
		cout<<"average accuracy: "<<SeaAvg/i<<endl;
	}
	catch (CError &e)
	{
		cout<<e.Description<<endl;
		if(e.Level<=0)
			return 0;
	}

	return 0;
}

