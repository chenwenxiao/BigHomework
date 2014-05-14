#ifndef _CLASSIFIER_H_
#define _CLASSIFIER_H_

#include "Audio.h"
#include "SGModel.h"
#include "GMModel.h"
#include <fstream>
#include <string>
#include <vector>
using namespace std;

extern classes types[];
extern string classNames[];
extern string modelListFilenamesMFCC[];
extern string modelListFilenamesLPCC[];
extern string modelListFilenamesPLP[];
extern string modelListFilenamesPROSODY[];

class Classifier
{
public:
	Audio audio;

	Classifier(models model, int classesNum, scoreModes scoreMode, featureTypes fType);
	~Classifier();

	void Process();							// main processing part

private:
	ofstream fout;
	ifstream fin;

	models model;
	int classesNum;
	scoreModes scoreMode;
	featureTypes fType;

	vector<SGModel> cSGModels;
	vector<GMModel> cGMModels;

	void ReadModelFiles();					// read .mdl files

	void VAD();								// Voice Activity Detection algorithm
	void Filtrate();						// filtrate few silence frames between the voice frames
	void CompareSVModels();					// compare scores of s/v models, modify some silence frames' types

	void OutputSVMFeatureFiles1();			// Level 1 output prosody features, for SVM_1
	void OutputSVMFeatureFiles2();			// Level 2 output mfcc/lpcc/plp features, for SVM_2, SVM_3 and SVM_4

	void ClassifyLevel1();					// using prosody features, to "WithMusic" and "WithoutMusic"
	void ClassifyLevel2();					// using mfcc/lpcc/plp features

	int GetBestClass(vector<vector<float>> scores, int frameNum, int validClassNo);
	int MaxNo(vector<double> scores);
};

#endif
