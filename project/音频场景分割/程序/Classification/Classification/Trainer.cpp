#include "Trainer.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>
using namespace std;

Trainer::Trainer(models model, featureTypes fType, classes classesForTraining[], int classesNum, int mixNumOfGMM)
{
	this->model = model;
	this->fType = fType;
	this->classesNum = classesNum;
	manualLabel.SetClassesNum(classesNum);

	int vecDim;
	if (fType == PROSODY) {
		vecDim = VEC_DIM_PROSODY;
	} else {
		vecDim = VEC_DIM;
	}

	cSGModels.clear();
	SGModel cSGModel;
	cGMModels.clear();
	GMModel cGMModel;

	switch (model)
	{
		case SGM:
			for (int i = 0; i < classesNum; i++) {
				cSGModel.type = classesForTraining[i];
				cSGModels.push_back(cSGModel);
			}
			cSGModel.type = WithMusic;
			cSGModels.push_back(cSGModel);
			cSGModel.type = WithoutMusic;
			cSGModels.push_back(cSGModel);
			break;

		case GMM:
			for (int i = 0; i < classesNum; i++) {
				cGMModel.type = classesForTraining[i];
				cGMModels.push_back(cGMModel);
			}
			cGMModel.type = WithMusic;
			cGMModels.push_back(cGMModel);
			cGMModel.type = WithoutMusic;
			cGMModels.push_back(cGMModel);
			break;
	}
}

Trainer::~Trainer()
{
}

void Trainer::Train()
{
	ofstream fout;
	if (model == SVMM) {
		if (fType == PROSODY) {
			fout.open("svm/svm_trainset_1.txt");
			fout.close();
		} else {
			fout.open("svm/svm_trainset_2.txt");
			fout.close();
			fout.open("svm/svm_trainset_3.txt");
			fout.close();
			fout.open("svm/svm_trainset_4.txt");
			fout.close();
		}
	}

	Read();
	cout << "training......" << endl;
	switch (model)
	{
		case SGM:
			if (fType == PROSODY) {
				for (int i = classesNum; i < classesNum+2; i++) {
					cSGModels.at(i).calParams();
				}
			} else {
				for (int i = 0; i < classesNum; i++) {
					cSGModels.at(i).calParams();
				}
			}
			OutputToFile();
			cout << "done!" << endl;
			break;

		case GMM:
			cout << "classesNum = " << classesNum << endl;
			if (fType == PROSODY) {
				for (int i = classesNum; i < classesNum+2; i++) {
					cGMModels.at(i).GMMInit(fType);
					cGMModels.at(i).GMMIterate(fType);
					cout << "GMMIterate done ~ " << i << endl;
				}
			} else {
				for (int i = 0; i < classesNum; i++) {
					cGMModels.at(i).GMMInit(fType);
					cGMModels.at(i).GMMIterate(fType);
					cout << "GMMIterate done ~ " << i << endl;
				}
			}
			OutputToFile();
			cout << "done!" << endl;
			break;
	}
}

void Trainer::Read()
{
	ifstream fin;
	int data_num;
	string datalistFilename;
	string waveFilename;
	string manualLabelFilename;
	string featureFilename;					// mfcc/lpcc/plp file, or pitch file when feature type is PROSODY
	string featureFilename2;				// formant file when feature type is PROSODY
	switch (fType)
	{
		case MFCC:
			datalistFilename = "data/train/datalist_mfcc.txt";
			break;
		case LPCC:
			datalistFilename = "data/train/datalist_lpcc.txt";
			break;
		case PLP:
			datalistFilename = "data/train/datalist_plp.txt";
			break;
		case PROSODY:
			datalistFilename = "data/train/datalist_prosody.txt";
			break;
	}
	fin.open(datalistFilename.c_str());
	fin >> data_num;
	for (int i = 0; i < data_num; i++) {
		fin >> waveFilename;
		fin >> manualLabelFilename;
		fin >> featureFilename;
		if (fType == PROSODY) {
			fin >> featureFilename2;
		} else {
			featureFilename2 = "";
		}
		cout << "reading...... " << waveFilename << endl;
		ReadAFile(manualLabelFilename, featureFilename, featureFilename2);
	}
	fin.close();
}

void Trainer::ReadAFile(string manualLabelFilename, string featureFilename, string featureFilename2)
{
	// read manual label file
	manualLabel.ReadFile(manualLabelFilename);
	vector<float> feature;

	if (fType == PROSODY) {
		ifstream fin1, fin2;
		stringstream ss;
		string sTemp;
		float fTemp;
		bool undefined;
		features.clear();
		fin1.open(featureFilename.c_str());
		fin2.open(featureFilename2.c_str());
		while (!fin1.eof() && !fin2.eof()) {
			feature.clear();
			undefined = false;
			for (int i = 0; i < 6; i++) {
				fin1 >> sTemp;
				if (sTemp == "--undefined--") {
					undefined = true;
					for (int j = i+1; j < 6; j++) {
						fin1 >> sTemp;
					}
					for (int j = 0; j < 24; j++) {
						fin2 >> sTemp;
					}
					break;
				} else {
					ss << sTemp;
					ss >> fTemp;
					ss.clear();
					feature.push_back(fTemp);
				}
			}
			if (undefined) {
				feature.clear();
				features.push_back(feature);
				continue;
			}
			undefined = false;
			for (int i = 0; i < 24; i++) {
				fin2 >> sTemp;
				if (sTemp == "--undefined--") {
					undefined = true;
					for (int j = i+1; j < 24; j++) {
						fin2 >> sTemp;
					}
					break;
				} else {
					ss << sTemp;
					ss >> fTemp;
					ss.clear();
					feature.push_back(fTemp);
				}
			}
			if (undefined) {
				feature.clear();
				features.push_back(feature);
				continue;
			}
			features.push_back(feature);
		}
		features.pop_back();
		fin1.close();
		fin2.close();
	} else {
		// read mfcc/lpcc/plp feature file
		fp = fopen(featureFilename.c_str(), "rb");
		int cellSize = sizeof(float);
		int num = (_filelength(_fileno(fp)) - 12) / cellSize;
		float *buf = new float[num];
		fseek(fp, 12, SEEK_SET);
		fread(buf, cellSize, num, fp);
		fclose(fp);

		// get mfcc/lpcc/plp feature vector of each frame
		features.clear();
		for (int i = 0; i < num/VEC_DIM; i++) {
			feature.clear();
			for (int j = 0; j < VEC_DIM; j++) {
				feature.push_back(buf[i*VEC_DIM+j]);
			}
			features.push_back(feature);
		}

		delete[] buf;
	}

	ofstream fout1, fout2, fout3, fout4;

	int clipSize = FRAME_NUM_CLIP;
	if (fType == PROSODY) {
		clipSize = 1.5*FRAME_NUM_CLIP;
	}
	int clipOverlap = FRAME_NUM_OVERLAP;
	if (fType == PROSODY) {
		clipOverlap = 1.5*FRAME_NUM_OVERLAP;
	}

	TimeSeg ts;
	int n;
	int leftClip, rightClip;
	switch (model)
	{
		case SGM:
			for (int i = 0; i < classesNum; i++) {
				for (int j = 0; j < manualLabel.cTimeSegsNum.at(i); j++) {
					ts = manualLabel.cTimeSegs.at(i).at(j);
					if (fType == PROSODY) {
						if (ts.leftFrame < clipSize) {
							leftClip = 0;
						} else {
							leftClip = ceil((double)(ts.leftFrame+1-clipSize) / (clipSize-clipOverlap));
						}
						if (ts.rightFrame < clipSize) {
							rightClip = 0;
						} else {
							rightClip = ceil((double)(ts.rightFrame+1-clipSize) / (clipSize-clipOverlap)) - 1;
						}
						for (int k = leftClip; k <= rightClip; k++) {
							if (features.at(k).size() == 0) {
								continue;
							}
//							if (ts.type == Music || ts.type == SpeechMusic || ts.type == AdMusic) {
							if (ts.type == Music || ts.type == SpeechMusic || ts.type == Ad || ts.type == Song) {
								cSGModels.at(classesNum).features.push_back(features.at(k));
//							} else if (ts.type == Speech || ts.type == Ad) {
							} else if (ts.type == Speech) {
								cSGModels.at(classesNum+1).features.push_back(features.at(k));
							}
						}
					} else {
						for (int k = ts.leftFrame; k <= ts.rightFrame; k++) {
							if (k >= features.size()) {
								break;
							}
							cSGModels.at(i).features.push_back(features.at(k));
						}
					}
				}
			}
			break;

		case GMM:
			for (int i = 0; i < classesNum; i++) {
				for (int j = 0; j < manualLabel.cTimeSegsNum.at(i); j++) {
					ts = manualLabel.cTimeSegs.at(i).at(j);
					if (fType == PROSODY) {
						if (ts.leftFrame < clipSize) {
							leftClip = 0;
						} else {
							leftClip = ceil((double)(ts.leftFrame+1-clipSize) / (clipSize-clipOverlap));
						}
						if (ts.rightFrame < clipSize) {
							rightClip = 0;
						} else {
							rightClip = ceil((double)(ts.rightFrame+1-clipSize) / (clipSize-clipOverlap)) - 1;
						}
						for (int k = leftClip; k <= rightClip; k++) {
							if (features.at(k).size() == 0) {
								continue;
							}
//							if (ts.type == Music || ts.type == SpeechMusic || ts.type == AdMusic) {
							if (ts.type == Music || ts.type == SpeechMusic || ts.type == Ad || ts.type == Song) {
								cGMModels.at(classesNum).features.push_back(features.at(k));
//							} else if (ts.type == Speech || ts.type == Ad) {
							} else if (ts.type == Speech) {
								cGMModels.at(classesNum+1).features.push_back(features.at(k));
							}
						}
					} else {
						for (int k = ts.leftFrame; k <= ts.rightFrame; k++) {
							if (k >= features.size()) {
								break;
							}
							cGMModels.at(i).features.push_back(features.at(k));
						}
					}
				}
			}
			break;

		case SVMM:
			for (int i = 0; i < classesNum; i++) {
				for (int j = 0; j < manualLabel.cTimeSegsNum.at(i); j++) {
					ts = manualLabel.cTimeSegs.at(i).at(j);
					if (fType == PROSODY) {
						fout1.open("svm/svm_trainset_1.txt", ios_base::app);
						if (ts.leftFrame < clipSize) {
							leftClip = 0;
						} else {
							leftClip = ceil((double)(ts.leftFrame+1-clipSize) / (clipSize-clipOverlap));
						}
						if (ts.rightFrame < clipSize) {
							rightClip = 0;
						} else {
							rightClip = ceil((double)(ts.rightFrame+1-clipSize) / (clipSize-clipOverlap)) - 1;
						}
						for (int k = leftClip; k <= rightClip; k++) {
							if (features.at(k).size() == 0) {
								continue;
							}
//							if (ts.type == Music || ts.type == SpeechMusic || ts.type == AdMusic) {
							if (ts.type == Music || ts.type == SpeechMusic || ts.type == Ad || ts.type == Song) {
								fout1 << classesNum+1 << " ";
								for (int m = 0; m < VEC_DIM_PROSODY; m++) {
									fout1 << m+1 << ":" << features.at(k).at(m) << " ";
								}
								fout1 << endl;
//							} else if (ts.type == Speech || ts.type == Ad) {
							} else if (ts.type == Speech) {
								fout1 << classesNum+2 << " ";
								for (int m = 0; m < VEC_DIM_PROSODY; m++) {
									fout1 << m+1 << ":" << features.at(k).at(m) << " ";
								}
								fout1 << endl;
							}
						}
						fout1.close();
					} else {
						fout2.open("svm/svm_trainset_2.txt", ios_base::app);
						fout3.open("svm/svm_trainset_3.txt", ios_base::app);
						fout4.open("svm/svm_trainset_4.txt", ios_base::app);
						for (int k = ts.leftFrame; k <= ts.rightFrame; k++) {
							if (k >= features.size()) {
								break;
							}
							if (ts.type == Music || ts.type == SpeechMusic || ts.type == Ad || ts.type == Song) {
								fout2 << (int)ts.type+1 << " ";
								fout4 << (int)ts.type+1 << " ";
								for (int m = 0; m < VEC_DIM; m++) {
									fout2 << m+1 << ":" << features.at(k).at(m) << " ";
									fout4 << m+1 << ":" << features.at(k).at(m) << " ";
								}
								fout2 << endl;
								fout4 << endl;
							} else if (ts.type == Speech) {
								fout3 << (int)ts.type+1 << " ";
								fout4 << (int)ts.type+1 << " ";
								for (int m = 0; m < VEC_DIM; m++) {
									fout3 << m+1 << ":" << features.at(k).at(m) << " ";
									fout4 << m+1 << ":" << features.at(k).at(m) << " ";
								}
								fout3 << endl;
								fout4 << endl;
							}
						}
						fout2.close();
						fout3.close();
						fout4.close();
					}
				}
			}
			break;
	}
}

void Trainer::OutputToFile()
{
	switch (model)
	{
		case SGM:
			if (fType == PROSODY) {
				for (int i = classesNum; i < classesNum+2; i++) {
					cSGModels.at(i).outputParamToFile(fType);
				}
			} else {
				for (int i = 0; i < classesNum; i++) {
					cSGModels.at(i).outputParamToFile(fType);
				}
			}
			break;

		case GMM:
			if (fType == PROSODY) {
				for (int i = classesNum; i < classesNum+2; i++) {
					cGMModels.at(i).outputParamToFile(fType);
				}
			} else {
				for (int i = 0; i < classesNum; i++) {
					cGMModels.at(i).outputParamToFile(fType);
				}
			}
			break;
	}
}
