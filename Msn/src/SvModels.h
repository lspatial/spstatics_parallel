#pragma once
#include <iostream>
#include "Matrix.h"
using namespace std;

struct Config 
{
	unsigned bShapefile;			// ��������������shapefile��ʽ��true�������ı���ʽ��false��
	unsigned long nSamples;		// ������
	unsigned long nBlockPoints;	// ��ɢ�����С
	unsigned long nStrata;		// ����[MSN]
	Vector weight;				// �����Ȩ��[MSN]
	string sSamples;			// �����ļ�
	string sBlockPoints;		// ������ɢ�ļ�
	string sSvParams;			// ���������ļ�
	string sSamplePopRatio;		// �����������ֵ����[BSHADE]
};

enum SvModelType
{
	Sv_Unk = 0,		// δ֪
	Sv_Sph = 1,		// ��״ģ��
	Sv_Gau = 2,		// Gaussģ��
	Sv_Exp = 3,		// ָ��ģ��
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
	SvModel & operator[](unsigned long nIndex);	//ע�⣺������1��ʼ
	unsigned long Length(void);

private:
	unsigned long nLen;
	SvModel *models;
};
