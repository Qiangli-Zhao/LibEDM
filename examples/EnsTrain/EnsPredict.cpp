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

//
#ifndef UNIX
#include <windows.h>
#endif
#include "bpnn.h"
#include "b-svm.h"
#include "C45.h"
#include "NaiveBayes.h"
#include "BagGener.h"
#include "FS.h"


//percent of total instances that will be used as test set
const double TESTPERCENT=0.1;
//distribution of each kind of base classifier
const double BPNNPERC=0.40;
const double C45PERC=0.20;
//there are bugs in the save/restore function libsvm, so we don't use it
const double SVMPERC=0.00;
const double NAVBPERC=0.20;


int main(int argc, char* argv[])
{
	CASE_INFO CaseInfo;
	MATRIX matrix;
	char FileName[]="wine";

#ifndef UNIX
 	char dd[512];
 	GetCurrentDirectory(511,dd);
#endif
	//reading instances from files
	//Read structure of instances
	CDataFile::GetNames("wine.names", CaseInfo);
	//read instances
	CDataFile::read_matrix ("wine.data", matrix,CaseInfo);
	//display
	printf("FileName: %s\n",FileName);
	printf("Samples: %d\n",matrix.height);
	printf("Attributes: %d\n",matrix.width);
	printf("Classes: %d\n",CaseInfo.ClassNum);

	//prepare the test dataset
	MATRIX TestSet;
	int Sample_num=(int)(matrix.height*TESTPERCENT);
	CDataFile::BootStrapData(FileName,0,"testSet",matrix,CaseInfo,Sample_num,TestSet);

	//classifiers loading from files: still not support on Linux
#ifndef UNIX
	//base classifiers training
	//register base classifier builders
	CBagGener BagGener;
	BagGener.ModFileReg("BPNN",&(CBpnn::FileCreate));
	BagGener.ModFileReg("C45",&(CC45::FileCreate));
	BagGener.ModFileReg("NAIVEBAYES",&(CNaiveBayes::FileCreate));
	//base classifiers and their corresponding weights(boosting may create classifiers with weights)
	vector<CClassifier*> UserClses;
	TribeStr UserTribe;
	//classifiers can also be loaded from formerly created files
	BagGener.FileCreate(FileName,UserClses,UserTribe);

	//ensemble can also be loaded from formerly created files
	CEnsPruner *EnsPruner=new CEnsPruner(FileName,0,UserClses);
	//predicting on the test set
	EnsPruner->Tribe.Pt=0;
	CEnsPruner::VotePredict(EnsPruner->Tribe,TestSet,CaseInfo,UserClses);
	printf("%s: File=%s, Accuracy(on TS)%.4f\n\n",EnsPruner->ClsName.c_str(),
		FileName,EnsPruner->Tribe.Pt);

	//remove ensemble
	delete EnsPruner;
	//remove all base classifiers generated in this round
	BagGener.Flush();
#else
	printf("Loading classifiers from files is not supporting on Linux!\n");
#endif

	return 0;
}

