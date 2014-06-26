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


#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
using namespace std;
#include "Obj.h"
#include "DataSet.h"
#include "Classifier.h"
#include "Ensemble.h"
#include "Prediction.h"
#include "EnsemblePruner.h"
#include "Cluster.h"
using namespace libep;

const char MyName[MAX_OBJECT_NAME_LENGTH]="Clustering";

//classifier is transform into a double array, each item means if it is predict a instance correctly(correct: 1, wrong: 0)
CCluster::CCluster(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,const vector<CPrediction*> &Predictions,double Lamda)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	if(Ensemble.GetSize()!=(int)Predictions.size())
		throw(CError("EnsemblePruner: wrong size for user-defined weights!",100,0));
	//Info
	int CaseNum=ValidatingSet.GetInfo().Height;
	int ClassNum=ValidatingSet.GetInfo().ClassNum;
	int EnsembleSize=Ensemble.GetSize();
	//start time for training
	clock_t start=clock();

	//initialize weights of pruner
	for(int i=0;i<EnsembleSize;i++)
		Weights.push_back(1.0/EnsembleSize);
	//k-mean
	double BestDistance=CaseNum;
	//classifiers are clustered
	vector<ClusterNodeArray> BestClusters;
	//centroid for each cluster
	vector<int> BestCentroids;
	//test all possible clusters size
	for(int k=2;k<EnsembleSize;k++)
	{
		vector<ClusterNodeArray> Clusters;
		vector<int> Centroids;
		//clustering with given target size k
		k_mean(k,Predictions,Clusters,Centroids);
		//the obtained clusters may less than k
		int ClusterNum=(int)Clusters.size();

		//average disagreement(distance) between each centroid
		double Distance=0;
		for(int i=0;i<ClusterNum;i++)
			for(int j=i+1;j<ClusterNum;j++)
				Distance+=kappa(Predictions[Centroids[i]]->GetPredictedLabelIndices(),
					Predictions[Centroids[j]]->GetPredictedLabelIndices(),ClassNum);
		//disagreement is not increase?
		Distance/=(ClusterNum*(ClusterNum-1)/2);
		//disagreement is not increase, so we don't need to increase the number of clusters
		if(Distance>=BestDistance)
			break;
		BestDistance=Distance;
		BestClusters=Clusters;
		BestCentroids=Centroids;
	}

	//prune in each cluster
	for(int i=0;i<(int)BestClusters.size();)
	{
		//for each cluster, sort classifiers in accuracy ascend order
		sort(BestClusters[i].begin(),BestClusters[i].end(),AccuracyOrder);

		//OldAccr-accuracy before any operation
		double OldAccuracy;
		{
			CPrediction *Prediction=Ensemble.Classify(ValidatingSet,Predictions,Weights);
			OldAccuracy=Prediction->GetAccuracy();
			delete Prediction;
		}

		bool ClusterRemoved=false;
		for(int j=0;j<(int)BestClusters[i].size();)
		{
			//each classifier
			int Classifier=BestClusters[i][j].Classifier;
			//try remove it
			double OriginalWeight=Weights[Classifier];
			Weights[Classifier]=0;
			double Accuracy;
			{
				CPrediction *Prediction=Ensemble.Classify(ValidatingSet,Predictions,Weights);
				Accuracy=Prediction->GetAccuracy();
				delete Prediction;
			}

			int LastClassifier=BestClusters[i][BestClusters[i].size()-1].Classifier;
			double kappa1=kappa(Predictions[Classifier]->GetPredictedLabelIndices(),
				Predictions[LastClassifier]->GetPredictedLabelIndices(),ClassNum);
			if(kappa1<Lamda)
			{
				//by disagreement criterion we have to remove this classifier,
				//but this will make the accuracy begin to decrease,
				//so we keep this classifier and stop trying to remove
				if(Accuracy<OldAccuracy)
				{
					Weights[Classifier]=OriginalWeight;
					break;
				}
			}

			//remove
			if(kappa1<Lamda || Accuracy>OldAccuracy)
			{
				//remove from cluster
				BestClusters[i].erase(BestClusters[i].begin()+j);
				OldAccuracy=Accuracy;

				//dispatch its weight
				//if it's the last classifier, remove corresponding cluster
				if(BestClusters[i].size()<=0)
				{
					//remove cluster
					ClusterRemoved=true;
					BestClusters.erase(BestClusters.begin()+i);

					//dispatch the cluster's weight evenly to left classifiers in all other clusters
					double SumKp=0;
					for(int k=0;k<(int)BestClusters.size();k++)
						SumKp+=BestClusters[k].size();
					for(int k=0;k<(int)BestClusters.size();k++)
						for(int l=0;l<(int)BestClusters[k].size();l++)
						{
							int Cid=BestClusters[k][l].Classifier;
							Weights[Cid]+=(OriginalWeight/SumKp);
						}

					break;
				}

				//not empty after remove this classifier
				//sum of kappa between the classifier just removed to all other classifiers still in the cluster
				double SumKp=0;
				vector<double> Kps;
				for(int k=0;k<(int)BestClusters[i].size();k++)
				{
					int Cid=BestClusters[i][k].Classifier;
					//if each classifier in the cluster is same as the top accurate one, all kappa is zero, as well as their sum
					double Kp=fabs(kappa(Predictions[Classifier]->GetPredictedLabelIndices(),
						Predictions[Cid]->GetPredictedLabelIndices(),ClassNum))+0.000001;
					SumKp+=Kp;
					Kps.push_back(Kp);
				}
				//dispatch the classifier's weight to classifiers left in the cluster, with considering the distance to it
				for(int k=0;k<(int)BestClusters[i].size();k++)
				{
					int Cid=BestClusters[i][k].Classifier;
					Weights[Cid]+=(OriginalWeight*Kps[k]/SumKp);
				}
			}
			else//need not remove it, reenter the cluster
			{
				Weights[Classifier]=OriginalWeight;
				j++;
			}
		}//for j
		if(!ClusterRemoved)
			i++;
	}// for i

// 	for(int i=0;i<EnsembleSize;i++)
// 		if(Tribe.Weights[i]>0)
// 		{
//			sprintf(TempStr,"%d(%4f),",i,Tribe.Weights[i]);
//			ResultLog(TempStr);
// 		}

	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

CCluster::CCluster(const CEnsemble &UEnsemble,const CDataset &ValidatingSet,double Lamda)
:CEnsemblePruner(UEnsemble)
{
	Name=MyName;
	//Info
	int CaseNum=ValidatingSet.GetInfo().Height;
	int ClassNum=ValidatingSet.GetInfo().ClassNum;
	int EnsembleSize=Ensemble.GetSize();
	//start time for training
	clock_t start=clock();

	//get predictions
	vector<CPrediction*> *Predictions=Ensemble.AllClassify(ValidatingSet);
	//initialize weights of pruner
	for(int i=0;i<EnsembleSize;i++)
		Weights.push_back(1.0/EnsembleSize);
	//k-mean
	double BestDistance=CaseNum;
	//classifiers are clustered
	vector<ClusterNodeArray> BestClusters;
	//centroid for each cluster
	vector<int> BestCentroids;
	//test all possible clusters size
	for(int k=2;k<EnsembleSize;k++)
	{
		vector<ClusterNodeArray> Clusters;
		vector<int> Centroids;
		//clustering with given target size k
		k_mean(k,*Predictions,Clusters,Centroids);
		//the obtained clusters may less than k
		int ClusterNum=(int)Clusters.size();

		//average disagreement(distance) between each centroid
		double Distance=0;
		for(int i=0;i<ClusterNum;i++)
			for(int j=i+1;j<ClusterNum;j++)
				Distance+=kappa((*Predictions)[Centroids[i]]->GetPredictedLabelIndices(),
				(*Predictions)[Centroids[j]]->GetPredictedLabelIndices(),ClassNum);
		//disagreement is not increase?
		Distance/=(ClusterNum*(ClusterNum-1)/2);
		//disagreement is not increase, so we don't need to increase the number of clusters
		if(Distance>=BestDistance)
			break;
		BestDistance=Distance;
		BestClusters=Clusters;
		BestCentroids=Centroids;
	}

	//prune in each cluster
	for(int i=0;i<(int)BestClusters.size();)
	{
		//for each cluster, sort classifiers in accuracy ascend order
		sort(BestClusters[i].begin(),BestClusters[i].end(),AccuracyOrder);

		//OldAccr-accuracy before any operation
		double OldAccuracy;
		{
			CPrediction *Prediction=Ensemble.Classify(ValidatingSet,*Predictions,Weights);
			OldAccuracy=Prediction->GetAccuracy();
			delete Prediction;
		}

		bool ClusterRemoved=false;
		for(int j=0;j<(int)BestClusters[i].size();)
		{
			//each classifier
			int Classifier=BestClusters[i][j].Classifier;
			//try remove it
			double OriginalWeight=Weights[Classifier];
			Weights[Classifier]=0;
			double Accuracy;
			{
				CPrediction *Prediction=Ensemble.Classify(ValidatingSet,*Predictions,Weights);
				Accuracy=Prediction->GetAccuracy();
				delete Prediction;
			}

			int LastClassifier=BestClusters[i][BestClusters[i].size()-1].Classifier;
			double kappa1=kappa((*Predictions)[Classifier]->GetPredictedLabelIndices(),
				(*Predictions)[LastClassifier]->GetPredictedLabelIndices(),ClassNum);
			if(kappa1<Lamda)
			{
				//by disagreement criteria we will remove this classifier,
				//but doing this will make the total accuracy decrease,
				//so we just keep this classifier, won't remove it
				if(Accuracy<OldAccuracy)
				{
					Weights[Classifier]=OriginalWeight;
					break;
				}
			}

			//remove
			if(kappa1<Lamda || Accuracy>OldAccuracy)
			{
				//remove from cluster
				BestClusters[i].erase(BestClusters[i].begin()+j);
				OldAccuracy=Accuracy;

				//dispatch its weight
				//if it's the last classifier, remove corresponding cluster
				if(BestClusters[i].size()<=0)
				{
					//remove cluster
					ClusterRemoved=true;
					BestClusters.erase(BestClusters.begin()+i);

					//dispatch the cluster's weight evenly to left classifiers in all other clusters
					double SumKp=0;
					for(int k=0;k<(int)BestClusters.size();k++)
						SumKp+=BestClusters[k].size();
					for(int k=0;k<(int)BestClusters.size();k++)
						for(int l=0;l<(int)BestClusters[k].size();l++)
						{
							int Cid=BestClusters[k][l].Classifier;
							Weights[Cid]+=(OriginalWeight/SumKp);
						}

						break;
				}

				//not empty after remove this classifier
				//sum of kappa between the classifier just removed to all other classifiers still in the cluster
				double SumKp=0;
				vector<double> Kps;
				for(int k=0;k<(int)BestClusters[i].size();k++)
				{
					int Cid=BestClusters[i][k].Classifier;
					//if each classifier in the cluster is same as the top accurate one, all kappa is zero, as well as their sum
					double Kp=fabs(kappa((*Predictions)[Classifier]->GetPredictedLabelIndices(),
						(*Predictions)[Cid]->GetPredictedLabelIndices(),ClassNum))+0.000001;
					SumKp+=Kp;
					Kps.push_back(Kp);
				}
				//dispatch the classifier's weight to classifiers left in the cluster, with considering the distance to it
				for(int k=0;k<(int)BestClusters[i].size();k++)
				{
					int Cid=BestClusters[i][k].Classifier;
					Weights[Cid]+=(OriginalWeight*Kps[k]/SumKp);
				}
			}
			else//need not remove it, reenter the cluster
			{
				Weights[Classifier]=OriginalWeight;
				j++;
			}
		}//for j
		if(!ClusterRemoved)
			i++;
	}// for i

	//release prediction
	for(int i=0;i<EnsembleSize;i++)
		delete ((*Predictions)[i]);
	delete Predictions;
	//time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

//output:
//Clusters-classifiers for each cluster
//centers: classifier near the center of a cluster is used as centroid
int CCluster::k_mean(int m,const vector<CPrediction*> &Predictions,vector<ClusterNodeArray> &Clusters,vector<int> &Centers)
{
	int CaseNum=Predictions[0]->GetCaseNum();
	int ClassNum=Predictions[0]->GetClassNum();
	int EnsembleSize=(int)Predictions.size();
	Clusters.clear();

	//initialize cluster
	//randomly select Centroids for all clusters
	vector<DoubleArray> Centroids;
	for(int i=0;i<m;i++)
	{
		vector<double> Centroid;
		for(int j=0;j<CaseNum;j++)
		{
			const BoolArray &Correctness=Predictions[i]->GetCorrectness();
			Centroid.push_back((const double)Correctness[j]);
		}
		//n-th classifier is selected as the n-th cluster's centroid
		Centroids.push_back(Centroid);

		vector<ClusterNodeStr> Cluster;
		Clusters.push_back(Cluster);
	}

	//
	double DiffSum;
	do
	{
		//empty all clusters
		for(int i=0;i<(int)Clusters.size();i++)
			Clusters[i].clear();
		//dispatch all classifiers
		for(int i=0;i<EnsembleSize;i++)
		{
			//each classifier
			ClusterNodeStr ClusterNode;
			ClusterNode.Classifier=i;
			ClusterNode.Accuracy=Predictions[i]->GetAccuracy();
			const BoolArray &Correctness=Predictions[i]->GetCorrectness();

			//put classifier into the cluster with minimum distance to it
			double MinDis=CaseNum*ClassNum*ClassNum;
			int MinCluster;
			//all Centroids
			for(int j=0;j<(int)Clusters.size();j++)
			{
				double Distance=0;
				//distance to a Centroid
				for(int k=0;k<CaseNum;k++)
				{
					double diff=Correctness[k]-Centroids[j][k];
					Distance+=(diff*diff);
				}
				if(MinDis>Distance)
				{
					MinDis=Distance;
					MinCluster=j;
				}
			}
			//put classifier into cluster with minimum distance
			Clusters[MinCluster].push_back(ClusterNode);
		}
		//remove the empty clusters
		for(int i=0;i<(int)Clusters.size();)
		{
			if(Clusters[i].size()<=0)
			{
				Clusters.erase(Clusters.begin()+i);
				Centroids.erase(Centroids.begin()+i);
				continue;
			}
			i++;
		}

		//Calculate new center
		DiffSum=0;
		for(int j=0;j<(int)Clusters.size();j++)
		{
			//
			for(int k=0;k<CaseNum;k++)
			{
				//k-th component
				double sum=0;
				for(int i=0;i<(int)Clusters[j].size();i++)
				{
					int Cid=Clusters[j][i].Classifier;
					sum+=(Predictions[Cid]->GetCorrectness()[k]);
				}
				sum/=Clusters[j].size();

				//distance between new center and old one
				double diff=sum-Centroids[j][k];
				DiffSum+=(diff*diff);
				Centroids[j][k]=sum;
			}
		}
	}
	while(DiffSum>0.1);

	//but the real centroid is the nearest point to the center
	Centers.clear();
	for(int i=0;i<(int)Clusters.size();i++)
	{
		double MinDis=ClassNum*ClassNum*CaseNum+1;
		int MinP=-1;
		for(int j=0;j<(int)Clusters[i].size();j++)
		{
			//distances
			double Distance=0;
			int Classifier=Clusters[i][j].Classifier;
			for(int k=0;k<CaseNum;k++)
			{
				double diff=(Predictions[Classifier]->GetCorrectness()[k])-Centroids[i][k];
				Distance+=(diff*diff);
			}
			//Minimum distance
			if(MinDis>Distance)
			{
				MinDis=Distance;
				MinP=Classifier;
			}
		}
		Centers.push_back(MinP);
	}

	return 0;
}

//M(a,a)/m
//number of instances satisfying this condition: pred(h1)=Cls1 and pred(h2)=Cls2
double CCluster::Theta(const vector<int> &V1,const vector<int> &V2,int Class1,int Class2)
{
	int CaseNum=(int)V1.size();

	double Theta1=0;
	for(int i=0;i<CaseNum;i++)
	{
		if(V1[i]==Class1 && V2[i]==Class2)
			Theta1+=1;
	}
	Theta1/=CaseNum;

	return Theta1;
}

//kappa: the less, the more diversity
double CCluster::kappa(const vector<int> &V1,const vector<int> &V2,int ClassNum)
{
	int CaseNum=(int)V1.size();

	double Theta1=0;
	for(int j=0;j<ClassNum;j++)
		Theta1+=Theta(V1,V2,j,j);

	double Theta2=0;
	for(int i=0;i<ClassNum;i++)
	{
		double sum1=0;
		for(int j=0;j<ClassNum;j++)
			sum1+=Theta(V1,V2,i,j);

		double sum2=0;
		for(int j=0;j<ClassNum;j++)
			sum2+=Theta(V1,V2,j,i);

		Theta2+=(sum1*sum2);
	}

	double kappa1=Theta1-Theta2;
	if(kappa1!=0)
	{
		if(Theta2==1)
			cout<<"Theta2==1!"<<endl;

		kappa1/=(1-Theta2);
	}
// 	if(kappa1<0)
// 		printf("negative kappa!\n");
// 	if(kappa1>1)
// 		cout<<"kappa >1!"<<endl;

	return kappa1;
}

