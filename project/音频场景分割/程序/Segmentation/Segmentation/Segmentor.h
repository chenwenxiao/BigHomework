#ifndef _SEGMENTOR_H_
#define _SEGMENTOR_H_

#include "Audio.h"
#include "ManualLabel.h"
#include <fstream>
#include <string>
#include <vector>
using namespace std;

extern classes types[];
extern string classNames[];

class Segmentor
{
public:
	int dataNum;
	vector<Audio *> audios;
	vector<ManualLabel *> manualLabels;
	vector<Audio *> audiosTemp;
	vector<ManualLabel *> manualLabelsTemp;

	Segmentor(int classesNum);
	~Segmentor();

	void AddData(string scoresFilename, string manualLabelFilename);
	void ProcessAll();

private:
	ofstream fout;
	ifstream fin;

	int classesNum;

	float totalManualTimeAll;
	float totalManualTime;
	vector<float> cTotalManualTime;
	float totalDetectTime;
	vector<float> cTotalDetectTime;
	float totalCorrectTime;
	vector<float> cTotalCorrectTime;
	float totalWrongTime;
	vector<float> cTotalWrongTime;
	vector<vector<float>> misclassifiedTime;
	vector<vector<float>> totalMisclassifiedTime;

	float epsilon;

	float FindThreshold();
	void Modify(float th, bool isThDone);
	void CalFalseAR(float &fa, float &fr);

	void Process(int dataNo);
	void CutByRules(int dataNo);			// modify some clips' types, based on some rules

	void CalErrorRate(int dataNo);			// calculate error rate, based on time length
	void OutputTotalErrorRateToFile();

	bool BelongsTo(TimeSeg tsA, TimeSeg tsB);

	void Clear();
};

#endif
