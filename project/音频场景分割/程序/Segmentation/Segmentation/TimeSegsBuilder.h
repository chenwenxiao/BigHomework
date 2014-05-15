#ifndef _TIMESEGSBUILDER_H
#define _TIMESEGSBUILDER_H

#include <vector>
#include "Audio.h"

class TimeSegsBuilder
{
public:
	TimeSegsBuilder();
	virtual ~TimeSegsBuilder();
	void create(vector<Frame> frames, int frameNum);
	vector<TimeSeg> product();
private:
	vector<TimeSeg> timeSegs;
};


#endif
