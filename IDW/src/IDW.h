#pragma once
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
class CIDW
{
public:
	CIDW(void);
	~CIDW(void);
public:
	CIDW(float cellsize,int sample);
public:
	struct Sample_Array 
	{
		int i;
		int j;
		float value;
	};
public://定义要插值的范围
	typedef struct extent_info {
		double minX;
		double maxX;
		double minY;
		double maxY;
	} extent_info;
public:
	void Read_data_sample(char *filename2,double **Array);//读取采样点数据
	//void Tran_sample(float **Array);//采样点数据格式转换
	int Count_subsample(double minY,double maxY,double **Array);//统计每个子区域中包含的采样点个数，总方向上分割
	void Sub_array(int sub_sample,float MinY,float MaxY,double **Array,double *Array_X,double *Array_Y,double *Array_Z);//将子区域中的采样点坐标及值分开存储
public:
	float GetXcorner();
	float GetYcorner();
	int GetNrow();
	int GetNcol();
	int GetNsample();
	float GetCellsize();
	void SetNrow(int num);
	void SetNcol(int num);
	void SetNsample(int num);
	void Set_Singlesize(int single);
	void Set_Buffersize(int buffer);
public:
	struct Sample_Array *Tran_Array;
private:
	int Nrow;
	int Ncol;
	float xllcorner;
	float yllcorner;
	float h;
	float nodata;
	int Nsample;
	int single_size;
	int buffer_size;
public:
	extent_info extent_All;//整个插值区域的范围
};

