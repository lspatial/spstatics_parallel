#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

class CGouTIN
{
public:
	CGouTIN(void);
	~CGouTIN(void);
public:
    struct Point2D
	{
		float X;//X坐标
		float Y;//Y坐标
		float Z;//属性值
	};
	/// <summary>
	/// 三角形
	/// </summary>
public:
	struct Triangle
	{
		 int v0;    //顶点0
		 int v1;    //顶点1
		 int v2;    //顶点2
		 int v01;  //与边邻接的三角形索引
		 int v12;  //与边邻接的三角形索引
		 int v20;  //与边邻接的三角形索引

		 Point2D CentrePoint;//三角形外接圆圆心
	};

	/// <summary>
	/// 三角形顶点
	/// </summary>
public:
	struct TriPoint
	{
		 int ID;
		 string Attribute;//标识是否为有效点
		 double X;
		 double Y;//点坐标信息
		 int NextPoint;
		 int OverlapPoint;
	};
public:
	CGouTIN(int pointnum);//构造函数参数：散点个数
public:
	void Read_Points(char* filename,float **ScatterPoints);//读取点信息
	void BuildTIN(float **ScatterPoints,int samplenum);//构TIN函数
	int FindDataBox(int samplenum);//寻找最大外接矩形
	void CreatePointIndex(int samplenum);//创建点索引
	void BuildSuperTriangle(int samplenum);//创建超三角形
	void InsertPoint(int PointNum, int StartTri);//逐点插入
	int FindTriangleEncloseP(int PointNum, int StartTri, int *TriContainP, int *EdgeNum);//寻找点所在的三角形，-1  不在查找的范围内，0  和三角形顶点重合，  1  在三角形边上，2  在三角形内
	void InitializeTriangle2(int PointNum, int TriContainP, int EdgeNum);//点在三角形边上，形成新三角形并建立拓扑关系
	void InitializeTriangle1(int PointNum, int TriContainP);//点在三角形中，形成新三角形并建立拓扑关系
	bool Delaunary(int PointNum, int TriInNew, int TriInStack);//LOP优化 
	bool PushNStack(int TriNum);//进栈
	bool PushStack(int TriNum);
	int PopStack();
	int PopNStack();//出栈

	int read_samplenum(const char* pDataSource,const char* pLayerName,char** pSpatialRefWkt);
	int read_vector(const char* pDataSource,const char* pLayerName,char** pSpatialRefWkt,float **Sample_Array);
	void SetPointNum(int num);//设置散点数量
	int GetPointNum();//返回散点数量
	void Point2DToTriPoint(float **Points,TriPoint *tTriPoint,int samplenum);
	//int Point2DToTriPoint(Point2D *Points);//2D坐标点转化为TIN的散点（要多出4个，数据外包矩形）
	void Print_TIN(int **TIN_Array);
	int TIN_Main(const char* pDataSource,const char* pLayerName,char *resultFile);//本类的主函数
public:
	//void Read_Points(char* filename,float **ArrayPoints);//读取点信息

	//void ParePro_Main();//本类的主函数
	void Sort(float **Points,int samplenum);//散点排序
	void GetDataBox();//获取数据大外包矩形
	void Split_Points(int rank,float **ArrayPoints);//分配子进程的散点
	int Count_subsample(float minY,float maxY,float **Array);
	void Sub_array(int sub_sample,float MinY,float MaxY,float **Array,float **Sub_Array);
	void Print_time(double time1,double time2);
	void createFile(char *Filename);
	void WriteFile(char *Filename,int Tnum,int **T_Array);
	void Find_fourCorner(float **subArray,int *id,int samplenum);
	void Pointnum_line(int *id,int *num);//计算边界线上顶点的数目
	void Point_line(int *point_array,int *id,int all_num,int point_num,int tag);
	int Combine_twoline(int *this_line,int *next_line,int **tri_twoline,int this_point,int next_point,int tri_num);
public:
	float MaxX, MinX, MaxY, MinY;//外包矩形
	float **ArrayPoints;//存储点的数组
public:
	int PointNum;//散点数量
	int AllTriangle;//三角形数量
	int *PointIndex;//创建索引
	TriPoint *ScatterPointArray;//散点
	Triangle *TriangleArray;//三角形
private:
	float **Points_array;//存储所读取的点信息
	Point2D *ScatterPoints;//结构体数组
	double xMin;
	double yMin;
	double xMax;
	double yMax;
	int size;
	int binRow;
	int binCol;
	int Allbin;
	int Nbin;
	int top;
	int ntop;
	int *SStack; //= new int[10000];
	int *NStack; //= new int[10000];
	int NumberSamePoint;
};

