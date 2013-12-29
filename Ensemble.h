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


#ifndef  ENSEMBLE_INC
#define  ENSEMBLE_INC

namespace libep
{

	class CClassifier;

	class CEnsemble : public CClassifier
	{
	public:
		typedef struct CreatorRegisterStr
		{
			double			Ratio;// percent of this classifier in ensemble
			CreateFunc		*Creator;//entry of construction function building classifier from training data
			const void		*Params;//Create parameters for the construction
		}CreatorRegisterStr;
		typedef struct FileCreatorRegisterStr
		{
			string			ClassifierType;//type name of the classifier
			FileCreateFunc	*Creator;//entry of the function restoring classifier from files
		}FileCreatorRegisterStr;

	protected:
		//build an empty ensemble
		CEnsemble(){};

	public:
		virtual ~CEnsemble();

		//save all classifiers into file, and can be reconstructed from it later
		virtual int Save(const string &Path,const string &FileName) const;
		//register building function entry and type-name of each type of classifier
		template <class T> static int Register()
		{
			string ClassifierType=T::GetStaticName();
			//don't allow duplicated register entry
			for(int i=0;i<(int)FileCreators.size();i++)
				if(ClassifierType==FileCreators[i].ClassifierType)
					return 0;

			FileCreatorRegisterStr FileCreatorRegister;
			FileCreatorRegister.ClassifierType=ClassifierType;
			FileCreatorRegister.Creator=T::FileCreate;
			FileCreators.push_back(FileCreatorRegister);
			return 0;
		}
		//restoring from files
		CEnsemble(const string &Path,const string &FileName);

		//remove all classifiers of an ensemble
		bool Flush();

		//vote predicting
		virtual CPrediction *Classify(const CDataset &DataSet) const;
		//vote predicting, if we have prediction of each classifiers
		virtual CPrediction *Classify(const CDataset &DataSet,const vector<CPrediction*> &Predictions) const;
		//vote predicting, using user-defined weights vector
		virtual CPrediction *Classify(const CDataset &DataSet,const vector<double> &UserWeights) const;
		//vote predicting, using user-defined weights vector
		virtual CPrediction *Classify(const CDataset &DataSet,const vector<CPrediction*> &Predictions,const vector<double> &UserWeights) const;
		
		//
		virtual bool Dump(const string &FileName) const
		{
			Save("",FileName);
			return true;
		};

		virtual CClassifier* Clone() const
		{
			//create a empty ensemble
			CEnsemble *Ensemble=new CEnsemble;

			//copy base classifiers and weights of this ensemble to the new one
			for(int i=0;i<(int)Classifiers.size();i++)
			{
				CClassifier *Cls=Classifiers[i]->Clone();
				Ensemble->Classifiers.push_back(Cls);
				Ensemble->Weights.push_back(Weights[i]);
			}

			return Ensemble;
		};

		//return all the classifiers of ensemble
		int GetSize()const
		{
			return (int)Weights.size();
		};
		//return all the classifiers of ensemble that can participate in predict
		int GetRealSize()const
		{
			int Count=0;
			for(int i=0;i<(int)Weights.size();i++)
				if(Weights[i]>0)
					Count++;
			return Count;
		};
		//return all the classifiers of ensemble
		const vector<CClassifier*> &GetAllClassifiers()const
		{
			return Classifiers;
		};
		//return weights for all the classifiers
		const vector<double> &GetWeights()const
		{
			return Weights;
		};
		//return predictions of each classifiers
		vector<CPrediction*> *AllClassify(const CDataset &DataSet) const;

		static string GetStaticName()
		{
			return StaticName;
		};

	protected:

	protected:
		static vector<FileCreatorRegisterStr>	FileCreators;
		//classifiers are allowed to update, even after added to ensemble
		vector<CClassifier*> Classifiers;
		vector<double> Weights;

	private:
		const static string StaticName;
	};
}

#endif
