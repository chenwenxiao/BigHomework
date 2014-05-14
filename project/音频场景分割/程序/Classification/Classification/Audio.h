#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "Utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

extern string classNames[];

class Audio
{
public:
	string waveFilename;
	string featureFilename;
	string pitchFilename;
	string formantFilename;

	vector<Frame> frames;
	int frameNum;
	vector<Clip> clips;
	vector<vector<float>> prosodyFeatures;

	int classesNum;							// number of classes
	vector<TimeSeg> timeSegs;				// all time segments
	int timeSegsNum;						// number of segments
	vector<vector<TimeSeg>> cTimeSegs;		// various classes' time segments
	vector<int> cTimeSegsNum;				// numbers of various classes' time segments

	double eMax;							// maximum Energy

	Audio();
	~Audio();

	void SetClassesNum(int classesNum);

	void ReadFile(string waveFilename, string featureFilename, string pitchFilename, string formantFilename);
											// read .wav file and feature files (.mfcc/.lpcc/.plp file)
	void UpdateTimeSegs();					// identify/update time segments, based on types of frames

	void OutputToFile();					// output time segments to ".out" file

private:
	FILE *fp;
	double zero;							// zero point

	void ReadWaveFile();
	void ReadFeatureFiles();

	double E(int frameSize, short *buf);	// calculate Energy of a frame
	int ZCR(int frameSize, short *buf);		// calculate Zero-Cross Rate of a frame

	short sgn(double num);					// sgn function

	void Clear();
};

#endif
