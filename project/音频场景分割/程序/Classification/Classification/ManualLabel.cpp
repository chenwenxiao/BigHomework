#include "ManualLabel.h"
#include <iostream>
#include <cmath>
using namespace std;

ManualLabel::ManualLabel()
{
	timeSegs.clear();
	cTimeSegs.clear();
	cTimeSegsNum.clear();
	frameTypes.clear();
	cClipNums.clear();
}

ManualLabel::~ManualLabel()
{
}

void ManualLabel::SetClassesNum(int classesNum)
{
	this->classesNum = classesNum;
	vector<TimeSeg> newTimeSegs;
	newTimeSegs.clear();
	for (int i = 0; i < classesNum+3; i++) {
		cTimeSegs.push_back(newTimeSegs);
		cClipNums.push_back(0);
	}
}

void ManualLabel::ReadFile(string filename)
{
	Clear();
	int timeSegsNum;
	float time;
	string className;
	TimeSeg ts;

	fin.open(filename.c_str());
	fin >> timeSegsNum;
	this->timeSegsNum = timeSegsNum;
	for (int i = 0; i < timeSegsNum; i++) {
		fin >> time;
		ts.leftTime = time;
		fin >> time;
		ts.rightTime = time;
		fin >> className;
		for (int j = 0; j < classesNum; j++) {
			if (className == classNames[j]) {
				ts.type = types[j];
				cTimeSegs.at(j).push_back(ts);
			}
		}
		timeSegs.push_back(ts);
		if (className != "silence" && className != "voice") {
			ts.type = Voice;
			cTimeSegs.at((int)Voice).push_back(ts);
		}
	}
	fin.close();

	GetMoreInfo(&timeSegs);
	for (int i = 0; i < classesNum+3; i++) {
		GetMoreInfo(&cTimeSegs.at(i));
		cTimeSegsNum.push_back(cTimeSegs.at(i).size());
	}

	int sampleNum = time * SAMPLE_RATE;
	int frameNum = ceil((double)(sampleNum-FRAME_SIZE) / (FRAME_SIZE-OVERLAPPING_SIZE)) + 1;
	for (int i = 0; i < frameNum; i++) {
		frameTypes.push_back(Silence);
	}
	for (int i = 2; i < classesNum+3; i++) {
		for (int j = 0; j < cTimeSegs.at(i).size(); j++) {
			ts = cTimeSegs.at(i).at(j);
			for (int k = ts.leftFrame; k <= ts.rightFrame; k++) {
				frameTypes.at(k) = ts.type;
			}
		}
	}

	int m;
	for (int i = 0; i < classesNum+3; i++) {
		cClipNums.push_back(0);
	}
	for (int i = 2; i < classesNum+3; i++) {
		for (int j = 0; j < cTimeSegs.at(i).size(); j++) {
			ts = cTimeSegs.at(i).at(j);
			for (int k = ts.leftFrame; k <= ts.rightFrame; k += (FRAME_NUM_CLIP-FRAME_NUM_OVERLAP)) {
				cClipNums.at(i)++;
			}
		}
	}
}

void ManualLabel::GetMoreInfo(vector<TimeSeg> *tss)
{
	for (int i = 0; i < tss->size(); i++) {
		tss->at(i).leftSample = tss->at(i).leftTime * SAMPLE_RATE;
		if (tss->at(i).leftSample < FRAME_SIZE) {
			tss->at(i).leftFrame = 0;
		} else {
			tss->at(i).leftFrame = ceil((double)(tss->at(i).leftSample+1-FRAME_SIZE) / (FRAME_SIZE-OVERLAPPING_SIZE));
		}
		tss->at(i).rightSample = tss->at(i).rightTime * SAMPLE_RATE - 1;
		if (tss->at(i).rightSample < FRAME_SIZE) {
			tss->at(i).rightFrame = 0;
		} else {
			tss->at(i).rightFrame = ceil((double)(tss->at(i).rightSample+1-FRAME_SIZE) / (FRAME_SIZE-OVERLAPPING_SIZE)) - 1;
		}
	}
}

void ManualLabel::Clear()
{
	timeSegs.clear();
	for (int i = 0; i < classesNum+3; i++) {
		cTimeSegs.at(i).clear();
	}
	cTimeSegsNum.clear();
	frameTypes.clear();
	cClipNums.clear();
}
