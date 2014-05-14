#include "Segmentor.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void main()
{
	ifstream fin;
	int num;
	string scoresFilename;
	string manualLabelFilename;

	int classesNum = 7;
	Segmentor segmentor(classesNum);

	fin.open("data/datalist.txt");
	fin >> num;
	segmentor.dataNum = num;
	for (int i = 0; i < num; i++) {
		fin >> scoresFilename;
		fin >> manualLabelFilename;
		cout << "reading...... " << scoresFilename << endl;
		segmentor.AddData(scoresFilename, manualLabelFilename);
	}
	cout << "processing...... " << endl;
	segmentor.ProcessAll();
	cout << "done!" << endl;
	fin.close();
}