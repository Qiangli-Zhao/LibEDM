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


#ifndef  SEA_OP_INCENSEMBLE_INC
#define  SEA_OP_INCENSEMBLE_INC

namespace libedm
{
	class CIncrementalTrunkEnsemble;
	class CSEA_OP : public CIncrementalTrunkEnsemble
	{
	public:
		typedef struct ParamStr
		{
			CreateFunc *Batch;
			void *BatchParams;
		}ParamStr;

	public:
		//base classifier function entry and corresponding parameters
		CSEA_OP(int MaxSize,CreateFunc *Batch=CC45::Create,const void *BatchParams=NULL);
		static CIncrementalTrunkEnsemble *Create(int MaxSize,const void *UParams)
		{
			if(UParams==NULL)
				return new CSEA_OP(MaxSize);

			const ParamStr *Param=(const ParamStr*)UParams;
			return new CSEA_OP(MaxSize,Param->Batch,Param->BatchParams);
		}

		void Train(const CDataset &Dataset);
		void Train(const CDataset &Dataset, const CClassifier *Classifier);

	private:
		CreatorRegisterStr Creator;
		int MaxSize;
		vector<double> Qualities;
	};
}

#endif

