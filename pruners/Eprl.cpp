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
#include <time.h>
#include <math.h>
#include <algorithm>
#include "result.h"
#include "Classifier.h"
#include "EnsPruner.h"
using namespace std;


//两个向量相加, 和赋予第一个
void operator +=(vector<double> &a,const vector<double> &b)
{
	for(int i=0;i<(int)a.size();i++)
		a[i]+=b[i];
}

//向量各元素都赋值于一个标量值
void operator <<(vector<double> &a,const double &b)
{
	for(int i=0;i<(int)a.size();i++)
		a[i]=b;
}

//两个向量相加, 返回和
vector<double> operator +(const vector<double> &a,const vector<double> &b)
{
	vector<double> c;

	for(int i=0;i<(int)a.size();i++)
		c.push_back(a[i]+b[i]);

	return c;
}

//向量各元素都乘上一个标量值
vector<double> operator *(const vector<double> &a,const double &b)
{
	vector<double> c;

	for(int i=0;i<(int)a.size();i++)
		c.push_back(a[i]*b);

	return c;
}

//向量笛卡儿乘积
double operator *(const vector<double> &a,const vector<double> &b)
{
	double q=0;
	for(int i=0;i<(int)a.size();i++)
		q+=a[i]*b[i];
	return q;
}

//二值求最大, Pos＝0或1, Pos表示元素的位置, MaxVal即为最大值
int MaxAct(const vector<double> &a,vector<double> b,int Pos,double &MaxVal)
{
	b[Pos]=0;
	double q0=a*b;
	b[Pos]=1;
	double q1=a*b;

	if(q0-q1>=0)
	{
		MaxVal=q0;
		return 0;
	}

	MaxVal=q1;
	return 1;
}

//MaxVal即为最大值
double MaxAct(const vector<double> &a)
{
	int i;
	vector<double> b(a.size(),1);

	for(i=0;i<(int)a.size();i++)
		if(a[i]<0)
			b[i]=0;

	return a*b;
}

int Eprl(const char *FileName,const MATRIX &DataSet,CASE_INFO &CaseInfo,MATRIX &TestSet,MATRIX &ValSet,
		 vector<PRED_RESULT> &PredResults,vector<CClassifier*> Clses,BpnnStatStr *Stat)
{
	int i,j,k;
	int CaseNum=PredResults[0].CaseNum;
	int ClassNum=PredResults[0].ClassNum;
	int ClsNum=(int)PredResults.size();
	clock_t start, finish;
	double  duration;
	start = clock();


	double Lampda=0.9,Epsilon=0.6,Alpha=0.1,Gamma=1,MaxVal;
	//s,a的属性（即为状态）: 每一个元素代表相应的分类器是否选择, 最后一个代表当前分类器
	vector<double> Phi(ClsNum,0);
	//trace
	vector<double> Elig(ClsNum,0);
	//函数参数组初始值任意
	vector<double> Theta;
	for(i=0;i<ClsNum;i++)
		Theta.push_back((double)rand()/RAND_MAX-0.5);
	//回报值（精度）
	double Reward,OldR;
	bool QuitFlag=false;
	TribeStr Tribe;
	//1－选择当前分类器, 0－不选择
	int Action;


	for(k=0;k<1000;k++)
	{
		//向量各元素清0
		Phi<<0;
		Elig<<0;
		//函数参数值增量, 动作的回报（只最后一个动作完成后才有回报）
		double Delta;

		//利用场景进行学习, 场景的每一步
		//在第i次循环时选择i＋1的动作, 但是已经利用i＋1次的动作, 修改了函数参数, 因此循环次数如下: 
		for(j=0,i=rand()%ClsNum;j<ClsNum;j++,i=(i+1)%ClsNum)
		{
			//e-greedy
			if((double)rand()/RAND_MAX>Epsilon)
			{
				//根据最大函数值, 选择动作
				Action=MaxAct(Theta,Phi,i,MaxVal);
				Elig=Elig*Gamma*Lampda;
			}
			else
			{
				if((double)rand()/RAND_MAX<0.5)
					Action=0;
				else
					Action=1;
				Elig<<0;
			}
			//新状态
			Phi[i]=Action;
		}
		//对action进行追踪
		Elig+=Phi;
		//trace归一化
		double SumElig=0;
		for(j=0;j<(int)Elig.size();j++)
			SumElig+=Elig[j];
		for(j=0;j<(int)Elig.size();j++)
			Elig[j]/=SumElig;

		//计算回报值: 最后一个选择完成前, 回报值都是0
		Tribe.Weight.clear();
		for(i=0;i<ClsNum;i++)
			Tribe.Weight.push_back(Phi[i]);
		//计算加入后的分类精度
		CalcFitness(FileName,Tribe,ValSet,PredResults,CaseInfo);
		Reward=Tribe.Pt;
		//根据最大函数值, 选择动作
		MaxVal=MaxAct(Theta);
		//回报－老的函数值（参数 X 属性）＋新参数组的
		Delta=Reward-Theta*Phi;
		//新旧函数值的差值
		Delta=Delta+Gamma*MaxVal;
		//更新函数参数
		Theta=Theta+Elig*(Alpha*Delta);
		//出口条件
		//		if(abs(Alpha*Delta)<0.0001)break;
		/*		for(i=0;i<(int)Elig.size();i++)
		if(Elig[i]*Alpha*Delta>0.0001)
		break;
		if(i>=(int)Elig.size())
		QuitFlag=true;
		*/		//逐步减小
		Epsilon*=(1-0.000001);
	}
	//	while(true);

	//最后选择
	Phi<<0;
	for(i=0;i<ClsNum;i++)
	{
		Action=MaxAct(Theta,Phi,i,MaxVal);
		Phi[i]=Action;
	}


	char TempStr[1024];
	sprintf(TempStr,"\nEprl: 文件%s, 选择分类器: \n",FileName);
	ResultLog(TempStr);
	int SelNum=0;
	for(i=0;i<(int)Phi.size();i++)
		if(Phi[i]>0)
		{
			sprintf(TempStr,"%d,",i);
			ResultLog(TempStr);
			SelNum++;
		}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sprintf(TempStr,"\nEprl: 文件%s, 分类器数目%d, 学习完精度%.4f, 选择用时%.4f\n",
		FileName,SelNum,Reward,duration);
	ResultLog(TempStr);
	Stat->ClsNum+=SelNum;
	Stat->TotalTime+=duration;
	Stat->TotalTmpAccr+=Reward;

	start = clock();
	CalcFitness(FileName,Tribe,TestSet,CaseInfo,Clses);
	finish = clock();
	sprintf(TempStr,"Eprl: 文件%s, 分类器数目%d, 精度%.4f, 预测用时%.4f\n\n",
		FileName,SelNum,Tribe.Pt,duration);
	ResultLog(TempStr);

	//统计
	Stat->TotalPredTime+=duration;
	Stat->TotalAccr+=Tribe.Pt;
	Stat->Accrs.push_back(Tribe.Pt);

	return 0;
}

