#pragma once
#include "Matrix.h"
#include "lis.h"
#include "SvModels.h"
#include <iostream>
using namespace std;

class MSN
{
public:
	MSN(void);
	~MSN(void);
	
public:
	static double Cov(double dist, SvModel &model);
	static Vector Cov(Vector &dist, SvModel &model);
	static double Distance(Vector &v0, Vector &v1, unsigned long nDim = 2);
	static void Distance(Vector &v0, Matrix &m, Vector &d, unsigned long nDim = 2);
	
	//MSNģ�����Է�����
	static Matrix FillCMatrix(LIS_MATRIX &C, Matrix &samples, SvModels &models, Config &config, double dblZoomIn=1);
	static Vector FillDVector(LIS_VECTOR &D, Matrix &samples, Matrix &blkpts, SvModels &models, Config &config, double dblZoomIn=1);
	static double CAA(Matrix &blkPts, SvModels &models, Config &config, int nProcs, int nProcRank, int nRoot=0 );	//����ƽ��Э������ʱ��
	
	//��Ԫ�طֲ��ڲ�ͬ���̵�LIS_VECTOR���ݻ�۵�root���̵�Vector������
	static void Gather(LIS_VECTOR &v0, Vector &v1, int nProcs, int nProcRank, int nRoot=0);
	
	//��Ԫ�طֲ��ڲ�ͬ���̵�Matrix���ݻ�۵�root���̵�Matirx������
	static void Gather(Matrix &mLoc, Matrix &mGlob, int is, int ie, int nProcs, int nProcRank, int nRoot=0);
	
	//������Ȩ�ؼ�Lagrange������MSN�����ֵ���ƣ�
	static void AbstractWtLg(Vector &wu, Matrix &samples, Vector &wt, Vector &mu, Config &config);
	
	//�����ֵ����
	static double EstPopulationMean(Matrix &samples, Vector &wt);

	//�����ֵ���Ʒ���
	static double EstPopulationVar(double caa, Vector &wu, Vector &di, Config &config, double dblZoomIn=1);
private:
	static bool IsEqual(double d1, double d2);
	
	//����ǰ������LIS_VECTOR�Ĳ���Ԫ�ؿ�����Vector������
	static void GetCurProcValue(LIS_VECTOR &v, Vector &d);

};
