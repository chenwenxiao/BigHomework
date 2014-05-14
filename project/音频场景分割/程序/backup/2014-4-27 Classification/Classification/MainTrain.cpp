#include "Utility.h"
#include "Trainer.h"
#include <iostream>
using namespace std;

void main()
{
	models model = SGM;
	featureTypes fType = PROSODY;
// 	classes classesForTraining[] = {Silence, Voice, Speech, Ad, Music, SpeechMusic, AdMusic};
	classes classesForTraining[] = {Silence, Voice, Speech, Music, SpeechMusic, Ad, Song};
	int classesNum = 7;
	int mixNumOfGMM = 5;

	Trainer trainer(model, fType, classesForTraining, classesNum, mixNumOfGMM);
	trainer.Train();
}
