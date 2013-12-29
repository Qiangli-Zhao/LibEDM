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


//�����������, �͸����һ��
void operator +=(vector<double> &a,const vector<double> &b)
{
	for(int i=0;i<(int)a.size();i++)
		a[i]+=b[i];
}

//������Ԫ�ض���ֵ��һ������ֵ
void operator <<(vector<double> &a,const double &b)
{
	for(int i=0;i<(int)a.size();i++)
		a[i]=b;
}

//�����������, ���غ�
vector<double> operator +(const vector<double> &a,const vector<double> &b)
{
	vector<double> c;

	for(int i=0;i<(int)a.size();i++)
		c.push_back(a[i]+b[i]);

	return c;
}

//������Ԫ�ض�����һ������ֵ
vector<double> operator *(const vector<double> &a,const double &b)
{
	vector<double> c;

	for(int i=0;i<(int)a.size();i++)
		c.push_back(a[i]*b);

	return c;
}

//�����ѿ����˻�
double operator *(const vector<double> &a,const vector<double> &b)
{
	double q=0;
	for(int i=0;i<(int)a.size();i++)
		q+=a[i]*b[i];
	return q;
}

//��ֵ�����, Pos��0��1, Pos��ʾԪ�ص�λ��, MaxVal��Ϊ���ֵ
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

//MaxVal��Ϊ���ֵ
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
	//s,a�����ԣ���Ϊ״̬��: ÿһ��Ԫ�ش�����Ӧ�ķ������Ƿ�ѡ��, ���һ������ǰ������
	vector<double> Phi(ClsNum,0);
	//trace
	vector<double> Elig(ClsNum,0);
	//�����������ʼֵ����
	vector<double> Theta;
	for(i=0;i<ClsNum;i++)
		Theta.push_back((double)rand()/RAND_MAX-0.5);
	//�ر�ֵ�����ȣ�
	double Reward,OldR;
	bool QuitFlag=false;
	TribeStr Tribe;
	//1��ѡ��ǰ������, 0����ѡ��
	int Action;


	for(k=0;k<1000;k++)
	{
		//������Ԫ����0
		Phi<<0;
		Elig<<0;
		//��������ֵ����, �����Ļر���ֻ���һ��������ɺ���лر���
		double Delta;

		//���ó�������ѧϰ, ������ÿһ��
		//�ڵ�i��ѭ��ʱѡ��i��1�Ķ���, �����Ѿ�����i��1�εĶ���, �޸��˺�������, ���ѭ����������: 
		for(j=0,i=rand()%ClsNum;j<ClsNum;j++,i=(i+1)%ClsNum)
		{
			//e-greedy
			if((double)rand()/RAND_MAX>Epsilon)
			{
				//���������ֵ, ѡ����
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
			//��״̬
			Phi[i]=Action;
		}
		//��action����׷��
		Elig+=Phi;
		//trace��һ��
		double SumElig=0;
		for(j=0;j<(int)Elig.size();j++)
			SumElig+=Elig[j];
		for(j=0;j<(int)Elig.size();j++)
			Elig[j]/=SumElig;

		//����ر�ֵ: ���һ��ѡ�����ǰ, �ر�ֵ����0
		Tribe.Weight.clear();
		for(i=0;i<ClsNum;i++)
			Tribe.Weight.push_back(Phi[i]);
		//��������ķ��ྫ��
		CalcFitness(FileName,Tribe,ValSet,PredResults,CaseInfo);
		Reward=Tribe.Pt;
		//���������ֵ, ѡ����
		MaxVal=MaxAct(Theta);
		//�ر����ϵĺ���ֵ������ X ���ԣ����²������
		Delta=Reward-Theta*Phi;
		//�¾ɺ���ֵ�Ĳ�ֵ
		Delta=Delta+Gamma*MaxVal;
		//���º�������
		Theta=Theta+Elig*(Alpha*Delta);
		//��������
		//		if(abs(Alpha*Delta)<0.0001)break;
		/*		for(i=0;i<(int)Elig.size();i++)
		if(Elig[i]*Alpha*Delta>0.0001)
		break;
		if(i>=(int)Elig.size())
		QuitFlag=true;
		*/		//�𲽼�С
		Epsilon*=(1-0.000001);
	}
	//	while(true);

	//���ѡ��
	Phi<<0;
	for(i=0;i<ClsNum;i++)
	{
		Action=MaxAct(Theta,Phi,i,MaxVal);
		Phi[i]=Action;
	}


	char TempStr[1024];
	sprintf(TempStr,"\nEprl: �ļ�%s, ѡ�������: \n",FileName);
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
	sprintf(TempStr,"\nEprl: �ļ�%s, ��������Ŀ%d, ѧϰ�꾫��%.4f, ѡ����ʱ%.4f\n",
		FileName,SelNum,Reward,duration);
	ResultLog(TempStr);
	Stat->ClsNum+=SelNum;
	Stat->TotalTime+=duration;
	Stat->TotalTmpAccr+=Reward;

	start = clock();
	CalcFitness(FileName,Tribe,TestSet,CaseInfo,Clses);
	finish = clock();
	sprintf(TempStr,"Eprl: �ļ�%s, ��������Ŀ%d, ����%.4f, Ԥ����ʱ%.4f\n\n",
		FileName,SelNum,Tribe.Pt,duration);
	ResultLog(TempStr);

	//ͳ��
	Stat->TotalPredTime+=duration;
	Stat->TotalAccr+=Tribe.Pt;
	Stat->Accrs.push_back(Tribe.Pt);

	return 0;
}

