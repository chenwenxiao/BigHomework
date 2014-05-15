#include "Audio.h"
#include "CutByRulesBuilder.h"

CutByRulesBuilder::CutByRulesBuilder(int _dataNo, vector<Audio *> _audios)
{
	dataNo = _dataNo;
	audios = _audios;
}

CutByRulesBuilder::~CutByRulesBuilder()
{

}

void CutByRulesBuilder::stepA()
{
	vector<int> iTempVec;
	iTempVec.clear();
	for (int i = 0; i < audios.at(dataNo)->frames.size(); i++) {
		if (audios.at(dataNo)->frames.at(i).type != Silence) {
			if (iTempVec.size() == 0)
				iTempVec.push_back(i);
			else
				continue;
		}
		else {
			if (iTempVec.size() != 0) {
				iTempVec.push_back(i - 1);
				voiceNos.push_back(iTempVec);
				iTempVec.clear();
			}
			else
				continue;
		}
	}
	if (iTempVec.size() != 0) {
		iTempVec.push_back(audios.at(dataNo)->frames.size() - 1);
		voiceNos.push_back(iTempVec);
		iTempVec.clear();
	}

}

void CutByRulesBuilder::stepB()
{
	vector<int> bestClassNums;
	int maxNum;
	int maxNo;
	int m;
	for (int i = 0; i < voiceNos.size(); i++) {
		for (int j = voiceNos.at(i).at(0); j <= voiceNos.at(i).at(1); j += (FRAME_NUM_CLIP - FRAME_NUM_OVERLAP)) {
			bestClassNums.clear();
			for (int k = 0; k < classesNum + 3; k++)
				bestClassNums.push_back(0);
			for (m = 0; m < FRAME_NUM_CLIP; m++) {
				if (j + m > voiceNos.at(i).at(1))
					break;
				bestClassNums.at((int)audios.at(dataNo)->frames.at(j + m).type)++;
				if (audios.at(dataNo)->frames.at(j + m).type == Unknown);
				//+…–Œ¥¥¶¿Ìunkown
			}
			maxNum = bestClassNums.at(2);
			maxNo = 2;
			for (int k = 3; k < classesNum + 3; k++)
			if (bestClassNums.at(k) > maxNum)
				maxNum = bestClassNums.at(k), maxNo = k;
			clip.leftFrame = j;
			clip.rightFrame = j + m;
			if (clip.leftFrame < FRAME_NUM_CLIP)
				clip.leftClip = 0;
			else
				clip.leftClip = ceil((double)(clip.leftFrame + 1 - FRAME_NUM_CLIP) / (FRAME_NUM_CLIP - FRAME_NUM_OVERLAP));
			if (clip.rightFrame < FRAME_NUM_CLIP)
				clip.rightClip = 0;
			else
				clip.rightClip = ceil((double)(clip.rightFrame + 1 - FRAME_NUM_CLIP) / (FRAME_NUM_CLIP - FRAME_NUM_OVERLAP)) - 1;
			clip.type = types[maxNo];
			audios.at(dataNo)->clips.push_back(clip);
			if (m < FRAME_NUM_CLIP)
				break;
		}
	}


}

void CutByRulesBuilder::stepC()
{
	classes type1, type2, type3;
	bool isFirst, isLast;
	int n = 0;
	for (int i = 0; i < audios.at(dataNo)->clips.size() - 2; i++) {
		if (audios.at(dataNo)->clips.at(i).rightFrame - FRAME_NUM_OVERLAP != audios.at(dataNo)->clips.at(i + 1).leftFrame ||
			audios.at(dataNo)->clips.at(i + 1).rightFrame - FRAME_NUM_OVERLAP != audios.at(dataNo)->clips.at(i + 2).leftFrame)
			continue;

		if (i == 0)
			isFirst = true;
		else if (audios.at(dataNo)->clips.at(i).leftFrame != audios.at(dataNo)->clips.at(i - 1).rightFrame - FRAME_NUM_OVERLAP)
			isFirst = true;
		else
			isFirst = false;

		if (i + 2 == audios.at(dataNo)->clips.size() - 1)
			isLast = true;
		else if (audios.at(dataNo)->clips.at(i + 2).rightFrame - FRAME_NUM_OVERLAP != audios.at(dataNo)->clips.at(i + 3).leftFrame)
			isLast = true;
		else
			isLast = false;

		type1 = audios.at(dataNo)->clips.at(i).type;
		type2 = audios.at(dataNo)->clips.at(i + 1).type;
		type3 = audios.at(dataNo)->clips.at(i + 2).type;
		if (type1 != type2 && type2 != type3 && type3 != type1)
			audios.at(dataNo)->clips.at(i + 1).type = type1;
		if (type1 == type3 && type1 != type2)
			audios.at(dataNo)->clips.at(i + 1).type = type1;
		if (isFirst && type1 != type2 && type2 == type3)
			audios.at(dataNo)->clips.at(i).type = type2;
		if (isLast && type1 == type2 && type2 != type3)
			audios.at(dataNo)->clips.at(i + 2).type = type2;
	}

}

void CutByRulesBuilder::stepD()
{
	int tempMax, tempNum;
	for (int i = 0; i < audios.at(dataNo)->clips.size(); i++) {
		clip = audios.at(dataNo)->clips.at(i);
		for (int j = clip.leftFrame; j <= clip.rightFrame; j++) {
			if (j >= audios.at(dataNo)->frames.size())
				break;
			audios.at(dataNo)->frames.at(j).type = clip.type;
		}
	}
}

vector<Audio *> CutByRulesBuilder::product()
{
	return audios;
}