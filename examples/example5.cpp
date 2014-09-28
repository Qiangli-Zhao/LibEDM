/*
Copyright (c) 2009-2010 Qiang-Li Zhao
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither name of copyright holders nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.


THIS SOURCE CODE IS SUPPLIED ¡°AS IS¡± WITHOUT WARRANTY OF ANY KIND,
AND ITS AUTHOR AND THE JOURNAL OF MACHINE LEARNING RESEARCH (JMLR)
AND JMLR¡¯S PUBLISHERS AND DISTRIBUTORS, DISCLAIM ANY AND ALL
WARRANTIES, INCLUDING BUT NOT LIMITED TO ANY IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND
ANYWARRANTIES OR NON INFRINGEMENT. THE USER ASSUMES ALL LIABILITY
AND RESPONSIBILITY FOR USE OF THIS SOURCE CODE, AND NEITHER THE
AUTHOR NOR JMLR, NOR JMLR¡¯S PUBLISHERS AND DISTRIBUTORS, WILL BE
LIABLE FOR DAMAGES OF ANY KIND RESULTING FROM ITS USE. Without
limiting the generality of the foregoing, neither the author, nor
JMLR, nor JMLR¡¯s publishers and distributors, warrant that the
Source Code will be error-free, will operate without interruption,
or will meet the needs of the user.
*/


#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
using namespace std;
#include "Obj.h"
#include "zString.h"
#include "Statistic.h"
using namespace libedm;


int main(int argc, char* argv[])
{
	int nRetCode = 0;
	int i,k;


	//open rank file
	ifstream DataFile;
	DataFile.open("Ranks.txt");
	if(DataFile.fail())
	{
		cout<<"Rank.txt not found!"<<endl;
		return 1;
	}
	//buffer to analyze a line of input file
	char buf[8192];
	//Ranks of each methods on all data sets
	vector<DoubleArray> AllRanks;
	while(!DataFile.eof())
	{
		//read a line
		DataFile.getline(buf,sizeof(buf));
		if(DataFile.fail())
		{
			if(DataFile.eof())
				continue;
			cout<<"error reading!"<<endl;
			return 1;
		}

		//read data from the line
		basic_istringstream<char> DataLine(buf);
		//rank of all methods on this data set
		vector<double> Ranks;
		//number of ranks has read 
		while(!DataLine.eof())
		{
			//read a value
			string Word;
			DataLine>>Word;
			//read failed
			if(DataLine.fail())
				break;

			//read a rank number
			double Rank=CzString::ToDouble(Word);
			Ranks.push_back(Rank);
		}
		AllRanks.push_back(Ranks);
	}
	DataFile.close();

	//number of comparing methods
	int MethodNum=(int)(AllRanks[0]).size();
	//number of data set
	int DatasetNum=(int)AllRanks.size();
	//each algorithm: average ranks for all datasets
	vector<double> AvgRanks(MethodNum,0);
	for(k=0;k<DatasetNum;k++)
		for(i=0;i<MethodNum;i++)
			AvgRanks[i]+=AllRanks[k][i];
	for(i=0;i<MethodNum;i++)
		AvgRanks[i]/=DatasetNum;

	//Friedman test
	cout<<"values for Friedman test:"<<endl;
	double Skf;
	double Ff=CStat::Ff(AvgRanks,DatasetNum,Skf);
	cout<<"Kai2F="<<Skf<<", Ff("<<AvgRanks.size()-1<<", "<<(AvgRanks.size()-1)*(DatasetNum-1)<<")="<<Ff<<endl;

	//BH test
	vector<RankStr> Tab;
	CStat::BH(DatasetNum,AvgRanks,Tab);
	//Bergmann-Hommel test(all pairwise tests)
	//values needed for each pairwise comparisons sorted by its p-value(one hypothesis each line)
	//hypothesis: the column "vs" means the two algorithms being compared(1-the 1st algorithms, 2-the 2nd,...,9-the 9th)
	//support nine compared algorithms at the most
	cout<<endl<<"hypothesis and values of Bergmann-Hommel test";
	cout<<endl<<"No\tvs\tz\tp\tAPV"<<endl;
	for(i=0;i<(int)Tab.size();i++)
		cout<<i<<"\t"<<Tab[i].vs<<"\t"<<Tab[i].z<<"\t"<<Tab[i].p<<"\t"<<Tab[i].APV<<endl;


	return nRetCode;
}


