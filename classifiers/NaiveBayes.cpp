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
#include "NaiveBayes.h"
using namespace libep;


//number of division for a continuous attribute's value range
const char	MyName[MAX_OBJECT_NAME_LENGTH]="NAIVEBAYES";
const string CNaiveBayes::StaticName=MyName;

CNaiveBayes::~CNaiveBayes()
{
}

void CNaiveBayes::Reset()
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
				for(int j=0;j<SplitNum;j++)
					Estims[i][k].ContEst.Vals[j]=1.0;
			}
		}
}

CNaiveBayes::CNaiveBayes(const CDataset &TrainSet,int USplitNum)
{
	Name=MyName;
	SplitNum=USplitNum;
	//start time for training
	clock_t start=clock();

	//data
	const MATRIX &OrgData=TrainSet.GetData();
	const CASE_INFO &OrgInfo=TrainSet.GetInfo();

	//initialize all data structure
	for(int i=0;i<OrgInfo.ValidWidth-1;i++)
	{
		//each attribute
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
			//Laplace estimator
			Estim.ContEst.Count=SplitNum;
			Estim.ContEst.Max=OrgInfo.ValidAttrs[i].Max;
			Estim.ContEst.Min=OrgInfo.ValidAttrs[i].Min;
			for(int j=0;j<SplitNum;j++)
				Estim.ContEst.Vals.push_back(1);
		}

		//for each attribute: all class label
		vector<EstimatorStr> EstiAttr;
		for(int j=0;j<OrgInfo.ClassNum;j++)
			EstiAttr.push_back(Estim);
		//all attributes
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
						int ValNo;

						if(OrgInfo.ValidAttrs[j].Max==OrgInfo.ValidAttrs[j].Min)
							ValNo=0;
						else
							ValNo=(int)((OrgData[i][j].Cont-OrgInfo.ValidAttrs[j].Min)*10/
								(OrgInfo.ValidAttrs[j].Max-OrgInfo.ValidAttrs[j].Min));
						if(ValNo>=SplitNum)
							ValNo=SplitNum-1;
						if(ValNo<0)
							ValNo=0;
						Estims[j][Class].ContEst.Vals[ValNo]++;
						Estims[j][Class].ContEst.Count++;
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
					for(int k=0;k<SplitNum;k++)
						Estims[i][j].ContEst.Vals[k]/=Estims[i][j].ContEst.Count;
				}
				break;
			default:
				break;
		}//switch
	}//for attr

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

bool CNaiveBayes::Dump(const string &FileName) const
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
					OutFile<<": Type=continuous, SplitNum="<<SplitNum<<endl;
				for(int k=0;k<SplitNum;k++)
				{
					double cof=Estims[i][j].ContEst.Vals[k];
					OutFile<<"\t"<<cof;
				}
				OutFile<<endl;
				break;
			default:
				break;
			}//switch
		}
	}//for attr

	OutFile.close();
	return true;
}

CPrediction *CNaiveBayes::Classify(const CDataset &DataSet) const
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
							int ValNo;

							if(OrgInfo.ValidAttrs[j].Max==OrgInfo.ValidAttrs[j].Min)
								ValNo=0;
							else
								ValNo=(int)((Val-Estims[j][k].ContEst.Min)*10/
									(Estims[j][k].ContEst.Max-Estims[j][k].ContEst.Min));
							if(ValNo>=SplitNum)
								ValNo=SplitNum-1;
							if(ValNo<0)
								ValNo=0;
							Prob*=Estims[j][k].ContEst.Vals[ValNo];

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

int CNaiveBayes::Save(const string &Path,const string &FileName) const
{
	//write in file
	ofstream OutFile;
	OutFile.open((Path+FileName+"."+Name).c_str(),ios_base::trunc|ios_base::out|ios_base::binary);
	if(OutFile.fail())
	{
		return 1;
	}
	OutFile.write((char*)&SplitNum,sizeof(SplitNum));

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
					//redundant
					int ValNum=(int)Estims[i][j].DiscEst.AttrCount.size();
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
				{
					//redundant
					double Max=Estims[i][j].ContEst.Max;
					OutFile.write((char*)&Max,sizeof(Max));
					double Min=Estims[i][j].ContEst.Min;
					OutFile.write((char*)&Min,sizeof(Min));

					for(int k=0;k<SplitNum;k++)
					{
						double cof=Estims[i][j].ContEst.Vals[k];
						OutFile.write((char*)&cof,sizeof(cof));
					}
				}
				break;
			default:
				break;
			}//switch
		}
	}//for attributes

	OutFile.close();
	return 0;
}

CNaiveBayes::CNaiveBayes(const string &Path,const string &FileName)
{
	Name=MyName;
	//start time for training
	clock_t start=clock();

	//reading from file
	ifstream InFile;
	InFile.open((Path+FileName+"."+Name).c_str(),ios_base::in|ios_base::binary);
	if(InFile.fail())
	{
		throw(CError("NaiveBayes: fail open saved file!",100,0));
	}
	InFile.read((char*)&SplitNum,sizeof(SplitNum));

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
					InFile.read((char*)&Estim.ContEst.Max,sizeof(Estim.ContEst.Max));
					InFile.read((char*)&Estim.ContEst.Min,sizeof(Estim.ContEst.Min));
					for(int k=0;k<SplitNum;k++)
					{
						double cof;
						InFile.read((char*)&cof,sizeof(cof));
						Estim.ContEst.Vals.push_back(cof);
					}
				}
				break;
			default:
				break;
			}//switch
			EstiAttr.push_back(Estim);
		}
		Estims.push_back(EstiAttr);
	}//for attributes

	InFile.close();

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

void CNaiveBayes::Train(const CDataset &TrainSet)
{
	//start time for training
	clock_t start=clock();

	//data
	const MATRIX &OrgData=TrainSet.GetData();
	const CASE_INFO &OrgInfo=TrainSet.GetInfo();

	//if range of a continuous attribute changed (extended), should we re-calculate all existed statistics?
	//we can't, some information has lost. We can only extend the first and the last intervals

	//statistics
	for(int i=0;i<OrgInfo.Height;i++)
	{
		//label of instance
		int Class=OrgData[i][OrgInfo.ValidWidth-1].Discr;
		//each attribute
		for(int j=0;j<OrgInfo.ValidWidth-1;j++)
			switch(OrgInfo.ValidAttrs[j].AttType)
			{
				case ATT_DISCRETE:
					{
						//value of this attribute
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
						int ValNo;

						if(OrgInfo.ValidAttrs[j].Max==OrgInfo.ValidAttrs[j].Min)
							ValNo=0;
						else
							ValNo=(int)((OrgData[i][j].Cont-Estims[j][Class].ContEst.Min)*10/
							(Estims[j][Class].ContEst.Max-Estims[j][Class].ContEst.Min));
						if(ValNo>=SplitNum)
							ValNo=SplitNum-1;
						if(ValNo<0)
							ValNo=0;
						Estims[j][Class].ContEst.Vals[ValNo]++;
						Estims[j][Class].ContEst.Count++;
					}
					break;
				default:
					break;
			}//case: attribute type
	}//for data

	//calculate all other statistics
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
				for(int k=0;k<SplitNum;k++)
					Estims[i][j].ContEst.Vals[k]/=Estims[i][j].ContEst.Count;
			}
			break;
		default:
			break;
		}//switch
	}//for attributes

	//time consumed
	CreatingTime+=((double)(clock() - start) / CLOCKS_PER_SEC);
}

CClassifier *CNaiveBayes::Clone() const
{
	CNaiveBayes *Cls=new CNaiveBayes(*this);

	return Cls;
}

