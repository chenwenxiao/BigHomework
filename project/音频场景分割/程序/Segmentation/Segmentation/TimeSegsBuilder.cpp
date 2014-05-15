#include "TimeSegsBuilder.h"
#include "Audio.h"
#include <vector>

TimeSegsBuilder::TimeSegsBuilder()
{
	timeSegs.clear();
}

TimeSegsBuilder::~TimeSegsBuilder()
{

}

void TimeSegsBuilder::create(vector<Frame> frames, int frameNum)
{
	TimeSeg ts; 
	ts.leftFrame = 0;
	ts.leftSample = 0;
	ts.leftTime = 0;
	classes type = frames.at(0).type;
	for (int i = 1; i < frameNum; i++) {
		if (frames.at(i).type == type) {
			continue;
		}
		else {
			ts.rightFrame = i - 1;
			ts.rightSample = i * FRAME_SIZE - (i - 1) * OVERLAPPING_SIZE - OVERLAPPING_SIZE / 2 - 1;
			ts.rightTime = (float)(ts.rightSample + 1) / SAMPLE_RATE;
			ts.type = type;
			timeSegs.push_back(ts);
			type = frames.at(i).type;
			ts.leftFrame = i;
			ts.leftSample = i * FRAME_SIZE - (i - 1) * OVERLAPPING_SIZE - OVERLAPPING_SIZE / 2;
			ts.leftTime = ts.rightTime;
		}
	}
	ts.rightFrame = frameNum - 1;
	ts.rightSample = frameNum * FRAME_SIZE - (frameNum - 1) * OVERLAPPING_SIZE - 1;
	ts.rightTime = (float)(ts.rightSample + 1) / SAMPLE_RATE;
	ts.type = type;
	timeSegs.push_back(ts);
}

vector<TimeSeg> TimeSegsBuilder::product()
{
	return timeSegs;
}