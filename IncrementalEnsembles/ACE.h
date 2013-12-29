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


#ifndef  ACE_INCENSEMBLE_INC
#define  ACE_INCENSEMBLE_INC

namespace libep
{
	class CIncrementalEnsemble;
	class CACE : public CIncrementalEnsemble
	{
	public:
		typedef struct ParamStr
		{
			double	Alpha;//confidence level
			int		Sa;//short term memory size
			int		Sc;//long term memory size
			int		u;//adjustment factor of ensemble
			CreateFunc *Online;
			CreateFunc *Batch;
			const void *OnlineParams;
			const void *BtachParams;
		}ParamStr;

	public:
		//base classifier function entry and corresponding parameters
		CACE(int MaxSize,double Alpha=0.01,int Sa=30,int Sc=200,double u=3.0,
			IncermentalCreateFunc *Online=CGaussNaiveBayes::Create,const void *OnlineParams=NULL,
			CreateFunc *Batch=CC45::Create,const void *BatchParams=NULL);
		~CACE();

		void Train(const CDataset &Dataset);

	private:
		typedef struct ClassifierInfoStr
		{
			//history data
			//this classifier's prediction for the memorized instances
			//0-failed, 1-success
			vector<double>	CR;
			vector<double> As;
			vector<double> ALowers;
			vector<double> AUppers;
// 			//recent data
// 			double		A;
// 			double		ALower;
// 			double		AUpper;
			//
			int			CreateTime;
		}ClassifierInfoStr;

	private:
		//parameters
		int MaxSize;
		IncrementorRegisterStr OnlineLearner;
		CreatorRegisterStr BatchLearner;
		const double	Alpha;//confidence level
		const int		Sa;//short term memory size
		const int		Sc;//long term memory size
		const double	u;//adjustment factor of ensemble
		//reverse gauss distribution for Z(alpha/2)
		const double Za2;

		//memory
		CDataset *Memory;
		//classifier's run time data
		vector<ClassifierInfoStr> Infos;
		//if the on-line classifier JUST created?
		bool OnlineJustCreated;
	};
}

#endif

