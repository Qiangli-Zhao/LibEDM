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


#ifndef ENSEMBLE_PRUNER_H
#define ENSEMBLE_PRUNER_H

namespace libep
{
	//base class of all ensemble pruning algorithms
	class CEnsemblePruner : public CClassifier
	{
	public:
		//predict a dataset
		virtual CPrediction *Classify(const CDataset &DataSet) const
		{
			return Ensemble.Classify(DataSet,Weights);
		};
		//classifier is saved into file, and can be reconstructed from it
		virtual int Save(const string &Path,const string &FileName) const;
		virtual bool Dump(const string &FileName) const
		{
			int st=Save("",FileName);
			return (st==0);
		};
		virtual CClassifier *Clone() const
		{
			CEnsemblePruner *Pruner=new CEnsemblePruner(Ensemble);
			Pruner->Weights.assign(Weights.begin(),Weights.end());

			return Pruner;
		}

		//vote predicting, if we have prediction of each classifiers
		CPrediction *Classify(const CDataset &DataSet,const vector<CPrediction*> &Predictions) const
		{
			return Ensemble.Classify(DataSet,Predictions,Weights);
		}

		//user defined pruned ensemble
		CEnsemblePruner(const CEnsemble &UEnsemble,const vector<double> &UserWeights);
		virtual ~CEnsemblePruner();
		//load an ensemble pruner from initialization file
		CEnsemblePruner(const string &Path,const string &FileName,const CEnsemble &UEnsemble);

		const vector<double> &GetWeights() const
		{
			return Weights;
		}

		int GetSize() const
		{
			int Size=0;
			for(int i=0;i<(int)Weights.size();i++)
				if(Weights[i]!=0)
					Size++;

			return Size;
		}

		static string GetStaticName()
		{
			return StaticName;
		};

	protected:
		//un-pruned ensemble
		CEnsemblePruner(const CEnsemble &UEnsemble):Ensemble(UEnsemble){};

	protected:
		const CEnsemble &Ensemble;
		vector<double>	Weights;

	private:
		const static string StaticName;
	};

	typedef CEnsemblePruner *PrunerOnPredictionCreator(const CEnsemble &Ensemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions, const void *Params);
	typedef CEnsemblePruner *PrunerCreator(const CEnsemble &Ensemble,const CDataset &ValidatingSet,const void *Params);

}
#endif

