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

#ifndef ORIENTORDER_ENSPRUNER
#define ORIENTORDER_ENSPRUNER

namespace libedm
{
	class  COrientOrder: public CEnsemblePruner
	{
	private:
		typedef struct OrderStr
		{
			double	Cosine;//cosine of angle between ref vector to each signature vector
			double	Angle;//angle between ref vector to each signature vector
			int		Cls;//classifier id (after ordering, the sequence of classifiers may be changed)
		}OrderStr;
		static bool CosDescOrder(const OrderStr &x,const OrderStr &y)
		{
			return x.Cosine>y.Cosine;
		}

	public:
		COrientOrder(const CEnsemble &Ensemble,const CDataset &ValidatingSet);
		static CEnsemblePruner *Create(const CEnsemble &Ensemble,const CDataset &ValidatingSet,const void *Params)
		{
			return new COrientOrder(Ensemble,ValidatingSet);
		}
		//predict a dataset
		COrientOrder(const CEnsemble &Ensemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions);
		static CEnsemblePruner *CreateOnPrediction(const CEnsemble &Ensemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions, const void *Params)
		{
			return new COrientOrder(Ensemble,ValidatingSet,Predictions);
		}
	};
}

#endif
