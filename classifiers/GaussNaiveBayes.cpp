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


#include <set>
#include <map>
#include <cmath>
#include <fstream>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "IncrementalClassifier.h"
#include "Prediction.h"
#include "GaussNaiveBayes.h"
#include "Statistic.h"
using namespace libep;


//number of division for a continuous attribute's value range
const char	MyName[MAX_OBJECT_NAME_LENGTH]="GAUSS_NAIVEBAYES";
const string CGaussNaiveBayes::StaticName=MyName;

CGaussNaiveBayes::~CGaussNaiveBayes()
{
}

void CGaussNaiveBayes::Reset()
{
	int Width=(int)Estims.size();
	int ClassNum=(int)Estims[0].size();
	//initialize all data structure
	for(int i=0;i<Width;i++)
		for(int k=0;k<ClassNum;k++)
		{
			if(Estims[i][k].AttType==ATT_DISCRETE)
			{
				//Laplace estimator
				Estims[i][k].DiscEst.Count=1;
				int ValNum=(int)Estims[i][k].DiscEst.AttrCount.size();
				for(int j=0;j<ValNum;j++)
					Estims[i][k].DiscEst.AttrCount[j]=(1.0/ValNum);
			}
			//continuous attribute
			else
			{
				Estims[i][k].ContEst.Count=0;
				Estims[i][k].ContEst.Mean=0;
				Estims[i][k].ContEst.StDevia=0;
			}
		}
}

CGaussNaiveBayes::CGaussNaiveBayes(const CDataset &TrainSet)
{
	Name=MyName;
	//start time for training
	clock_t start=clock();

	//data
	const MATRIX &OrgData=TrainSet.GetData();
	const CASE_INFO &OrgInfo=TrainSet.GetInfo();

	//initialize all data structure
	for(int i=0;i<OrgInfo.ValidWidth-1;i++)
	{
		EstimatorStr Estim;
		Estim.AttType=OrgInfo.ValidAttrs[i].AttType;
		if(Estim.AttType==ATT_DISCRETE)
		{
			//Laplace estimator
			Estim.DiscEst.Count=1;
			int ValNum=(int)OrgInfo.ValidAttrs[i].Disc.size();
			for(int j=0;j<ValNum;j++)
				Estim.DiscEst.AttrCount.push_back(1.0/ValNum);
		}
		//continuous attribute
		else
		{
			Estim.ContEst.Count=0;
			Estim.ContEst.Mean=0;
			Estim.ContEst.StDevia=0;
		}

		vector<EstimatorStr> EstiAttr;
		for(int j=0;j<OrgInfo.ClassNum;j++)
			EstiAttr.push_back(Estim);
		Estims.push_back(EstiAttr);
	}

	//statistics
	for(int i=0;i<OrgInfo.Height;i++)
	{
		int Class=OrgData[i][OrgInfo.ValidWidth-1].Discr;
		for(int j=0;j<OrgInfo.ValidWidth-1;j++)
			switch(OrgInfo.ValidAttrs[j].AttType)
			{
				case ATT_DISCRETE:
					{
						int Val=OrgData[i][j].Discr;
						Estims[j][Class].DiscEst.Count++;
						//j: attribute, Class: label, Val: value of attribute
						Estims[j][Class].DiscEst.AttrCount[Val]++;
					}
					break;
				case ATT_CONTINUOUS:
				case ATT_DATETIME:
					{
						double Val=OrgData[i][j].Cont;
						Estims[j][Class].ContEst.Vals.push_back(Val);
						Estims[j][Class].ContEst.Count++;
						Estims[j][Class].ContEst.Mean+=Val;
					}
					break;
				default:
					break;
			}
	}//for data

	//get all statistics needed
	for(int i=0;i<OrgInfo.ValidWidth-1;i++)
	{
		switch(OrgInfo.ValidAttrs[i].AttType)
		{
			case ATT_DISCRETE:
				for(int j=0;j<OrgInfo.ClassNum;j++)
				{
					int ValNum=(int)OrgInfo.ValidAttrs[i].Disc.size();
					for(int k=0;k<ValNum;k++)
						Estims[i][j].DiscEst.AttrCount[k]/=Estims[i][j].DiscEst.Count;
				}
				break;
			case ATT_CONTINUOUS:
			case ATT_DATETIME:
				for(int j=0;j<OrgInfo.ClassNum;j++)
				{
					int ValNum=Estims[i][j].ContEst.Count;
					Estims[i][j].ContEst.Mean/=ValNum;
					double Sum=0;
					for(int k=0;k<ValNum;k++)
					{
						double Diff=Estims[i][j].ContEst.Vals[k]-Estims[i][j].ContEst.Mean;
						Sum+=(Diff*Diff);
					}
					Estims[i][j].ContEst.StDevia=sqrt(Sum/(ValNum-1));
					//we don't need it anymore
					Estims[i][j].ContEst.Vals.clear();
				}
				break;
			default:
				break;
		}//switch
	}//for attr

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

bool CGaussNaiveBayes::Dump(const string &FileName) const
{
	//write in file
	ofstream OutFile;
	OutFile.open(FileName.c_str());
	if(OutFile.fail())
	{
		return false;
	}

	int AttrNum=(int)Estims.size();
	OutFile<<"AttrNum="<<AttrNum<<endl;
	int ClassNum=(int)Estims[0].size();
	OutFile<<"ClassNum="<<ClassNum<<endl;
	for(int i=0;i<AttrNum;i++)
	{
		OutFile<<"Attribute"<<i;
		for(int j=0;j<ClassNum;j++)
		{
			switch(Estims[i][j].AttType)
			{
			case ATT_DISCRETE:
				{
					int ValNum=(int)Estims[i][j].DiscEst.AttrCount.size();
					if(j==0)
						OutFile<<": Type=Discrete,ValueNumber="<<ValNum<<endl;

					for(int k=0;k<ValNum;k++)
					{
						double cof=Estims[i][j].DiscEst.AttrCount[k];
						OutFile<<"\t"<<cof;
					}
					OutFile<<endl;
				}
				break;
			case ATT_CONTINUOUS:
			case ATT_DATETIME:
				if(j==0)
					OutFile<<": Type=continuous"<<endl;
				OutFile<<"\tCount="<<Estims[i][j].ContEst.Count<<"\tMean="<<Estims[i][j].ContEst.Mean<<
					",Deviation="<<Estims[i][j].ContEst.StDevia<<endl;
				break;
			default:
				break;
			}//switch
		}
	}//for attr

	OutFile.close();
	return true;
}

CPrediction *CGaussNaiveBayes::Classify(const CDataset &DataSet) const
{
	//start time for training
	clock_t Start=clock();

	//data
	const MATRIX &OrgData=DataSet.GetData();
	const CASE_INFO &OrgInfo=DataSet.GetInfo();

	//malloc space for result
	vector<double> Line(OrgInfo.ClassNum,0);
	DoubleArray2d Result(OrgInfo.Height,Line);

	//predict each instance of this data set
	for(int i=0;i<OrgInfo.Height;i++)
	{
		double CProb=0;
		for(int k=0;k<OrgInfo.ClassNum;k++)
		{
			double Prob=1.0;
			for(int j=0;j<OrgInfo.ValidWidth-1;j++)
			{
				switch(OrgInfo.ValidAttrs[j].AttType)
				{
					case ATT_DISCRETE:
						{
							int Val=OrgData[i][j].Discr;
							Prob*=Estims[j][k].DiscEst.AttrCount[Val];
						}
						break;
					case ATT_CONTINUOUS:
					case ATT_DATETIME:
						{
							double Val=OrgData[i][j].Cont;
							double pow=(Val-Estims[j][k].ContEst.Mean)/Estims[j][k].ContEst.StDevia;
							Prob*=CStat::Gauss(pow);
						}
						break;
					default:
						break;
				}
			}
			Result[i][k]=Prob;
			CProb+=Prob;
		}
		for(int k=0;k<OrgInfo.ClassNum;k++)
			Result[i][k]/=CProb;
	}//for data

	return (new CPrediction(DataSet,Result,clock() - Start));
}

int CGaussNaiveBayes::Save(const string &Path,const string &FileName) const
{
	//write in file
	ofstream OutFile;
	OutFile.open((Path+FileName+"."+Name).c_str(),ios_base::trunc|ios_base::out|ios_base::binary);
	if(OutFile.fail())
	{
		return 1;
	}

	int AttrNum=(int)Estims.size();
	OutFile.write((char*)&AttrNum,sizeof(AttrNum));
	int ClassNum=(int)Estims[0].size();
	OutFile.write((char*)&ClassNum,sizeof(ClassNum));
	for(int i=0;i<AttrNum;i++)
	{
		for(int j=0;j<ClassNum;j++)
		{
			OutFile.write((char*)&Estims[i][j].AttType,sizeof(Estims[i][j].AttType));
			switch(Estims[i][j].AttType)
			{
			case ATT_DISCRETE:
				{
					int ValNum=(int)Estims[i][j].DiscEst.AttrCount.size();
					if(j==0)
						OutFile.write((char*)&ValNum,sizeof(ValNum));

					for(int k=0;k<ValNum;k++)
					{
						double cof=Estims[i][j].DiscEst.AttrCount[k];
						OutFile.write((char*)&cof,sizeof(cof));
					}
				}
				break;
			case ATT_CONTINUOUS:
			case ATT_DATETIME:
				OutFile.write((char*)&Estims[i][j].ContEst.Count,sizeof(Estims[i][j].ContEst.Count));
				OutFile.write((char*)&Estims[i][j].ContEst.Mean,sizeof(Estims[i][j].ContEst.Mean));
				OutFile.write((char*)&Estims[i][j].ContEst.StDevia,sizeof(Estims[i][j].ContEst.StDevia));
				break;
			default:
				break;
			}//switch
		}
	}//for attr

	OutFile.close();
	return 0;
}

CGaussNaiveBayes::CGaussNaiveBayes(const string &Path,const string &FileName)
{
	Name=MyName;
	//start time for training
	clock_t start=clock();

	//reading fome file
	ifstream InFile;
	InFile.open((Path+FileName+"."+Name).c_str(),ios_base::in|ios_base::binary);
	if(InFile.fail())
	{
		throw(CError("NaiveBayes: fail open saved file!",100,0));
	}

	int AttrNum,ClassNum;
	InFile.read((char*)&AttrNum,sizeof(AttrNum));
	InFile.read((char*)&ClassNum,sizeof(ClassNum));
	for(int i=0;i<AttrNum;i++)
	{
		vector<EstimatorStr> EstiAttr;
		for(int j=0;j<ClassNum;j++)
		{
			EstimatorStr Estim;
			InFile.read((char*)&Estim.AttType,sizeof(Estim.AttType));

			switch(Estim.AttType)
			{
			case ATT_DISCRETE:
				{
					int ValNum;
					InFile.read((char*)&ValNum,sizeof(ValNum));
					for(int k=0;k<ValNum;k++)
					{
						double cof;
						InFile.read((char*)&cof,sizeof(cof));
						Estim.DiscEst.AttrCount.push_back(cof);
					}
				}
				break;
			case ATT_CONTINUOUS:
			case ATT_DATETIME:
				{
					int ValNum;
					InFile.read((char*)&ValNum,sizeof(ValNum));
					Estim.ContEst.Count=ValNum;

					double cof;
					InFile.read((char*)&cof,sizeof(cof));
					Estim.ContEst.Mean=cof;
					InFile.read((char*)&cof,sizeof(cof));
					Estim.ContEst.StDevia=cof;
				}
				break;
			default:
				break;
			}//switch
			EstiAttr.push_back(Estim);
		}
		Estims.push_back(EstiAttr);
	}//for attr

	InFile.close();

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

void CGaussNaiveBayes::Train(const CDataset &TrainSet)
{
	//start time for training
	clock_t start=clock();

	//data
	const MATRIX &OrgData=TrainSet.GetData();
	const CASE_INFO &OrgInfo=TrainSet.GetInfo();
	if((int)Estims.size()!=OrgInfo.ValidWidth-1)
		throw(CError("NaiveBayes: invalid training data!",200,0));


	//save old statistics, for we have new training data
	for(int i=0;i<OrgInfo.ValidWidth-1;i++)
	{
		for(int j=0;j<OrgInfo.ClassNum;j++)
			if(Estims[i][j].AttType==ATT_CONTINUOUS||Estims[i][j].AttType==ATT_DATETIME)
			{
				Estims[i][j].ContEst.OldMean=Estims[i][j].ContEst.Mean;
				Estims[i][j].ContEst.OldCount=Estims[i][j].ContEst.Count;
				//new values will add to this
				Estims[i][j].ContEst.Mean=Estims[i][j].ContEst.Count*Estims[i][j].ContEst.Mean;
			}
	}

	//statistics
	for(int i=0;i<OrgInfo.Height;i++)
	{
		int Class=OrgData[i][OrgInfo.ValidWidth-1].Discr;
		for(int j=0;j<OrgInfo.ValidWidth-1;j++)
			switch(OrgInfo.ValidAttrs[j].AttType)
			{
				case ATT_DISCRETE:
					{
						int Val=OrgData[i][j].Discr;
						Estims[j][Class].DiscEst.Count++;
						//j: attribute, Class: label, Val: value of attribute
						Estims[j][Class].DiscEst.AttrCount[Val]++;
					}
					break;
				case ATT_CONTINUOUS:
				case ATT_DATETIME:
					{
						double Val=OrgData[i][j].Cont;
						Estims[j][Class].ContEst.Count++;
						//save all new values
						Estims[j][Class].ContEst.Vals.push_back(Val);
						//add new valuse
						Estims[j][Class].ContEst.Mean+=Val;
					}
					break;
				default:
					break;
			}
	}//for data

	//get all statistics needed
	for(int i=0;i<OrgInfo.ValidWidth-1;i++)
	{
		switch(OrgInfo.ValidAttrs[i].AttType)
		{
		case ATT_DISCRETE:
			for(int j=0;j<OrgInfo.ClassNum;j++)
			{
				int ValNum=(int)OrgInfo.ValidAttrs[i].Disc.size();
				for(int k=0;k<ValNum;k++)
					Estims[i][j].DiscEst.AttrCount[k]/=Estims[i][j].DiscEst.Count;
			}
			break;
		case ATT_CONTINUOUS:
		case ATT_DATETIME:
			for(int j=0;j<OrgInfo.ClassNum;j++)
			{
				double OldMean=Estims[i][j].ContEst.OldMean;
				double OldCount=Estims[i][j].ContEst.OldCount;
				double OldDevia=Estims[i][j].ContEst.StDevia;
				//mean
				double Mean=(Estims[i][j].ContEst.Mean/=Estims[i][j].ContEst.Count);
				//standard deviation: from old to new one
				//sum of square of old values minus new mean
				double Sum=OldDevia*OldDevia*(OldCount-1);
				Sum+=(OldCount*(Mean-OldMean)*(Mean-OldMean));

				//process new data
				int ValNum=(int)Estims[i][j].ContEst.Vals.size();
				for(int k=0;k<ValNum;k++)
				{
					double Diff=Estims[i][j].ContEst.Vals[k]-Mean;
					Sum+=(Diff*Diff);
				}
				Estims[i][j].ContEst.StDevia=sqrt(Sum/(Estims[i][j].ContEst.Count-1));
				//we don't need it anymore
				Estims[i][j].ContEst.Vals.clear();
			}
			break;
		default:
			break;
		}//switch
	}//for attr

	//time consumed
	CreatingTime+=((double)(clock()-start)/CLOCKS_PER_SEC);
}

CClassifier *CGaussNaiveBayes::Clone() const
{
	CGaussNaiveBayes *Cls=new CGaussNaiveBayes(*this);

	return Cls;
}

