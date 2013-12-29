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

#ifndef CPM_ENSPRUNER
#define CPM_ENSPRUNER

namespace libep
{
	class  CPMEP: public CEnsemblePruner
	{
	public:
		CPMEP(const CEnsemble &Ensemble,const CDataset &ValidatingSet);
		static CEnsemblePruner *Create(const CEnsemble &Ensemble,const CDataset &ValidatingSet,const void *Params)
		{
			return new CPMEP(Ensemble,ValidatingSet);
		}
		//predict a dataset
		CPMEP(const CEnsemble &Ensemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions);
		static CEnsemblePruner *CreateOnPrediction(const CEnsemble &Ensemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions, const void *Params)
		{
			return new CPMEP(Ensemble,ValidatingSet,Predictions);
		}

	private:
		typedef struct TreeNodeStr
		{
			int					Classifier;//id of classifier of this node
			int					Count;//number of instances
			vector<TreeNodeStr>	SubNodes;//sub nodes
		}TreeNodeStr;

		//table: statistics of all classifiers' prediction results
		typedef struct CaseClassRecStr
		{
			int			Correct;//0-wrong, 1-correct, >1-total correctness//number of correct prediction
			
			//only for last row
			int			Classifier;//id of classifier
			//		TreeNodeStr *NodeLink;//the first appearance of the classifier in FP-tree
		}CaseClassRecStr;
		typedef vector<CaseClassRecStr> CaseClassArrayStr;
		static bool CorrectDescOrder(const CaseClassRecStr &a,const CaseClassRecStr &b)
		{
			return (a.Correct>b.Correct);
		}

		//
		typedef struct SelClassifierStr
		{
			set<int>	Set;
			int			Count;
		}SelClassifierStr;
		static bool SelClassCountDescOrder(const SelClassifierStr &a,const SelClassifierStr&b)
		{
			return (a.Count>b.Count);
		}
		static bool SelClassSetSizeOrder(const SelClassifierStr &a,const SelClassifierStr&b)
		{
			return (a.Set.size()<b.Set.size());
		}

		//
		typedef struct TreePathStr
		{
			vector<int>	Classifiers;// a path in FP-tree
			int			Count;//instances that on this path
		}TreePathStr;
		static bool CountDescOrder(const TreePathStr &a,const TreePathStr &b)
		{
			return(a.Count>b.Count);
		}
		static bool ClassNumOrder(const TreePathStr &a,const TreePathStr &b)
		{
			return(a.Classifiers.size()<b.Classifiers.size());
		}

	private:
		//case-label table
		int BuildCaseClassTab(vector<CaseClassArrayStr> &CaseClassTab,const CDataset &ValidatingSet,
			const vector<CPrediction*> &Predictions);
//		int SelectCaseClassTab(vector<CaseClassArrayStr> &SelCaseClassTab,const vector<CaseClassArrayStr> &CaseClassTab,const int k);
		void Dump(const string &FileName,const vector<CaseClassArrayStr> &CaseClassTab);

		//FP-tree
		int BuildFPTree(TreeNodeStr &Root,const vector<CaseClassArrayStr> &CaseClassTab,int k);
//		int deltree(TreeNodeStr **Root);

		//path table
//		int BuildPathTab(vector<TreePathStr> &TreePathTable,const vector<CaseClassArrayStr> &CaseClassTab,int k);
		int BuildPathTab(vector<TreePathStr> &TreePathTable,const TreeNodeStr &Root,TreePathStr &TreePath,int k);
		void Dump(const string &FileName,const vector<TreePathStr> &TreePathTable);

		void Dump(const string &FileName,const vector<SelClassifierStr> &SelClassifiers);
	};
}

#endif

