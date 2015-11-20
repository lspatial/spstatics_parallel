#pragma once
#include "Matrix.h"
#include "MSN.h"
#include <iostream>
using namespace std;

class MSN_Helper
{
public:

	static void LoadSamples(Config &config, Matrix &mat);	//效率更高（少一次深拷贝）
	static void LoadBlockPoints(Config &config, Matrix &mat);

	static void LoadSvParams(const Config &config, SvModels &models);
	static void LoadConfig(string sConfig, Config &config);

	static void Save(Vector &v, string sfile);
	static void Save(Matrix &m, string sfile);

};
