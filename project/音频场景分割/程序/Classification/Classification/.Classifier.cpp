#include "Classifier.h"
#include <iostream>
#include <cmath>
using namespace std;

Classifier::Classifier(models model, int classesNum, scoreModes scoreMode, featureTypes fType)
{
	this->model = model;
	this->scoreMode = scoreMode;
	this->fType = fType;

	this->classesNum = classesNum;
	audio.SetClassesNum(classesNum);

	ReadModelFiles();
}

Classifier::~Classifier()
{
}

void Classifier::Process()
{
	// Voice Activity Detection
	VAD();

	// Level 1 classification
//	if (model == SVMM || model == GMM) {
	if (model == SVMM) {
	 	OutputSVMFeatureFiles1();
	}
	ClassifyLevel1();

	audio.UpdateTimeSegs();

	// Level 2 classification
//	if (model == SVMM || model == GMM) {
	if (model == SVMM) {
		OutputSVMFeatureFiles2();
	}
	ClassifyLevel2();

	audio.OutputToFile();
}

void Classifier::ReadModelFiles()
{
	int modelsNum;
	string modelListFilename;

	models readModel;
	if (model == SGM || model == GMM) {
		readModel = model;
	}
	if (model == SVMM) {
		readModel = GMM;
	}

	switch (fType)
	{
		case MFCC:
			modelListFilename = modelListFilenamesMFCC[(int)readModel];
			break;
		case LPCC:
			modelListFilename = modelListFilenamesLPCC[(int)readModel];
			break;
		case PLP:
			modelListFilename = modelListFilenamesPLP[(int)readModel];
			break;
	}
	fin.open(modelListFilename.c_str());
	fin >> modelsNum;
	string *modelFilenames = new string[modelsNum+2];
	for (int i = 0; i < modelsNum; i++) {
		fin >> modelFilenames[i];
	}
	fin.close();

	modelListFilename = modelListFilenamesPROSODY[(int)readModel];
	fin.open(modelListFilename.c_str());
	fin >> modelsNum;
	for (int i = 0; i < modelsNum; i++) {
		fin >> modelFilenames[classesNum+i];
	}
	fin.close();

	cSGModels.clear();
	SGModel cSGModel;
	cGMModels.clear();
	GMModel cGMModel;

	int ivalue;
	float fvalue;
	vector<float> tempVec;
	int vecDim;

	for (int i = 0; i < classesNum+2; i++) {
		if (i < classesNum) {
			vecDim = VEC_DIM;
		} else {
			vecDim = VEC_DIM_PROSODY;
		}
		switch (readModel)
		{
			case SGM:
				cSGModel.type = types[i];
				cSGModel.featureMean.clear();
				cSGModel.featureVariance.clear();
				fin.open(modelFilenames[i].c_str());
				for (int j = 0; j < vecDim; j++) {
					fin >> fvalue;
					cSGModel.featureMean.push_back(fvalue);
				}
				for (int j = 0; j < vecDim; j++) {
					fin >> fvalue;
					cSGModel.featureVariance.push_back(fvalue);
				}
				cSGModels.push_back(cSGModel);
				fin.close();
				break;

			case GMM:
				cGMModel.type = types[i];
				cGMModel.alpha.clear();
				cGMModel.featureMean.clear();
				cGMModel.featureCovariance.clear();
				fin.open(modelFilenames[i].c_str());
				fin >> ivalue;
				cGMModel.mixNum = ivalue;
				for (int j = 0; j < cGMModel.mixNum; j++) {
					fin >> fvalue;
					cGMModel.alpha.push_back(fvalue);
					tempVec.clear();
					for (int k = 0; k < vecDim; k++) {
						fin >> fvalue;
						tempVec.push_back(fvalue);
					}
					cGMModel.featureMean.push_back(tempVec);
					tempVec.clear();
					for (int k = 0; k < vecDim; k++) {
						fin >> fvalue;
						tempVec.push_back(fvalue);
					}
					cGMModel.featureCovariance.push_back(tempVec);
				}
				cGMModels.push_back(cGMModel);
				fin.close();
				break;
		}
	}
}

void Classifier::VAD()
{
	double eH = audio.eMax * 0.168 * 0.168;
	double eL = audio.eMax * 0.06 * 0.06;
	double zcrS = 30;

	// energy >= e_h, voice
	for (int i = 0; i < audio.frameNum; i++) {
		if (audio.frames.at(i).energy >= eH) {
			audio.frames.at(i).type = Voice;
		}
	}

	// search forward and backward, energy >= e_l, voice
	for (int i = 0; i < audio.frameNum; i++) {
		if (audio.frames.at(i).type == Voice) {
			if (i != 0 && audio.frames.at(i-1).type == Silence) {
				for (int j = i-1; j >= 0; j--) {
					if (audio.frames.at(j).type == Silence && audio.frames.at(j).energy >= eL) {
						audio.frames.at(j).type = Voice;
					} else {
						break;
					}
				}
			}
			if (i != audio.frameNum-1 && audio.frames.at(i+1).type == Silence) {
				for (int j = i+1; j < audio.frameNum; j++) {
					if (audio.frames.at(j).type == Silence && audio.frames.at(j).energy >= eL) {
						audio.frames.at(j).type = Voice;
					} else {
						break;
					}
				}
			}
		}
	}

	// search forward and backward, zcr >= 3*zcr_s, voice
	int search_num = 0;
	for (int i = 0; i < audio.frameNum; i++) {
		if (audio.frames.at(i).type == Voice) {
			if (i != 0 && audio.frames.at(i-1).type == Silence) {
				search_num = 0;
				for (int j = i-1; j >= 0; j--) {
					if (audio.frames.at(j).type == Silence) {
						if (audio.frames.at(j).zcr < 3*zcrS || search_num >= FRAME_SIZE) {
							break;
						} else {
							audio.frames.at(j).type = Voice;
							search_num++;
						}
					} else {
						break;
					}
				}
			}
			if (i != audio.frameNum-1 && audio.frames.at(i+1).type == Silence) {
				search_num = 0;
				for (int j = i+1; j < audio.frameNum; j++) {
					if (audio.frames.at(j).type == Silence) {
						if (audio.frames.at(j).zcr < 3*zcrS || search_num >= FRAME_SIZE) {
							break;
						} else {
							audio.frames.at(j).type = Voice;
							search_num++;
						}
					} else {
						break;
					}
				}
			}
		}
	}

	// filtrate few silence frames between the voice frames
	Filtrate();

	// identify the time boundaries
	audio.UpdateTimeSegs();

	CompareSVModels();
	Filtrate();
	audio.UpdateTimeSegs();
}

void Classifier::Filtrate()
{
	int search_num = 0;
	for (int i = 0; i < audio.frameNum; i++) {
		if (audio.frames.at(i).type == Silence) {
			search_num++;
		} else {
			if (search_num == 0) {
				continue;
			} else if (search_num >= FRAME_NUM_FILT+1) {
				search_num = 0;
			} else {
				for (int j = i-1; j > i-1-search_num; j--) {
					audio.frames.at(j).type = Voice;
				}
				search_num = 0;
			}
		}
	}
	if (search_num <= FRAME_NUM_FILT && search_num > 0) {
		for (int i = audio.frameNum-1; i > audio.frameNum-1-search_num; i--) {
			audio.frames.at(i).type = Voice;
		}
	}
}

void Classifier::CompareSVModels()
{
	models readModel;
	if (model == SGM || model == GMM) {
		readModel = model;
	}
	if (model == SVMM) {
		readModel = GMM;
	}

	TimeSeg ts;
	vector<vector<float>> featureClip;
	vector<vector<float>> scores;
	vector<float> tempFVec;
	for (int i = 0; i < 2; i++) {
		scores.push_back(tempFVec);
	}
	int m;
	for (int i = 0; i < audio.cTimeSegs.at((int)Silence).size(); i++) {
		ts = audio.cTimeSegs.at((int)Silence).at(i);
		for (int j = ts.leftFrame; j <= ts.rightFrame; j += (FRAME_NUM_CLIP-FRAME_NUM_OVERLAP)) {
			featureClip.clear();
			for (m = 0; m < FRAME_NUM_CLIP; m++) {
				if (j+m > ts.rightFrame) {
					break;
				}
				featureClip.push_back(audio.frames.at(j+m).feature);
			}
			for (int k = 0; k < 2; k++) {
				scores.at(k).clear();
			}
			switch (readModel)
			{
				case SGM:
					for (int k = 0; k < 2; k++) {
						for (int l = 0 ; l < featureClip.size(); l++) {
							scores.at(k).push_back(cSGModels.at(k).score(featureClip.at(l), fType));
						}
					}
					break;
				case GMM:
					for (int k = 0; k < 2; k++) {
						for (int l = 0 ; l < featureClip.size(); l++) {
							scores.at(k).push_back(cGMModels.at(k).score(featureClip.at(l), fType));
						}
					}
					break;
			}
			if (GetBestClass(scores, featureClip.size(), 0) == (int)Voice) {
				for (m = 0; m < FRAME_NUM_CLIP; m++) {
					if (j+m > ts.rightFrame) {
						break;
					}
					audio.frames.at(j+m).type = Voice;
				}
				if (m < FRAME_NUM_CLIP) {
					break;
				}
			} else {
				break;
			}
		}
		for (int j = ts.rightFrame; j >= ts.leftFrame; j -= (FRAME_NUM_CLIP-FRAME_NUM_OVERLAP)) {
			featureClip.clear();
			for (m = 0; m < FRAME_NUM_CLIP; m++) {
				if (j-m < ts.leftFrame) {
					break;
				}
				featureClip.push_back(audio.frames.at(j-m).feature);
			}
			for (int k = 0; k < 2; k++) {
				scores.at(k).clear();
			}
			switch (readModel)
			{
				case SGM:
					for (int k = 0; k < 2; k++) {
						for (int l = 0 ; l < featureClip.size(); l++) {
							scores.at(k).push_back(cSGModels.at(k).score(featureClip.at(l), fType));
						}
					}
					break;
				case GMM:
					for (int k = 0; k < 2; k++) {
						for (int l = 0 ; l < featureClip.size(); l++) {
							scores.at(k).push_back(cGMModels.at(k).score(featureClip.at(l), fType));
						}
					}
					break;
			}
			if (GetBestClass(scores, featureClip.size(), 0) == (int)Voice) {
				for (m = 0; m < FRAME_NUM_CLIP; m++) {
					if (j-m < ts.leftFrame) {
						break;
					}
					audio.frames.at(j-m).type = Voice;
				}
				if (m < FRAME_NUM_CLIP) {
					break;
				}
			} else {
				break;
			}
		}
	}
}

void Classifier::OutputSVMFeatureFiles1()
{
	int clipSize = 1.5*FRAME_NUM_CLIP;
	int clipOverlap = 1.5*FRAME_NUM_OVERLAP;

	string outFilename = "svm/svm_testset_1_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
	fout.open(outFilename.c_str());
	TimeSeg ts;
	int leftClip, rightClip;
	for (int i = 0; i < audio.cTimeSegs.at((int)Voice).size(); i++) {
		ts = audio.cTimeSegs.at((int)Voice).at(i);
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
		for (int j = leftClip; j <= rightClip; j++) {
			if (audio.prosodyFeatures.at(j).size() == 0) {
				continue;
			}
			fout << "0 ";
			for (int k = 0; k < VEC_DIM_PROSODY; k++) {
				fout << k+1 << ":" << audio.prosodyFeatures.at(j).at(k) << " ";
			}
			fout << endl;
		}
	}
	fout.close();

	string scaleFilename = "svm/svm_testset_1_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
	string command = "svm-scale -r svm/svm_train_prosody.range " + outFilename + ">" + scaleFilename;
	system(command.c_str());
}

void Classifier::OutputSVMFeatureFiles2()
{
	string outFilename2;
	switch (fType)
	{
		case MFCC:
			outFilename2 = "svm/svm_testset_2_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
			break;
		case LPCC:
			outFilename2 = "svm/svm_testset_2_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
			break;
		case PLP:
			outFilename2 = "svm/svm_testset_2_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
			break;
	}

	string outFilename3;
	switch (fType)
	{
		case MFCC:
			outFilename3 = "svm/svm_testset_3_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
			break;
		case LPCC:
			outFilename3 = "svm/svm_testset_3_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
			break;
		case PLP:
			outFilename3 = "svm/svm_testset_3_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
			break;
	}

	string outFilename4;
	switch (fType)
	{
		case MFCC:
			outFilename4 = "svm/svm_testset_4_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
			break;
		case LPCC:
			outFilename4 = "svm/svm_testset_4_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
			break;
		case PLP:
			outFilename4 = "svm/svm_testset_4_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".txt";
			break;
	}

	ofstream fout2, fout3, fout4;
	fout2.open(outFilename2.c_str());
	fout3.open(outFilename3.c_str());
	fout4.open(outFilename4.c_str());
	TimeSeg ts;
	for (int i = 0; i < classesNum+2; i++) {
		if (types[i] != Voice && types[i] != WithMusic && types[i] != WithoutMusic) {
			continue;
		}
		for (int j = 0; j < audio.cTimeSegs.at(i).size(); j++) {
			ts = audio.cTimeSegs.at(i).at(j);
			for (int k = ts.leftFrame; k <= ts.rightFrame; k++) {
				if (types[i] == WithMusic) {
					fout2 << "0 ";
					for (int m = 0; m < VEC_DIM; m++) {
						fout2 << m+1 << ":" << audio.frames.at(k).feature.at(m) << " ";
					}
					fout2 << endl;
				}
				if (types[i] == WithoutMusic) {
					fout3 << "0 ";
					for (int m = 0; m < VEC_DIM; m++) {
						fout3 << m+1 << ":" << audio.frames.at(k).feature.at(m) << " ";
					}
					fout3 << endl;
				}
				if (types[i] == Voice) {
					fout4 << "0 ";
					for (int m = 0; m < VEC_DIM; m++) {
						fout4 << m+1 << ":" << audio.frames.at(k).feature.at(m) << " ";
					}
					fout4 << endl;
				}
			}
		}
	}
	fout2.close();
	fout3.close();
	fout4.close();

	string scaleFilename2;
	switch (fType)
	{
		case MFCC:
			scaleFilename2 = "svm/svm_testset_2_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
			break;
		case LPCC:
			scaleFilename2 = "svm/svm_testset_2_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
			break;
		case PLP:
			scaleFilename2 = "svm/svm_testset_2_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
			break;
	}

	string scaleFilename3;
	switch (fType)
	{
		case MFCC:
			scaleFilename3 = "svm/svm_testset_3_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
			break;
		case LPCC:
			scaleFilename3 = "svm/svm_testset_3_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
			break;
		case PLP:
			scaleFilename3 = "svm/svm_testset_3_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
			break;
	}

	string scaleFilename4;
	switch (fType)
	{
		case MFCC:
			scaleFilename4 = "svm/svm_testset_4_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
			break;
		case LPCC:
			scaleFilename4 = "svm/svm_testset_4_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
			break;
		case PLP:
			scaleFilename4 = "svm/svm_testset_4_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
			break;
	}

	string command;

	switch (fType)
	{
		case MFCC:
			command = "svm-scale -r svm/svm_train_mfcc.range " + outFilename2 + ">" + scaleFilename2;
			break;
		case LPCC:
			command = "svm-scale -r svm/svm_train_lpcc.range " + outFilename2 + ">" + scaleFilename2;
			break;
		case PLP:
			command = "svm-scale -r svm/svm_train_plp.range " + outFilename2 + ">" + scaleFilename2;
			break;
	}
	system(command.c_str());

	switch (fType)
	{
		case MFCC:
			command = "svm-scale -r svm/svm_train_mfcc.range " + outFilename3 + ">" + scaleFilename3;
			break;
		case LPCC:
			command = "svm-scale -r svm/svm_train_lpcc.range " + outFilename3 + ">" + scaleFilename3;
			break;
		case PLP:
			command = "svm-scale -r svm/svm_train_plp.range " + outFilename3 + ">" + scaleFilename3;
			break;
	}
	system(command.c_str());

	switch (fType)
	{
		case MFCC:
			command = "svm-scale -r svm/svm_train_mfcc.range " + outFilename4 + ">" + scaleFilename4;
			break;
		case LPCC:
			command = "svm-scale -r svm/svm_train_lpcc.range " + outFilename4 + ">" + scaleFilename4;
			break;
		case PLP:
			command = "svm-scale -r svm/svm_train_plp.range " + outFilename4 + ">" + scaleFilename4;
			break;
	}
	system(command.c_str());
}

void Classifier::ClassifyLevel1()
{
	TimeSeg ts;
	vector<float> scores(2);
	int leftClip, rightClip;
	int maxNo;
	int m, n;
	string scaleFilename, resultFilename;
	string command;
	int iTemp;
	string sTemp;
	double dTemp;

	double probabilities[2];
	int labels[2];
	bool unknown;

//	if (model == SVMM || model == GMM) {
	if (model == SVMM) {
		scaleFilename = "svm/svm_testset_1_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
		resultFilename = "svm/svm_testset_1_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
		command = "svm-predict -b 1 " + scaleFilename + " svm/svm_1.model " + resultFilename;
		system(command.c_str());
		fin.open(resultFilename.c_str());
	}

	int clipSize = 1.5*FRAME_NUM_CLIP;
	int clipOverlap = 1.5*FRAME_NUM_OVERLAP;

	fin >> sTemp;
	for (int i = 0; i < 2; i++) {
		fin >> iTemp;
		labels[i] = iTemp-1;
	}
	for (int i = 0; i < audio.cTimeSegs.at((int)Voice).size(); i++) {
		ts = audio.cTimeSegs.at((int)Voice).at(i);
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
		n = ts.leftFrame;
		for (int j = leftClip; j <= rightClip; j++) {
			unknown = false;
			if (audio.prosodyFeatures.at(j).size() == 0) {
				continue;
			}
			switch (model)
			{
				case SGM:
					for (int k = classesNum; k < classesNum+2; k++) {
						scores.at(k-classesNum) = cSGModels.at(k).score(audio.prosodyFeatures.at(j), PROSODY);
					}
					if (abs(scores.at(0)-scores.at(1)) < (double)1/(2+1)) {
						unknown = true;
					} else {
						if (scores.at(0) > scores.at(1)) {
							maxNo = classesNum;
						} else {
							maxNo = classesNum+1;
						}
					}
					break;
				case GMM:
					for (int k = classesNum; k < classesNum+2; k++) {
						scores.at(k-classesNum) = cGMModels.at(k).score(audio.prosodyFeatures.at(j), PROSODY);
					}
					if (abs(scores.at(0)-scores.at(1)) < (double)1/(2+1)) {
						unknown = true;
					} else {
						if (scores.at(0) > scores.at(1)) {
							maxNo = classesNum;
						} else {
							maxNo = classesNum+1;
						}
					}
// 					fin >> iTemp;
// 					maxNo = iTemp-1;
// 					for (int k = 0; k < 2; k++) {
// 						fin >> dTemp;
// 						probabilities[k] = dTemp;
// 					}
// 					if (abs(probabilities[0]-probabilities[1]) < (double)1/(2+1)) {
// 						unknown = true;
// 					}
					break;
				case SVMM:
	 				fin >> iTemp;
 					maxNo = iTemp-1;
					for (int k = 0; k < 2; k++) {
						fin >> dTemp;
						probabilities[k] = dTemp;
					}
					if (abs(probabilities[0]-probabilities[1]) < (double)1/(2+1)) {
						unknown = true;
					}
					break;
			}
			if (!unknown) {
				for (m = 0; m < clipSize; m++) {
					if (n+m > ts.rightFrame) {
						break;
					}
					audio.frames.at(n+m).type = types[maxNo];
				}
			}
			n += (clipSize-clipOverlap);
		}
	}

//	if (model == SVMM || model == GMM) {
	if (model == SVMM) {
		fin.close();
	}
}

void Classifier::ClassifyLevel2()
{
	TimeSeg ts;
	vector<double> scores;
	int maxNo;

 	string scaleFilename2, scaleFilename3, scaleFilename4;
 	string resultFilename2, resultFilename3, resultFilename4;
	string command;
	ifstream fin2, fin3, fin4;
	int iTemp;
	string sTemp;
	double dTemp;

	int *labels = new int[classesNum-2];
	bool unknown;
	double maxProb, maxProb2;
	int maxProbNo;

	if (model == SVMM) {
		switch (fType)
		{
			case MFCC:
				scaleFilename2 = "svm/svm_testset_2_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
				break;
			case LPCC:
				scaleFilename2 = "svm/svm_testset_2_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
				break;
			case PLP:
				scaleFilename2 = "svm/svm_testset_2_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
				break;
		}

		switch (fType)
		{
			case MFCC:
				scaleFilename3 = "svm/svm_testset_3_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
				break;
			case LPCC:
				scaleFilename3 = "svm/svm_testset_3_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
				break;
			case PLP:
				scaleFilename3 = "svm/svm_testset_3_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
				break;
		}

		switch (fType)
		{
			case MFCC:
				scaleFilename4 = "svm/svm_testset_4_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
				break;
			case LPCC:
				scaleFilename4 = "svm/svm_testset_4_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
				break;
			case PLP:
				scaleFilename4 = "svm/svm_testset_4_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".scale";
				break;
		}

		switch (fType)
		{
			case MFCC:
				resultFilename2 = "svm/svm_testset_2_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
				break;
			case LPCC:
				resultFilename2 = "svm/svm_testset_2_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
				break;
			case PLP:
				resultFilename2 = "svm/svm_testset_2_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
				break;
		}

		switch (fType)
		{
			case MFCC:
				resultFilename3 = "svm/svm_testset_3_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
				break;
			case LPCC:
				resultFilename3 = "svm/svm_testset_3_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
				break;
			case PLP:
				resultFilename3 = "svm/svm_testset_3_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
				break;
		}

		switch (fType)
		{
			case MFCC:
				resultFilename4 = "svm/svm_testset_4_mfcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
				break;
			case LPCC:
				resultFilename4 = "svm/svm_testset_4_lpcc_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
				break;
			case PLP:
				resultFilename4 = "svm/svm_testset_4_plp_" + audio.waveFilename.substr(10, audio.waveFilename.length()-14) + ".result";
				break;
		}

		switch (fType)
		{
			case MFCC:
				command = "svm-predict -b 1 " + scaleFilename2 + " svm/svm_2_mfcc.model " + resultFilename2;
				break;
			case LPCC:
				command = "svm-predict -b 1 " + scaleFilename2 + " svm/svm_2_lpcc.model " + resultFilename2;
				break;
			case PLP:
				command = "svm-predict -b 1 " + scaleFilename2 + " svm/svm_2_plp.model " + resultFilename2;
				break;
		}
		system(command.c_str());

// 		switch (fType)
// 		{
// 			case MFCC:
// 				command = "svm-predict -b 1 " + scaleFilename3 + " svm/svm_3_mfcc.model " + resultFilename3;
// 				break;
// 			case LPCC:
// 				command = "svm-predict -b 1 " + scaleFilename3 + " svm/svm_3_lpcc.model " + resultFilename3;
// 				break;
// 			case PLP:
// 				command = "svm-predict -b 1 " + scaleFilename3 + " svm/svm_3_plp.model " + resultFilename3;
// 				break;
// 		}
// 		system(command.c_str());

		switch (fType)
		{
			case MFCC:
				command = "svm-predict -b 1 " + scaleFilename4 + " svm/svm_4_mfcc.model " + resultFilename4;
				break;
			case LPCC:
				command = "svm-predict -b 1 " + scaleFilename4 + " svm/svm_4_lpcc.model " + resultFilename4;
				break;
			case PLP:
				command = "svm-predict -b 1 " + scaleFilename4 + " svm/svm_4_plp.model " + resultFilename4;
				break;
		}
		system(command.c_str());

		fin2.open(resultFilename2.c_str());
		fin3.open(resultFilename3.c_str());
		fin4.open(resultFilename4.c_str());
	}

// 	// withmusic --> music, speechmusic, admusic
	// withmusic --> music, speechmusic, ad, song
	fin2 >> sTemp;
	for (int i = 0; i < 4; i++) {
		fin2 >> iTemp;
		labels[i] = iTemp-1;
	}
	for (int i = 0; i < audio.cTimeSegs.at((int)WithMusic).size(); i++) {
		ts = audio.cTimeSegs.at((int)WithMusic).at(i);
		for (int j = ts.leftFrame; j <= ts.rightFrame; j++) {
			unknown = false;
			scores.clear();
			switch (model)
			{
				case SGM:
// 					for (int k = 4; k <= 6; k++) {
					for (int k = 3; k <= 6; k++) {
						scores.push_back(cSGModels.at(k).score(audio.frames.at(j).feature, fType));
					}
					maxNo = MaxNo(scores) + 3;
					break;
				case GMM:
// 					for (int k = 4; k <= 6; k++) {
					for (int k = 3; k <= 6; k++) {
						scores.push_back(cGMModels.at(k).score(audio.frames.at(j).feature, fType));
					}
					maxNo = MaxNo(scores) + 3;
					break;
				case SVMM:
					fin2 >> iTemp;
					maxNo = iTemp-1;
					for (int k = 0; k < 4; k++) {
						fin2 >> dTemp;
						scores.push_back(dTemp);
					}
					break;
			}
			audio.frames.at(j).scores.clear();
			for (int k = 0; k < 4; k++) {
				audio.frames.at(j).scores.push_back(scores.at(k));
			}
			audio.frames.at(j).type = types[maxNo];
		}
	}

// 	// withoutmusic --> speech, ad
	// withoutmusic --> speech
	for (int i = 0; i < audio.cTimeSegs.at((int)WithoutMusic).size(); i++) {
		ts = audio.cTimeSegs.at((int)WithoutMusic).at(i);
		for (int j = ts.leftFrame; j <= ts.rightFrame; j++) {
			switch (model)
			{
				case SGM:
// 					scores.clear();
// 					for (int k = 2; k <= 3; k++) {
// 						scores.push_back(cSGModels.at(k).score(audio.frames.at(j).feature, fType));
// 					}
// 					maxNo = MaxNo(scores) + 2;
					maxNo = 2;
					break;
				case GMM:
// 					scores.clear();
// 					for (int k = 2; k <= 3; k++) {
// 						scores.push_back(cGMModels.at(k).score(audio.frames.at(j).feature, fType));
// 					}
// 					maxNo = MaxNo(scores) + 2;
					maxNo = 2;
					break;
				case SVMM:
// 					fin3 >> iTemp;
// 					maxNo = iTemp-1;
					maxNo = 2;
					break;
			}
			audio.frames.at(j).type = types[maxNo];
		}
	}

	// other voice --> speech, ad, music, speechmusic, admusic
	fin4 >> sTemp;
	for (int i = 0; i < 5; i++) {
		fin4 >> iTemp;
		labels[i] = iTemp-1;
	}
	for (int i = 0; i < audio.cTimeSegs.at((int)Voice).size(); i++) {
		ts = audio.cTimeSegs.at((int)Voice).at(i);
		for (int j = ts.leftFrame; j <= ts.rightFrame; j++) {
			unknown = false;
			scores.clear();
			switch (model)
			{
				case SGM:
					for (int k = 2; k < classesNum; k++) {
						scores.push_back(cSGModels.at(k).score(audio.frames.at(j).feature, fType));
					}
					maxNo = MaxNo(scores) + 2;
					break;
				case GMM:
					for (int k = 2; k < classesNum; k++) {
						scores.push_back(cGMModels.at(k).score(audio.frames.at(j).feature, fType));
					}
					maxNo = MaxNo(scores) + 2;
					break;
				case SVMM:
					fin4 >> iTemp;
					maxNo = iTemp-1;
					for (int k = 0; k < 5; k++) {
						fin4 >> dTemp;
						scores.push_back(dTemp);
					}
					break;
			}
			audio.frames.at(j).scores.clear();
			for (int k = 0; k < 5; k++) {
				audio.frames.at(j).scores.push_back(scores.at(k));
			}
			audio.frames.at(j).type = types[maxNo];
		}
	}

	if (model == SVMM) {
		fin2.close();
		fin3.close();
		fin4.close();
	}
	delete []labels;
}

int Classifier::GetBestClass(vector<vector<float>> scores, int frameNum, int validClassNo)
{
	int bestClassNo;
	float sum, maxSum;
	switch (scoreMode)
	{
		case SUM:
			maxSum = 0;
			for (int i = 0; i < frameNum; i++) {
				maxSum += scores.at(validClassNo).at(i);
			}
			bestClassNo = validClassNo;
			for (int i = validClassNo+1; i < scores.size(); i++) {
				sum = 0;
				for (int j = 0; j < frameNum; j++) {
					sum += scores.at(i).at(j);
				}
				if (sum > maxSum) {
					maxSum = sum;
					bestClassNo = i;
				}
			}
			break;
		case VOTE:
			vector<float> maxScores;
			vector<int> bestClassNos;
			for (int i = 0; i < frameNum; i++) {
				maxScores.push_back(scores.at(validClassNo).at(i));
				bestClassNos.push_back(validClassNo);
			}
			for (int i = 0; i < frameNum; i++) {
				for (int j = validClassNo+1; j < scores.size(); j++) {
					if (scores.at(j).at(i) > maxScores.at(i)) {
						maxScores.at(i) = scores.at(j).at(i);
						bestClassNos.at(i) = j;
					}
				}
			}
			vector<int> bestClassNums;
			for (int i = 0; i < scores.size(); i++) {
				bestClassNums.push_back(0);
			}
			for (int i = 0; i < frameNum; i++) {
				bestClassNums.at(bestClassNos.at(i))++;
			}
			int maxNum = bestClassNums.at(validClassNo);
			bestClassNo = validClassNo;
			for (int i = validClassNo+1; i < scores.size(); i++) {
				if (bestClassNums.at(i) > maxNum) {
					maxNum = bestClassNums.at(i);
					bestClassNo = i;
				}
			}
			break;
	}
	return bestClassNo;
}

int Classifier::MaxNo(vector<double> scores)
{
	int maxNo = 0;
	double maxScore = scores.at(0);
	for (int i = 1; i < scores.size(); i++) {
		if (scores.at(i) > maxScore) {
			maxScore = scores.at(i);
			maxNo = i;
		}
	}
	return maxNo;
}
