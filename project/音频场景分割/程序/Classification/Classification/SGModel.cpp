#include "SGModel.h"
#include <fstream>
#include <cmath>

SGModel::SGModel(int vecDim)
{
	this->features.clear();
	this->featureMean.clear();
	this->featureVariance.clear();
	for (int i = 0; i < vecDim; i++) {
		this->featureMean.push_back(0);
		this->featureVariance.push_back(0);
	}
}

void SGModel::calParams()
{
	for (int i = 0; i < VEC_DIM; i++) {
		for (int j = 0; j < features.size(); j++) {
			featureMean.at(i) += features.at(j).at(i);
		}
		featureMean.at(i) /= features.size();
	}
	for (int i = 0; i < VEC_DIM; i++) {
		for (int j = 0; j < features.size(); j++) {
			featureVariance.at(i) += pow(features.at(j).at(i)-featureMean.at(i), 2);
		}
		featureVariance.at(i) /= features.size();
	}
}

float SGModel::score(vector<float> feature, featureTypes fType)
{
	int vecDim;
	if (fType == PROSODY) {
		vecDim = VEC_DIM_PROSODY;
	} else {
		vecDim = VEC_DIM;
	}
	float score = 0;
	for (int i = 0; i < vecDim; i++) {
		score -= log(sqrt(featureVariance.at(i)));
		score -= pow(feature.at(i)-featureMean.at(i), 2) / 2 / featureVariance.at(i);
	}
	return score;
}

void SGModel::outputParamToFile(featureTypes fType)
{
	ofstream fout;
	char *modelFilename;
	int length = strlen(classNames[(int)type].c_str()) + 15;
	modelFilename = new char[length];
	memset(modelFilename, 0, length);
	strcat(modelFilename, "model/SGM/");
	switch (fType)
	{
		case MFCC:
			strcat(modelFilename, "MFCC/");
			break;
		case LPCC:
			strcat(modelFilename, "LPCC/");
			break;
		case PLP:
			strcat(modelFilename, "PLP/");
			break;
		case PROSODY:
			strcat(modelFilename, "PROSODY/");
			break;
	}
	strcat(modelFilename, classNames[(int)type].c_str());
	strcat(modelFilename, ".mdl");
	fout.open(modelFilename);
	for (int i = 0; i < VEC_DIM; i++) {
		fout << featureMean.at(i) << " ";
	}
	fout << endl;
	for (int i = 0; i < VEC_DIM; i++) {
		fout << featureVariance.at(i) << " ";
	}
	fout << endl;
	fout.close();
}
