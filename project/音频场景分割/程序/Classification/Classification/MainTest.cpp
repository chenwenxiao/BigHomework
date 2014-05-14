#include "Utility.h"
#include "Trainer.h"
#include <iostream>
using namespace std;

void main()
{
	models model = GMM;
	featureTypes fType = MFCC;
// 	classes classesForTraining[] = {Silence, Voice, Speech, Ad, Music, SpeechMusic, AdMusic};
	classes classesForTraining[] = {Speech, Music, Ad};//, Music, SpeechMusic, Ad, Song};
	int classesNum = 3;
	int mixNumOfGMM = 1;

	Trainer trainer(model, fType, classesForTraining, classesNum, mixNumOfGMM);
	trainer.Train();
}

/*#include "Classifier.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void main()
{
	ofstream fout;
	ifstream fin;
	int data_num;
	string waveFilename;
	string manualLabelFilename;
	string featureFilename;
	string pitchFilename;
	string formantFilename;

	models model = SVMM;
	int classesNum = 7;
	scoreModes scoreMode = VOTE;

	featureTypes fType = MFCC;
	string datalistFilename;
	switch (fType)
	{
		case MFCC:
			datalistFilename = "data/test/datalist_mfcc.txt";
			break;
		case LPCC:
			datalistFilename = "data/test/datalist_lpcc.txt";
			break;
		case PLP:
			datalistFilename = "data/test/datalist_plp.txt";
			break;
	}

	Classifier classifier(model, classesNum, scoreMode, fType);

	fin.open(datalistFilename.c_str());
	fin >> data_num;
	for (int i = 0; i < data_num; i++) {
		fin >> waveFilename;
		fin >> manualLabelFilename;
		fin >> featureFilename;
		fin >> pitchFilename;
		fin >> formantFilename;
		cout << "processing...... " << waveFilename << endl;
		classifier.audio.ReadFile(waveFilename, featureFilename, pitchFilename, formantFilename);
 		classifier.Process();
	}
	cout << "done!" << endl;
	fin.close();
}
*/