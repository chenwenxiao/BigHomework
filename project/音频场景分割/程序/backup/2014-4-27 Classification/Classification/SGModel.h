#ifndef _SGMODEL_H_
#define _SGMODEL_H_

#include "Utility.h"
#include "ClassificationModel.h"
#include <vector>
using namespace std;

extern string classNames[];

class SGModel : public ClassificationModel
{
public:
	SGModel() {};
	SGModel(int vecDim);
	vector<float> featureMean;
	vector<float> featureVariance;
	void calParams();
	float score(vector<float> feature, featureTypes fType);
	void outputParamToFile(featureTypes fType);
};

#endif
