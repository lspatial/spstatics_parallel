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
		float X;//X����
		float Y;//Y����
		float Z;//����ֵ
	};
	/// <summary>
	/// ������
	/// </summary>
public:
	struct Triangle
	{
		 int v0;    //����0
		 int v1;    //����1
		 int v2;    //����2
		 int v01;  //����ڽӵ�����������
		 int v12;  //����ڽӵ�����������
		 int v20;  //����ڽӵ�����������

		 Point2D CentrePoint;//���������ԲԲ��
	};

	/// <summary>
	/// �����ζ���
	/// </summary>
public:
	struct TriPoint
	{
		 int ID;
		 string Attribute;//��ʶ�Ƿ�Ϊ��Ч��
		 double X;
		 double Y;//��������Ϣ
		 int NextPoint;
		 int OverlapPoint;
	};
public:
	CGouTIN(int pointnum);//���캯��������ɢ�����
public:
	void Read_Points(char* filename,float **ScatterPoints);//��ȡ����Ϣ
	void BuildTIN(float **ScatterPoints,int samplenum);//��TIN����
	int FindDataBox(int samplenum);//Ѱ�������Ӿ���
	void CreatePointIndex(int samplenum);//����������
	void BuildSuperTriangle(int samplenum);//������������
	void InsertPoint(int PointNum, int StartTri);//������
	int FindTriangleEncloseP(int PointNum, int StartTri, int *TriContainP, int *EdgeNum);//Ѱ�ҵ����ڵ������Σ�-1  ���ڲ��ҵķ�Χ�ڣ�0  �������ζ����غϣ�  1  �������α��ϣ�2  ����������
	void InitializeTriangle2(int PointNum, int TriContainP, int EdgeNum);//���������α��ϣ��γ��������β��������˹�ϵ
	void InitializeTriangle1(int PointNum, int TriContainP);//�����������У��γ��������β��������˹�ϵ
	bool Delaunary(int PointNum, int TriInNew, int TriInStack);//LOP�Ż� 
	bool PushNStack(int TriNum);//��ջ
	bool PushStack(int TriNum);
	int PopStack();
	int PopNStack();//��ջ

	int read_samplenum(const char* pDataSource,const char* pLayerName,char** pSpatialRefWkt);
	int read_vector(const char* pDataSource,const char* pLayerName,char** pSpatialRefWkt,float **Sample_Array);
	void SetPointNum(int num);//����ɢ������
	int GetPointNum();//����ɢ������
	void Point2DToTriPoint(float **Points,TriPoint *tTriPoint,int samplenum);
	//int Point2DToTriPoint(Point2D *Points);//2D�����ת��ΪTIN��ɢ�㣨Ҫ���4��������������Σ�
	void Print_TIN(int **TIN_Array);
	int TIN_Main(const char* pDataSource,const char* pLayerName,char *resultFile);//�����������
public:
	//void Read_Points(char* filename,float **ArrayPoints);//��ȡ����Ϣ

	//void ParePro_Main();//�����������
	void Sort(float **Points,int samplenum);//ɢ������
	void GetDataBox();//��ȡ���ݴ��������
	void Split_Points(int rank,float **ArrayPoints);//�����ӽ��̵�ɢ��
	int Count_subsample(float minY,float maxY,float **Array);
	void Sub_array(int sub_sample,float MinY,float MaxY,float **Array,float **Sub_Array);
	void Print_time(double time1,double time2);
	void createFile(char *Filename);
	void WriteFile(char *Filename,int Tnum,int **T_Array);
	void Find_fourCorner(float **subArray,int *id,int samplenum);
	void Pointnum_line(int *id,int *num);//����߽����϶������Ŀ
	void Point_line(int *point_array,int *id,int all_num,int point_num,int tag);
	int Combine_twoline(int *this_line,int *next_line,int **tri_twoline,int this_point,int next_point,int tri_num);
public:
	float MaxX, MinX, MaxY, MinY;//�������
	float **ArrayPoints;//�洢�������
public:
	int PointNum;//ɢ������
	int AllTriangle;//����������
	int *PointIndex;//��������
	TriPoint *ScatterPointArray;//ɢ��
	Triangle *TriangleArray;//������
private:
	float **Points_array;//�洢����ȡ�ĵ���Ϣ
	Point2D *ScatterPoints;//�ṹ������
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

