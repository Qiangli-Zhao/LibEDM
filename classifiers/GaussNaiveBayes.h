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


#ifndef  GAUSSNAIVEBAYES_Head_File
#define  GAUSSNAIVEBAYES_Head_File

namespace libep
{

	class CDataset;
	class CGaussNaiveBayes : public CIncrementalClassifier
	{
	public:
		virtual CPrediction *Classify(const CDataset &DataSet) const;
		virtual bool Dump(const string &FileName) const;
		virtual int Save(const string &Path,const string &FileName) const;
		//
		virtual void Train(const CDataset &Dataset);
		virtual void Reset();
		virtual CClassifier *Clone() const;

	public:
		CGaussNaiveBayes(){};
		CGaussNaiveBayes(const CGaussNaiveBayes &a)
		{
			Estims=a.Estims;
		};

		~CGaussNaiveBayes();

		CGaussNaiveBayes(const CDataset &TrainData);
		static void SetParams()
		{
		}
// 		static CClassifier *Create(const CDataset &TrainData,const void* UParams)
// 		{
// 			return new CGaussNaiveBayes(TrainData);
// 		}
// 
		static CIncrementalClassifier *Create(const CDataset &TrainData,const void* UParams)
		{
			return new CGaussNaiveBayes(TrainData);
		}

		CGaussNaiveBayes(const string &Path,const string &FileName);
		static CClassifier *FileCreate(const string &Path,const string &FileName)
		{
			return new CGaussNaiveBayes(Path,FileName);
		}

		static string GetStaticName()
		{
			return StaticName;
		};

	private:
		//data structures for statistics of continuous attribute
		typedef struct ContEstStr
		{
			double Mean;
			double StDevia;
			int Count;//number of values (for incremental learning)

			//these data are for run time usage
			vector<double>	Vals;//values of each instances is not required when classifying
			double OldMean;
			int OldCount;
			ContEstStr &operator =(const ContEstStr &a)
			{
				if(this==&a)  
					return *this;  

				Mean=a.Mean;
				StDevia=a.StDevia;
				Count=a.Count;
				OldMean=a.OldMean;
				OldCount=a.OldCount;
				Vals.assign(a.Vals.begin(),a.Vals.end());

				return *this;  
			};
		}ContEstStr;
		//for discrete attribute
		//for each class label, each value for an attribute has a statistics
		typedef struct DiscEstStr
		{
			int				Count;//instances count for each class label
			vector<double>	AttrCount;//instances count: each value of an attribute for each class label
			DiscEstStr &operator =(const DiscEstStr &a)
			{
				if(this==&a)  
					return *this;  

				Count=a.Count;
				AttrCount.assign(a.AttrCount.begin(),a.AttrCount.end());

				return *this;  
			};
		}DiscEstStr;
		//an attribute(ContEst or DiscEst: only one should be used)
		typedef struct EstimatorStr
		{
			ContEstStr	ContEst;
			DiscEstStr	DiscEst;
			int			AttType;
			EstimatorStr &operator =(const EstimatorStr &a)
			{
				if(this==&a)  
					return *this;  

				ContEst=a.ContEst;
				DiscEst=a.DiscEst;
				AttType=a.AttType;

				return *this;  
			};
		}EstimatorStr;
		//all attributes for a class type
		typedef vector<EstimatorStr> EstimatorsStr;

	private:
		vector<EstimatorsStr> Estims;//每个属性根据类别数目有多项//each class type has an entry
		const static string StaticName;
	};
}//namespace

#endif

