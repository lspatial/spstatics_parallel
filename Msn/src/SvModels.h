#pragma once
#include <iostream>
#include "Matrix.h"
using namespace std;

struct Config 
{
	unsigned bShapefile;			// 输入样本数据是shapefile格式（true）还是文本格式（false）
	unsigned long nSamples;		// 样本数
	unsigned long nBlockPoints;	// 离散总体大小
	unsigned long nStrata;		// 层数[MSN]
	Vector weight;				// 各层的权重[MSN]
	string sSamples;			// 样本文件
	string sBlockPoints;		// 总体离散文件
	string sSvParams;			// 半变异参数文件
	string sSamplePopRatio;		// 样本与总体均值比例[BSHADE]
};

enum SvModelType
{
	Sv_Unk = 0,		// 未知
	Sv_Sph = 1,		// 球状模型
	Sv_Gau = 2,		// Gauss模型
	Sv_Exp = 3,		// 指数模型
};
#pragma once

class SvModel
{
public:
	SvModel(void)
	{
		type = Sv_Unk;
		nugget = 0;
		psill = 0;
		range = 0;
	}

public:
	SvModelType type;
	double nugget, psill, range;
};

class SvModels
{
public:
	SvModels(void);
	SvModels(unsigned long nLen);
	SvModels(SvModels &models);
	~SvModels(void);
	void Resize(unsigned long nSize);
	void Print(void);
	SvModels operator=(SvModels &models);
	SvModel & operator[](unsigned long nIndex);	//注意：索引从1开始
	unsigned long Length(void);

private:
	unsigned long nLen;
	SvModel *models;
};
