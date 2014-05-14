#include "GMModel.h"
#include <fstream>
#include <cmath>
using namespace std;

GMModel::GMModel(int mixNumOfGMM, int vecDim)
{
	this->features.clear();
	this->mixNum = mixNumOfGMM;
	this->alpha.clear();
	this->featureMean.clear();
	this->featureCovariance.clear();

	vector<float> tempFVec;
	tempFVec.clear();
	for (int j = 0; j < vecDim; j++) {
		tempFVec.push_back(0);
	}
	for (int i = 0; i < mixNumOfGMM; i++) {
		this->alpha.push_back(0);
		this->featureMean.push_back(tempFVec);
		this->featureCovariance.push_back(tempFVec);
	}
}

float GMModel::score(vector<float> feature, featureTypes fType)
{
	int vecDim;
	if (fType == PROSODY) {
		vecDim = VEC_DIM_PROSODY;
	} else {
		vecDim = VEC_DIM;
	}
	float s = 0;
	for (int i = 0; i < vecDim; i++) {
		for (int j = 0; j < mixNum; j++) {
			s += log(alpha.at(j));
			s -= log(sqrt(featureCovariance.at(j).at(i)));
			s -= pow(feature.at(i)-featureMean.at(j).at(i), 2) / 2 / featureCovariance.at(j).at(i);
		}
	}
	return s;
}

double GMModel::p(vector<float> feature, int mixNo, featureTypes fType)
{
	int vecDim;
	if (fType == PROSODY) {
		vecDim = VEC_DIM_PROSODY;
	} else {
		vecDim = VEC_DIM;
	}

	double result = 1;
	double pp;
	for (int i = 0; i < vecDim; i++) {
		pp = pow((double)feature.at(i)-featureMean.at(mixNo).at(i), 2) / 2;
		pp = pp / featureCovariance.at(mixNo).at(i);
		pp = 0 - pp;
		pp = exp(pp);
		pp = pp / sqrt(2*PI*featureCovariance.at(mixNo).at(i));
		result *= pp;
	}
	return result;
}

void GMModel::outputParamToFile(featureTypes fType)
{
	int vecDim;
	if (fType == PROSODY) {
		vecDim = VEC_DIM_PROSODY;
	} else {
		vecDim = VEC_DIM;
	}

	ofstream fout;
	char *modelFilename;
	int length = strlen(classNames[(int)type].c_str()) + 15;
	modelFilename = new char[length];
	memset(modelFilename, 0, length);
	strcat(modelFilename, "model/GMM/");
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
	fout << mixNum << endl;
	for (int i = 0; i < mixNum; i++) {
		fout << alpha.at(i) << endl;
		for (int j = 0; j < vecDim; j++) {
			fout << featureMean.at(i).at(j) << " ";
		}
		fout << endl;
		for (int j = 0; j < vecDim; j++) {
			fout << featureCovariance.at(i).at(j) << " ";
		}
		fout << endl;
	}
	fout.close();
}

void GMModel::GMMInit(featureTypes fType)
{
	GMModel *gmm = this;
	float epsilon = 1.0;
	bool ifBreak = false;
	vector<vector<float>> centers;
	vector<vector<float>> newCenters;
	for (int i = 0; i < gmm->mixNum; i++) {
		vector<float> Vf = gmm->features.at(i);
		centers.push_back(Vf);
		newCenters.push_back(Vf);
	}
	int minDistNo;
	int tempInt;
	vector<vector<int>> featureNos;
	vector<int> tempIVec;
	featureNos.clear();
	for (int i = 0; i < gmm->mixNum; i++) {
		featureNos.push_back(tempIVec);
	}

	int vecDim;
	if (fType == PROSODY) {
		vecDim = VEC_DIM_PROSODY;
	}
	else {
		vecDim = VEC_DIM;
	}

	while (true) {
		for (int i = 0; i < gmm->mixNum; i++) {
			featureNos.at(i).clear();
		}
		for (int i = 0; i < gmm->features.size(); i++) {
			minDistNo = MinDistNo(gmm->features.at(i), centers);
			featureNos.at(minDistNo).push_back(i);
		}

		for (int i = 0; i < gmm->mixNum; i++) {
			for (int j = 0; j < vecDim; j++) {
				newCenters.at(i).at(j) = 0;
				for (int k = 0; k < featureNos.at(i).size(); k++) {
					tempInt = featureNos.at(i).at(k);
					newCenters.at(i).at(j) += gmm->features.at(tempInt).at(j);
				}
				newCenters.at(i).at(j) /= featureNos.at(i).size();
			}
		}

		for (int i = 0; i < gmm->mixNum; i++) {
			if (Distance(newCenters.at(i), centers.at(i)) < epsilon) {
				ifBreak = true;
			}
			for (int j = 0; j < vecDim; j++) {
				centers.at(i).at(j) = newCenters.at(i).at(j);
			}
		}
		if (ifBreak) {
			break;
		}
	}

	for (int i = 0; i < gmm->mixNum; i++) {
		gmm->alpha.at(i) = (float)featureNos.at(i).size() / gmm->features.size();
		for (int j = 0; j < vecDim; j++) {
			gmm->featureMean.at(i).at(j) = centers.at(i).at(j);
			for (int k = 0; k < featureNos.at(i).size(); k++) {
				tempInt = featureNos.at(i).at(k);
				gmm->featureCovariance.at(i).at(j) += pow(gmm->features.at(tempInt).at(j) - centers.at(i).at(j), 2);
			}
			gmm->featureCovariance.at(i).at(j) /= featureNos.at(i).size();
		}
	}
}

void GMModel::GMMIterate(featureTypes fType)
{
	GMModel *gmm = this;
	vector<vector<double>> p;
	vector<double> tempFVec;
	for (int i = 0; i < gmm->features.size(); i++) {
		tempFVec.push_back(0);
	}
	for (int i = 0; i < gmm->mixNum; i++) {
		p.push_back(tempFVec);
	}
	double pSum;

	int vecDim;
	if (fType == PROSODY) {
		vecDim = VEC_DIM_PROSODY;
	}
	else {
		vecDim = VEC_DIM;
	}

	int n = 0;
	while (n < 11) {
		for (int i = 0; i < gmm->features.size(); i++) {
			pSum = 0;
			for (int j = 0; j < gmm->mixNum; j++) {
				p.at(j).at(i) = gmm->alpha.at(j) * gmm->p(gmm->features.at(i), j, fType);
				pSum += p.at(j).at(i);
			}
			for (int j = 0; j < gmm->mixNum; j++) {
				p.at(j).at(i) /= pSum;
			}
		}

		for (int i = 0; i < gmm->mixNum; i++) {
			gmm->alpha.at(i) = 0;
			for (int j = 0; j < vecDim; j++) {
				gmm->featureMean.at(i).at(j) = 0;
				gmm->featureCovariance.at(i).at(j) = 0;
			}
			for (int j = 0; j < gmm->features.size(); j++) {
				gmm->alpha.at(i) += p.at(i).at(j);
				for (int k = 0; k < vecDim; k++) {
					gmm->featureMean.at(i).at(k) += p.at(i).at(j) * gmm->features.at(j).at(k);
				}
			}
			pSum = gmm->alpha.at(i);
			gmm->alpha.at(i) /= gmm->features.size();
			for (int j = 0; j < vecDim; j++) {
				gmm->featureMean.at(i).at(j) /= pSum;
			}
			for (int j = 0; j < gmm->features.size(); j++) {
				for (int k = 0; k < vecDim; k++) {
					gmm->featureCovariance.at(i).at(k) += p.at(i).at(j) * pow(gmm->features.at(j).at(k) - gmm->featureMean.at(i).at(k), 2);
				}
			}
			for (int j = 0; j < vecDim; j++) {
				gmm->featureCovariance.at(i).at(j) /= pSum;
			}
		}

		n++;
	}
}

float GMModel::Distance(vector<float> vecA, vector<float> vecB)
{
	float dist = 0;
	for (int i = 0; i < vecA.size(); i++) {
		dist += pow(vecA.at(i) - vecB.at(i), 2);
	}
	dist = sqrt(dist);
	return dist;
}

int GMModel::MinDistNo(vector<float> vecA, vector<vector<float>> vecs)
{
	int no = 0;
	float minDist = Distance(vecA, vecs.at(0));
	float temp;
	for (int i = 1; i < vecs.size(); i++) {
		temp = Distance(vecA, vecs.at(i));
		if (temp < minDist) {
			minDist = temp;
			no = i;
		}
	}
	return no;
}
