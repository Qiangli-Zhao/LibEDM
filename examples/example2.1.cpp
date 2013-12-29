#include <string>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "bpnn.h"
#include "Prediction.h"
using namespace libep;

int main(int argc, char* argv[])
{
	//load a BP neural network from a file
	try
	{
		//in current directory
		CBpnn BP2("","BP1.sav");
		cout<<"Type of the classifier: "<<BP2.GetName()<<endl;
		cout<<"Time creating the classifier: "<<BP2.GetCreateTime()<<endl;
		//dump to check
		BP2.Dump("BP2.dmp");
	}
	catch (CError &e)
	{
		cout<<e.Description<<endl;
		if(e.Level<=0)
		{
			return 0;
		}
	}


	return 0;
}

