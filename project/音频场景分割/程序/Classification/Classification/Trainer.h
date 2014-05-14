#ifndef _TRAINER_H_
#define _TRAINER_H_

#include "ManualLabel.h"
#include "SGModel.h"
#include "GMModel.h"
#include <io.h>
#include <vector>
#include <string>
using namespace std;

class Trainer
{
public:
	models model;
	featureTypes fType;
	int classesNum;

	vector<ClassificationModel*> cModels;

	Trainer(models model, featureTypes fType, classes classesForTraining[], int classesNum, int mixNumOfGMM = 1);
	~Trainer();

	void Train();

private:
	FILE *fp;
	vector<vector<float>> features;

	ManualLabel manualLabel;

	void Read();
	void ReadAFile(string manualLabelFilename, string featureFilename, string featureFilename2);
	void OutputToFile();					// output features to file for SVM training

};

#endif
