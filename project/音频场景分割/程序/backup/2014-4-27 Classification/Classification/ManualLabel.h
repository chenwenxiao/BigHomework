#ifndef _MANUALLABEL_H_
#define _MANUALLABEL_H_

#include "Utility.h"
#include <fstream>
#include <vector>
#include <string>
using namespace std;

extern classes types[];
extern string classNames[];

class ManualLabel
{
public:
	int classesNum;							// number of classes
	vector<TimeSeg> timeSegs;				// all time segments
	int timeSegsNum;						// number of segments
	vector<vector<TimeSeg>> cTimeSegs;		// various classes' time segments
	vector<int> cTimeSegsNum;				// number of various classes' time segments
	vector<classes> frameTypes;				// type of every frame
	vector<int> cClipNums;					// number of various classes' clips

	ManualLabel();
	~ManualLabel();

	void SetClassesNum(int classesNum);

	void ReadFile(string filename);			// read .tss file

private:
	ifstream fin;

	void GetMoreInfo(vector<TimeSeg> *tss);	// get more information of a time segment

	void Clear();
};

#endif
