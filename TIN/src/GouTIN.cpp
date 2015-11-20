#include "GouTIN.h"
#include "mpi.h"
#include <stdlib.h>
#include <iomanip>
#include <cmath>
#include "ogrsf_frmts.h"
#include "gdal_priv.h"
#include <gdal_alg.h>
#include "cpl_string.h"
#include<gdal.h>
#include <ogr_spatialref.h>
#include <gdal_priv.h>
#pragma comment(lib,"gdal_i.lib")
#define NoData -1
#define InvalidID -11111
#define allowance 0.00000000000001
using namespace std;

CGouTIN::CGouTIN(void)
{
}


CGouTIN::~CGouTIN(void)
{
}
CGouTIN::CGouTIN(int pointnum)
{
	//PointNum=pointnum;
}
//本函数返回矢量点的数目
int CGouTIN::read_samplenum(const char* pDataSource,const char* pLayerName,char** pSpatialRefWkt)
{
	//将类CIDW做参数
	OGRRegisterAll();
	OGRDataSource *poDS=OGRSFDriverRegistrar::Open(pDataSource,FALSE);
	//OGRDataSource *poDS=OGRSFDriverRegistrar::Open("point.shp",FALSE);
	if( poDS == NULL )
	{
		printf( "[ERROR] zmw Open failed.\n" );
		exit( 1 );
	}


	OGRLayer *poLayer = poDS->GetLayerByName(pLayerName);
	//cout<<f3.c_str()<<endl;
	//cout<<"here2"<<endl;
	//OGRSpatialReference * sref= poLayer->GetSpatialRef();
	//sref->exportToWkt(pSpatialRefWkt);
	//cout<<"here3"<<endl;
	OGRFeature *poFeature;

	poLayer->ResetReading();
	//cout<<"here4"<<endl;
	int count = 0;//统计散点数
	while((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		count++;
	}
	OGRDataSource::DestroyDataSource( poDS );
	return count;
}

int CGouTIN::read_vector(const char* pDataSource,const char* pLayerName,char** pSpatialRefWkt,float **Sample_Array)
{
	//将位置信息和属性信息存放在数组Sample_Array中
	//cout<<"here4.4"<<endl;
	OGRRegisterAll();
	//cout<<filename<<endl;
	OGRDataSource *poDS=OGRSFDriverRegistrar::Open(pDataSource,FALSE);
	if( poDS == NULL )
	{
		printf( "[ERROR] Open failed.\n" );
		exit( 1 );
	}
	//cout<<"here4.5"<<endl;
	
	OGRLayer *poLayer = poDS->GetLayerByName(pLayerName);

	//OGRSpatialReference * sref= poLayer->GetSpatialRef();
	//sref->exportToWkt(pSpatialRefWkt);

	OGRFeature *poFeature;

	poLayer->ResetReading();
	/*int count = 0;//统计散点数
	while((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		count++;
	}
	poLayer->ResetReading();*/

	//不需要下面的存储空间，故注释掉
	//*ptArray = annAllocPts(count, 2);
	//*ptValues = new double[count];
	//cout<<"here5"<<endl;
	int idx = 0;
	double x=0.0;
	double y=0.0;
	while((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		//(*ptValues)[idx] = poFeature->GetFieldAsDouble(fieldIdx);
		Sample_Array[idx][2]=poFeature->GetFieldAsDouble(2);//读取属性值
		//cout<<"value:"<<Sample_Array[idx][2]<<endl;
		OGRGeometry *poGeometry;

		poGeometry = poFeature->GetGeometryRef();
		if( poGeometry != NULL&& wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
		{
			OGRPoint *poPoint=(OGRPoint *)poGeometry;
			x=poPoint->getX();
			y=poPoint->getY();
			//存储位置信息
			Sample_Array[idx][0]=x;
			Sample_Array[idx][1]=y;
			//(*ptArray)[idx][0] = x;
			//(*ptArray)[idx][1] = y;
			if(idx==0)
			{
				//extent.minX = extent.maxX = x;
				//extent.minY = extent.maxY = y;
				MinX=x;
				MaxX=x;
				MinY=y;
				MaxY=y;
			}
			else
			{
				if(x>MaxX)
					MaxX = x;
				if(x<MinX)
					MinX = x;
				if(y>MaxY)
					MaxY = y;
				if(y<MinY)
					MinY = y;
			}
		}
		else
		{
			printf( "[ERROR] No point geometry\n" );
			return 1;
		}
		OGRFeature::DestroyFeature( poFeature );
		idx++;
	}

	OGRDataSource::DestroyDataSource( poDS );
	return 0;
}
//读取散点
void CGouTIN::Read_Points(char* filename,float **ScatterPoints)
{

	ifstream i_file;
	i_file.open(filename);
	char buffer[1024];

	float *result=new float[PointNum*3];//数组大小应大于采样点数目
	
	if (i_file.is_open())
	{
		string buffer;
		i_file>>buffer;
		i_file>>buffer;
		i_file>>buffer;

		float out_test;
		int i=0;
		while(i_file>>out_test)//以文本文件中的行为顺序，逐个读入，不管列
		{

			//cout<<std::fixed<<out_test<<endl;
			result[i]=out_test;
			i++;

		}
	}
	else
		cout << "打开文件：" <<filename << " 时出错！"; 
	i_file.close();
	int ii=0;
	//转换成矩阵
	MaxX=-999999999;
	MaxY=-999999999;
	MinX=999999999;
	MinY=999999999;
	for(int i=0;i<PointNum;i++)
	{
		ScatterPoints[i][0]=result[ii];
		ScatterPoints[i][1]=result[ii+1];
		if (ScatterPoints[i][0]<MinX)
		{
			MinX=ScatterPoints[i][0];
		}
		if (ScatterPoints[i][0]>MaxX)
		{
			MaxX=ScatterPoints[i][0];
		}
		if (ScatterPoints[i][1]<MinY)
		{
			MinY=ScatterPoints[i][1];
		}
		if (ScatterPoints[i][1]>MaxY)
		{
			MaxY=ScatterPoints[i][1];
		}
		ScatterPoints[i][2]=result[ii+2];
		ii=ii+3;
	}
	delete []result;


	i_file.close();


}
/// 2D坐标点转化为TIN的散点（要多出4个，数据外包矩形）
//我这里将2D坐标点直接存在数组中
void CGouTIN::Point2DToTriPoint(float **Points,TriPoint *tTriPoint,int samplenum)
{
	//TriPoint *tTriPoint = new TriPoint[PointNum + 4];

	for (int i = 0; i < samplenum; i++)
	{
		tTriPoint[i].X = Points[i][0];
		tTriPoint[i].Y = Points[i][1];
		tTriPoint[i].NextPoint = -1;
		tTriPoint[i].OverlapPoint = -1;
		//cout<<"tTriPoint[i].X="<<tTriPoint[i].X<<endl;
		//cout<<"tTriPoint[i].Y="<<tTriPoint[i].Y<<endl;
	}

	//return tTriPoint;
}
//对读入的点排序:按x值的从小到大排序//应该是按Y吧
void CGouTIN::Sort(float **Points,int samplenum)
{
	double *tX = new double[samplenum];

	for (int i = 0; i < samplenum; i++)
	{
		tX[i] =Points[i][1];
	}

	for (int i = 1; i < samplenum; i++)
	{
		double tFlag = tX[i];
		//Point2D tSFlag = Points[i];
		double tSFlag_0=Points[i][0];
		double tSFlag_1=Points[i][1];
		double tSFlag_2=Points[i][2];

		int j = i;

		while (j > 0 && tX[j - 1] < tFlag)
		{
			tX[j] = tX[j - 1];
			//Points[j] = Points[j - 1];
			Points[j][0]=Points[j-1][0];
			Points[j][1]=Points[j-1][1];
			Points[j][2]=Points[j-1][2];
			j--;
		}

		tX[j] = tFlag;
		//Points[j] = tSFlag;
		Points[j][0]=tSFlag_0;
		Points[j][1]=tSFlag_1;
		Points[j][2]=tSFlag_2;
	}
}
void CGouTIN::Split_Points(int rank,float **ArrayPoints)
{

}
void CGouTIN::BuildTIN(float **ScatterPoints,int samplenum)
{
	int ScatterPointNum =samplenum;// GetPointNum();//散点数量
	//int ScatterPointArray = Point2DToTriPoint(ScatterPoints);//三角形数量

	//结构体数组声明

	ScatterPointArray = new TriPoint[samplenum + 4];

	Point2DToTriPoint(ScatterPoints,ScatterPointArray,samplenum);

	int top = 0;
	int ntop = 0;
	AllTriangle = 0;
	Allbin=0;
	Allbin=FindDataBox(samplenum);
	CreatePointIndex(samplenum);
	BuildSuperTriangle(samplenum);

	SStack= new int[10000];
	NStack= new int[10000];

	int Point_Num;
	//PointIndex = new int[Allbin];
	for (int i = 0; i < Allbin; i++)
	{
		if (PointIndex[i] == NoData) continue;

		Point_Num = PointIndex[i];
		//cout<<i<<"eee:"<<Point_Num<<endl;
		while (Point_Num != NoData)
		{
			//cout<<AllTriangle<<endl;
			
			//cout<<"fff:"<<Point_Num<<endl;
			InsertPoint(Point_Num, AllTriangle - 1);

			Point_Num = ScatterPointArray[Point_Num].NextPoint;
		}
	}
	//Print_TIN();
}
//打印构建的三角形
void CGouTIN::Print_TIN(int **TIN_Array)
{
	for (int i=0;i<AllTriangle;i++)
	{
		//cout<<"Triangle"<<i<<":"<<"v0:"<<TriangleArray[i].v0<<"   "<<"v1:"<<TriangleArray[i].v1<<"   "<<"v2:"<<TriangleArray[i].v2<<endl;
		TIN_Array[i][0]=TriangleArray[i].v0;
		TIN_Array[i][1]=TriangleArray[i].v1;
		TIN_Array[i][2]=TriangleArray[i].v2;
	}
}
int CGouTIN::FindDataBox(int samplenum)
{
	//cout<<ScatterPointArray[0].X<<endl;
	xMin = ScatterPointArray[0].X;
	yMin = ScatterPointArray[0].Y;
	xMax = ScatterPointArray[0].X;
	yMax = ScatterPointArray[0].Y;

	//寻找最大外接矩形
	for (int i = 0; i <samplenum; i++)
	{
		if (ScatterPointArray[i].X < xMin) xMin = ScatterPointArray[i].X;
		if (ScatterPointArray[i].Y < yMin) yMin = ScatterPointArray[i].Y;
		if (ScatterPointArray[i].X > xMax) xMax = ScatterPointArray[i].X;
		if (ScatterPointArray[i].Y > yMax) yMax = ScatterPointArray[i].Y;
	}

	double dx = xMax - xMin;
	double dy = yMax - yMin;

	int perbin = (int)(sqrt((double)samplenum));
	size = (int)(sqrt(dx * dy / perbin));

	binCol = (int)(dx / size) + 1;
	binRow = (int)(dy / size) + 1;
	//cout<<xMin<<endl;
	Allbin = binRow * binCol;
	//cout<<Allbin<<endl;
	PointIndex = new int[Allbin];
	return Allbin;
}
void CGouTIN::CreatePointIndex(int samplenum)
{
	int i, icell, jcell;
	//int Nbin=0;
	//cout<<Allbin<<endl;
	for (i = 0; i < Allbin; i++) 
	{
		PointIndex[i] = NoData;
		//cout<<i<<PointIndex[i]<<endl;
	}
	for (i = 0; i <samplenum; i++)
	{
		jcell = (int)((ScatterPointArray[i].X - xMin) / size);
		icell = (int)((ScatterPointArray[i].Y - yMin) / size);
		Nbin = icell * binCol + jcell;
		//cout<<Nbin<<endl;
		if (PointIndex[Nbin] == NoData)
		{
			PointIndex[Nbin] = i;
			ScatterPointArray[i].NextPoint = NoData;
		}
		else
		{
			ScatterPointArray[i].NextPoint = PointIndex[Nbin];
			PointIndex[Nbin] = i;
		}
	}
	//调试
	//调试
}
void CGouTIN::BuildSuperTriangle(int samplenum)
{
	//第一个点
	ScatterPointArray[samplenum].Attribute = InvalidID;
	ScatterPointArray[samplenum].X = xMin;
	ScatterPointArray[samplenum].Y = yMin;
	ScatterPointArray[samplenum].NextPoint = NoData;

	//第二个点
	ScatterPointArray[samplenum + 1].Attribute = InvalidID;
	ScatterPointArray[samplenum + 1].X = xMax;
	ScatterPointArray[samplenum + 1].Y = yMin;
	ScatterPointArray[samplenum + 1].NextPoint = NoData;

	//第三个点
	ScatterPointArray[samplenum + 2].Attribute = InvalidID;
	ScatterPointArray[samplenum + 2].X = xMax;
	ScatterPointArray[samplenum + 2].Y = yMax;
	ScatterPointArray[samplenum + 2].NextPoint = NoData;

	//第四个点
	ScatterPointArray[samplenum + 3].Attribute = InvalidID;
	ScatterPointArray[samplenum + 3].X = xMin;
	ScatterPointArray[samplenum + 3].Y = yMax;
	ScatterPointArray[samplenum + 3].NextPoint = NoData;

	int nn = samplenum * 5 + 4;//是否需要修改×××××××××××××××
	TriangleArray = new Triangle[nn];
/*
	for (int i = 0; i < nn; i++)
	{
		TriangleArray[i] = new Triangle();
	}*/

	//两个超三角形
	TriangleArray[0].v0 =samplenum;
	TriangleArray[0].v1 =samplenum + 1;
	TriangleArray[0].v2 =samplenum + 2;
	TriangleArray[0].v01 = NoData;
	TriangleArray[0].v12 = NoData;
	TriangleArray[0].v20 = 1;

	TriangleArray[1].v0 =samplenum;
	TriangleArray[1].v1 =samplenum + 2;
	TriangleArray[1].v2 =samplenum + 3;
	TriangleArray[1].v01 = 0;
	TriangleArray[1].v12 = NoData;
	TriangleArray[1].v20 = NoData;

	AllTriangle = AllTriangle + 2;
}
void CGouTIN::InsertPoint(int Point_Num, int StartTri)
{
	
	int TriInStack, TriInNew;
	top = 0;
	ntop = 0;
	for (int m = 0; m < 10000; m++)
	{
		SStack[m] = 0;
		NStack[m] = 0;
	}
	//调试
	//调试
	int TriContainP=0;
	int EdgeNum=0;
	int flag = FindTriangleEncloseP(Point_Num, StartTri, &TriContainP, &EdgeNum);// 寻找点所在的三角形，-1  不在查找的范围内，0  和三角形顶点重合，  1  在三角形边上，2  在三角形内
	//cout<<"fff:"<<TriContainP<<endl;
	//cout<<"flag="<<flag<<endl;
	if (flag <= 0)
	{
		NumberSamePoint++;
		return;
	}
	else if (flag == 1)
		InitializeTriangle2(Point_Num, TriContainP, EdgeNum);  //点在边上的情况
	else if (flag == 2)
		InitializeTriangle1(Point_Num, TriContainP);   //点在三角形内

	while ((top) > 0)
	{
		TriInStack = PopStack(); ///形成的新三角形的邻接三角形
		TriInNew = PopNStack(); //形成的新三角形
		if (TriInStack == NoData) continue;
		Delaunary(Point_Num, TriInNew, TriInStack);//LOP优化
	}
}
/// <summary>
/// 寻找点所在的三角形，-1  不在查找的范围内，0  和三角形顶点重合，  1  在三角形边上，2  在三角形内
/// </summary>
/// <param name="PointNum">点的索引号</param>
/// <param name="StartTri">起始三角形</param>
/// <param name="TriContainP">包含点的三角形</param>
/// <param name="EdgeNum">边的编号</param>
/// <returns></returns>
int CGouTIN::FindTriangleEncloseP(int Point_Num, int StartTri,  int *TriContainP,  int *EdgeNum)
{
	TriContainP = 0;
	EdgeNum = 0;

	int *pv = new int[3]; // 存放三角形的三个顶点
	int i, j;
	double a, b, px, py;
	double x_min, y_min, x_max, y_max;
	TriPoint pli, plj; //三角形一条边的两个顶点
	double xx, yy;
	//cout<<Point_Num<<endl;
	px = ScatterPointArray[Point_Num].X;
	py = ScatterPointArray[Point_Num].Y;
	//调试
/*
	if (Point_Num==715)
	{
		int aaa=10;
	}*/
	//调试
	//判断点是否在外接矩形内
	if (px > xMax || px < xMin || py > yMax || py < yMin) return -1;

loop:
	if (StartTri < 0) return -1;
	//cout<<"StartTri:"<<StartTri<<endl;
	pv[0] = TriangleArray[StartTri].v0;
	pv[1] = TriangleArray[StartTri].v1;
	pv[2] = TriangleArray[StartTri].v2;

	//判断与三角形的顶点是否重合
	for (i = 0; i < 3; i++)
	{
		xx = ScatterPointArray[pv[i]].X;
		yy = ScatterPointArray[pv[i]].Y;

		if (((xx - px) * (xx - px) + (yy - py) * (yy - py)) < allowance)
		{
			ScatterPointArray[Point_Num].OverlapPoint = pv[i];
			return 0;
		}
	}

	//面积坐标法判断三角形的每条边
	for (i = 0; i < 3; i++)
	{
		j = i + 1;
		if (j > 2) j = 0;

		pli.X = ScatterPointArray[pv[i]].X;
		pli.Y = ScatterPointArray[pv[i]].Y;
		plj.X = ScatterPointArray[pv[j]].X;
		plj.Y = ScatterPointArray[pv[j]].Y;

		a = (plj.X - pli.X) * (py - pli.Y);
		b = (plj.Y - pli.Y) * (px - pli.X);

		if (abs(a - b) <= 0.0001) //判断是否重合边
		{
			x_min = min(pli.X, plj.X);
			x_max = max(pli.X, plj.X);
			y_min = min(pli.Y, plj.Y);
			y_max = max(pli.Y, plj.Y);
			if (px <= x_max && px >= x_min && py <= y_max && py >= y_min)
			{
				TriContainP = &StartTri;
				EdgeNum = &i;
				return 1;
			}
		}

		//需要调整
		//cout<<"a-b="<<a-b<<"    "<<"Point_num="<<Point_Num<<endl;
		if ((a - b) < 0)
		{
			if (i == 0)
			{
				StartTri = TriangleArray[StartTri].v01;
				//goto loop;
			}

			if (i == 1)
			{
				StartTri = TriangleArray[StartTri].v12;
				//goto loop;
			}
			if (i == 2)
			{
				StartTri = TriangleArray[StartTri].v20;
				//goto loop;
			}

		}
		//需要调整
	}

	TriContainP = &StartTri;
	return 2;
}
void CGouTIN::InitializeTriangle2(int Point_Num, int TriContainP, int EdgeNum)
{
	int atri;  //与当前三角形具有公共边的三角形
	int k1, k2, k3, ak12, ak23, ak31; //当前三角形信息
	int v1, v2, v3, av12, av23, av31;  //邻接三角形信息
	int p1, p2, p3, p4, ap12, ap23, ap34, ap41;  //整个四边形信息

	ap12 = 0;  //注意
	ap23 = 0;
	p2 = 0;
	//cout<<TriContainP<<endl;
	k1 = TriangleArray[TriContainP].v0;
	k2 = TriangleArray[TriContainP].v1;
	k3 = TriangleArray[TriContainP].v2;
	ak12 = TriangleArray[TriContainP].v01;
	ak23 = TriangleArray[TriContainP].v12;
	ak31 = TriangleArray[TriContainP].v20;

	if (EdgeNum == 0)
	{
		p1 = k1; p3 = k2; p4 = k3;
		ap34 = ak23; ap41 = ak31;
		//存在问题
		atri = TriangleArray[TriContainP].v01;
		if (atri != -1)
		{

			v1 = TriangleArray[atri].v0;
			v2 = TriangleArray[atri].v1;
			v3 = TriangleArray[atri].v2;
			av12 = TriangleArray[atri].v01;
			av23 = TriangleArray[atri].v12;
			av31 = TriangleArray[atri].v20;

			if ((k1 == v1 && k2 == v2) || (k1 == v2 && k2 == v1))
			{
				p2 = v3;
				ap12 = av23;
				ap23 = av31;
			}
			else if ((k1 == v2 && k2 == v3) || (k1 == v3 && k2 == v2))
			{
				p2 = v1;
				ap12 = av31;
				ap23 = av12;
			}
			else if ((k1 == v3 && k2 == v1) || (k1 == v1 && k2 == v3))
			{
				p2 = v2;
				ap12 = av12;
				ap23 = av23;
			}

			//调整邻接三角形拓扑关系
			if (ap12 == ap41) //凹四边形的情况1
			{
				if (ap12 != NoData)
				{

					if (TriangleArray[ap12].v01 == atri) TriangleArray[ap12].v01 = TriContainP;
					else if (TriangleArray[ap12].v01 == TriContainP) TriangleArray[ap12].v01 = AllTriangle + 1;

					if (TriangleArray[ap12].v12 == atri) TriangleArray[ap12].v12 = TriContainP;
					else if (TriangleArray[ap12].v12 == TriContainP) TriangleArray[ap12].v12 = AllTriangle + 1;

					if (TriangleArray[ap12].v20 == atri) TriangleArray[ap12].v20 = TriContainP;
					else if (TriangleArray[ap12].v20 == TriContainP) TriangleArray[ap12].v20 = AllTriangle + 1;
				}
			}
			else
			{
				if (ap12 != NoData)
				{
					if (TriangleArray[ap12].v01 == atri) TriangleArray[ap12].v01 = TriContainP;
					else if (TriangleArray[ap12].v12 == atri) TriangleArray[ap12].v12 = TriContainP;
					else if (TriangleArray[ap12].v20 == atri) TriangleArray[ap12].v20 = TriContainP;

				}
				if (ap41 != NoData)
				{
					if (TriangleArray[ap41].v01 == TriContainP) TriangleArray[ap41].v01 = AllTriangle + 1;
					else if (TriangleArray[ap41].v12 == TriContainP) TriangleArray[ap41].v12 = AllTriangle + 1;
					else if (TriangleArray[ap41].v20 == TriContainP) TriangleArray[ap41].v20 = AllTriangle + 1;

				}
			}

			if (ap23 == ap34) //凹四边形的情况2
			{
				if (ap23 != NoData)
				{
					if (TriangleArray[ap23].v01 == atri) TriangleArray[ap23].v01 = atri;
					else if (TriangleArray[ap23].v01 == TriContainP) TriangleArray[ap23].v01 = AllTriangle;

					if (TriangleArray[ap23].v12 == atri) TriangleArray[ap23].v12 = atri;
					else if (TriangleArray[ap23].v12 == TriContainP) TriangleArray[ap23].v12 = AllTriangle;

					if (TriangleArray[ap23].v20 == atri) TriangleArray[ap23].v20 = atri;
					else if (TriangleArray[ap23].v20 == TriContainP) TriangleArray[ap23].v20 = AllTriangle;

				}
			}
			else
			{
				if (ap23 != NoData)
				{
					if (TriangleArray[ap23].v01 == atri) TriangleArray[ap23].v01 = atri;
					else if (TriangleArray[ap23].v12 == atri) TriangleArray[ap23].v12 = atri;
					else if (TriangleArray[ap23].v20 == atri) TriangleArray[ap23].v20 = atri;

				}
				if (ap34 != NoData)
				{
					if (TriangleArray[ap34].v01 == TriContainP) TriangleArray[ap34].v01 = AllTriangle;
					else if (TriangleArray[ap34].v12 == TriContainP) TriangleArray[ap34].v12 = AllTriangle;
					else if (TriangleArray[ap34].v20 == TriContainP) TriangleArray[ap34].v20 = AllTriangle;

				}
			}

			//形成的新的四个三角形	

			TriangleArray[TriContainP].v0 = p1; TriangleArray[TriContainP].v01 = ap12;
			TriangleArray[TriContainP].v1 = p2; TriangleArray[TriContainP].v12 = atri;
			TriangleArray[TriContainP].v2 = Point_Num; TriangleArray[TriContainP].v20 = AllTriangle + 1;


			TriangleArray[atri].v0 = p2; TriangleArray[atri].v01 = ap23;
			TriangleArray[atri].v1 = p3; TriangleArray[atri].v12 = AllTriangle;
			TriangleArray[atri].v2 = Point_Num; TriangleArray[atri].v20 = TriContainP;


			TriangleArray[AllTriangle].v0 = p3; TriangleArray[AllTriangle].v01 = ap34;
			TriangleArray[AllTriangle].v1 = p4; TriangleArray[AllTriangle].v12 = AllTriangle + 1;
			TriangleArray[AllTriangle].v2 = Point_Num; TriangleArray[AllTriangle].v20 = atri;


			TriangleArray[AllTriangle + 1].v0 = p4; TriangleArray[AllTriangle + 1].v01 = ap41;
			TriangleArray[AllTriangle + 1].v1 = p1; TriangleArray[AllTriangle + 1].v12 = TriContainP;
			TriangleArray[AllTriangle + 1].v2 = Point_Num; TriangleArray[AllTriangle + 1].v20 = AllTriangle;


			PushNStack(TriContainP);
			PushNStack(atri);
			PushNStack(AllTriangle);
			PushNStack(AllTriangle + 1);
			PushStack(ap12);
			PushStack(ap23);
			PushStack(ap34);
			PushStack(ap41);

			AllTriangle = AllTriangle + 2;
		}
		else
		{
			if (ap41 != NoData)
			{
				if (TriangleArray[ap41].v01 == TriContainP) TriangleArray[ap41].v01 = AllTriangle;
				else if (TriangleArray[ap41].v12 == TriContainP) TriangleArray[ap41].v12 = AllTriangle;
				else if (TriangleArray[ap41].v20 == TriContainP) TriangleArray[ap41].v20 = AllTriangle;

			}
			//形成的新的两个三角形	
			TriangleArray[TriContainP].v0 = p3; TriangleArray[TriContainP].v01 = ap34;
			TriangleArray[TriContainP].v1 = p4; TriangleArray[TriContainP].v12 = AllTriangle;
			TriangleArray[TriContainP].v2 = Point_Num; TriangleArray[TriContainP].v20 = atri;

			TriangleArray[AllTriangle].v0 = p4; TriangleArray[AllTriangle].v01 = ap41;
			TriangleArray[AllTriangle].v1 = p1; TriangleArray[AllTriangle].v12 = atri;
			TriangleArray[AllTriangle].v2 = Point_Num; TriangleArray[AllTriangle].v20 = TriContainP;

			PushNStack(TriContainP);
			PushNStack(AllTriangle);
			PushStack(ap34);
			PushStack(ap41);

			AllTriangle = AllTriangle + 1;

		}
	}
	else if (EdgeNum == 1)
	{
		p1 = k2; p3 = k3; p4 = k1;
		ap34 = ak31; ap41 = ak12;

		atri = TriangleArray[TriContainP].v12;
		if (atri != -1)
		{

			v1 = TriangleArray[atri].v0; av12 = TriangleArray[atri].v01;
			v2 = TriangleArray[atri].v1; av23 = TriangleArray[atri].v12;
			v3 = TriangleArray[atri].v2; av31 = TriangleArray[atri].v20;

			if ((k2 == v1 && k3 == v2) || (k2 == v2 && k3 == v1))
			{
				p2 = v3;
				ap12 = av23;
				ap23 = av31;
			}
			else if ((k2 == v2 && k3 == v3) || (k2 == v3 && k3 == v2))
			{
				p2 = v1;
				ap12 = av31;
				ap23 = av12;
			}
			else if ((k2 == v3 && k3 == v1) || (k2 == v1 && k3 == v3))
			{
				p2 = v2;
				ap12 = av12;
				ap23 = av23;
			}
			//修改邻接三角形的拓扑关系
			if (ap12 == ap41)
			{
				if (ap12 != NoData)
				{
					if (TriangleArray[ap12].v01 == atri) TriangleArray[ap12].v01 = TriContainP;
					else if (TriangleArray[ap12].v01 == TriContainP) TriangleArray[ap12].v01 = AllTriangle + 1;

					if (TriangleArray[ap12].v12 == atri) TriangleArray[ap12].v12 = TriContainP;
					else if (TriangleArray[ap12].v12 == TriContainP) TriangleArray[ap12].v12 = AllTriangle + 1;

					if (TriangleArray[ap12].v20 == atri) TriangleArray[ap12].v20 = TriContainP;
					else if (TriangleArray[ap12].v20 == TriContainP) TriangleArray[ap12].v20 = AllTriangle + 1;

				}
			}
			else
			{
				if (ap12 != NoData)
				{

					if (TriangleArray[ap12].v01 == atri) TriangleArray[ap12].v01 = TriContainP;
					else if (TriangleArray[ap12].v12 == atri) TriangleArray[ap12].v12 = TriContainP;
					else if (TriangleArray[ap12].v20 == atri) TriangleArray[ap12].v20 = TriContainP;

				}
				if (ap41 != NoData)
				{

					if (TriangleArray[ap41].v01 == TriContainP) TriangleArray[ap41].v01 = AllTriangle + 1;
					else if (TriangleArray[ap41].v12 == TriContainP) TriangleArray[ap41].v12 = AllTriangle + 1;
					else if (TriangleArray[ap41].v20 == TriContainP) TriangleArray[ap41].v20 = AllTriangle + 1;

				}
			}

			if (ap23 == ap34)
			{
				if (ap23 != NoData)
				{

					if (TriangleArray[ap23].v01 == atri) TriangleArray[ap23].v01 = atri;
					else if (TriangleArray[ap23].v01 == TriContainP) TriangleArray[ap23].v01 = AllTriangle;

					if (TriangleArray[ap23].v12 == atri) TriangleArray[ap23].v12 = atri;
					else if (TriangleArray[ap23].v12 == TriContainP) TriangleArray[ap23].v12 = AllTriangle;

					if (TriangleArray[ap23].v20 == atri) TriangleArray[ap23].v20 = atri;
					else if (TriangleArray[ap23].v20 == TriContainP) TriangleArray[ap23].v20 = AllTriangle;
				}
			}
			else
			{
				if (ap23 != NoData)
				{
					if (TriangleArray[ap23].v01 == atri) TriangleArray[ap23].v01 = atri;
					else if (TriangleArray[ap23].v12 == atri) TriangleArray[ap23].v12 = atri;
					else if (TriangleArray[ap23].v20 == atri) TriangleArray[ap23].v20 = atri;

				}
				if (ap34 != NoData)
				{

					if (TriangleArray[ap34].v01 == TriContainP) TriangleArray[ap34].v01 = AllTriangle;
					else if (TriangleArray[ap34].v12 == TriContainP) TriangleArray[ap34].v12 = AllTriangle;
					else if (TriangleArray[ap34].v20 == TriContainP) TriangleArray[ap34].v20 = AllTriangle;

				}
			}

			//Here to build the Triangle
			//形成的新的四个三角形	

			TriangleArray[TriContainP].v0 = p1; TriangleArray[TriContainP].v01 = ap12;
			TriangleArray[TriContainP].v1 = p2; TriangleArray[TriContainP].v12 = atri;
			TriangleArray[TriContainP].v2 = Point_Num; TriangleArray[TriContainP].v20 = AllTriangle + 1;


			TriangleArray[atri].v0 = p2; TriangleArray[atri].v01 = ap23;
			TriangleArray[atri].v1 = p3; TriangleArray[atri].v12 = AllTriangle;
			TriangleArray[atri].v2 = Point_Num; TriangleArray[atri].v20 = TriContainP;


			TriangleArray[AllTriangle].v0 = p3; TriangleArray[AllTriangle].v01 = ap34;
			TriangleArray[AllTriangle].v1 = p4; TriangleArray[AllTriangle].v12 = AllTriangle + 1;
			TriangleArray[AllTriangle].v2 = Point_Num; TriangleArray[AllTriangle].v20 = atri;


			TriangleArray[AllTriangle + 1].v0 = p4; TriangleArray[AllTriangle + 1].v01 = ap41;
			TriangleArray[AllTriangle + 1].v1 = p1; TriangleArray[AllTriangle + 1].v12 = TriContainP;
			TriangleArray[AllTriangle + 1].v2 = Point_Num; TriangleArray[AllTriangle + 1].v20 = AllTriangle;

			//把新的三角形及其邻接三角形放入栈中
			PushNStack(TriContainP);
			PushNStack(atri);
			PushNStack(AllTriangle);
			PushNStack(AllTriangle + 1);
			PushStack(ap12);
			PushStack(ap23);
			PushStack(ap34);
			PushStack(ap41);

			AllTriangle = AllTriangle + 2;
		}
		else
		{
			if (ap41 != NoData)
			{
				if (TriangleArray[ap41].v01 == TriContainP) TriangleArray[ap41].v01 = AllTriangle;
				else if (TriangleArray[ap41].v12 == TriContainP) TriangleArray[ap41].v12 = AllTriangle;
				else if (TriangleArray[ap41].v20 == TriContainP) TriangleArray[ap41].v20 = AllTriangle;

			}
			//形成的新的两个三角形	
			TriangleArray[TriContainP].v0 = p3; TriangleArray[TriContainP].v01 = ap34;
			TriangleArray[TriContainP].v1 = p4; TriangleArray[TriContainP].v12 = AllTriangle;
			TriangleArray[TriContainP].v2 = Point_Num; TriangleArray[TriContainP].v20 = atri;

			TriangleArray[AllTriangle].v0 = p4; TriangleArray[AllTriangle].v01 = ap41;
			TriangleArray[AllTriangle].v1 = p1; TriangleArray[AllTriangle].v12 = atri;
			TriangleArray[AllTriangle].v2 = Point_Num; TriangleArray[AllTriangle].v20 = TriContainP;

			PushNStack(TriContainP);
			PushNStack(AllTriangle);
			PushStack(ap34);
			PushStack(ap41);

			AllTriangle = AllTriangle + 1;
		}
	}
	else if (EdgeNum == 2)
	{
		p1 = k3; p3 = k1; p4 = k2;
		ap34 = ak12; ap41 = ak23;

		atri = TriangleArray[TriContainP].v20;

		if (atri != -1)
		{
			v1 = TriangleArray[atri].v0;
			v2 = TriangleArray[atri].v1;
			v3 = TriangleArray[atri].v2;
			av12 = TriangleArray[atri].v01;
			av23 = TriangleArray[atri].v12;
			av31 = TriangleArray[atri].v20;

			if ((k3 == v1 && k1 == v2) || (k3 == v2 && k1 == v1))
			{
				p2 = v3;
				ap12 = av23;
				ap23 = av31;
			}
			else if ((k3 == v2 && k1 == v3) || (k3 == v3 && k1 == v2))
			{
				p2 = v1;
				ap12 = av31;
				ap23 = av12;
			}
			else if ((k3 == v3 && k1 == v1) || (k3 == v1 && k1 == v3))
			{
				p2 = v2;
				ap12 = av12;
				ap23 = av23;
			}

			//Here to Modify the neighboring Triangle
			if (ap12 == ap41)
			{
				if (ap12 != NoData)
				{

					if (TriangleArray[ap12].v01 == atri) TriangleArray[ap12].v01 = TriContainP;
					else if (TriangleArray[ap12].v01 == TriContainP) TriangleArray[ap12].v01 = AllTriangle + 1;

					if (TriangleArray[ap12].v12 == atri) TriangleArray[ap12].v12 = TriContainP;
					else if (TriangleArray[ap12].v12 == TriContainP) TriangleArray[ap12].v12 = AllTriangle + 1;

					if (TriangleArray[ap12].v20 == atri) TriangleArray[ap12].v20 = TriContainP;
					else if (TriangleArray[ap12].v20 == TriContainP) TriangleArray[ap12].v20 = AllTriangle + 1;
				}
			}
			else
			{
				if (ap12 != NoData)
				{

					if (TriangleArray[ap12].v01 == atri) TriangleArray[ap12].v01 = TriContainP;
					else if (TriangleArray[ap12].v12 == atri) TriangleArray[ap12].v12 = TriContainP;
					else if (TriangleArray[ap12].v20 == atri) TriangleArray[ap12].v20 = TriContainP;

				}
				if (ap41 != NoData)
				{

					if (TriangleArray[ap41].v01 == TriContainP) TriangleArray[ap41].v01 = AllTriangle + 1;
					else if (TriangleArray[ap41].v12 == TriContainP) TriangleArray[ap41].v12 = AllTriangle + 1;
					else if (TriangleArray[ap41].v20 == TriContainP) TriangleArray[ap41].v20 = AllTriangle + 1;
				}
			}

			if (ap23 == ap34)
			{
				if (ap23 != NoData)
				{

					if (TriangleArray[ap23].v01 == atri) TriangleArray[ap23].v01 = atri;
					else if (TriangleArray[ap23].v01 == TriContainP) TriangleArray[ap23].v01 = AllTriangle;

					if (TriangleArray[ap23].v12 == atri) TriangleArray[ap23].v12 = atri;
					else if (TriangleArray[ap23].v12 == TriContainP) TriangleArray[ap23].v12 = AllTriangle;

					if (TriangleArray[ap23].v20 == atri) TriangleArray[ap23].v20 = atri;
					else if (TriangleArray[ap23].v20 == TriContainP) TriangleArray[ap23].v20 = AllTriangle;
				}
			}
			else
			{
				if (ap23 != NoData)
				{

					if (TriangleArray[ap23].v01 == atri) TriangleArray[ap23].v01 = atri;
					else if (TriangleArray[ap23].v12 == atri) TriangleArray[ap23].v12 = atri;
					else if (TriangleArray[ap23].v20 == atri) TriangleArray[ap23].v20 = atri;

				}
				if (ap34 != NoData)
				{

					if (TriangleArray[ap34].v01 == TriContainP) TriangleArray[ap34].v01 = AllTriangle;
					else if (TriangleArray[ap34].v12 == TriContainP) TriangleArray[ap34].v12 = AllTriangle;
					else if (TriangleArray[ap34].v20 == TriContainP) TriangleArray[ap34].v20 = AllTriangle;

				}
			}
			//形成的新的四个三角形	

			TriangleArray[TriContainP].v0 = p1; TriangleArray[TriContainP].v01 = ap12;
			TriangleArray[TriContainP].v1 = p2; TriangleArray[TriContainP].v12 = atri;
			TriangleArray[TriContainP].v2 = Point_Num; TriangleArray[TriContainP].v20 = AllTriangle + 1;


			TriangleArray[atri].v0 = p2; TriangleArray[atri].v01 = ap23;
			TriangleArray[atri].v1 = p3; TriangleArray[atri].v12 = AllTriangle;
			TriangleArray[atri].v2 = Point_Num; TriangleArray[atri].v20 = TriContainP;


			TriangleArray[AllTriangle].v0 = p3; TriangleArray[AllTriangle].v01 = ap34;
			TriangleArray[AllTriangle].v1 = p4; TriangleArray[AllTriangle].v12 = AllTriangle + 1;
			TriangleArray[AllTriangle].v2 = Point_Num; TriangleArray[AllTriangle].v20 = atri;


			TriangleArray[AllTriangle + 1].v0 = p4; TriangleArray[AllTriangle + 1].v01 = ap41;
			TriangleArray[AllTriangle + 1].v1 = p1; TriangleArray[AllTriangle + 1].v12 = TriContainP;
			TriangleArray[AllTriangle + 1].v2 = Point_Num; TriangleArray[AllTriangle + 1].v20 = AllTriangle;

			//把新的三角形及其邻接三角形放入栈中
			PushNStack(TriContainP);
			PushNStack(atri);
			PushNStack(AllTriangle);
			PushNStack(AllTriangle + 1);
			PushStack(ap12);
			PushStack(ap23);
			PushStack(ap34);
			PushStack(ap41);

			AllTriangle = AllTriangle + 2;
		}

		else
		{
			if (ap41 != NoData)
			{
				if (TriangleArray[ap41].v01 == TriContainP) TriangleArray[ap41].v01 = AllTriangle;
				else if (TriangleArray[ap41].v12 == TriContainP) TriangleArray[ap41].v12 = AllTriangle;
				else if (TriangleArray[ap41].v20 == TriContainP) TriangleArray[ap41].v20 = AllTriangle;

			}
			//形成的新的两个三角形	
			TriangleArray[TriContainP].v0 = p3; TriangleArray[TriContainP].v01 = ap34;
			TriangleArray[TriContainP].v1 = p4; TriangleArray[TriContainP].v12 = AllTriangle;
			TriangleArray[TriContainP].v2 = Point_Num; TriangleArray[TriContainP].v20 = atri;

			TriangleArray[AllTriangle].v0 = p4; TriangleArray[AllTriangle].v01 = ap41;
			TriangleArray[AllTriangle].v1 = p1; TriangleArray[AllTriangle].v12 = atri;
			TriangleArray[AllTriangle].v2 = Point_Num; TriangleArray[AllTriangle].v20 = TriContainP;

			PushNStack(TriContainP);
			PushNStack(AllTriangle);
			PushStack(ap34);
			PushStack(ap41);

			AllTriangle = AllTriangle + 1;
		}
	}

}
void CGouTIN::InitializeTriangle1(int Point_Num, int TriContainP)
{
	int k0, k1, k2, ak0, ak1, ak2;

	k0 = TriangleArray[TriContainP].v0;
	k1 = TriangleArray[TriContainP].v1;
	k2 = TriangleArray[TriContainP].v2;
	ak0 = TriangleArray[TriContainP].v01;
	ak1 = TriangleArray[TriContainP].v12;
	ak2 = TriangleArray[TriContainP].v20;

	// v0,v1 和 PointNum 形成TriContainP三角形
	TriangleArray[TriContainP].v0 = k0;
	TriangleArray[TriContainP].v1 = k1;
	TriangleArray[TriContainP].v2 = Point_Num;
	TriangleArray[TriContainP].v01 = ak0;
	TriangleArray[TriContainP].v12 = AllTriangle;
	TriangleArray[TriContainP].v20 = AllTriangle + 1;

	// v1,v2 和 PointNum 形成AllTriangle三角形
	TriangleArray[AllTriangle].v0 = k1;
	TriangleArray[AllTriangle].v1 = k2;
	TriangleArray[AllTriangle].v2 = Point_Num;
	TriangleArray[AllTriangle].v01 = ak1;
	TriangleArray[AllTriangle].v12 = AllTriangle + 1;
	TriangleArray[AllTriangle].v20 = TriContainP;

	//修改相邻三角形的拓扑关系
	if (ak1 != NoData)
	{
		if (TriangleArray[ak1].v01 == TriContainP) TriangleArray[ak1].v01 = AllTriangle;
		else if (TriangleArray[ak1].v12 == TriContainP) TriangleArray[ak1].v12 = AllTriangle;
		else if (TriangleArray[ak1].v20 == TriContainP) TriangleArray[ak1].v20 = AllTriangle;
	}

	// v3,v1和 PointNum 形成AllTriangle+1三角形
	TriangleArray[AllTriangle + 1].v0 = k2;
	TriangleArray[AllTriangle + 1].v1 = k0;
	TriangleArray[AllTriangle + 1].v2 = Point_Num;
	TriangleArray[AllTriangle + 1].v01 = ak2;
	TriangleArray[AllTriangle + 1].v12 = TriContainP;
	TriangleArray[AllTriangle + 1].v20 = AllTriangle;

	//修改相邻三角形的拓扑关系
	if (ak2 != NoData)
	{
		if (TriangleArray[ak2].v01 == TriContainP) TriangleArray[ak2].v01 = AllTriangle + 1;
		else if (TriangleArray[ak2].v12 == TriContainP) TriangleArray[ak2].v12 = AllTriangle + 1;
		else if (TriangleArray[ak2].v20 == TriContainP) TriangleArray[ak2].v20 = AllTriangle + 1;

	}

	PushNStack(TriContainP);
	PushNStack(AllTriangle);
	PushNStack(AllTriangle + 1);
	PushStack(ak0);
	PushStack(ak1);
	PushStack(ak2);

	AllTriangle = AllTriangle + 2;

}
bool CGouTIN::Delaunary(int Point_Num, int TriInNew, int TriInStack)
{
	bool ValueReturn = false; //返回值

	int trisa1;
	int trisa2;
	trisa1 =0;
	trisa2 =0;
	int *vtrin = new int[3];
	int *atrin = new int[3];
	int *vtris = new int[3];
	int *atris = new int[3];

	vtrin[0] = TriangleArray[TriInNew].v0;
	vtrin[1] = TriangleArray[TriInNew].v1;
	vtrin[2] = TriangleArray[TriInNew].v2;
	atrin[0] = TriangleArray[TriInNew].v01;
	atrin[1] = TriangleArray[TriInNew].v12;
	atrin[2] = TriangleArray[TriInNew].v20;

	vtris[0] = TriangleArray[TriInStack].v0;
	vtris[1] = TriangleArray[TriInStack].v1;
	vtris[2] = TriangleArray[TriInStack].v2;
	atris[0] = TriangleArray[TriInStack].v01;
	atris[1] = TriangleArray[TriInStack].v12;
	atris[2] = TriangleArray[TriInStack].v20;

	int commonv1, commonv2, anthorv3;       //用于存放公共边和第三点
	commonv1 = commonv2 = anthorv3 = 0;
	int i, ii, iii, commonedgeinstack;      //寻找公共边在两个三角形中的编号
	int j, jj, commonedgeinnew;
	commonedgeinnew = commonedgeinstack = 0;
	for (i = 0; i < 3; i++)
	{
		ii = i + 1;
		if (ii > 2) ii = 0;
		iii = ii + 1;
		if (iii > 2) iii = 0;
		for (j = 0; j < 3; j++)
		{
			jj = j + 1;
			if (jj > 2) jj = 0;
			if ((vtris[i] == vtrin[j] && vtris[ii] == vtrin[jj]) ||
				(vtris[i] == vtrin[jj] && vtris[ii] == vtrin[j]))
			{
				commonedgeinstack = i;
				commonedgeinnew = j;
				commonv1 = vtris[i];
				commonv2 = vtris[ii];
				anthorv3 = vtris[iii];
				goto loop1;
			}
		}
	}

loop1:
	//插入点
	double xp = ScatterPointArray[Point_Num].X;
	double yp = ScatterPointArray[Point_Num].Y;


	//公共边上两点
	double x1 = ScatterPointArray[commonv1].X;
	double y1 = ScatterPointArray[commonv1].Y;
	double x2 = ScatterPointArray[commonv2].X;
	double y2 = ScatterPointArray[commonv2].Y;

	//第三点
	double x3 = ScatterPointArray[anthorv3].X;
	double y3 = ScatterPointArray[anthorv3].Y;

	double x13 = x1 - x3;
	double y13 = y1 - y3;

	double x23 = x2 - x3;
	double y23 = y2 - y3;

	double x1p = x1 - xp;
	double y1p = y1 - yp;

	double x2p = x2 - xp;
	double y2p = y2 - yp;

	double acos = x13 * x23 + y13 * y23;
	double bcos = x2p * x1p + y2p * y1p;
	double ccos = (x13 * y23 - x23 * y13) * bcos + (x2p * y1p - x1p * y2p) * acos;

	if ((acos < 0.0 && bcos < 0.0) || ccos < 0.0)
	{

		if (TriangleArray[TriInStack].v01 == TriInNew)
		{
			trisa1 = TriangleArray[TriInStack].v12;
			trisa2 = TriangleArray[TriInStack].v20;
		}
		else if (TriangleArray[TriInStack].v12 == TriInNew)
		{
			trisa1 = TriangleArray[TriInStack].v20;
			trisa2 = TriangleArray[TriInStack].v01;
		}
		else if (TriangleArray[TriInStack].v20 == TriInNew)
		{
			trisa1 = TriangleArray[TriInStack].v01;
			trisa2 = TriangleArray[TriInStack].v12;
		}

		// 修改邻接三角形
		int middleinnew, backinnew; // 用于记录公共边的下一节点和边的编号
		int middleinstack, backinstack;

		middleinnew = commonedgeinnew + 1;
		if (middleinnew > 2) middleinnew = 0;
		backinnew = middleinnew - 1;
		if (backinnew < 0) backinnew = 2;

		middleinstack = commonedgeinstack + 1;
		if (middleinstack > 2) middleinstack = 0;
		backinstack = middleinstack - 1;
		if (backinstack < 0) backinstack = 2;

		int stacktemp = atris[middleinstack];// 临时变量
		int newtemp = atrin[middleinnew];

		vtris[middleinstack] = Point_Num;
		atris[middleinstack] = TriInNew;
		atris[backinstack] = atrin[middleinnew];

		//新形成的三角形1
		TriangleArray[TriInStack].v0 = vtris[0];
		TriangleArray[TriInStack].v1 = vtris[1];
		TriangleArray[TriInStack].v2 = vtris[2];
		TriangleArray[TriInStack].v01 = atris[0];
		TriangleArray[TriInStack].v12 = atris[1];
		TriangleArray[TriInStack].v20 = atris[2];


		//修改新形成的三角形1的邻接三角形的拓扑关系
		if (newtemp != NoData)
		{

			if (TriangleArray[newtemp].v01 != NoData)
				if (TriangleArray[newtemp].v01 == TriInNew) TriangleArray[newtemp].v01 = TriInStack;

			if (TriangleArray[newtemp].v12 != NoData)
				if (TriangleArray[newtemp].v12 == TriInNew) TriangleArray[newtemp].v12 = TriInStack;

			if (TriangleArray[newtemp].v20 != NoData)
				if (TriangleArray[newtemp].v20 == TriInNew) TriangleArray[newtemp].v20 = TriInStack;

		}


		//新形成的三角形2
		vtrin[middleinnew] = anthorv3;
		atrin[middleinnew] = TriInStack;
		atrin[backinnew] = stacktemp;


		TriangleArray[TriInNew].v0 = vtrin[0];
		TriangleArray[TriInNew].v1 = vtrin[1];
		TriangleArray[TriInNew].v2 = vtrin[2];
		TriangleArray[TriInNew].v01 = atrin[0];
		TriangleArray[TriInNew].v12 = atrin[1];
		TriangleArray[TriInNew].v20 = atrin[2];

		//修改新形成的三角形2的邻接三角形的拓扑关系
		if (stacktemp != NoData)
		{
			if (TriangleArray[stacktemp].v01 != NoData)
				if (TriangleArray[stacktemp].v01 == TriInStack) TriangleArray[stacktemp].v01 = TriInNew;

			if (TriangleArray[stacktemp].v12 != NoData)
				if (TriangleArray[stacktemp].v12 == TriInStack) TriangleArray[stacktemp].v12 = TriInNew;

			if (TriangleArray[stacktemp].v20 != NoData)
				if (TriangleArray[stacktemp].v20 == TriInStack) TriangleArray[stacktemp].v20 = TriInNew;

		}
		PushNStack(TriInNew); PushStack(trisa1);
		PushNStack(TriInStack); PushStack(trisa2);

		ValueReturn = true;
	}

	return ValueReturn;
}
bool CGouTIN::PushNStack(int TriNum)
{
	if (ntop >= 10000)
	{
		//MessageBox(null,"  栈溢出 !! ",NULL,MB_OK);
		return false;
	}
	NStack[ntop] = TriNum;
	ntop = ntop + 1;
	return true;
}
bool CGouTIN::PushStack(int TriNum)
{
	if (top >= 10000)
	{
		//MessageBox(NULL,"  栈溢出 !! ",NULL,MB_OK);
		return false;
	}

	SStack[top] = TriNum;
	top = top + 1;
	return true;
}
int CGouTIN::PopStack()
{
	int Element;
	if (top < 0)
	{
		//MessageBox(NULL," 栈空 !! ",NULL,MB_OK);
		return 0;
	}

	top = top - 1;
	Element = SStack[top];
	return Element;
}
int CGouTIN::PopNStack()
{
	int Element;
	if (ntop < 0)
	{   // MessageBox(NULL," 栈空 !! ",NULL,MB_OK);
		return 0;
	}
	ntop = ntop - 1;
	Element = NStack[ntop];
	return Element;
}
void CGouTIN::SetPointNum(int num)
{
	PointNum=num;
}
int CGouTIN::GetPointNum()
{
	return PointNum;
}
int CGouTIN::Count_subsample(float minY,float maxY,float **Array)
 {
	 int sub_samplenum=0;
	 for (int i=0;i<PointNum;i++)
	 {
		 if ((Array[i][1]>=minY)&&(Array[i][1]<=maxY))
		 {
			 sub_samplenum++;
		 }
	 }
	 return sub_samplenum;
 }
void CGouTIN::Sub_array(int sub_sample,float MinY,float MaxY,float **Array,float **Sub_Array)
{
	int t=0;
	for (int i=0;i<PointNum;i++)
	{
		if((Array[i][1]>=MinY)&&(Array[i][1]<=MaxY))
		{
			Sub_Array[t][0]=Array[i][0];
			Sub_Array[t][1]=Array[i][1];
			Sub_Array[t++][2]=Array[i][2];
			//cout<<"Sub_Array[t][0]="<<Sub_Array[t][0]<<endl;
			//cout<<"Sub_Array[t][1]="<<Sub_Array[t][1]<<endl;
		}
		if (t>=sub_sample)
		{
			break;
		}
	}
}
void CGouTIN:: Print_time(double time1,double time2)
{
	ofstream o_file;
	o_file.open("log.txt");
	if(o_file.is_open())
	{
		o_file<<"并行计算消耗时间："<<time1<<endl;
		o_file<<"结果输出消耗时间："<<time2<<endl;
	}
	else
	{
		cout<< "打开文件："<<"log.txt"<< " 时出错！"; 
	}

	o_file.close();
}
void CGouTIN::createFile(char *Filename)
{
	ofstream o_file;
	o_file.open(Filename);
	if(o_file.is_open())
	{
		o_file<<"Fir_Node"<<"	"<<"Sec_Node"<<"	"<<"Thi_Node"<<endl;
	}
	o_file.close();
}
void CGouTIN::WriteFile(char *Filename,int Tnum,int **T_Array)
{
	ofstream o_file;
	o_file.open(Filename,ios::app);//以追加的形式写入数据
	if(o_file.is_open())
	{
		for (int i=0;i<Tnum;i++)
		{
			o_file<<T_Array[i][0]<<"	"<<T_Array[i][1]<<"	"<<T_Array[i][2]<<endl;
		}
	}
	o_file.close();
}
//确定4个角点的ID号
void CGouTIN::Find_fourCorner(float **subArray,int *id,int samplenum)
{

}
//计算上边界线和下边界线上顶点的数目
void CGouTIN::Pointnum_line(int *id,int *num)
{

}
//保存边界线上顶点的ID
void CGouTIN::Point_line(int *point_array,int *id,int all_num,int point_num,int tag)
{

}
//相邻上下边界线上顶点连成三角网
int CGouTIN::Combine_twoline(int *this_line,int *next_line,int **tri_twoline,int this_point,int next_point,int tri_num)
{

}
//本类的主函数，并行过程
int CGouTIN::TIN_Main(const char* pDataSource,const char *pLayerName,char *resultFile)
{
	MPI_Init(NULL, NULL);
	int tid, numprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &tid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Status status;

	char* spatialrefWkt;
	int sample_num=0;
	sample_num=read_samplenum(pDataSource,pLayerName,&spatialrefWkt);//获取总点数
	//cout<<sample_num<<endl;
	SetPointNum(sample_num);
	//cout<<PointNum<<endl;
	//为读入散点数据申请数组空间
	float **Sample_Array=(float **)malloc(PointNum*sizeof(float *));
	for (int k=0;k<PointNum;k++)
		Sample_Array[k]=(float *)malloc(3*sizeof(float));
	if (Sample_Array==NULL)
	{
		cout<<"Faliure memory request!"<<endl;
		return 0;
	}
	//读取散点数据
	double t1 = MPI_Wtime();

	//Read_Points(sampleFile,Sample_Array);
	read_vector(pDataSource,pLayerName,&spatialrefWkt,Sample_Array);//读取散点位置信息及属性值
	
	//将读取的散点排序
	//Sort(Sample_Array);
	//根据整体区域插值范围以及进程号确定当前进程的插值范围
	//思路即是根据进程数纵向剖分:最上面一条带0进程负责，依次往下分配
	float extentcurr_maxX=0;
	float extentcurr_minX=0;
	float extentcurr_maxY=0;
	float extentcurr_minY=0;
	//按照纵向剖分时，X方向范围不变，直接赋值即可
	extentcurr_maxX=MaxX;
	extentcurr_minX=MinX;
	//根据进程数确定Y方向上的范围

	extentcurr_maxY=MinY+(numprocs-tid)*(MaxY-MinY)/numprocs;//Y方向上最大值
	extentcurr_minY=MinY+(numprocs-tid-1)*(MaxY-MinY)/numprocs;//Y方向上最小值
	
	//确定子区域的采样点数量并存储
	int sub_samplenum=0;//子区域的采样点个数

	sub_samplenum=Count_subsample(extentcurr_minY,extentcurr_maxY,Sample_Array);//调用函数返回子区域中采样点的个数

	
	//申请存放子进程中的采样点空间
	float **Sub_Array=(float **)malloc(sub_samplenum*sizeof(float *));
	for (int k=0;k<sub_samplenum;k++)
		Sub_Array[k]=(float *)malloc(3*sizeof(float));
	if (Sub_Array==NULL)
	{
		cout<<"Faliure memory request!"<<endl;
		return 0;
	}
	//初始化
	for (int i=0;i<sub_samplenum;i++)
	{
		for (int j=0;j<3;j++)
		{
			Sub_Array[i][j]=0;
		}
	}
	Sub_array(sub_samplenum,extentcurr_minY,extentcurr_maxY,Sample_Array,Sub_Array);

	//将读取的散点排序
	Sort(Sub_Array,sub_samplenum);


	//子进程生成三角网
	BuildTIN(Sub_Array,sub_samplenum);
	/*边界处理部分*/
	//定义每个子区域中左上、右上、左下、右下位置点的ID
	int id_lrtb[4];
	//寻找四个角点的ID
	Find_fourCorner(Sub_Array,id_lrtb,sub_samplenum);

	int topbot_tri[2];//上边界线、下边界线上顶点的数目
	topbot_tri[0]=0;
	topbot_tri[1]=0;
	Pointnum_line(id_lrtb,topbot_tri);//计算边界上顶点的数目

	int *toppoint_array=(int *)malloc(topbot_tri[0]*sizeof(int));//数组，存放上边界线上顶点的ID
	int *bottompoint_array=(int *)malloc(topbot_tri[1]*sizeof(int));//数组，存放下边界线上顶点的ID

	Point_line(toppoint_array,id_lrtb,sub_samplenum,topbot_tri[0],1);//tag值取1，处理上边界线
	Point_line(toppoint_array,id_lrtb,sub_samplenum,topbot_tri[1],0);//tag值取0，处理下边界线


	//相邻进程之间传递：约定当前进程的上边界线向上上一个进程传递，与上一个进程的下边界线处理
	int nextline_point=0;//存储下一个进程上边界线的顶点数目
	int thisline_point=topbot_tri[0];
	if (tid>0)
	{
		//非0进程发送
		MPI_Send(&thisline_point,1,MPI_INT,tid-1,1,MPI_COMM_WORLD);
	}
	if (tid<numprocs-1)
	{
		//除去最后一个进程，都要接收
		MPI_Recv(&nextline_point,1,MPI_INT,tid+1,1,MPI_COMM_WORLD,&status);
	}
	//申请边界线顶点数组的存储空间
	int *nextpoint_array=(int *)malloc(nextline_point*sizeof(int));//
	if (tid>0)
	{
		//非0进程发送
		MPI_Send(toppoint_array,topbot_tri[0],MPI_INT,tid-1,1,MPI_COMM_WORLD);
	}
	if (tid<numprocs-1)
	{
		//除去最后一个进程，都要接收
		MPI_Recv(nextpoint_array,topbot_tri[0],MPI_INT,tid+1,1,MPI_COMM_WORLD,&status);
	}
	
	int max_twoline=2*(thisline_point+nextline_point);//边界部分新生成三角形的最大数量
	int trinum_twoline=0;//统计边界线上生成的三角形数目
	int **TIN_twoline=(int **)malloc(max_twoline*sizeof(int *));
	for (int k=0;k<max_twoline;k++)
		TIN_twoline[k]=(int *)malloc(3*sizeof(int));
	//相邻边界线上顶点生成三角网
	Combine_twoline(bottompoint_array,nextpoint_array,TIN_twoline,topbot_tri[1],nextline_point,trinum_twoline);
	/*边界处理部分*/
	//申请存放三角网信息的数组
	int **TIN_Array=(int **)malloc(AllTriangle*sizeof(int *));
	for (int k=0;k<AllTriangle;k++)
		TIN_Array[k]=(int *)malloc(3*sizeof(int));
	//将三角网信息转存到数组中
	Print_TIN(TIN_Array);

	double t2 = MPI_Wtime();



	// 创建输出数据集，格式为GeoTiff格式  
	if(tid==0)
	{
		createFile(resultFile);//创建要输出的文件
	}
	//依次写数据
	int write=0;
	if (tid==0)
	{
		WriteFile(resultFile,AllTriangle,TIN_Array);
		write=1;
		if (numprocs>1)
		{
			MPI_Send(&write,1,MPI_INT,tid+1,1,MPI_COMM_WORLD);
		}
	}
	else  
	{
		MPI_Recv(&write,1,MPI_INT,tid-1,1,MPI_COMM_WORLD,&status); 
		if(write==1)
		{
			WriteFile(resultFile,AllTriangle,TIN_Array);
			if(tid!=numprocs-1)
			{
				MPI_Send(&write,1,MPI_INT,tid+1,1,MPI_COMM_WORLD);
			}
		}
	} 
	double t3 = MPI_Wtime();
	//Print_time(t2-t1,t3-t2);
	if (tid==0)
	{
		cout<<"[DEBUG][TIMESPAN][IO]"<<t3-t2<<endl;
		cout<<"[DEBUG][TIMESPAN][COMPUTING]"<<t2-t1<<endl;
		cout<<"[DEBUG][TIMESPAN][TOTAL]"<<t3-t1<<endl;
	}
	free(Sample_Array);
	free(Sub_Array);
	MPI_Finalize();
}
