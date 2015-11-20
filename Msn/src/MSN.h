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
	
	//MSN模型线性方程组
	static Matrix FillCMatrix(LIS_MATRIX &C, Matrix &samples, SvModels &models, Config &config, double dblZoomIn=1);
	static Vector FillDVector(LIS_VECTOR &D, Matrix &samples, Matrix &blkpts, SvModels &models, Config &config, double dblZoomIn=1);
	static double CAA(Matrix &blkPts, SvModels &models, Config &config, int nProcs, int nProcRank, int nRoot=0 );	//总体平均协方差（最耗时）
	
	//将元素分布在不同进程的LIS_VECTOR数据汇聚到root进程的Vector数组中
	static void Gather(LIS_VECTOR &v0, Vector &v1, int nProcs, int nProcRank, int nRoot=0);
	
	//将元素分布在不同进程的Matrix数据汇聚到root进程的Matirx数组中
	static void Gather(Matrix &mLoc, Matrix &mGlob, int is, int ie, int nProcs, int nProcRank, int nRoot=0);
	
	//样本点权重及Lagrange参数（MSN总体均值估计）
	static void AbstractWtLg(Vector &wu, Matrix &samples, Vector &wt, Vector &mu, Config &config);
	
	//总体均值估计
	static double EstPopulationMean(Matrix &samples, Vector &wt);

	//总体均值估计方差
	static double EstPopulationVar(double caa, Vector &wu, Vector &di, Config &config, double dblZoomIn=1);
private:
	static bool IsEqual(double d1, double d2);
	
	//将当前进程下LIS_VECTOR的部分元素拷贝至Vector数组中
	static void GetCurProcValue(LIS_VECTOR &v, Vector &d);

};
