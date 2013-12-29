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
#include <ctime>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <set>
#include <iostream>
#include <iomanip>
#include <Windows.h>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "ArffData.h"
#include "UCIData.h"
#include "Classifier.h"
#include "IncrementalClassifier.h"
#include "Ensemble.h"
#include "IncrementalEnsemble.h"
#include "IncrementalTrunkEnsemble.h"
#include "Prediction.h"
#include "EnsemblePruner.h"
#include "bpnn.h"
#include "C45.h"
#include "svm.h"
#include "b-svm.h"
#include "NaiveBayes.h"
#include "GaussNaiveBayes.h"
#include "Bagging.h"
#include "SelectAll.h"
#include "SelectBest.h"
#include "Gasen.h"
#include "FS.h"
#include "OrientOrder.h"
#include "MDSQ.h"
#include "Cluster.h"
#include "PMEP.h"
#include "SEA.h"
#include "SEA_OP.h"
#include "AWE.h"
#include "FCAE.h"
#include "ACE.h"
#include "Win.h"
#include "DateTime.h"
#include "CrossValidate.h"
#include "Statistic.h"
using namespace libep;

//10-folders cross validation: comparison of all ensemble pruning algorithms
//directory of datasets
const string DATA_FILE_PATH="./data/";
//10-folders cross validation
const int FOLDERS=10;
//percent of total instances that will be used as original training set(the rest is used as test set)
const double ORIGINAL_TRAINSET_RATIO=0.9;
//percent of instances in original training set, which will be bootstrap-sampled as train set of base classifiers
const double TRAINSET_RATIO=0.5;
//distribution of each kind of base classifier
int ENSEMBLE_SIZE=30;
const double BPNN_RATIO=0.40;
const double C45_RATIO=0.20;
const double SVM_RATIO=0.20;
const double NAIVEBAYES_RATIO=0.10;
const double GAUSSNAIVEBAYES_RATIO=0.10;

//define this if you don't want accuracy on validation set
#define CALC_TMP_ACCR

//正在测试某个选择方法？
#undef TEST_PRUNING

//name of datasets files
const string FileName[]=
{
#ifdef TEST_PRUNING
//for debug
	"german-numeric",
	//"wine",
	//"hayes-roth",
	//"zoo",
	//"ad",
#else
//new


	"page",
	"waveform",
	"cleveland",
	"cancer",
	"pima",
	"bupa",
	"wine",
	"austra",
	"german-numeric",
	"balance-scale",
	"crx",
	"car",

//所有bpnn都相同（只对于同构？）
//	"ionosphere",
//	"segmentation",
//	"house-votes-84",
//	"breast-cancer-wisconsin",

//太大, 算不完
// 	"adult",//8.5
// 	"poker-hand",//9.5
// 	"letter-recognition",//11
// 	"ad",//3

//正式
//	"abalone",//1.5

//	"splice",//1
//	"agaricus-lepiota(mushroom)",//0.5
	"optdigits",
	"zoo",
	"spambase",
	"transfusion",
	"vehicle(statlog)",
	"yeast",
	"cmc",
	"dermatology",
	"hayes-roth",
	"imports-85(autos)",
	"tic-tac-toe",
	"kr-vs-kp",
	"wdbc",
	"iris",
	"hypothyroid",
	"sick",
	"heart",
	"glass",
	"sick-euthyroid",

#endif
};
//number of datasets
const int FILENUM=(int)FileName->size();


//statistic for each ensemble pruning algorithm
class StatStr
{
public:
	StatStr(){};
	StatStr(const CClassifier *Classifier,const CPrediction *Prediction)
	{
		BuildingTime=Classifier->GetCreateTime();
		AccuracyOfTestset=Prediction->GetAccuracy();
		PredictTime=Prediction->GetCreateTime();
	}
	double			ClssifierNum;//number of selected classifier
	double			AccuracyOfTestset;//total accuracy
	double			AccuracyOfValset;//accuracy for validation set
	double			PredictTime;//time used for predicting test set
	double			BuildingTime;//time of pruning
	vector<double>	Accuracys;//accuracy of each round for cross validation
	double			Variance;//variance
};


const string RESULT_FILE="./bootstrap/result.txt";
void ResultLog(const char *Msg)
{
	FILE *ft=fopen(RESULT_FILE.c_str(),"at");
	fprintf(ft,"%s",Msg);
	fclose(ft);
}


/*int main(int argc, char* argv[])
{
// 	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
//	_CrtSetBreakAlloc(2520868);


	//
	const int FILENUM=15;
	const int TrunkSize=500;
	const int EnsembleSize=11;
	string FileNames[FILENUM]=
	{
		"elecNormNew",
		"covtypeNorm",
		"poker-lsn",
		"airlines",
		"HypS",
		"HypF",
		"RBFb",
		"RBFgr",
		"RBFnd",
		"SeaS",
		"SeaF",
		"TreeS",
		"TreeF",
		"LedM",
		"LedND",
//		"HypFs",
// 		"SeaSs",
// 		"RBFgrs",
// 		"TreeSs",
// 		"TreeFs",
	};

	for(int Name=0;Name<FILENUM;Name++)
	{
		//name
		string File="./moadata/";
		File=File+FileNames[Name];
		cout<<FileNames[Name];

 		//
		int ChartInt=1;
		int Extra=0;
		ifstream DataFile;
		CASE_INFO Info;
// 		//we need to know the number of instances, i.e. we need to read all data in
		try
		{
			CArffData DataSet;
 			DataSet.Load(File+".arff");
			Info=DataSet.GetInfo();
			//
//			ChartInt=Info.Height/(100*TrunkSize);
//			Extra=(Info.Height%(100*TrunkSize)+TrunkSize-1)/TrunkSize;
			if(ChartInt<=0)
			{
				ChartInt=1;
				Extra=0;
			}
		}
		catch (CError &e)
		{
			cout<<e.Description<<endl;
			return 1;
		}

		//open
		DataFile.open((File+".arff").c_str());
		//we already know the data description, so skip this part
		{
			CArffData DataInfo;
			DataInfo.LoadInfo(DataFile);
		}

		//out file
		File="./bootstrap/";
		File=File+FileNames[Name]+".det";
		ofstream DetailFile(File.c_str());
		//parameters
		CC45::ParamStr BatchParam;
		BatchParam.MINOBJS=2;
		BatchParam.CF=0.25;
		BatchParam.Epsilon=1e-3;
		BatchParam.WillPrune=false;
		CreateFunc *Func=CC45::Create;

// 		CBpnn::RpropParamStr BatchParam;
// 		BatchParam.MaxEpoch=200;
// 		BatchParam.MinMSE=0.01;
// 		BatchParam.HideNode=0;
// 		CreateFunc *Func=CBpnn::RpropCreate;

		const double Ratio=0.2;
		CFCAE Eae((int)(EnsembleSize*1.0/Ratio),0.1,EnsembleSize,Func,&BatchParam);
//		vector<double> EaeAccs;
		double EaeAvg=0;
		double EaeChartAvg=0;
		double EaeSizeAvg=0;
		double EaePTime=0;

		//classifiers
 		CSEA Sea(EnsembleSize,Func,&BatchParam);
// 		vector<double> SeaAccs;
		double SeaAvg=0;
		double SeaChartAvg=0;
		double SeaSizeAvg=0;
		double SeaPTime=0;

		//classifiers
		CSEA_OP Sea_OP(EnsembleSize,Func,&BatchParam);
		// 		vector<double> SeaAccs;
		double Sea_OPAvg=0;
		double Sea_OPChartAvg=0;
		double Sea_OPSizeAvg=0;
		double Sea_OPPTime=0;

		CAWE Awe(EnsembleSize,Func,&BatchParam);
// 		vector<double> AweAccs;
		double AweAvg=0;
		double AweChartAvg=0;
		double AweSizeAvg=0;
		double AwePTime=0;

		CACE Ace(EnsembleSize,0.01,100,TrunkSize,3.0,CGaussNaiveBayes::Create,NULL,Func,&BatchParam);
// 		vector<double> AceAccs;
		double AceAvg=0;
		double AceChartAvg=0;
		double AceSizeAvg=0;
		double AcePTime=0;

		CWin Win(1,Func,&BatchParam);
		// 		vector<double> AceAccs;
		double WinAvg=0;
		double WinChartAvg=0;
		double WinSizeAvg=0;
		double WinPTime=0;

		CWin Win10(EnsembleSize,Func,&BatchParam);
		// 		vector<double> AceAccs;
		double Win10Avg=0;
		double Win10ChartAvg=0;
		double Win10SizeAvg=0;
		double Win10PTime=0;

		int i;
		for(i=0;!DataFile.eof();)
		{
			//
 			CArffData DataSet;
			DataSet.Load(Info,DataFile,TrunkSize);
			if(DataSet.GetInfo().Height<=0)
				break;

			//
			{
				double Acc;
				CPrediction *Prediction=Sea.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				SeaPTime=+Prediction->GetCreateTime();
				delete Prediction;
//	 			cout<<"\tSEA("<<Sea.GetRealSize()<<"): "<<Acc;
// 				SeaAccs.push_back(Acc);
				SeaAvg+=Acc;
				SeaChartAvg+=Acc;
				SeaSizeAvg+=Sea.GetRealSize();
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Sea_OP.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				Sea_OPPTime=+Prediction->GetCreateTime();
				delete Prediction;
				Sea_OPAvg+=Acc;
				Sea_OPChartAvg+=Acc;
				Sea_OPSizeAvg+=Sea_OP.GetRealSize();
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Awe.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				AwePTime=+Prediction->GetCreateTime();
				delete Prediction;
//				cout<<"\tAWE("<<Awe.GetRealSize()<<"): "<<Acc;
// 				AweAccs.push_back(Acc);
				AweAvg+=Acc;
				AweChartAvg+=Acc;
				AweSizeAvg+=Awe.GetRealSize();
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Ace.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				AcePTime=+Prediction->GetCreateTime();
				delete Prediction;
// 	 			cout<<"\tAWE("<<Ace.GetRealSize()<<"): "<<Acc;
// 				AceAccs.push_back(Acc);
				AceAvg+=Acc;
				AceChartAvg+=Acc;
				AceSizeAvg+=Ace.GetRealSize();
			}
 			//
			{
				double Acc;
				CPrediction *Prediction=Eae.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				EaePTime=+Prediction->GetCreateTime();
				delete Prediction;
//	 			cout<<"\tAWE("<<Eae.GetRealSize()<<"): "<<Acc;
//				EaeAccs.push_back(Acc);
				EaeAvg+=Acc;
				EaeChartAvg+=Acc;
				EaeSizeAvg+=Eae.GetRealSize();
// 				cout<<Eae.GetSize()<<":"<<Eae.GetRealSize()<<endl;
			}
 			//
			{
				double Acc;
				CPrediction *Prediction=Win.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				WinPTime=+Prediction->GetCreateTime();
				delete Prediction;
//	 			cout<<"\tAWE("<<Win.GetRealSize()<<"): "<<Acc;
//				WinAccs.push_back(Acc);
				WinAvg+=Acc;
				WinChartAvg+=Acc;
				WinSizeAvg+=Win.GetRealSize();
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Win10.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				Win10PTime=+Prediction->GetCreateTime();
				delete Prediction;
				//	 			cout<<"\tAWE("<<Win.GetRealSize()<<"): "<<Acc;
				//				WinAccs.push_back(Acc);
				Win10Avg+=Acc;
				Win10ChartAvg+=Acc;
				Win10SizeAvg+=Win10.GetRealSize();
			}

			i++;
			if(i%ChartInt==0)
			{
				DetailFile<<WinChartAvg/ChartInt<<"\t";
				DetailFile<<Win10ChartAvg/ChartInt<<"\t";
				DetailFile<<SeaChartAvg/ChartInt<<"\t";
				DetailFile<<Sea_OPChartAvg/ChartInt<<"\t";
				DetailFile<<AweChartAvg/ChartInt<<"\t";
				DetailFile<<AceChartAvg/ChartInt<<"\t";
				DetailFile<<EaeChartAvg/ChartInt<<endl;

				WinChartAvg=0;
				Win10ChartAvg=0;
				SeaChartAvg=0;
				Sea_OPChartAvg=0;
				AweChartAvg=0;
				AceChartAvg=0;
				EaeChartAvg=0;
			}

			//build classifier Ci
			CClassifier *Cls=Func(DataSet,&BatchParam);
			Win.Train(DataSet,Cls);
			Win10.Train(DataSet,Cls);
			Sea.Train(DataSet,Cls);
			Sea_OP.Train(DataSet,Cls);
			Awe.Train(DataSet,Cls);
			Ace.Train(DataSet);
			Eae.Train(DataSet,Cls);
			delete Cls;
		}//one file

		//extra
		if(Extra!=0)
		{
			DetailFile<<WinChartAvg/Extra<<"\t";
			DetailFile<<Win10ChartAvg/Extra<<"\t";
			DetailFile<<SeaChartAvg/Extra<<"\t";
			DetailFile<<Sea_OPChartAvg/Extra<<"\t";
			DetailFile<<AweChartAvg/Extra<<"\t";
			DetailFile<<AceChartAvg/Extra<<"\t";
			DetailFile<<EaeChartAvg/Extra<<endl;
		}

		cout<<setprecision(8);

		cout<<"\t"<<WinAvg/i;
		cout<<"\t"<<Win10Avg/i;
		cout<<"\t"<<SeaAvg/i;
		cout<<"\t"<<Sea_OPAvg/i;
		cout<<"\t"<<AweAvg/i;
		cout<<"\t"<<AceAvg/i;
		cout<<"\t"<<EaeAvg/i;
		//
		cout<<"\t"<<WinPTime/i;
		cout<<"\t"<<Win10PTime/i;
		cout<<"\t"<<SeaPTime/i;
		cout<<"\t"<<Sea_OPPTime/i;
		cout<<"\t"<<AwePTime/i;
		cout<<"\t"<<AcePTime/i;
		cout<<"\t"<<EaePTime/i;
		//
		cout<<"\t"<<Win.GetCreateTime()/i;
		cout<<"\t"<<Win10.GetCreateTime()/i;
		cout<<"\t"<<Sea.GetCreateTime()/i;
		cout<<"\t"<<Sea_OP.GetCreateTime()/i;
		cout<<"\t"<<Awe.GetCreateTime()/i;
		cout<<"\t"<<Ace.GetCreateTime()/i;
		cout<<"\t"<<Eae.GetCreateTime()/i;
		//
		cout<<"\t"<<WinSizeAvg/i;
		cout<<"\t"<<Win10SizeAvg/i;
		cout<<"\t"<<SeaSizeAvg/i;
		cout<<"\t"<<Sea_OPSizeAvg/i;
		cout<<"\t"<<AweSizeAvg/i;
		cout<<"\t"<<AceSizeAvg/i;
		cout<<"\t"<<EaeSizeAvg/i;
		//
		cout<<endl;

		//
		DetailFile.close();
		DataFile.close();
	}//all file
	return 0;

}*/


int main(int argc, char* argv[])
{
// 	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	const int FILENUM=3;
	const int TrunkSize=500;
	const int EnsembleSize=10;
	string FileNames[FILENUM]=
	{
		"connect-4",
		"adult",
		"kddcup",
	};

	for(int Name=0;Name<FILENUM;Name++)
	{
		//name
		string File="./data/";
		File=File+FileNames[Name];
		cout<<FileNames[Name];

		//
		int ChartInt=1;
		int Extra=0;
		ifstream DataFile;
		CASE_INFO Info;
		// 		//we need to know the number of instances, i.e. we need to read all data in
		try
		{
			CUCIData DataSet;
			DataSet.Load(File+".names", File+".data");
			Info=DataSet.GetInfo();
			//
// 			ChartInt=Info.Height/(100*TrunkSize);
// 			Extra=(Info.Height%(100*TrunkSize)+TrunkSize-1)/TrunkSize;
			if(ChartInt<=0)
			{
				ChartInt=1;
				Extra=0;
			}
		}
		catch (CError &e)
		{
			cout<<e.Description<<endl;
			continue;
		}

		//open
		DataFile.open((File+".data").c_str());

		//out file
		File="./bootstrap/";
		File=File+FileNames[Name]+".det";
		ofstream DetailFile(File.c_str());
		//parameters
		CC45::ParamStr BatchParam;
		BatchParam.MINOBJS=2;
		BatchParam.CF=0.25;
		BatchParam.Epsilon=1e-3;
		BatchParam.WillPrune=false;
		CreateFunc *Func=CC45::Create;

// 		CBpnn::RpropParamStr BatchParam;
// 		BatchParam.MaxEpoch=200;
// 		BatchParam.MinMSE=0.01;
// 		BatchParam.HideNode=0;
// 		CreateFunc *Func=CBpnn::RpropCreate;

		const double Ratio=0.2;
		CFCAE Eae((int)(EnsembleSize*1.0/Ratio),0.1,EnsembleSize,Func,&BatchParam);
		//		vector<double> EaeAccs;
		double EaeAvg=0;
		double EaeChartAvg=0;
		double EaeSizeAvg=0;
		double EaePTime=0;

		//classifiers
		CSEA Sea(EnsembleSize,Func,&BatchParam);
		// 		vector<double> SeaAccs;
		double SeaAvg=0;
		double SeaChartAvg=0;
		double SeaSizeAvg=0;
		double SeaPTime=0;

		//classifiers
		CSEA_OP Sea_OP(EnsembleSize,Func,&BatchParam);
		// 		vector<double> SeaAccs;
		double Sea_OPAvg=0;
		double Sea_OPChartAvg=0;
		double Sea_OPSizeAvg=0;
		double Sea_OPPTime=0;

		CAWE Awe(EnsembleSize,Func,&BatchParam);
		// 		vector<double> AweAccs;
		double AweAvg=0;
		double AweChartAvg=0;
		double AweSizeAvg=0;
		double AwePTime=0;

		CACE Ace(EnsembleSize,0.01,100,TrunkSize,3.0,CGaussNaiveBayes::Create,NULL,Func,&BatchParam);
		// 		vector<double> AceAccs;
		double AceAvg=0;
		double AceChartAvg=0;
		double AceSizeAvg=0;
		double AcePTime=0;

		CWin Win(1,Func,&BatchParam);
		// 		vector<double> AceAccs;
		double WinAvg=0;
		double WinChartAvg=0;
		double WinSizeAvg=0;
		double WinPTime=0;

		CWin Win10(EnsembleSize,Func,&BatchParam);
		// 		vector<double> AceAccs;
		double Win10Avg=0;
		double Win10ChartAvg=0;
		double Win10SizeAvg=0;
		double Win10PTime=0;


		int i;
		for(i=0;!DataFile.eof();)
		{
			//
			CUCIData DataSet;
			DataSet.Load(Info,DataFile,TrunkSize);
			if(DataSet.GetInfo().Height<=0)
				break;

			//
			{
				double Acc;
				CPrediction *Prediction=Sea.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				SeaPTime=+Prediction->GetCreateTime();
				delete Prediction;
				//	 			cout<<"\tSEA("<<Sea.GetRealSize()<<"): "<<Acc;
				// 				SeaAccs.push_back(Acc);
				SeaAvg+=Acc;
				SeaChartAvg+=Acc;
				SeaSizeAvg+=Sea.GetRealSize();
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Sea_OP.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				Sea_OPPTime=+Prediction->GetCreateTime();
				delete Prediction;
				Sea_OPAvg+=Acc;
				Sea_OPChartAvg+=Acc;
				Sea_OPSizeAvg+=Sea_OP.GetRealSize();
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Awe.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				AwePTime=+Prediction->GetCreateTime();
				delete Prediction;
				//				cout<<"\tAWE("<<Awe.GetRealSize()<<"): "<<Acc;
				// 				AweAccs.push_back(Acc);
				AweAvg+=Acc;
				AweChartAvg+=Acc;
				AweSizeAvg+=Awe.GetRealSize();
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Ace.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				AcePTime=+Prediction->GetCreateTime();
				delete Prediction;
				// 	 			cout<<"\tAWE("<<Ace.GetRealSize()<<"): "<<Acc;
				// 				AceAccs.push_back(Acc);
				AceAvg+=Acc;
				AceChartAvg+=Acc;
				AceSizeAvg+=Ace.GetRealSize();
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Eae.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				EaePTime=+Prediction->GetCreateTime();
				delete Prediction;
				//	 			cout<<"\tAWE("<<Eae.GetRealSize()<<"): "<<Acc;
				//				EaeAccs.push_back(Acc);
				EaeAvg+=Acc;
				EaeChartAvg+=Acc;
				EaeSizeAvg+=Eae.GetRealSize();
				// 				cout<<Eae.GetSize()<<":"<<Eae.GetRealSize()<<endl;
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Win.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				WinPTime=+Prediction->GetCreateTime();
				delete Prediction;
				//	 			cout<<"\tAWE("<<Win.GetRealSize()<<"): "<<Acc;
				//				WinAccs.push_back(Acc);
				WinAvg+=Acc;
				WinChartAvg+=Acc;
				WinSizeAvg+=Win.GetRealSize();
			}
			//
			{
				double Acc;
				CPrediction *Prediction=Win10.Classify(DataSet);
				Acc=Prediction->GetAccuracy();
				Win10PTime=+Prediction->GetCreateTime();
				delete Prediction;
				//	 			cout<<"\tAWE("<<Win.GetRealSize()<<"): "<<Acc;
				//				WinAccs.push_back(Acc);
				Win10Avg+=Acc;
				Win10ChartAvg+=Acc;
				Win10SizeAvg+=Win10.GetRealSize();
			}

			i++;
			if(i%ChartInt==0)
			{
				DetailFile<<WinChartAvg/ChartInt<<"\t";
				DetailFile<<Win10ChartAvg/ChartInt<<"\t";
				DetailFile<<SeaChartAvg/ChartInt<<"\t";
				DetailFile<<Sea_OPChartAvg/ChartInt<<"\t";
				DetailFile<<AweChartAvg/ChartInt<<"\t";
				DetailFile<<AceChartAvg/ChartInt<<"\t";
				DetailFile<<EaeChartAvg/ChartInt<<endl;

				WinChartAvg=0;
				Win10ChartAvg=0;
				SeaChartAvg=0;
				Sea_OPChartAvg=0;
				AweChartAvg=0;
				AceChartAvg=0;
				EaeChartAvg=0;
			}

			//build classifier Ci
			CClassifier *Cls=Func(DataSet,&BatchParam);
			Win.Train(DataSet,Cls);
			Win10.Train(DataSet,Cls);
			Sea.Train(DataSet,Cls);
			Sea_OP.Train(DataSet,Cls);
			Awe.Train(DataSet,Cls);
			Ace.Train(DataSet);
			Eae.Train(DataSet,Cls);
			delete Cls;
		}//one file

		//extra
		if(Extra!=0)
		{
			DetailFile<<WinChartAvg/Extra<<"\t";
			DetailFile<<Win10ChartAvg/Extra<<"\t";
			DetailFile<<SeaChartAvg/Extra<<"\t";
			DetailFile<<Sea_OPChartAvg/Extra<<"\t";
			DetailFile<<AweChartAvg/Extra<<"\t";
			DetailFile<<AceChartAvg/Extra<<"\t";
			DetailFile<<EaeChartAvg/Extra<<endl;
		}

		cout<<setprecision(8);

		cout<<"\t"<<WinAvg/i;
		cout<<"\t"<<Win10Avg/i;
		cout<<"\t"<<SeaAvg/i;
		cout<<"\t"<<Sea_OPAvg/i;
		cout<<"\t"<<AweAvg/i;
		cout<<"\t"<<AceAvg/i;
		cout<<"\t"<<EaeAvg/i;
		//
		cout<<"\t"<<WinPTime/i;
		cout<<"\t"<<Win10PTime/i;
		cout<<"\t"<<SeaPTime/i;
		cout<<"\t"<<Sea_OPPTime/i;
		cout<<"\t"<<AwePTime/i;
		cout<<"\t"<<AcePTime/i;
		cout<<"\t"<<EaePTime/i;
		//
		cout<<"\t"<<Win.GetCreateTime()/i;
		cout<<"\t"<<Win10.GetCreateTime()/i;
		cout<<"\t"<<Sea.GetCreateTime()/i;
		cout<<"\t"<<Sea_OP.GetCreateTime()/i;
		cout<<"\t"<<Awe.GetCreateTime()/i;
		cout<<"\t"<<Ace.GetCreateTime()/i;
		cout<<"\t"<<Eae.GetCreateTime()/i;
		//
		cout<<"\t"<<WinSizeAvg/i;
		cout<<"\t"<<Win10SizeAvg/i;
		cout<<"\t"<<SeaSizeAvg/i;
		cout<<"\t"<<Sea_OPSizeAvg/i;
		cout<<"\t"<<AweSizeAvg/i;
		cout<<"\t"<<AceSizeAvg/i;
		cout<<"\t"<<EaeSizeAvg/i;
		//
		cout<<endl;

		//
		DetailFile.close();
		DataFile.close();
	}//all file
	return 0;

}

/*//10-folders cross validation
int main(int argc, char* argv[])
{
	int nRetCode = 0;
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
//	_CrtSetBreakAlloc(219830);
//#define BELOW_NORMAL_PRIORITY_CLASS       0x00004000
//	SetPriorityClass(GetCurrentProcess(),0x00004000);
	//random seeds
	srand((unsigned)time(NULL));

#ifndef TEST_PRUNING
	//validate all data files
// 	for(int i=0;i<FILENUM;i++)
// 		CDataset DataSet(DATA_FILE_PATH+FileName[i]+".names",DATA_FILE_PATH+FileName[i]+".data");
#endif

	char TempStr[1024];
	int Count=0;

	//
	ifstream DataFile;
	DataFile.open("./moadata/HypS.arff");
	if(DataFile.fail())
	{
		throw(CError("open information file failed!",304,0));
	}
	//
	CArffData DataInfo;
	DataInfo.LoadInfo(DataFile);
	CASE_INFO Info=DataInfo.GetInfo();

	CSEA Sea(EnsembleSize,CC45::Create,NULL);
	vector<double> SeaAccs;
	double SeaAvg=0;
	CAWE Awe(EnsembleSize,CC45::Create,NULL);
	vector<double> AweAccs;
	double AweAvg=0;

	CFCAE::ParamStr FCAEParams;
	FCAEParams.Alpha=0.1;
	FCAEParams.Func=CC45::Create;
	FCAEParams.MaxSize=EnsembleSize;
	FCAEParams.Params=NULL;
	CFCAE Eae(&FCAEParams);
	vector<double> EaeAccs;
	double EaeAvg=0;

	int i;
	for(i=0;!DataFile.eof();)
	{
		CArffData DataSet;
		DataSet.Load(Info,DataFile,TrunkSize);
		if(DataSet.GetInfo().Height<=0)
			break;

// 		cout<<i<<":";
		//
		{
			double Acc;
			CPrediction *Prediction=Sea.Classify(DataSet);
			Acc=Prediction->GetAccuracy();
			delete Prediction;
			cout<<Acc;
//  			cout<<"\tSEA("<<Sea.GetRealSize()<<"): "<<Acc;
			SeaAccs.push_back(Acc);
			SeaAvg+=Acc;
		}
		//
		{
			double Acc;
			CPrediction *Prediction=Awe.Classify(DataSet);
			Acc=Prediction->GetAccuracy();
			delete Prediction;
			cout<<"\t"<<Acc;
// 			cout<<"\tAWE("<<Awe.GetRealSize()<<"): "<<Acc;
			AweAccs.push_back(Acc);
			AweAvg+=Acc;
		}
		//
		{
			double Acc;
			CPrediction *Prediction=Eae.Classify(DataSet);
			Acc=Prediction->GetAccuracy();
			delete Prediction;
			cout<<"\t"<<Acc;
// 			cout<<"\tAWE("<<Eae.GetRealSize()<<"): "<<Acc;
			EaeAccs.push_back(Acc);
			EaeAvg+=Acc;
		}
		//
		cout<<endl;
		i++;

 		Sea.Train(DataSet);
 		Awe.Train(DataSet);
		Eae.Train(DataSet);
	}

 	cout<<"SEA: "<<SeaAvg/i<<endl;
 	cout<<"AWE: "<<AweAvg/i<<endl;
	cout<<"FCAE: "<<EaeAvg/i<<endl;

	DataFile.close();
  	return 0;

#ifndef TEST_PRUNING
	while(true)
//		for(ENSEMBLE_SIZE=20;ENSEMBLE_SIZE<100;ENSEMBLE_SIZE+=10)
#endif
	{
		Count++;

		FILE *fs=fopen("stat.txt","at");
		fprintf(fs,"//ENSEMBLE_SIZE=%d\n",ENSEMBLE_SIZE);
		fclose(fs);
		//all datasets
		for(int i=0;i<FILENUM;i++)
		{
			sprintf(TempStr,"StartTime: %s\n",CDateTime::Now().FormatDateTime().c_str());printf("%s",TempStr);ResultLog(TempStr);
			CUCIData DataSet;
			DataSet.Load(DATA_FILE_PATH+FileName[i]+".names",DATA_FILE_PATH+FileName[i]+".data");
			//training set: remove instances with unknown label
			DataSet.RemoveUnknownInstance();
			//pre-process if needed
			//CDataFile::RefAttrs((DATA_FILE_PATH+FileName[i]+".data").c_str(), TrainData,CaseInfo);

			const CASE_INFO &CaseInfo=DataSet.GetInfo();
			int CaseNum=CaseInfo.Height;
			const MATRIX &TrainData=DataSet.GetData();
			sprintf(TempStr,"FileName: %s\n",FileName[i].c_str());printf("%s",TempStr);ResultLog(TempStr);
			sprintf(TempStr,"Samples: %d\n",CaseInfo.Height);printf("%s",TempStr);ResultLog(TempStr);
			sprintf(TempStr,"Attributes: %d\n",CaseInfo.Width);printf("%s",TempStr);ResultLog(TempStr);
			sprintf(TempStr,"Classes: %d\n",CaseInfo.ClassNum);printf("%s",TempStr);ResultLog(TempStr);


			vector<StatStr> Statistics;
			CBpnn::ParamStr BpnnParam;
			BpnnParam.LearnRate=0.9;
			BpnnParam.Momentum=0.5;
			BpnnParam.HideNode=0;
			BpnnParam.MaxEpoch=20000;
			BpnnParam.MinMSE=0.01;
			CClassifier *Classifier=CBpnn::Create(DataSet,(void*)&BpnnParam);
			Classifier->Dump("a.txt");
			CPrediction *Prediction=Classifier->Classify(DataSet);
			cout<<Classifier->GetCreateTime()<<":"<<Prediction->GetAccuracy()<<endl;
// 			CrossValidate<StatStr>(1, DataSet,&(CBpnn::Create),(void*)&Param,Statistics);
// 			cout<<Statistics[0].BuildingTime<<":"<<Statistics[0].AccuracyOfTestset<<endl;
			return 0;
			{
 				//测试基分类器
//				CC45 tcls(DataSet);
//				tcls.Dump("a.txt");
//				tcls.Save("./","a");
//				CC45 tclf("./","a");
//				tclf.Dump("aa.txt");
//				tclf.Save("./","aa");
//				return 0;
			}

			//register base classifier builders
			CBagging::ParamStr BaggingParam;
			BaggingParam.DataPerc=1.0;
			BaggingParam.TotalModel=ENSEMBLE_SIZE;
			CEnsemble::CreatorRegisterStr Creator;
			Creator.Creator=CBpnn::RpropCreate;
			Creator.Params=NULL;
			Creator.Ratio=BPNN_RATIO;
			BaggingParam.Creators.push_back(Creator);
// 			CEnsemble::Register(C45_RATIO,&(CC45::Create),NULL);
// 			CEnsemble::Register(SVM_RATIO,&(CSVM::Create),NULL);
// 			CEnsemble::Register(NAIVEBAYES_RATIO,&(CNaiveBayes::Create),NULL);
// 			CEnsemble::Register(GAUSSNAIVEBAYES_RATIO,&(CGaussNaiveBayes::Create),NULL);
			//register ensemble pruners
			vector<PrunerCreator*> EnsemblePFactory;
			EnsemblePFactory.push_back((PrunerCreator*)&CSelectAll::Create);
			EnsemblePFactory.push_back((PrunerCreator*)&CSelectBest::Create);
//			EnsemblePFactory.push_back((PrunerCreator*)&(CGasen::Create));
			EnsemblePFactory.push_back((PrunerCreator*)&CForwardSelect::Create);
			EnsemblePFactory.push_back((PrunerCreator*)&COrientOrder::Create);
			EnsemblePFactory.push_back((PrunerCreator*)&CMDSQ::Create);
//			EnsemblePFactory.push_back((PrunerCreator*)&CCluster::Create);
//			EnsemblePFactory.push_back((PrunerCreator*)&CPMEP::Create);
			//number of all ensemble pruning methods
			int STATNUM=(int)EnsemblePFactory.size();
			//Statistics
			StatStr Stat;
			memset(&Stat,0,sizeof(Stat));
			vector<StatStr> Stats(STATNUM,Stat);

			//10-folder cross validation
			for(int k=0;k<FOLDERS;k++)
			{
				//all of the instances are group into original dataset and test set
				CDataset OrgTrainSet,TestSet;
				int Sample_num=(int)(CaseNum*ORIGINAL_TRAINSET_RATIO);
				DataSet.SplitData(Sample_num,OrgTrainSet,TestSet);
				//bootstrap sampling: get train set of this folder
				Sample_num=(int)(Sample_num*TRAINSET_RATIO);
				CDataset TrainSet;
				OrgTrainSet.BootStrap(Sample_num,TrainSet);
				//build validation set needed by all ensemble pruning algorithms
				CDataset ValSet;
				TrainSet.BootStrap(Sample_num,ValSet);

				//bagging model generating
				CBagging BaggingEnsemble(TrainSet,&BaggingParam);
				//
				{
					//only get accuracy of first classifier for each type of classifier
					const vector<const CClassifier*> &Classifiers=BaggingEnsemble.GetAllClassifiers();
					string Type="";
					for(int j=0;j<(int)Classifiers.size();j++)
					{
						double Accuracy=0;
						//if(Classifiers[j]->GetName()!=Type)
						{
							Type=Classifiers[j]->GetName();
							CPrediction *Prediction=Classifiers[j]->Classify(TestSet);
							Accuracy=Prediction->GetAccuracy();
							delete Prediction;
						}
						sprintf(TempStr,"%s: Folder=%d, %d of %s, Accuracy=%.4f, Time=%.4f\n",
							FileName[i].c_str(),k,j,Classifiers[j]->GetName().c_str(),
							Accuracy,Classifiers[j]->GetCreateTime());
						ResultLog(TempStr);
					}
				}
				//save all of the classifiers into files
// 				BaggingEnsemble.Save(FileName[i].c_str());
				//classifiers can also be loaded from formerly created files
//				BaggingEnsemble.Create(FileName[i].c_str(),UserClses,UserTribe);
				//each classifier predict on the validation set
				vector<CPrediction*> *Predictions=BaggingEnsemble.AllClassify(ValSet);

				for(int j=0;j<STATNUM;j++)
				{
					//ensemble pruning
					CEnsemblePruner *PrunedEnsemble=(*EnsemblePFactory[j])(BaggingEnsemble,ValSet,(*Predictions),NULL);
					//ensemble can also be loaded from formerly created files
//					CEnsemblePruner *EnsPruner=new CEnsemblePruner(FileName[i].c_str(),0,UserClses);

					int SelNum=0;
					{
						//show selected classifiers
						sprintf(TempStr,"%s: File=%s, Classifiers: \n",PrunedEnsemble->GetName().c_str(),FileName[i].c_str());
						ResultLog(TempStr);
						//show selected classifiers
						const vector<double> &Selected=PrunedEnsemble->GetWeights();
						for(int l=0;l<ENSEMBLE_SIZE;l++)
							if(Selected[l]>0)
							{
								sprintf(TempStr,"%d,",l);
								ResultLog(TempStr);
								SelNum++;
							}
						//show accuracy on validating set
						CPrediction *Prediction=PrunedEnsemble->Classify(ValSet,*Predictions);
						sprintf(TempStr,"\n%s: File=%s, ClsNum=%d, Accuracy(on VS)%.4f, Time(pruning)%.4f\n",PrunedEnsemble->GetName().c_str(),
							FileName[i].c_str(),SelNum,Prediction->GetAccuracy(),PrunedEnsemble->GetCreateTime());
						ResultLog(TempStr);
						Stats[j].ClssifierNum+=SelNum;
						Stats[j].BuildingTime+=PrunedEnsemble->GetCreateTime();
						Stats[j].AccuracyOfValset+=Prediction->GetAccuracy();
						delete Prediction;
					}

					{
						CPrediction *TestsetPrediction=PrunedEnsemble->Classify(TestSet);
						//the following code dose the same thing
						// 					PROB_RESULT *BpnnResult=NULL;
						// 					EnsPruner->Classify(TestSet,CaseInfo,&BpnnResult);
						// 					PRED_RESULT Pred_Result;
						// 					GetPreResult(BpnnResult,&Pred_Result,TestSet);
						// 					double accuracy=BpnnResult.Accuracy;
						// 					free_prob(BpnnResult);
						sprintf(TempStr,"%s: File=%s, ClsNum=%d, Accuracy(on TS)%.4f, Time(predicting)%.4f\n\n",PrunedEnsemble->GetName().c_str(),
							FileName[i].c_str(),SelNum,TestsetPrediction->GetAccuracy(),TestsetPrediction->GetCreateTime());
						ResultLog(TempStr);
						//stat
						Stats[j].PredictTime+=(TestsetPrediction->GetCreateTime());
						Stats[j].AccuracyOfTestset+=(TestsetPrediction->GetAccuracy());
						Stats[j].Accuracys.push_back(TestsetPrediction->GetAccuracy());

						delete TestsetPrediction;
					}

					delete PrunedEnsemble;
				}//for STATNUM

				for(int j=0;j<(int)(*Predictions).size();j++)
				{
					delete ((*Predictions)[j]);
				}
				delete Predictions;
			}//for k folder
			ResultLog("----------------------------------------------------------------------------------------------------\n");

			//write the statistics of this folder for all compared algorithms into file
			fs=fopen("stat.txt","at");
			fprintf(fs,"%s: \n",FileName[i].c_str());
			for(int k=0;k<STATNUM;k++)
			{
//				if(k!=7)continue;
				Stats[k].ClssifierNum/=FOLDERS;
				Stats[k].AccuracyOfTestset/=FOLDERS;
				Stats[k].BuildingTime/=FOLDERS;
				Stats[k].AccuracyOfValset/=FOLDERS;
				Stats[k].PredictTime/=FOLDERS;
				double Vari=0,dis=0;
				for(int j=0;j<FOLDERS;j++)
				{
					dis=(Stats[k].Accuracys[j]-Stats[k].AccuracyOfTestset);
					Vari+=(dis*dis);
				}
				Stats[k].Variance=sqrt(Vari/FOLDERS)/2;

				fprintf(fs,"\t\t\t\t%f\t\t%f\t\t%f\t\t%f\t\t%f\t\t%f\n",
					Stats[k].BuildingTime,Stats[k].ClssifierNum,
					Stats[k].AccuracyOfValset*100,Stats[k].AccuracyOfTestset*100,Stats[k].Variance*100,
					Stats[k].PredictTime);
			}//k
			fclose(fs);
		}//for i file
#ifndef TEST_PRUNING
		printf("//--%03d---------------------------------------------------------------------------------------------\n",Count);
		fs=fopen("stat.txt","at");
		fprintf(fs,"//--%03d---------------------------------------------------------------------------------------------------------------\n",Count);
		fclose(fs);
#endif
	}//while true
	ResultLog("----------------------------------------------------------------------------------------------------\n");

	return nRetCode;
}

*/