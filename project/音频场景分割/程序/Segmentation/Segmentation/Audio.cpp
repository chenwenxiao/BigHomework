#include "Audio.h"
#include "io.h"
#include <iostream>
#include <cmath>
#include <sstream>
using namespace std;

Audio::Audio()
{
	frames.clear();
	clips.clear();
	timeSegs.clear();
	cTimeSegs.clear();
	cTimeSegsNum.clear();
}

Audio::~Audio()
{
}

void Audio::SetClassesNum(int classesNum)
{
	this->classesNum = classesNum;
	vector<TimeSeg> newTimeSegs;
	for (int i = 0; i < classesNum+3; i++) {
		cTimeSegs.push_back(newTimeSegs);
	}
	for (int i = 0; i < classesNum+3; i++) {
		cTimeSegs.at(i).clear();
	}
}

void Audio::ReadScoreFile(string scoresFilename)
{
	this->scoresFilename = scoresFilename;
	Clear();

	ifstream fin;
	int iTemp;
	double dTemp;
	Frame frame;
	fin.open(scoresFilename.c_str());
	fin >> iTemp;
	frameNum = iTemp;
	for (int i = 0; i < frameNum; i++) {
		fin >> iTemp;
		frame.type = types[iTemp];
		frame.scores.clear();
		fin >> iTemp;
		for (int j = 0; j < iTemp; j++) {
			fin >> dTemp;
			frame.scores.push_back(dTemp);
		}
		frames.push_back(frame);
	}
	fin.close();
}

void Audio::UpdateTimeSegs()
{
	timeSegs.clear();
	TimeSeg ts;
	ts.leftFrame = 0;
	ts.leftSample = 0;
	ts.leftTime = 0;
	classes type = frames.at(0).type;
	for (int i = 1; i < frameNum; i++) {
		if (frames.at(i).type == type) {
			continue;
		} else {
			ts.rightFrame = i - 1;
			ts.rightSample = i*FRAME_SIZE - (i-1)*OVERLAPPING_SIZE - OVERLAPPING_SIZE/2 - 1;
			ts.rightTime = (float)(ts.rightSample+1) / SAMPLE_RATE;
			ts.type = type;
			timeSegs.push_back(ts);
			type = frames.at(i).type;
			ts.leftFrame = i;
			ts.leftSample = i*FRAME_SIZE - (i-1)*OVERLAPPING_SIZE - OVERLAPPING_SIZE/2;
			ts.leftTime = ts.rightTime;
		}
	}
	ts.rightFrame = frameNum-1;
	ts.rightSample = frameNum*FRAME_SIZE - (frameNum-1)*OVERLAPPING_SIZE - 1;
	ts.rightTime = (float)(ts.rightSample+1) / SAMPLE_RATE;
	ts.type = type;
	timeSegs.push_back(ts);
	timeSegsNum = timeSegs.size();

	for (int i = 0; i < classesNum+3; i++) {
		cTimeSegs.at(i).clear();
	}
	cTimeSegsNum.clear();
	for (int i = 0; i < timeSegs.size(); i++) {
		cTimeSegs.at((int)(timeSegs.at(i).type)).push_back(timeSegs.at(i));
	}
	for (int i = 0; i < classesNum+3; i++) {
		cTimeSegsNum.push_back(cTimeSegs.at(i).size());
	}
}

void Audio::OutputToFile()
{
	string outFilename = "result/" + scoresFilename.substr(5, scoresFilename.length()-12) + ".out";
	ofstream fout;
	fout.open(outFilename.c_str());
	fout << timeSegs.size() << endl;
	for (int i = 0; i < timeSegs.size(); i++) {
		fout << timeSegs.at(i).leftTime << " " 
			<< timeSegs.at(i).rightTime << " " 
			<< classNames[(int)timeSegs.at(i).type] << " " 
			<< timeSegs.at(i).rightTime-timeSegs.at(i).leftTime << endl;
	}
	fout.close();
}

void Audio::Clear()
{
	frames.clear();
	clips.clear();
	timeSegs.clear();
	for (int i = 0; i < classesNum+3; i++) {
		cTimeSegs.at(i).clear();
	}
	cTimeSegsNum.clear();
}
