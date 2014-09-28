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


#include <cmath>
#include <string>
#include <set>
#include <algorithm>
#include <fstream>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "Ensemble.h"
#include "Prediction.h"
#include "EnsemblePruner.h"
#include "PMEP.h"
using namespace libedm;

const char MyName[MAX_OBJECT_NAME_LENGTH]="PMEP";

CPMEP::CPMEP(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	if(Ensemble.GetSize()!=(int)Predictions.size())
		throw(CError("CPMEP: wrong size for user-defined weights!",100,0));
	//Info
	int CaseNum=ValidatingSet.GetInfo().Height;
	int EnsembleSize=Ensemble.GetSize();
	//start time for training
	clock_t start=clock();

	//table: instance-classifier-prediction
	vector<CaseClassArrayStr> CaseClassTab;
	BuildCaseClassTab(CaseClassTab,ValidatingSet,Predictions);
//	Dump("c.txt",CaseClassTab);
	//construct FP-tree
	TreeNodeStr Root;
	BuildFPTree(Root,CaseClassTab,EnsembleSize);
	
	vector<SelClassifierStr> SelClassifiers;
	//k: number of classifiers to be selected
	for(int k=1;k<=EnsembleSize/2*2+1;k+=2)
	{
		//path-table: paths with length of k/2+1
		vector<TreePathStr> TreePathTable;
		TreePathStr TreePath;
		BuildPathTab(TreePathTable,Root,TreePath,k/2+1);
//		Dump("cc.txt",TreePathTable);

		//selected classifier (no more than k)
		SelClassifierStr S,TempS;
		S.Count=0;
		//add paths until path-table is empty
		while((int)TreePathTable.size()>0 && (int)S.Set.size()<k)
		{
			//sort all paths by Count value and number of classifiers
			sort(TreePathTable.begin(),TreePathTable.end(),ClassNumOrder);
			stable_sort(TreePathTable.begin(),TreePathTable.end(),CountDescOrder);
//			Dump("TreePathTable.txt",TreePathTable);

			//temporally select all classifier of the first path
			TempS=S;
			TempS.Count+=TreePathTable[0].Count;
			for(int j=0;j<(int)TreePathTable[0].Classifiers.size();j++)
				TempS.Set.insert(TreePathTable[0].Classifiers[j]);

			//total size
			if((int)TempS.Set.size()<=k)
			{
				S=TempS;

				//remove classifiers of selected path from all rows of path-table
				for(int jj=0;jj<(int)TreePathTable[0].Classifiers.size();jj++)
				{
					for(int i=1;i<(int)TreePathTable.size();i++)
						for(int j=0;j<(int)TreePathTable[i].Classifiers.size();j++)
							if(TreePathTable[i].Classifiers[j]==TreePathTable[0].Classifiers[jj])
							{
								TreePathTable[i].Classifiers.erase(TreePathTable[i].Classifiers.begin()+j);
								break;
							}
				}
//				Dump("TreePathTable.txt",TreePathTable);

				//remove empty row from path-table
				for(int i=1;i<(int)TreePathTable.size();)
				{
					if(TreePathTable[i].Classifiers.size()<=0)
					{
						//the Count value of the path being removed is added
						TempS.Count+=TreePathTable[i].Count;

						TreePathTable.erase(TreePathTable.begin()+i);
						continue;
					}
					i++;
				}
				//this path is finished
				TreePathTable.erase(TreePathTable.begin());
//				Dump("TreePathTable.txt",TreePathTable);

				//merge same paths
				for(int i=0;i<(int)TreePathTable.size();i++)
				{
					set<int> A0;
					for(int jj=0;jj<(int)TreePathTable[i].Classifiers.size();jj++)
						A0.insert(TreePathTable[i].Classifiers[jj]);

					for(int j=i+1;j<(int)TreePathTable.size();)
					{
						set<int> A1;
						for(int jj=0;jj<(int)TreePathTable[j].Classifiers.size();jj++)
							A1.insert(TreePathTable[j].Classifiers[jj]);
						if(A0==A1)
						{
							TreePathTable[i].Count+=TreePathTable[j].Count;
							TreePathTable.erase(TreePathTable.begin()+j);
							continue;
						}
						j++;
					}
				}//for i
//				Dump("TreePathTable.txt",TreePathTable);
			}
			else//adding this path will make the size of selected classifiers greater than k, so skip it
				TreePathTable.erase(TreePathTable.begin());
		}//while
		SelClassifiers.push_back(S);

	}//for k
//	deltree(&&Root);

	//sort all sets by Count and size
	sort(SelClassifiers.begin(),SelClassifiers.end(),SelClassSetSizeOrder);
	stable_sort(SelClassifiers.begin(),SelClassifiers.end(),SelClassCountDescOrder);

	//set the weight of selected classifiers
	for(int i=0;i<EnsembleSize;i++)
		Weights.push_back(0);
	set<int>::iterator	i_Classifier;
	for(i_Classifier=SelClassifiers[0].Set.begin();i_Classifier!=SelClassifiers[0].Set.end();i_Classifier++)
	{
		for(int i=0;i<EnsembleSize;i++)
			if(*i_Classifier==i)
				Weights[i]=1.0;
	}

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

void CPMEP::Dump(const string &FileName,const vector<CaseClassArrayStr> &CaseClassTab)
{
	ofstream OutFile;
	OutFile.open(FileName.c_str());
	OutFile<<"Classifiers: "<<CaseClassTab[0].size()<<endl<<"Instances: "<<CaseClassTab.size()<<endl<<endl;

	for(int i=0;i<(int)CaseClassTab.size();i++)
	{
		for(int j=0;j<(int)CaseClassTab[i].size();j++)
			OutFile<<CaseClassTab[i][j].Correct<<","<<CaseClassTab[i][j].Classifier<<"\t\t";
		OutFile<<endl;
	}

	OutFile.close();
}

//CaseClassTab: 
int CPMEP::BuildCaseClassTab(vector<CaseClassArrayStr> &CaseClassTab,const CDataset &ValidatingSet,
	const vector<CPrediction*> &Predictions)
{
	//Info
	int CaseNum=ValidatingSet.GetInfo().Height;
	int EnsembleSize=Ensemble.GetSize();
	if(Predictions[0]->GetCaseNum()!=CaseNum)
	{
		printf("DataSet->height!=BpnnResult->CaseNum");
		return 1;
	}

	//construct and initialize the table
	CaseClassTab.clear();
	{
		//each instance a row
		//row
		vector<CaseClassRecStr> CaseClassArray;
		//item: each classifier a column
		CaseClassRecStr CaseClassRec;
		CaseClassRec.Correct=0;
//		CaseClassRec.NodeLink=NULL;
		//total column: classifier number +1
		//last column: number of classifiers that predict this instance correctly
		for(int k=0;k<=EnsembleSize;k++)
		{
			CaseClassRec.Classifier=k;
			CaseClassArray.push_back(CaseClassRec);
		}

		//total row=CaseNum+1
		//Last row: number of instances predicted correctly by this classifier and id of it
		for(int j=0;j<=CaseNum;j++)
			CaseClassTab.push_back(CaseClassArray);
	}

	//fill it
	for(int i=0;i<EnsembleSize;i++)
	{
		//
		for(int j=0;j<CaseNum;j++)
		{
			//is this prediction correct?
			if(Predictions[i]->GetCorrectness()[j])
			{
				if(CaseClassTab[j][i].Correct!=0)
				{
					printf("CaseClassTab[j][i].Correct!=0");
					return 2;
				}
				CaseClassTab[j][i].Correct++;
				//last column: number of classifiers that predict this instance correctly
				CaseClassTab[j][EnsembleSize].Correct++;
				//last row: number of instances correctly predicted by this classifier
				CaseClassTab[CaseNum][i].Correct++;
			}
		}
	}

	//sort the columns of the last row by descent order of corresponding classifiers' prediction accuracy
	sort(CaseClassTab[CaseNum].begin(),CaseClassTab[CaseNum].end(),CorrectDescOrder);
//	Dump("a.txt",CaseClassTab);
	//sort columns of other rows as the order of the last row
	for(int i=0;i<EnsembleSize;i++)
	{
		//are the left classifiers incorrectly predict all instances?
		if(CaseClassTab[CaseNum][i].Correct==0)break;
		//find each column's new position
		int k;
		for(k=i;k<EnsembleSize;k++)
			if(CaseClassTab[0][k].Classifier==CaseClassTab[CaseNum][i].Classifier)
				break;
		//don't need to change position?
		if(k==i)
			continue;

		//switch to new position(k -> i)
		CaseClassRecStr TempCaseClassRec;
		for(int j=0;j<CaseNum;j++)
		{
			TempCaseClassRec=CaseClassTab[j][i];
			CaseClassTab[j][i]=CaseClassTab[j][k];
			CaseClassTab[j][k]=TempCaseClassRec;
		}
	}
//	Dump("a.txt",CaseClassTab);

	return 0;
}

/*
//对正确分类器数目>=k的所有样本: 所有正确分类器形成新集合, 再对新集合所有样本包含的分类器排序
//k－对样本分类正确的最少分类器数目
int CPMEP::SelectCaseClassTab(vector<CaseClassArrayStr> &SelCaseClassTab,
					   const vector<CaseClassArrayStr> &CaseClassTab,const int k)
{
	//去掉最后一行（统计行）
	int OrgCaseNum=CaseClassTab.size()-1;
	int SelCaseNum;

	//选择
	SelCaseClassTab.clear();
	for(int i=0;i<OrgCaseNum;i++)
		if(k>0 && CaseClassTab[i][BPNNPERDATA].Correct>=k)
		{
			SelCaseClassTab.push_back(CaseClassTab[i]);
// 			//最后一列是按样本的正确统计
// 			SelCaseClassTab[SelCaseClassTab.size()-1][BPNNPERDATA].Correct=k;
		}
	SelCaseNum=SelCaseClassTab.size();
	//最后一行是按分类器的正确统计
	SelCaseClassTab.push_back(CaseClassTab[OrgCaseNum]);
	for(int i=0;i<=BPNNPERDATA;i++)
		SelCaseClassTab[SelCaseNum][i].Correct=0;

	//计算最后一行: i(行)=样本, j(列)=分类器
	for(int i=0;i<SelCaseNum;i++)
	{
//		l=0;
		for(int j=0;j<BPNNPERDATA;j++)
			//统计所有正确分类器
			if(SelCaseClassTab[i][j].Correct>0)
			{
				SelCaseClassTab[SelCaseNum][j].Correct++;
//				l++;
// 				if(l>=k)break;
			}
	}

	//需要按CaseClassTab最后一行correct字段值从大到小对整个CaseClassTab表排序
	sort(SelCaseClassTab[SelCaseNum].begin(),SelCaseClassTab[SelCaseNum].end(),CorrectDescOrder);
	//	Dump("a.txt",CaseClassTab);
	for(int i=0;i<BPNNPERDATA;i++)
	{
		//剩余的分类器对所有的样本分类错误（退出）
		if(SelCaseClassTab[SelCaseNum][i].Correct==0)break;
		//排序后分类器的顺序是否发生变化
		for(int l=i;l<BPNNPERDATA;l++)
			if(SelCaseClassTab[0][l].Classifier==SelCaseClassTab[SelCaseNum][i].Classifier)
				break;
		//现在位置小于最终位置, 不必交换
		if(l==i)
			continue;

		//样本内的分类器排序（第l个放在第i个）
		//现在位置在最终位置之后（只向前交换）
		CaseClassRecStr TempCaseClassRec;
		for(int j=0;j<SelCaseNum;j++)
		{
			TempCaseClassRec=SelCaseClassTab[j][i];
			SelCaseClassTab[j][i]=SelCaseClassTab[j][l];
			SelCaseClassTab[j][l]=TempCaseClassRec;
		}
	}

	//	Dump("a.txt",CaseClassTab);
	//	Dump("a.txt",SelCaseClassTab);
	return 0;
}
*/

//Root- build a FP-tree
//only use the first k classifier in each row of Case-classifier table
int CPMEP::BuildFPTree(TreeNodeStr &Root,const vector<CaseClassArrayStr> &CaseClassTab,int k)
{
	//last row of Case-classifier table is the statistics of prediction
	CaseClassArrayStr HTable;
	int CaseNum=(int)CaseClassTab.size()-1;
	int EnsembleSize=(int)CaseClassTab[0].size()-1;
	HTable=CaseClassTab[CaseNum];

	//root
	Root.Classifier=-1;
	Root.Count=0;
//	Root.NodeLink=NULL;
//	Root.Parent=NULL;
	//building
	for(int i=0;i<CaseNum;i++)
	{
		TreeNodeStr *Node=&Root;
		int m=0;
		//put instance into the tree
		for(int j=0;j<EnsembleSize;j++)
			//is j-th classifier predict i-th instance correctly
			if(CaseClassTab[i][j].Correct>0)
			{
				//find a sub-node whose classifier is j-th classifier in Case-classifier table
				bool FoundMatch=false;
				for(int l=0;l<(int)Node->SubNodes.size();l++)
					if(CaseClassTab[i][j].Classifier==Node->SubNodes[l].Classifier)
					{
						//goto deeper
						Node=&(Node->SubNodes[l]);
						FoundMatch=true;
						Node->Count++;
						break;
					}
				//not found, create one
				if(!FoundMatch)
				{
					TreeNodeStr SubNode;
					SubNode.Count=1;
					//j-th classifier in Case-classifier table
					SubNode.Classifier=CaseClassTab[i][j].Classifier;
// 					SubNode.Parent=Node;
// 					SubNode.NodeLink=NULL;
					//new sub-node
					Node->SubNodes.push_back(SubNode);
// 					//head table starts a link to each classifier
// 					if(HTable[j].NodeLink==NULL)
// 						HTable[j].NodeLink=&(Node->SubNodes[SubSize-1]);
// 					else
// 					{
// 						TreeNodeStr *TempNode=HTable[j].NodeLink;
// 						for(;TempNode->NodeLink!=NULL;TempNode=TempNode->NodeLink);
// 						TempNode->NodeLink=&(Node->SubNodes[SubSize-1]);
// 					}

					//goto deeper
					if(Node->SubNodes.size()>1)
						printf("");
					Node=&(Node->SubNodes.back());
				}

				if(++m>=k)
					break;
			}//if>0, for j
	}//for i

	return 0;
}

// remove each node (deep first)
// int CPMEP::deltree(TreeNodeStr **Root)
// {
// 	TreeNodeStr *Node=*Root,*TempNode;
// 
// 	while((*Root)->SubNodes.size()>0)
// 	{
// 		//no sub-node
// 		if(Node->SubNodes.size()==0)
// 		{
// 			//remove this node
// 			TempNode=Node->Parent;
// 			delete Node;
// 			Node=TempNode;
// 			//remove 1st sub-node
// 			Node->SubNodes.erase(Node->SubNodes.begin());
// 		}
// 		else//goto deeper
// 			Node=Node.SubNodes[0];
// 	}
// 
// 	delete (*Root);
// 	*Root=NULL;
// 	return 0;
// }
// 
void CPMEP::Dump(const string &FileName,const vector<TreePathStr> &TreePathTable)
{
	ofstream OutFile;
	OutFile.open(FileName.c_str());
	OutFile<<"TreePathNum="<<TreePathTable.size()<<endl;

	for(int i=0;i<(int)TreePathTable.size();i++)
	{
		OutFile<<"\tCount: "<<TreePathTable[i].Count<<", Classifier: "<<TreePathTable[i].Classifiers.size()<<"\t";
		for(int j=0;j<(int)TreePathTable[i].Classifiers.size();j++)
			OutFile<<TreePathTable[i].Classifiers[j]<<"\t";
		OutFile<<endl;
	}

	OutFile.close();
}


int CPMEP::BuildPathTab(vector<TreePathStr> &TreePathTable,const TreeNodeStr &Root,TreePathStr &TreePath,int k)
{
	const TreeNodeStr &Node=Root;

	//add this node to path
	if(Node.Classifier>=0)
	{
		TreePath.Classifiers.push_back(Node.Classifier);
		TreePath.Count=Node.Count;
	}

	//path length reach to k, return to parent node
	if((int)TreePath.Classifiers.size()>=k)
	{
		TreePathTable.push_back(TreePath);
		TreePath.Classifiers.pop_back();
		return 0;
	}

	//path still too short, enter the children nodes
	for(int i=0;i<(int)Node.SubNodes.size();i++)
		BuildPathTab(TreePathTable,Node.SubNodes[i],TreePath,k);

	//back to the parent
	if(Node.Classifier>=0)
		TreePath.Classifiers.pop_back();

	return 0;
}

/*
//直接从结果表得到pathtable
int CPMEP::BuildPathTab(vector<TreePathStr> &TreePathTable,const vector<CaseClassArrayStr> &CaseClassTab,int k)
{
	TreePathStr TreePath;
	TreePathTable.clear();

	//所有样本
	for(int i=0;i<(int)CaseClassTab.size()-1;i++)
	{
		TreePath.Classifiers.clear();
		TreePath.Count=1;
		m=0;
		//压缩样本, 取得前k个正确的分类器
		for(int l=0;l<k;l++)
		{
			//在样本中找到第l个正确的
			while(m<BPNNPERDATA && CaseClassTab[i][m].Correct<=0)
				m++;
			if(m>=BPNNPERDATA)
				break;
			TreePath.Classifiers.push_back(CaseClassTab[i][m].Classifier);
			m++;
		}
		if(l<k)continue;

		//查看PathTable中是否已有当前模式
		for(int j=0;j<(int)TreePathTable.size();j++)
		{
			for(int l=0;l<k;l++)
				if(TreePathTable[j].Classifiers[l]!=TreePath.Classifiers[l])
					break;
			//与当前匹配
			if(l>=k)
				break;
		}
		//找到
		if(j<(int)TreePathTable.size())
			TreePathTable[j].Count++;
		else
			TreePathTable.push_back(TreePath);
	}

	return 0;
}*/

void CPMEP::Dump(const string &FileName,const vector<SelClassifierStr> &SelClassifiers)
{
	ofstream OutFile;
	OutFile.open(FileName.c_str());
	
	for(int i=0;i<(int)SelClassifiers.size();i++)
	{
		OutFile<<"Classifiers: "<<SelClassifiers[i].Set.size()<<", Count: "<<SelClassifiers[i].Count<<endl;
		set<int>::const_iterator i_Classifier;
		for(i_Classifier=SelClassifiers[i].Set.begin();i_Classifier!=SelClassifiers[i].Set.end();i_Classifier++)
		{
			OutFile<<*i_Classifier<<",";
		}
		OutFile<<endl;
	}

	OutFile.close();
}

/*
int Kensemble(const char *FileName,const MATRIX &DataSet,CASE_INFO &CaseInfo,MATRIX &TestSet,MATRIX &ValidatingSet,
			  vector<PRED_RESULT> &PredResults,vector<CClassifier*> Clses,BpnnStatStr *Stat)
{
	clock_t start, finish;
	double  duration;
	start = clock();


	BPNNPERDATA=(int)Clses.size();
	//计算分类结果表
	vector<CaseClassArrayStr> CaseClassTab,SelCaseClassTab;
	BuildCaseClassTab(FileName,CaseClassTab,ValidatingSet,PredResults,CaseInfo);

	vector<SelClassifierStr> SelClassifiers;
	//k是选择的分类器总数
	for(int k=1;k<=BPNNPERDATA/2*2+1;k+=2)
	{
		SelectCaseClassTab(FileName,SelCaseClassTab,CaseClassTab,k/2+1);
		//根据k生成path表
		vector<TreePathStr> TreePathTable;
		BuildPathTab(TreePathTable,SelCaseClassTab,k/2+1);
		//Dump("b.txt",TreePathTable);

		//对path_table进行处理
		SelClassifierStr S,TempS;
		S.Count=0;
		while((int)TreePathTable.size()>0 && (int)S.Set.size()<k)
		{
			//按分类成功次数从大到小排序, 次数相同, 按分类器数目从少到多排序（越少越好）
			sort(TreePathTable.begin(),TreePathTable.end(),ClassNumOrder);
			stable_sort(TreePathTable.begin(),TreePathTable.end(),CountDescOrder);
			//			Dump("TreePathTable.txt",TreePathTable);
			//添加路径中的分类器
			TempS=S;
			TempS.Count+=TreePathTable[0].Count;
			for(int j=0;j<(int)TreePathTable[0].Classifiers.size();j++)
				TempS.Set.insert(TreePathTable[0].Classifiers[j]);

			//是否路径长度满足要求, 如不满足TempS的内容被丢弃
			if((int)TempS.Set.size()<=k)
			{
				S=TempS;
				//在path table中去掉已添加的分类器
				for(int i=1;i<(int)TreePathTable.size();i++)
					for(int jj=0;jj<(int)TreePathTable[0].Classifiers.size();jj++)
						for(int j=0;j<(int)TreePathTable[i].Classifiers.size();j++)
							if(TreePathTable[i].Classifiers[j]==TreePathTable[0].Classifiers[jj])
							{
								TreePathTable[i].Classifiers.erase(TreePathTable[i].Classifiers.begin()+j);
								break;
							}
				//				Dump("TreePathTable.txt",TreePathTable);
				//(除了新加入S的行, 内容是新加入行子集的行也变成空行）去掉空行
				for(int i=1;i<(int)TreePathTable.size();)
				{
					if(TreePathTable[i].Classifiers.size()<=0)
					{
						//将空行的count值计入
						TempS.Count+=TreePathTable[i].Count;

						TreePathTable.erase(TreePathTable.begin()+i);
						continue;
					}
					i++;
				}
				//无论如何, 删除本路径
				TreePathTable.erase(TreePathTable.begin());
				//				Dump("TreePathTable.txt",TreePathTable);

				//合并相同行
				for(int i=0;i<(int)TreePathTable.size();i++)
				{
					set<int> A0;
					for(int jj=0;jj<(int)TreePathTable[i].Classifiers.size();jj++)
						A0.insert(TreePathTable[i].Classifiers[jj]);

					for(int j=i+1;j<(int)TreePathTable.size();)
					{
						set<int> A1;
						for(int jj=0;jj<(int)TreePathTable[j].Classifiers.size();jj++)
							A1.insert(TreePathTable[j].Classifiers[jj]);
						if(A0==A1)
						{
							TreePathTable[i].Count+=TreePathTable[j].Count;
							TreePathTable.erase(TreePathTable.begin()+j);
							continue;
						}
						j++;
					}
				}//for i
				//				Dump("TreePathTable.txt",TreePathTable);
			}
			else//size大于k, 则不可能在使用少于k个分类器的情况下, 对本路径进行正确分类
				TreePathTable.erase(TreePathTable.begin());
		}//while
		SelClassifiers.push_back(S);
	}//for k

	//Count从大到小排序, 相同则分类器数目从小到大排序
	sort(SelClassifiers.begin(),SelClassifiers.end(),SelClassSetSizeOrder);
	stable_sort(SelClassifiers.begin(),SelClassifiers.end(),SelClassCountDescOrder);
	finish = clock();

	//记录选择分类器, 准备计算测试集
	TribeStr Tribe;
	for(int i=0;i<BPNNPERDATA;i++)
		Tribe.Weights.push_back(0);
	int SelClassNum=(int)SelClassifiers[0].Set.size();
	//列出并记录分类器
	char TempStr[1024];
	sprintf(TempStr,"k-ensemble: 文件%s, 选择分类器: \n",FileName);
	ResultLog(TempStr);
	set<int>::iterator	i_Classifier;
	for(i_Classifier=SelClassifiers[0].Set.begin();i_Classifier!=SelClassifiers[0].Set.end();i_Classifier++)
	{
		sprintf(TempStr,"%d,",*i_Classifier);
		ResultLog(TempStr);

		for(int i=0;i<BPNNPERDATA;i++)
			if(*i_Classifier==i)
				Tribe.Weights[i]=1;
	}
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	Tribe.Pt=0;
#ifdef CALC_TMP_ACCR
	VotePredict(FileName,Tribe,ValidatingSet,PredResults,CaseInfo);
#endif
	Stat->TotalTmpAccr+=Tribe.Pt;
	sprintf(TempStr,"\nk-ensemble: 文件%s, 分类器数目%d, 学习完精度%.4f, 选择用时%.4f\n",
		FileName,SelClassifiers[0].Set.size(),Tribe.Pt,duration);
	ResultLog(TempStr);
	//统计
	Stat->EnsembleSize+=SelClassifiers[0].Set.size();
	Stat->TotalTime+=duration;

	//测试集精度
	start = clock();
	VotePredict(FileName,Tribe,TestSet,CaseInfo,Clses);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sprintf(TempStr,"k-ensemble: 文件%s, 分类器数目%d, 精度%.4f, 预测用时%.4f\n\n",
		FileName,SelClassNum,Tribe.Pt,duration);
	ResultLog(TempStr);
	//统计
	Stat->TotalPredTime+=duration;
	Stat->TotalAccr+=Tribe.Pt;
	Stat->Accrs.push_back(Tribe.Pt);

	return 0;
}


int CPM_EP(const char *FileName,const MATRIX &DataSet,CASE_INFO &CaseInfo,MATRIX &TestSet,MATRIX &ValidatingSet,
		   vector<PRED_RESULT> &PredResults,vector<CClassifier*> Clses,BpnnStatStr *Stat)
{
	clock_t start, finish;
	double  duration;
	start = clock();


	BPNNPERDATA=(int)Clses.size();
	//计算分类结果表
	vector<CaseClassArrayStr> CaseClassTab,SelCaseClassTab;
	BuildCaseClassTab(FileName,CaseClassTab,ValidatingSet,PredResults,CaseInfo);
	TreeNodeStr *Root;

	vector<SelClassifierStr> SelClassifiers;
	//k是选择的分类器总数
	for(int k=1;k<=BPNNPERDATA/2*2+1;k+=2)
	{
		SelectCaseClassTab(FileName,SelCaseClassTab,CaseClassTab,k/2+1);
		//根据k值构造fp-tree
		BuildFPTree(FileName,&Root,SelCaseClassTab,k/2+1);

		//根据k生成path表
		vector<TreePathStr> TreePathTable;
		TreePathStr TreePath;
		BuildPathTab(TreePathTable,Root,TreePath,k/2+1);
		//Dump("b.txt",TreePathTable);

		//对path_table进行处理
		SelClassifierStr S,TempS;
		S.Count=0;
		while((int)TreePathTable.size()>0 && (int)S.Set.size()<k)
		{
			//按分类成功次数从大到小排序, 次数相同, 按分类器数目从少到多排序（越少越好）
			sort(TreePathTable.begin(),TreePathTable.end(),ClassNumOrder);
			stable_sort(TreePathTable.begin(),TreePathTable.end(),CountDescOrder);
			//			Dump("TreePathTable.txt",TreePathTable);
			//添加路径中的分类器
			TempS=S;
			TempS.Count+=TreePathTable[0].Count;
			for(int j=0;j<(int)TreePathTable[0].Classifiers.size();j++)
				TempS.Set.insert(TreePathTable[0].Classifiers[j]);

			//是否路径长度满足要求, 如不满足TempS的内容被丢弃
			if((int)TempS.Set.size()<=k)
			{
				S=TempS;
				//在path table中去掉已添加的分类器
				for(int i=1;i<(int)TreePathTable.size();i++)
					for(int jj=0;jj<(int)TreePathTable[0].Classifiers.size();jj++)
						for(int j=0;j<(int)TreePathTable[i].Classifiers.size();j++)
							if(TreePathTable[i].Classifiers[j]==TreePathTable[0].Classifiers[jj])
							{
								TreePathTable[i].Classifiers.erase(TreePathTable[i].Classifiers.begin()+j);
								break;
							}
				//				Dump("TreePathTable.txt",TreePathTable);
				//(除了新加入S的行, 内容是新加入行子集的行也变成空行）去掉空行
				for(int i=1;i<(int)TreePathTable.size();)
				{
					if(TreePathTable[i].Classifiers.size()<=0)
					{
						//将空行的count值计入
						TempS.Count+=TreePathTable[i].Count;

						TreePathTable.erase(TreePathTable.begin()+i);
						continue;
					}
					i++;
				}
				//无论如何, 删除本路径
				TreePathTable.erase(TreePathTable.begin());
				//				Dump("TreePathTable.txt",TreePathTable);

				//合并相同行
				for(int i=0;i<(int)TreePathTable.size();i++)
				{
					set<int> A0;
					for(int jj=0;jj<(int)TreePathTable[i].Classifiers.size();jj++)
						A0.insert(TreePathTable[i].Classifiers[jj]);

					for(int j=i+1;j<(int)TreePathTable.size();)
					{
						set<int> A1;
						for(int jj=0;jj<(int)TreePathTable[j].Classifiers.size();jj++)
							A1.insert(TreePathTable[j].Classifiers[jj]);
						if(A0==A1)
						{
							TreePathTable[i].Count+=TreePathTable[j].Count;
							TreePathTable.erase(TreePathTable.begin()+j);
							continue;
						}
						j++;
					}
				}//for i
				//				Dump("TreePathTable.txt",TreePathTable);
			}
			else//size大于k, 则不可能在使用少于k个分类器的情况下, 对本路径进行正确分类
				TreePathTable.erase(TreePathTable.begin());
		}//while
		SelClassifiers.push_back(S);

		deltree(&Root);
	}//for k

	//Count从大到小排序, 相同则分类器数目从小到大排序
	sort(SelClassifiers.begin(),SelClassifiers.end(),SelClassSetSizeOrder);
	stable_sort(SelClassifiers.begin(),SelClassifiers.end(),SelClassCountDescOrder);
	finish = clock();

	//记录选择分类器, 准备计算测试集
	TribeStr Tribe;
	for(int i=0;i<BPNNPERDATA;i++)
		Tribe.Weights.push_back(0);
	int SelClassNum=(int)SelClassifiers[0].Set.size();
	//列出并记录分类器
	char TempStr[1024];
	sprintf(TempStr,"CPM-EP: 文件%s, 选择分类器: \n",FileName);
	ResultLog(TempStr);
	set<int>::iterator	i_Classifier;
	for(i_Classifier=SelClassifiers[0].Set.begin();i_Classifier!=SelClassifiers[0].Set.end();i_Classifier++)
	{
		sprintf(TempStr,"%d,",*i_Classifier);
		ResultLog(TempStr);

		for(int i=0;i<BPNNPERDATA;i++)
			if(*i_Classifier==i)
				Tribe.Weights[i]=1;
	}
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	Tribe.Pt=0;
#ifdef CALC_TMP_ACCR
	VotePredict(FileName,Tribe,ValidatingSet,PredResults,CaseInfo);
#endif
	Stat->TotalTmpAccr+=Tribe.Pt;
	sprintf(TempStr,"\nCPM-EP: 文件%s, 分类器数目%d, 学习完精度%.4f, 选择用时%.4f\n",
		FileName,SelClassifiers[0].Set.size(),Tribe.Pt,duration);
	ResultLog(TempStr);
	//统计
	Stat->EnsembleSize+=SelClassifiers[0].Set.size();
	Stat->TotalTime+=duration;

	//测试集精度
	start = clock();
	VotePredict(FileName,Tribe,TestSet,CaseInfo,Clses);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sprintf(TempStr,"CPM-EP: 文件%s, 分类器数目%d, 精度%.4f, 预测用时%.4f\n\n",
		FileName,SelClassNum,Tribe.Pt,duration);
	ResultLog(TempStr);
	//统计
	Stat->TotalPredTime+=duration;
	Stat->TotalAccr+=Tribe.Pt;
	Stat->Accrs.push_back(Tribe.Pt);

	return 0;
}
*/

