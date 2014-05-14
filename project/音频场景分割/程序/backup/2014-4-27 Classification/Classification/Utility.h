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
#define VEC_DIM 39							// dimension of feature vectore (12 mfccs/lpccs/plps and energy)
#define VEC_DIM_PROSODY 30					// pitch + formant : 6+24=30

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

typedef enum CLASSES
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
} classes;

typedef enum MODELS
{
	SGM = 0, 
	GMM, 
	SVMM
} models;

typedef enum SCOREMODES
{
	SUM = 0, 
	VOTE
} scoreModes ;

typedef enum FEATURETYPES
{
	MFCC = 0, 
	LPCC, 
	PLP, 
	PROSODY
} featureTypes;

struct TimeSeg
{
	int leftSample;
	int rightSample;
	int leftFrame;
	int rightFrame;
	float leftTime;
	float rightTime;
	classes type;
};

struct Frame
{
	double energy;							// Short-Time Average Energy of the frame
	int zcr;								// Short-Time Zero-Cross Rate of the frame
	classes type;
	vector<float> feature;					// features of the frame
	vector<double> scores;					// scores of all classes
};

struct Clip
{
	int leftFrame;
	int rightFrame;
	int leftClip;
	int rightClip;
	classes type;
};

#endif
