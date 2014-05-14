#include "Segmentor.h"
#include <iostream>
#include <cmath>
using namespace std;

Segmentor::Segmentor(int classesNum)
{
	this->classesNum = classesNum;

	audios.clear();
	manualLabels.clear();

	totalManualTimeAll = 0;
	totalManualTime = 0;
	cTotalManualTime.clear();
	totalDetectTime = 0;
	cTotalDetectTime.clear();
	totalCorrectTime = 0;
	cTotalCorrectTime.clear();
	totalWrongTime = 0;
	cTotalWrongTime.clear();
	misclassifiedTime.clear();
	totalMisclassifiedTime.clear();
	vector<float> tempFloatVec;
	for (int i = 0; i < classesNum; i++) {
		tempFloatVec.push_back(0);
		cTotalManualTime.push_back(0);
		cTotalDetectTime.push_back(0);
		cTotalCorrectTime.push_back(0);
		cTotalWrongTime.push_back(0);
	}
	for (int i = 0; i < classesNum; i++) {
		misclassifiedTime.push_back(tempFloatVec);
		totalMisclassifiedTime.push_back(tempFloatVec);
	}

	fout.open("result/errorrate.txt");
	fout.close();

	epsilon = 0.05;
}

Segmentor::~Segmentor()
{
}

void Segmentor::AddData(string scoresFilename, string manualLabelFilename)
{
	Audio *audio = new Audio();
	audio->SetClassesNum(classesNum);
	audio->ReadScoreFile(scoresFilename);
	audios.push_back(audio);
	Audio *audioTemp = new Audio();
	audioTemp->SetClassesNum(classesNum);
	audioTemp->ReadScoreFile(scoresFilename);
	audiosTemp.push_back(audioTemp);

	ManualLabel *manualLabel = new ManualLabel();
	manualLabel->SetClassesNum(classesNum);
	manualLabel->ReadFile(manualLabelFilename);
	manualLabels.push_back(manualLabel);
	ManualLabel *manualLabelTemp = new ManualLabel();
	manualLabelTemp->SetClassesNum(classesNum);
	manualLabelTemp->ReadFile(manualLabelFilename);
	manualLabelsTemp.push_back(manualLabelTemp);
}

void Segmentor::ProcessAll()
{
	float th = FindThreshold();
	for (int i = 0; i < dataNum; i++) {
 		Modify(th, true);
		Process(i);
	}
	OutputTotalErrorRateToFile();
}

float Segmentor::FindThreshold()
{
	vector<float> thresholds;
	thresholds.clear();
	vector<float> fas;
	fas.clear();
	vector<float> frs;
	frs.clear();
	int n = 0;
	for (float th = 0.02; th <= 1.0; th += 0.02) {
		thresholds.push_back(th);
		Modify(th, false);
		fas.push_back(0);
		frs.push_back(0);
		CalFalseAR(fas.at(n), frs.at(n));
		n++;
	}

	float minDist = abs(fas.at(0)-frs.at(0));
	int minDistNo = 0;
	fout.open("result/threshold.txt");
	for (int i = 0; i < n; i++) {
		fout << thresholds.at(i) << " " << fas.at(i) << " " << frs.at(i) << endl;
		if (abs(fas.at(i)-frs.at(i)) < minDist) {
			minDist = abs(fas.at(i)-frs.at(i));
			minDistNo = i;
		}
	}
	fout << endl;
	fout << thresholds.at(minDistNo) << " " << fas.at(minDistNo) << " " << frs.at(minDistNo) << endl;
	fout.close();

	return thresholds.at(minDistNo);
}

void Segmentor::Modify(float th, bool isThDone)
{
	int scoresNum;
	double maxScore;
	double upScore, downScore;
	int inzoneNum;
	Frame frame;
	for (int i = 0; i < dataNum; i++) {
		for (int j = 0; j < audios.at(i)->frameNum; j++) {
			frame = audios.at(i)->frames.at(j);
			if (frame.scores.size() == 0) {
				continue;
			}
			scoresNum = frame.scores.size();
			maxScore = frame.scores.at(0);
			for (int j = 1; j < scoresNum; j++) {
				if (frame.scores.at(j) > maxScore) {
					maxScore = frame.scores.at(j);
				}
			}
			upScore = maxScore;
			if (maxScore >= 0) {
				downScore = maxScore*(1-th);
			} else {
				downScore = maxScore*(1+th);
			}
			inzoneNum = 0;
			for (int j = 0; j < scoresNum; j++) {
				if (frame.scores.at(j) >= downScore && frame.scores.at(j) <= upScore) {
					inzoneNum++;
				}
			}
			if (inzoneNum >= 3) {
				if (!isThDone) {
					audiosTemp.at(i)->frames.at(j).type = Unknown;
				} else {
					audios.at(i)->frames.at(j).type = Unknown;
				}
			} else {
				if (!isThDone) {
					audiosTemp.at(i)->frames.at(j).type = frame.type;
				} else {
					audios.at(i)->frames.at(j).type = frame.type;
				}
			}
		}
	}
}

void Segmentor::CalFalseAR(float &fa, float &fr)
{
	int faNum = 0, frNum = 0;
	int manualNotUnknownNum = 0, detectNotUnknownNum = 0;
	for (int i = 0; i < dataNum; i++) {
		for (int j = 0; j < audiosTemp.at(i)->frameNum; j++) {
			if (j >= manualLabels.at(i)->frameTypes.size()) {
				break;
			}
			if (manualLabels.at(i)->frameTypes.at(j) != Unknown) {
				manualNotUnknownNum++;
			}
			if (audiosTemp.at(i)->frames.at(j).type != Unknown) {
				detectNotUnknownNum++;
			}
//			if (manualLabels.at(i)->frameTypes.at(j) == Unknown && audiosTemp.at(i)->frames.at(j).type != Unknown) {
			if (manualLabels.at(i)->frameTypes.at(j) != audiosTemp.at(i)->frames.at(j).type && audiosTemp.at(i)->frames.at(j).type != Unknown) {
				faNum++;
			}
//			if (manualLabels.at(i)->frameTypes.at(j) != Unknown && audiosTemp.at(i)->frames.at(j).type == Unknown) {
			if (manualLabels.at(i)->frameTypes.at(j) != audiosTemp.at(i)->frames.at(j).type && manualLabels.at(i)->frameTypes.at(j) != Unknown) {
				frNum++;
			}
		}
	}
	fa = (float)faNum/detectNotUnknownNum;
	fr = (float)frNum/manualNotUnknownNum;
}

void Segmentor::Process(int dataNo)
{
	Clear();

	// modify some clips' types, based on some rules
	CutByRules(dataNo);
	audios.at(dataNo)->UpdateTimeSegs();

	// calculate error rate, based on time length and clip numbers
	CalErrorRate(dataNo);

	audios.at(dataNo)->OutputToFile();
}

void Segmentor::CutByRules(int dataNo)
{
	vector<vector<int>> voiceNos;
	vector<int> iTempVec;
	iTempVec.clear();
	for (int i = 0; i < audios.at(dataNo)->frames.size(); i++) {
		if (audios.at(dataNo)->frames.at(i).type != Silence) {
			if (iTempVec.size() == 0) {
				iTempVec.push_back(i);
			} else {
				continue;
			}
		} else {
			if (iTempVec.size() != 0) {
				iTempVec.push_back(i-1);
				voiceNos.push_back(iTempVec);
				iTempVec.clear();
			} else {
				continue;
			}
		}
	}
	if (iTempVec.size() != 0) {
		iTempVec.push_back(audios.at(dataNo)->frames.size()-1);
		voiceNos.push_back(iTempVec);
		iTempVec.clear();
	}

	Clip clip;
	vector<int> bestClassNums;
	int maxNum;
	int maxNo;
	int m;
	for (int i = 0; i < voiceNos.size(); i++) {
		for (int j = voiceNos.at(i).at(0); j <= voiceNos.at(i).at(1); j += (FRAME_NUM_CLIP-FRAME_NUM_OVERLAP)) {
			bestClassNums.clear();
			for (int k = 0; k < classesNum+3; k++) {
				bestClassNums.push_back(0);
			}
			for (m = 0; m < FRAME_NUM_CLIP; m++) {
				if (j+m > voiceNos.at(i).at(1)) {
					break;
				}
				bestClassNums.at((int)audios.at(dataNo)->frames.at(j+m).type)++;
				if (audios.at(dataNo)->frames.at(j+m).type == Unknown) {
				}
			}
			maxNum = bestClassNums.at(2);
			maxNo = 2;
			for (int k = 3; k < classesNum+3; k++) {
				if (bestClassNums.at(k) > maxNum) {
					maxNum = bestClassNums.at(k);
					maxNo = k;
				}
			}
			clip.leftFrame = j;
			clip.rightFrame = j+m;
			if (clip.leftFrame < FRAME_NUM_CLIP) {
				clip.leftClip = 0;
			} else {
				clip.leftClip = ceil((double)(clip.leftFrame+1-FRAME_NUM_CLIP) / (FRAME_NUM_CLIP-FRAME_NUM_OVERLAP));
			}
			if (clip.rightFrame < FRAME_NUM_CLIP) {
				clip.rightClip = 0;
			} else {
				clip.rightClip = ceil((double)(clip.rightFrame+1-FRAME_NUM_CLIP) / (FRAME_NUM_CLIP-FRAME_NUM_OVERLAP)) - 1;
			}
			clip.type = types[maxNo];
			audios.at(dataNo)->clips.push_back(clip);
			if (m < FRAME_NUM_CLIP) {
				break;
			}
		}
	}

	classes type1, type2, type3;
	bool isFirst, isLast;
	int n = 0;
	for (int i = 0; i < audios.at(dataNo)->clips.size()-2; i++) {
		if (audios.at(dataNo)->clips.at(i).rightFrame-FRAME_NUM_OVERLAP != audios.at(dataNo)->clips.at(i+1).leftFrame || 
				audios.at(dataNo)->clips.at(i+1).rightFrame-FRAME_NUM_OVERLAP != audios.at(dataNo)->clips.at(i+2).leftFrame) {
			continue;
		}

		if (i == 0) {
			isFirst = true;
		} else if (audios.at(dataNo)->clips.at(i).leftFrame != audios.at(dataNo)->clips.at(i-1).rightFrame-FRAME_NUM_OVERLAP) {
			isFirst = true;
		} else {
			isFirst = false;
		}
		if (i+2 == audios.at(dataNo)->clips.size()-1) {
			isLast = true;
		} else if (audios.at(dataNo)->clips.at(i+2).rightFrame-FRAME_NUM_OVERLAP != audios.at(dataNo)->clips.at(i+3).leftFrame) {
			isLast = true;
		} else {
			isLast = false;
		}

		type1 = audios.at(dataNo)->clips.at(i).type;
		type2 = audios.at(dataNo)->clips.at(i+1).type;
		type3 = audios.at(dataNo)->clips.at(i+2).type;
		if (type1 != type2 && type2 != type3 && type3 != type1) {
			audios.at(dataNo)->clips.at(i+1).type = type1;
		}
		if (type1 == type3 && type1 != type2) {
			audios.at(dataNo)->clips.at(i+1).type = type1;
		}
		if (isFirst && type1 != type2 && type2 == type3) {
			audios.at(dataNo)->clips.at(i).type = type2;
		}
		if (isLast && type1 == type2 && type2 != type3) {
			audios.at(dataNo)->clips.at(i+2).type = type2;
		}
	}

	int tempMax, tempNum;
	for (int i = 0; i < audios.at(dataNo)->clips.size(); i++) {
		clip = audios.at(dataNo)->clips.at(i);
		for (int j = clip.leftFrame; j <= clip.rightFrame; j++) {
			if (j >= audios.at(dataNo)->frames.size()) {
				break;
			}
			audios.at(dataNo)->frames.at(j).type = clip.type;
		}
	}
}

void Segmentor::CalErrorRate(int dataNo)
{
	int no1 = 0, no2 = 0;
	TimeSeg ts1, ts2;
	float correctTime = 0, wrongTime = 0, unknownTime = 0, manualUnknownTime = 0;
	vector<float> cManualTime;
	cManualTime.clear();
	vector<float> cDetectTime;
	cDetectTime.clear();
	vector<float> cCorrectTime;
	cCorrectTime.clear();
	vector<float> cWrongTime;
	cWrongTime.clear();
	for (int i = 0; i < classesNum; i++) {
		cManualTime.push_back(0);
		cDetectTime.push_back(0);
		cCorrectTime.push_back(0);
		cWrongTime.push_back(0);
	}
	for (int i = 0; i < manualLabels.at(dataNo)->timeSegs.size(); i++) {
		ts1 = manualLabels.at(dataNo)->timeSegs.at(i);
		if (ts1.type == Unknown) {
			continue;
		}
		cManualTime.at((int)ts1.type) += (ts1.rightTime-ts1.leftTime);
	}
	for (int i = 0; i < audios.at(dataNo)->timeSegs.size(); i++) {
		ts2 = audios.at(dataNo)->timeSegs.at(i);
		if (ts2.type == Unknown) {
			continue;
		}
		cDetectTime.at((int)ts2.type) += (ts2.rightTime-ts2.leftTime);
	}
	while (no1 < manualLabels.at(dataNo)->timeSegs.size() && no2 < audios.at(dataNo)->timeSegs.size()) {
		ts1 = manualLabels.at(dataNo)->timeSegs.at(no1);
		ts2 = audios.at(dataNo)->timeSegs.at(no2);
		if (ts1.type == Unknown) {
 			manualUnknownTime += (ts1.rightTime-ts1.leftTime);
			no1++;
			continue;
		}
		if (ts2.type == Unknown) {
			unknownTime += (ts2.rightTime-ts2.leftTime);
			no2++;
			continue;
		}
		if (BelongsTo(ts2, ts1)) {
			if (ts1.type == ts2.type) {
				correctTime += (ts2.rightTime-ts2.leftTime);
				cCorrectTime.at((int)ts2.type) += (ts2.rightTime-ts2.leftTime);
			} else {
				wrongTime += (ts2.rightTime-ts2.leftTime);
				cWrongTime.at((int)ts2.type) += (ts2.rightTime-ts2.leftTime);
				misclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts2.rightTime-ts2.leftTime);
				totalMisclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts2.rightTime-ts2.leftTime);
			}
			no2++;
		} else if (BelongsTo(ts1, ts2)) {
			if (ts1.type == ts2.type) {
				correctTime += (ts1.rightTime-ts1.leftTime);
				cCorrectTime.at((int)ts2.type) += (ts1.rightTime-ts1.leftTime);
			} else {
				wrongTime += (ts1.rightTime-ts1.leftTime);
				cWrongTime.at((int)ts2.type) += (ts1.rightTime-ts1.leftTime);
				misclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts1.rightTime-ts1.leftTime);
				totalMisclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts1.rightTime-ts1.leftTime);
			}
			no1++;
		} else if (abs(ts1.leftTime-ts2.leftTime) <= epsilon && abs(ts1.rightTime-ts2.rightTime) <= epsilon) {
			if (ts1.type == ts2.type) {
				correctTime += (ts2.rightTime-ts2.leftTime);
				cCorrectTime.at((int)ts2.type) += (ts2.rightTime-ts2.leftTime);
			} else {
				wrongTime += (ts2.rightTime-ts2.leftTime);
				cWrongTime.at((int)ts2.type) += (ts2.rightTime-ts2.leftTime);
				misclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts2.rightTime-ts2.leftTime);
				totalMisclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts2.rightTime-ts2.leftTime);
			}
			no1++;
			no2++;
		} else if (ts1.rightTime <= ts2.leftTime) {
			no1++;
		} else if (ts1.leftTime >= ts2.rightTime) {
			no2++;
		} else {
			if (ts1.rightTime > ts2.leftTime && ts1.rightTime < ts2.rightTime) {
				if (ts1.type == ts2.type) {
					correctTime += (ts1.rightTime-ts2.leftTime);
					cCorrectTime.at((int)ts2.type) += (ts1.rightTime-ts2.leftTime);
				} else {
					wrongTime += (ts1.rightTime-ts2.leftTime);
					cWrongTime.at((int)ts2.type) += (ts1.rightTime-ts2.leftTime);
					misclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts1.rightTime-ts2.leftTime);
					totalMisclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts1.rightTime-ts2.leftTime);
				}
				no1++;
			}
			if (ts1.leftTime > ts2.leftTime && ts1.leftTime < ts2.rightTime) {
				if (ts1.type == ts2.type) {
					correctTime += (ts2.rightTime-ts1.leftTime);
					cCorrectTime.at((int)ts2.type) += (ts2.rightTime-ts1.leftTime);
				} else {
					wrongTime += (ts2.rightTime-ts1.leftTime);
					cWrongTime.at((int)ts2.type) += (ts2.rightTime-ts1.leftTime);
					misclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts2.rightTime-ts1.leftTime);
					totalMisclassifiedTime.at((int)ts1.type).at((int)ts2.type) += (ts2.rightTime-ts1.leftTime);
				}
				no2++;
			}
		}
	}

	float totalTime = manualLabels.at(dataNo)->timeSegs.at(manualLabels.at(dataNo)->timeSegs.size()-1).rightTime;
	float manualTime = totalTime - manualUnknownTime;
	float detectTime = audios.at(dataNo)->timeSegs.at(audios.at(dataNo)->timeSegs.size()-1).rightTime - unknownTime;
	totalManualTimeAll += totalTime;
	totalManualTime += manualTime;
	totalDetectTime += detectTime;
	totalCorrectTime += correctTime;
	totalWrongTime += wrongTime;
	for (int i = 0; i < classesNum; i++) {
		cTotalManualTime.at(i) += cManualTime.at(i);
		cTotalDetectTime.at(i) += cDetectTime.at(i);
		cTotalCorrectTime.at(i) += cCorrectTime.at(i);
		cTotalWrongTime.at(i) += cWrongTime.at(i);
	}

	fout.open("result/errorrate.txt", ios_base::app);
	fout << audios.at(dataNo)->scoresFilename.substr(5, audios.at(dataNo)->scoresFilename.length()-12) << endl;
	fout << "	total time = " << totalTime << " s" << endl;
	fout << "	actual time = " << manualTime << " s" << endl;
	fout << "	detect time = " << detectTime << " s" << endl;
	fout << "	c_time = " << correctTime << " s" << endl;
	fout << "	w_time = " << wrongTime << " s" << endl;
	fout << "	correct% = " << (float)correctTime/detectTime*100 << "%" << endl;
	fout << "	w_error% = " << (float)wrongTime/detectTime*100 << "%" << endl;
	for (int i = 2; i < classesNum; i++) {
		fout << "	misclassify : " << endl;
		fout << "		" << classNames[i] << " : " << endl;
		fout << "			actual time = " << cManualTime.at(i) << " s" << endl;
		fout << "			detect time = " << cDetectTime.at(i) << " s" << endl;
		fout << "			c_time = " << cCorrectTime.at(i) << " s" << endl;
		fout << "			w_time = " << cWrongTime.at(i) << " s" << endl;
		fout << "			correct% = " << (float)cCorrectTime.at(i)/cDetectTime.at(i)*100 << "%" << endl;
		fout << "			w_error% = " << (float)cWrongTime.at(i)/cDetectTime.at(i)*100 << "%" << endl;
		for (int j = 2; j < classesNum; j++) {
			if (j != i) {
				fout << "		--> " << classNames[j] << " : " 
					<< misclassifiedTime.at(i).at(j) << " s (" 
					<< misclassifiedTime.at(i).at(j)/detectTime*100 << "%)" << endl;
			}
		}
	}
	fout << endl;
	fout.close();
}

void Segmentor::OutputTotalErrorRateToFile()
{
	fout.open("result/errorrate.txt", ios_base::app);
	fout << "total" << endl;
	fout << "	total time = " << totalManualTimeAll << " s" << endl;
	fout << "	actual time = " << totalManualTime << " s" << endl;
	fout << "	detect time = " << totalDetectTime << " s" << endl;
	fout << "	c_time = " << totalCorrectTime << " s" << endl;
	fout << "	w_time = " << totalWrongTime << " s" << endl;
	fout << "	correct% = " << (float)totalCorrectTime/totalDetectTime*100 << "%" << endl;
	fout << "	w_error% = " << (float)totalWrongTime/totalDetectTime*100 << "%" << endl;
	for (int i = 2; i < classesNum; i++) {
		fout << "	misclassify : " << endl;
		fout << "		" << classNames[i] << " : " << endl;
		fout << "			actual time = " << cTotalManualTime.at(i) << " s" << endl;
		fout << "			detect time = " << cTotalDetectTime.at(i) << " s" << endl;
		fout << "			c_time = " << cTotalCorrectTime.at(i) << " s" << endl;
		fout << "			w_time = " << cTotalWrongTime.at(i) << " s" << endl;
		fout << "			correct% = " << (float)cTotalCorrectTime.at(i)/cTotalDetectTime.at(i)*100 << "%" << endl;
		fout << "			w_error% = " << (float)cTotalWrongTime.at(i)/cTotalDetectTime.at(i)*100 << "%" << endl;
		for (int j = 2; j < classesNum; j++) {
			if (j != i) {
				fout << "		--> " << classNames[j] << " : " 
					<< totalMisclassifiedTime.at(i).at(j) << " s (" 
					<< totalMisclassifiedTime.at(i).at(j)/totalDetectTime*100 << "%)" << endl;
			}
		}
	}
	fout.close();
}

bool Segmentor::BelongsTo(TimeSeg tsA, TimeSeg tsB)
{
	if (tsA.leftTime > tsB.leftTime && tsA.rightTime < tsB.rightTime) {
		return true;
	} else if (abs(tsA.leftTime-tsB.leftTime) <= epsilon && tsA.rightTime < tsB.rightTime) {
		return true;
	} else if (tsA.leftTime > tsB.leftTime && abs(tsA.rightTime-tsB.rightTime) <= epsilon) {
		return true;
	} else {
		return false;
	}
}

void Segmentor::Clear()
{
	for (int i = 0; i < classesNum; i++) {
		misclassifiedTime.at(i).clear();
		for (int j = 0; j < classesNum; j++) {
			misclassifiedTime.at(i).push_back(0);
		}
	}
}
