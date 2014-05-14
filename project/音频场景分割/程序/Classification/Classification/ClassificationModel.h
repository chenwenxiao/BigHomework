#ifndef _CLASSIFICATIONMODEL_H_
#define _CLASSIFICATIONMODEL_H_

#include "Utility.h"
#include <vector>
using namespace std;

extern string classNames[];

class ClassificationModel
{
public :
	ClassificationModel() {};
	classes type;
	vector<vector<float>> features;
	virtual float score(vector<float> feature, featureTypes fType) = 0;
	virtual void outputParamToFile(featureTypes fType) = 0;
};

#endif
