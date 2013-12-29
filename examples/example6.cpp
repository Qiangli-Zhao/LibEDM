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
#include "Prediction.h"
#include "CrossValidate.h"
using namespace libep;

class StAccuracy
{
public:
	StAccuracy(const CDataset &Data,const CPrediction *Prediction)
	{
		Accuracy=Prediction->GetAccuracy();
	};

	double GetResult(){return Accuracy;};

private:
	double Accuracy;
};

int main(int argc, char* argv[])
{
	try
	{
		//read training data from file
		CUCIData TrainSet;
		TrainSet.Load("zoo.names","zoo.data");
		TrainSet.RemoveUnknownInstance();

		//Evaluate BPNN trainer with default parameters through cross validation
		//We want the accuracy of the trainer
		vector<StAccuracy> Accuracys;
		double AverageAccuracy=0;
		//three folders
		int Cross=3;
		CrossValidate<StAccuracy>(Cross,TrainSet,CBpnn::RpropCreate,NULL,Accuracys);

		//get average accuracy of each folder
		for(int i=0;i<Cross;i++)
		{
			double Acc=Accuracys[i].GetResult();
			cout<<"accuracy for round "<<i<<" is: "<<Acc<<endl;
			AverageAccuracy+=Acc;
		}
		AverageAccuracy/=Cross;
		cout<<"average accuracy is: "<<AverageAccuracy<<endl;
	}
	catch (CError &e)
	{
		cout<<e.Description<<endl;
		if(e.Level<=0)
			return 0;
	}

	return 0;
}

