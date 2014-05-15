#ifndef _CUTBYRULESBUILDER_H_
#define _CUTBYRULESBUILDER_H_

class CutByRulesBuilder
{
public:
	CutByRulesBuilder(int _dataNo, vector<Audio *> _audios);
	virtual ~CutByRulesBuilder();
	void stepA();
	void stepB();
	void stepC();
	void stepD();
	vector<Audio *> product();
private:
	vector<Audio *> audios;
	vector<vector<int>> voiceNos;
	Clip clip;
	int dataNo, classesNum;
};

#endif
