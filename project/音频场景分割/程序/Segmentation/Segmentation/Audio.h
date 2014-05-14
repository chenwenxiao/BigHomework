#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "Utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

extern classes types[];
extern string classNames[];

class Audio
{
public:
	string scoresFilename;

	vector<Frame> frames;
	int frameNum;
	vector<Clip> clips;

	int classesNum;							// number of classes
	vector<TimeSeg> timeSegs;				// all time segments
	int timeSegsNum;						// number of segments
	vector<vector<TimeSeg>> cTimeSegs;		// various classes' time segments
	vector<int> cTimeSegsNum;				// numbers of various classes' time segments

	Audio();
	~Audio();

	void SetClassesNum(int classesNum);
	void ReadScoreFile(string scoresFilename);
	void UpdateTimeSegs();					// identify/update time segments, based on types of frames
	void OutputToFile();					// output time segments to ".out" file

private:
	void Clear();
};

#endif
