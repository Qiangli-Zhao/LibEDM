#include <string>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "UCIData.h"
using namespace libedm;

int main(int argc, char* argv[])
{
	try
	{
		//create data set by reading from a description file and a data file
		CUCIData ds;
		ds.Load("example.names","example.data");
		ds.DumpInfo("Info.txt");
		ds.DumpData("Data.txt");
		//get instances of the data set
		MATRIX Matrix=ds.GetData();
		//get information of the data set
		CASE_INFO CaseInfo=ds.GetInfo();
		cout<<"number of instances: "<<CaseInfo.Height<<endl;
		cout<<"number of class labels: "<<CaseInfo.ClassNum<<endl;
		cout<<"number of all attributes: "<<CaseInfo.ReadWidth<<endl;
		cout<<"number of valid attributes: "<<CaseInfo.ValidWidth<<endl;
		//extract type of the 1st attribute
		int AttType=CaseInfo.ValidAttrs[0].AttType;
		//and first value of first instance
		if(AttType==ATT_DISCRETE)
		{
			cout<<"the first attribute is discrete."<<endl;
			cout<<"first value of first instance is (discrete): "<<Matrix[0][0].Discr<<endl;
		}
		else if(AttType==ATT_CONTINUOUS)
		{
			cout<<"the first attribute is continuous."<<endl;
			cout<<"first value of the first instance is (continuous): "<<Matrix[0][0].Cont<<endl;
		}

		//remove single-valued attributes
		ds.RemoveNullAttribute();
		ds.DumpInfo("Info1.txt");
		ds.DumpData("Data1.txt");
		//remove the instances whose label are unknown
		ds.RemoveUnknownInstance();
		ds.DumpInfo("Info2.txt");
		ds.DumpData("Data2.txt");

		//create a new data set by extending all discrete attributes of the original data set
		CDataset *Extds=ds.ExpandDiscrete();
		Extds->DumpInfo("Info3.txt");
		Extds->DumpData("Data3.txt");
		delete Extds;
	}
	catch (CError &e)
	{
		cout<<e.Description<<endl;
		if(e.Level<=0)
			exit(0);
	}

	return 0;
}

