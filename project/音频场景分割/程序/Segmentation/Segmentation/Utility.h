#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <vector>
using namespace std;

#define PI 3.14159265
#define SAMPLE_RATE 44100					// sample rate = 44.1kHz
#define FRAME_SIZE 1024						// number of samples in a frame
#define OVERLAPPING_SIZE 256				// number of overlapping samples of two frames
#define FRAME_NUM_CLIP 50					// number of frames in a clip
#define FRAME_NUM_OVERLAP 20				// number of overlapping frames of two clips
#define FRAME_NUM_Z 150						// number of frames for calculating to find zero point
#define FRAME_NUM_FILT 30					// filtrating these silence frames between voice frames

// typedef enum
// {
// 	Silence = 0, 
// 	Voice, 
// 	Speech, 
// 	Ad, 
// 	Music, 
// 	SpeechMusic, 
// 	AdMusic, 
// 	WithMusic, 
// 	WithoutMusic
// } classes;
enum classes
{
	Silence = 0,
	Voice,
	Speech,
	Music,
	SpeechMusic,
	Ad,
	Song,
	WithMusic,
	WithoutMusic,
	Unknown
};

class TimeSeg
{
public:
	int leftSample;
	int rightSample;
	int leftFrame;
	int rightFrame;
	float leftTime;
	float rightTime;
	classes type;
};

class Frame
{
public:
	classes type;
	vector<double> scores;					// scores of all classes
};

class Clip
{
public:
	int leftFrame;
	int rightFrame;
	int leftClip;
	int rightClip;
	classes type;
};

#endif
