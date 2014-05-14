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
	prosodyFeatures.clear();
	timeSegs.clear();
	cTimeSegs.clear();
	cTimeSegsNum.clear();
	zero = 0;
	eMax = 0;
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

void Audio::ReadFile(string waveFilename, string featureFilename,string pitchFilename, string formantFilename)
{
	this->waveFilename = waveFilename;
	this->featureFilename = featureFilename;
	this->pitchFilename = pitchFilename;
	this->formantFilename = formantFilename;
	Clear();
	ReadWaveFile();
	ReadFeatureFiles();
}

void Audio::ReadWaveFile()
{
	fp = fopen(waveFilename.c_str(), "rb");
	short *buf = new short[FRAME_SIZE];

	// find zero point
	fseek(fp, 44, SEEK_CUR);
	int frameread = 0;
	int sampleread = 0;
	while (frameread < FRAME_NUM_Z && feof(fp) == 0) {
		fread(buf, sizeof(short), FRAME_SIZE, fp);
		for (int i = 0; i < FRAME_SIZE; i++) {
			zero += buf[i];
			sampleread++;
		}
		frameread++;
	}
	zero /= sampleread;

	// read audio data, calculate Energy and Zero-Cross Rate
	fseek(fp, 44, SEEK_SET);
	double energy = 0;
	int zcr = 0;
	int numread;
	Frame frame;
	while (feof(fp) == 0) {
		numread = fread(buf, sizeof(short), FRAME_SIZE, fp);
		frame.energy = E(numread, buf);
		frame.zcr = ZCR(numread, buf);
		frame.type = Silence;
		frames.push_back(frame);
		if (feof(fp) == 0) {
			fseek(fp, 0-OVERLAPPING_SIZE*2, SEEK_CUR);
		}
	}
	fclose(fp);
	delete []buf;

	frames.pop_back();
	frameNum = frames.size();
}

void Audio::ReadFeatureFiles()
{
	// read mfcc/lpcc/plp feature file
	fp = fopen(featureFilename.c_str(), "rb");
	int cellSize = sizeof(float);
	int num = (_filelength(_fileno(fp)) - 12) / cellSize;
	float *buf = new float[num];
	fseek(fp, 12, SEEK_SET);
	fread(buf, cellSize, num, fp);
	fclose(fp);

	// get mfcc/lpcc/plp feature vector of each frame
	for (int i = 0; i < frameNum; i++) {
		frames.at(i).feature.clear();
		for (int j = 0; j < VEC_DIM; j++) {
			frames.at(i).feature.push_back(buf[i*VEC_DIM+j]);
		}
	}

	delete[] buf;

	// read pitch and formant file
	ifstream fin1, fin2;
	stringstream ss;
	string sTemp;
	float fTemp;
	vector<float> prosodyFeature;
	bool undefined;
	fin1.open(pitchFilename.c_str());
	fin2.open(formantFilename.c_str());
	while (!fin1.eof() && !fin2.eof()) {
		prosodyFeature.clear();
		undefined = false;
		for (int i = 0; i < 6; i++) {
			fin1 >> sTemp;
			if (sTemp == "--undefined--") {
				undefined = true;
				for (int j = i+1; j < 6; j++) {
					fin1 >> sTemp;
				}
				for (int j = 0; j < 24; j++) {
					fin2 >> sTemp;
				}
				break;
			} else {
				ss << sTemp;
				ss >> fTemp;
				ss.clear();
				prosodyFeature.push_back(fTemp);
			}
		}
		if (undefined) {
			prosodyFeature.clear();
			prosodyFeatures.push_back(prosodyFeature);
			continue;
		}
		undefined = false;
		for (int i = 0; i < 24; i++) {
			fin2 >> sTemp;
			if (sTemp == "--undefined--") {
				undefined = true;
				for (int j = i+1; j < 24; j++) {
					fin2 >> sTemp;
				}
				break;
			} else {
				ss << sTemp;
				ss >> fTemp;
				ss.clear();
				prosodyFeature.push_back(fTemp);
			}
		}
		if (undefined) {
			prosodyFeature.clear();
			prosodyFeatures.push_back(prosodyFeature);
			continue;
		}
		prosodyFeatures.push_back(prosodyFeature);
	}
	prosodyFeatures.pop_back();
	fin1.close();
	fin2.close();
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
	string outFilename = "result/" + waveFilename.substr(10, waveFilename.length()-14) + ".scores";
	Frame f;
	ofstream fout;
	fout.open(outFilename.c_str());
	fout << frames.size() << endl;
	for (int i = 0; i < frames.size(); i++) {
		f = frames.at(i);
		fout << f.type << " ";
		fout << f.scores.size();
		for (int j = 0; j < f.scores.size(); j++) {
			fout << " " << f.scores.at(j);
		}
		fout << endl;
	}
	fout.close();
}

double Audio::E(int frameSize, short *buf)
{
	double e = 0, avr_e = 0;
	for (int i = 0; i < frameSize; i++) {
		e = pow(buf[i]-zero, 2.0);
		avr_e += e;
		if (e > eMax) {
			eMax = e;
		}
	}
	avr_e /= frameSize;
	return avr_e;
}

int Audio::ZCR(int frameSize, short *buf)
{
	int zcr = 0;
	for (int i = 0; i < frameSize-1; i++) {
		zcr += abs(sgn(buf[i+1]-zero)-sgn(buf[i]-zero));
	}
	zcr /= 2;
	return zcr;
}

short Audio::sgn(double num)
{
	if (num > 0) {
		return 1;
	} else if (num < 0) {
		return -1;
	} else {
		return 0;
	}
}

void Audio::Clear()
{
	frames.clear();
	clips.clear();
	prosodyFeatures.clear();
	timeSegs.clear();
	for (int i = 0; i < classesNum+3; i++) {
		cTimeSegs.at(i).clear();
	}
	cTimeSegsNum.clear();
	zero = 0;
	eMax = 0;
}
