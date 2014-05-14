#include "Utility.h"
#include <string>

// classes types[] = 
// {
// 	Silence, 
// 	Voice, 
// 	Speech, 
// 	Ad, 
// 	Music, 
// 	SpeechMusic, 
// 	AdMusic, 
// 	WithMusic, 
// 	WithoutMusic
// };
// 
// string classNames[] = 
// {
// 	"silence", 
// 	"voice", 
// 	"speech", 
// 	"ad", 
// 	"music", 
// 	"speechmusic", 
// 	"admusic", 
// 	"withmusic", 
// 	"withoutmusic"
// };

classes types[] = 
{
	Silence, 
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

string classNames[] = 
{
	"silence", 
	"voice", 
	"speech", 
	"music", 
	"speechmusic", 
	"ad", 
	"song", 
	"withmusic", 
	"withoutmusic", 
	"unknown"
};

string modelListFilenamesMFCC[] = 
{
	"model/SGM/MFCC/modellist.txt", 
	"model/GMM/MFCC/modellist.txt"
};

string modelListFilenamesLPCC[] = 
{
	"model/SGM/LPCC/modellist.txt", 
	"model/GMM/LPCC/modellist.txt"
};

string modelListFilenamesPLP[] = 
{
	"model/SGM/PLP/modellist.txt", 
	"model/GMM/PLP/modellist.txt"
};

string modelListFilenamesPROSODY[] = 
{
	"model/SGM/PROSODY/modellist.txt", 
	"model/GMM/PROSODY/modellist.txt"
};
