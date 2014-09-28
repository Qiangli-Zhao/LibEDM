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
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <cassert>
using namespace std;
#include "Obj.h"
#include "Statistic.h"
using namespace libedm;

const double pi=3.1415926535897932;


//Friedman test: square of Xf
double CStat::SquareKaiF(vector<double> &Ranks,int N)
{
	double Sum=0;
	int k=(int)Ranks.size();
	for(int i=0;i<k;i++)
		Sum+=(Ranks[i]*Ranks[i]);
	Sum=12.0*N/k/(k+1)*(Sum-k*(k+1)*(k+1)/4.0);
	return Sum;
}

//Friedman test: Ff
double CStat::Ff(vector<double> &Ranks,int N,double &Skf)
{
	Skf=SquareKaiF(Ranks,N);
	int k=(int)Ranks.size();

	double Sum=(N-1)*Skf/(N*(k-1)-Skf);
	return Sum;
}

//factorial
double CStat::Multip(int n)
{
	if(n<=1)
		return 1.0;
	return 1.0*n*Multip(n-1);
}

//normal distribution: get probability
double CStat::Gauss(double x)
{
	double ax;
	if(x<0.0)
		ax=(-1)*x;
	else
		ax=x;
	if(ax>6.0)
		ax=6.0;

	int n=0;
	double sum=0.0;
	double contr=pow(ax,2*n+1)/(pow(2.0,n)*(2*n+1)*Multip(n));
	sum+=(n%2==0 ? 1 : -1)*contr;
	while(contr>0.00000001)
	{
		n++;
		double m=(2*n+1)*log((double)ax)-n*log((double)2)-log((double)2*n+1)-log((double)Multip(n));
		contr=exp(m);
//		contr=pow(ax,2*n+1)/(pow(2.0,n)*(2*n+1)*Multip(n));
		sum+=(n%2==0 ? 1 : -1)*contr;
	}
	sum=0.5+1/sqrt(2*pi)*sum;
	if(x<0)
		sum=1-sum;
	if(sum>1)
		sum=1;
	else if(sum<0)
		sum=0;
	return sum;
}

//normal distribution: probability to alpha
double CStat::rGauss(double p)
{
	double xp=0.0;
	double a=0.5;
	if(p<=0||p>=1)
	{
		throw(CError("Statistic: Invalid probability value!",100,0));
		return -1;
	}
	if(p==0.5)
		return 0;

	if(p>0.5)
	{
		while(p>a)
		{
			xp+=0.001;
			a=Gauss(xp);
		}
	}
	else
	{
		while(p<a)
		{
			xp-=0.001;
			a=Gauss(xp);
		}
	}

	return xp;
}

/*
bool operator <(const set<set<int>,IntSetOrder> &a,const set<set<int>,IntSetOrder> &b)
{
set<set<int>,IntSetOrder>::iterator sj,sk;

//元素集合数目少的在前
if(a.size()<b.size())
return true;
//元素集合数目相等
else if(a.size()==b.size())
{
for(int i=0;i<a.size();i++)
{
sj=a.begin()+i;
sk=b.begin()+i;
//第一个不相同元素集合较小的在前
if(*sj<*sk)
return true;
}
}

return false;
}
*/

void operator +=(set<set<int>,IntSetOrder> &E1,const set<set<int>,IntSetOrder> &E2)
{
	set<set<int>,IntSetOrder>::const_iterator i;
	for(i=E2.begin();i!=E2.end();i++)
		E1.insert(*i);
}

//minus operator
set<int> operator -(const set<int> &a,const set<int> &b)
{
	set<int> c;
	set<int>::const_iterator i;

	for(i=a.begin();i!=a.end();i++)
	{
		if(find(b.begin(),b.end(),*i)==b.end())
			c.insert(*i);
	}

	return c;
}

void operator +=(set<int> &E1,const set<int> &E2)
{
	set<int>::const_iterator i;
	for(i=E2.begin();i!=E2.end();i++)
		E1.insert(*i);
}

set<int> operator +(const set<int> &E1,const set<int> &E2)
{
	set<int> E3;
	set<int>::const_iterator i;
	for(i=E1.begin();i!=E1.end();i++)
		E3.insert(*i);
	for(i=E2.begin();i!=E2.end();i++)
		E3.insert(*i);

	return E3;
}

//Bergmann & Hommel test: statistic z
double CStat::z(double Ri,double Rj,int k,int N)
{
	return (Ri-Rj)/sqrt(k*(k+1)/6.0/N);
}

//return all possible nonempty division of C
//INPUT:
//		height: number of devision
//		C1: selected item until now
//OUTPUT:
//		E: all division
void CStat::division(const set<int> &C,int height,set<int> C1,set<set<int>,IntSetOrder> &E)
{
	int i;
	set<int>::const_iterator it_C;
	for(it_C=C.begin(),i=0;i<height;it_C++,i++);
	//reach to the last item in C
	if(height==C.size()-1)
	{
		//will not select *it_C
		if(!C1.empty())
			E.insert(C1);
		//select *it_C
		C1.insert(*it_C);
		E.insert(C1);
		return;
	}

	//if don't select *it_C
	division(C,height+1,C1,E);
	//if *it_C is selected
	C1.insert(*it_C);
	division(C,height+1,C1,E);
}

typedef set<int> int_set;
typedef set<int_set,IntSetOrder> order_int_set;
static map<int_set,order_int_set> CEMap;
//Bergmann & Hommel test: obtain exhaustive sets
//OUTPUT:
//		E: exhaustive sets
void CStat::obtainExhaustive(set<set<int>,IntSetOrder> &E, const set<int> &C,int Mul)
{
	set<int>::const_iterator	i,j;
	set<set<int>,IntSetOrder>::iterator		k,l,m;
	set<int>					C1;
	set<set<int>,IntSetOrder>				E1,E2;

	map<int_set,order_int_set>::iterator it_CEMap;
	typedef pair<int_set,order_int_set> CEMap_Pair;


	E.clear();
	//C1: set of all possible and distinct pairwise comparisons in C
	//Last classifier in set C
	int LastC;
	for(i=C.begin();i!=C.end();i++)
	{
		for(j=C.begin();j!=C.end();j++)
		{
			if(*i>=*j)
				continue;
			C1.insert(*i*Mul+*j);
		}
		LastC=*i;
	}
	if(!C1.empty())
		E.insert(C1);
	if(E.empty())
		return;

	C1.clear();
	set<set<int>,IntSetOrder> s_C1;
	division(C,0,C1,s_C1);
	for(k=s_C1.begin();k!=s_C1.end();k++)
	{
		//the last classifier must enter C2
		if(k->find(LastC)!=k->end())
			continue;
		//CEMap is a cache, from which we can find k's corresponding Exhaustive set
		it_CEMap=CEMap.find(*k);
		if(it_CEMap!=CEMap.end())
			E1=it_CEMap->second;
		else
		{
			obtainExhaustive(E1,*k,Mul);
			if(!E1.empty())
				CEMap.insert(CEMap_Pair(*k,E1));
		}
		//CEMap is a cache, from which we can find (C-k)'s corresponding Exhaustive set
		it_CEMap=CEMap.find(C-*k);
		if(it_CEMap!=CEMap.end())
			E2=it_CEMap->second;
		else
		{
			obtainExhaustive(E2,C-*k,Mul);
			if(!E2.empty())
				CEMap.insert(CEMap_Pair(C-*k,E2));
		}

		E+=E1;
		E+=E2;

		//No hypothesis enters E1 and E2 at the same time
		for(l=E1.begin();l!=E1.end();l++)
			for(m=E2.begin();m!=E2.end();m++)
			{
				C1.clear();
				C1=*l+*m;
				E.insert(C1);
			}
	}

	return;
}

bool libedm::operator ==(const RankStr &a,const RankStr &b)
{
	return a.vs==b.vs;
}

bool libedm::RankDescOrder(const RankStr &a,const RankStr &b)
{
	return (a.z>b.z);
}

//Bergmann-Hommel APVi: min{v;1}, where v=max{|I| ·min{pj, j ∈ I} : I exhaustive, i ∈I}
//Bergmann & Hommel test
void CStat::BH(int N,vector<double> &Ranks,vector<RankStr> &Tab)
{
	set<set<int>,IntSetOrder>::iterator j;
	set<int>::iterator k,l;


	//classifier set
	set<int> C;
	for(int i=0;i<(int)Ranks.size();i++)
		C.insert(i+1);
	int Mul=(int)pow((float)10,((int)C.size()+9)/10);
	//Exhaustive set: set of null hypotheses represented by combined id of two methods being compared
	//Set of Exhaustive set
	set<set<int>,IntSetOrder> E;
	CEMap.clear();
	obtainExhaustive(E,C,Mul);
	//initialize the control table
	RankStr Rank;
	for(k=C.begin();k!=C.end();k++)
		for(l=C.begin();l!=C.end();l++)
		{
			if(*k>=*l)
				continue;
			Rank.vs=*k*Mul+*l;
			Rank.z=fabs(z(Ranks[*k-1],Ranks[*l-1],(int)C.size(),N));
			Rank.p=(1-Gauss(Rank.z))*2;
			Rank.APV=Rank.p;
			Tab.push_back(Rank);
		}

	//E is set of exhaustive sets
	//acceptance set A =U{I : I exhaustive, min{Pi : i ∈ I} > α/|I|}
	vector<RankStr>::iterator m_It;
	for(j=E.begin();j!=E.end();j++)
	{
		//inspect one exhaustive set I
		//first item in I
		k=j->begin();
		Rank.vs=*k;
		m_It=find(Tab.begin(),Tab.end(),Rank);
		//find minimum p in I
		double MinP=m_It->p;
		for(k++;k!=j->end();k++)
		{
			Rank.vs=*k;
			m_It=find(Tab.begin(),Tab.end(),Rank);
			if(MinP>m_It->p)
				MinP=m_It->p;
		}
		//
		MinP*=j->size();
		if((MinP)>1)
			MinP=1;

//		FILE *ft=fopen("a.txt","at");
		//adjust p
		for(k=j->begin();k!=j->end();k++)
		{
			Rank.vs=*k;
			m_It=find(Tab.begin(),Tab.end(),Rank);
			if(MinP>m_It->APV)
				m_It->APV=MinP;
//			RankStr Rank=m_It->second;
//			fprintf(ft,"name=%d: z=%f,p=%g,APV=%g\n",*k,Rank.z,Rank.p,Rank.APV);
		}
//		fprintf(ft,"\n");
//		fclose(ft);
	}

	sort(Tab.begin(),Tab.end(),RankDescOrder);
	return;
}

