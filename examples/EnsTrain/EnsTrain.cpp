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

// EnsTrain.cpp : Defines the entry point for the console application.
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


//percent of total instances that will be used as original training set(the rest is used as test set)
const double ORGTRAINSETPERCENT=0.9;
//percent of instances in original training set, which will be bootstrap-sampled as train set of base classifiers
const double TRAINSETPERCENT=0.5;
//distribution of each kind of base classifier
int BPNNPERDATA=100;
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
	//training set: remove instances with unknown label
	CDataFile::PreForTraining(matrix);
	//display
	printf("FileName: %s\n",FileName);
	printf("Samples: %d\n",matrix.height);
	printf("Attributes: %d\n",matrix.width);
	printf("Classes: %d\n",CaseInfo.ClassNum);

	//prepare all datasets for training
	//all of the instances are group into original dataset and test set
	MATRIX OrgTrainSet,TestSet;
	int Sample_num=(int)(matrix.height*ORGTRAINSETPERCENT);
	CDataFile::SplitData(FileName,0,matrix,CaseInfo,Sample_num,OrgTrainSet,TestSet);
	//bootstrap sampling: get train set of this folder
	Sample_num=(int)(Sample_num*TRAINSETPERCENT);
	MATRIX TrainSet;
	CDataFile::BootStrapData(FileName,0,"TrainSet",OrgTrainSet,CaseInfo,Sample_num,TrainSet);
	//build validation set needed by all ensemble pruning algorithms
	char NNFileStr[128];
	sprintf(NNFileStr,"%s%02d",FileName,0);
	MATRIX ValSet;
	CDataFile::BootStrapData(NNFileStr,BPNNPERDATA,"ValSet",TrainSet,CaseInfo,Sample_num,ValSet);

	//base classifiers traing
	//register base classifier builders(need not to register the FileCreate function)
	CBagGener BagGener;
	BagGener.ModReg(BPNNPERC,&(CBpnn::Create));
	BagGener.ModReg(C45PERC,&(CC45::Create));
	BagGener.ModReg(NAVBPERC, &(CNaiveBayes::Create));
	//base classifiers and their corresponding weights(boosting may create classifiers with weights)
	vector<CClassifier*> UserClses;
	vector<double> Weights;
	//bagging model generating
 	BagGener.Create(FileName,TrainSet,CaseInfo,TestSet,BPNNPERDATA,1.0,UserClses,Weights);
#ifndef UNIX
	//save all of the classifiers into files
	BagGener.Save(FileName);
#endif

	//predict on the validation set
	vector<PRED_RESULT> PredResults;
	for(int j=0;j<BPNNPERDATA;j++)
	{
		PROB_RESULT *BpnnResult=NULL;
		//prediction time of each base classifier on validation set
		UserClses[j]->Classify(ValSet,CaseInfo,&BpnnResult);
		PRED_RESULT PredResult;
		GetPreResult(BpnnResult,&PredResult,ValSet);
		PredResults.push_back(PredResult);
		free_prob(BpnnResult);
	}//for j classifier

	//ensemble pruning
	CFwdSel *EnsPruner=new CFwdSel(CaseInfo,ValSet,PredResults,UserClses,Weights);
#ifndef UNIX
	//save it
	EnsPruner->Save(FileName,0);
#endif
	//show selected classifiers
	printf("%s: File=%s, Classifiers: \n",EnsPruner->ClsName.c_str(),FileName);
	int SelNum=0;
	for(int l=0;l<BPNNPERDATA;l++)
		if(EnsPruner->Tribe.Weights[l]>0)
		{
			printf("%d,",l);
			SelNum++;
		}

	//predicting on the validation set
	EnsPruner->Tribe.Pt=0;
	CEnsPruner::VotePredict(EnsPruner->Tribe,ValSet,PredResults,CaseInfo);
	printf("\n%s: File=%s, ClsNum=%d, Accuracy(on VS)%.4f\n",EnsPruner->ClsName.c_str(),
		FileName,SelNum,EnsPruner->Tribe.Pt);
	//predicting on the test set
	EnsPruner->Tribe.Pt=0;
	CEnsPruner::VotePredict(EnsPruner->Tribe,TestSet,CaseInfo,UserClses);
	printf("%s: File=%s, ClsNum=%d, Accuracy(on TS)%.4f\n\n",EnsPruner->ClsName.c_str(),
		FileName,SelNum,EnsPruner->Tribe.Pt);

	//remove ensemble
	delete EnsPruner;
	//remove all base classifiers generated in this round
	BagGener.Flush();

	return 0;
}

