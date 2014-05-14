#ifndef _GMMODEL_H_
#define _GMMODEL_H_

#include "Utility.h"
#include "ClassificationModel.h"
#include <vector>
using namespace std;

extern string classNames[];

class GMModel : public ClassificationModel
{
private:
	float Distance(vector<float> vecA, vector<float> vecB);
	int MinDistNo(vector<float> vecA, vector<vector<float>> vecs);

public:
	GMModel() {};
	GMModel(int mixNumOfGMM, int vecDim);
	int mixNum;
	vector<float> alpha;
	vector<vector<float>> featureMean;
	vector<vector<float>> featureCovariance;
	double p(vector<float> feature, int mixNo, featureTypes fType);
	float score(vector<float> feature, featureTypes fType);

	void GMMInit(featureTypes fType);				// GMM: initialize the parameters of a GMM (k-means algorithm)
	void GMMIterate(featureTypes fType);			// GMM: train by iterating (EM algorithm)


	void outputParamToFile(featureTypes fType);
};

#endif
