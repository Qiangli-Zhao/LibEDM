#include <string>
#include <iostream>
#include <fstream>
#include <istream>
#include <strstream>
#include <cstdarg>
using namespace std;
#include "DataFile.h"
using namespace libep;

string FileName=""

int main(int argc, char* argv[])
{
	try
	{
		string Directory=".\\data\\";
		string Filename=sick;
		CDataset ds(Directory+FileName+".names",Directory+FileName+".data");
		cout<<"finished :"<<Directory+FileName<<endl;
		ds.DumpInfo();
		ds.DumpData();

		ds.RemoveNullAttribute();
		ds.RemoveUnknownInstance();
		CDataset Extds;
		ds.ExtDatas(Extds);
		ds.RemoveNullAttribute();
	}
	catch (CError &e)
	{
		cout<<e.Description<<endl;
		if(e.Level<=0)
			exit(1);
	}

	return 0;
}