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


#ifndef STAT_INC
#define STAT_INC

namespace libep
{
	//function needed by statistic test
	//order of integer sets
	struct IntSetOrder
	{
		bool operator ()(const set<int> &a,const set<int> &b)
		{
			set<int>::const_iterator sj,sk;

			//smaller set first
			if(a.size()<b.size())
				return true;
			//if same size
			else if(a.size()==b.size())
			{
				for(sj=a.begin(),sk=b.begin();sj!=a.end();sj++,sk++)
				{
					//set with member less
					if(*sj<*sk)
						return true;
					else if(*sj>*sk)
						return false;
				}
			}

			return false;
		}
	};

	typedef struct RankStr
	{
		int	vs;//the two algorithms being compared
		double z;//z statistic 
		double p;//probability
		double APV;//adjusted probability
		double alpha;//adjusted alpha
	}RankStr;
	bool operator ==(const RankStr &a,const RankStr &b);
	bool RankDescOrder(const RankStr &a,const RankStr &b);

	class CStat
	{
	public:
		//Bergmann & Hommel test
		static void BH(int N,vector<double> &Ranks,vector<RankStr> &table);
		//Friedman test: Ff
		static double Ff(vector<double> &Ranks,int N,double &Skf);
	public:
		//factorial
		static double Multip(int n);
		//Gauss probability
		static double Gauss(double x);
		//reverse Gauss probability
		static double rGauss(double p);
	private:
		//Friedman test: Xf*Xf
		static double SquareKaiF(vector<double> &Ranks,int N);
		//Bergmann & Hommel test: statitic z
		static double z(double Ri,double Rj,int k,int N);
		//Bergmann & Hommel test: obtain exhaustive sets
		static void obtainExhaustive(set<set<int>,IntSetOrder> &E, const set<int> &C, int Mul);
		//return all possible nonempty division
		static void division(const set<int> &C,int height,set<int> C1,set<set<int>,IntSetOrder> &E);
	};
}

#endif

