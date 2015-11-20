#pragma once
#include <cstdlib>
//#include "ANN/ANN.h"
#include "ogrsf_frmts.h"
#include "gdal_priv.h"
#include "mpi.h"
#include "IDW.h"
#include <cmath>
#include <stdlib.h>
#include <exception>
#include <gdal_alg.h>
#include "cpl_string.h"
#include<gdal.h>
#include <ogr_spatialref.h>
#include <gdal_priv.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#pragma comment(lib,"gdal_i.lib")
#define EPS 0.00000001
#define  NODATA_DEFINE -3.40282346639e+038
using namespace std;
//����������ʸ�������Ŀ
int read_samplenum(const char* filename,const char* pLayerName,char** pSpatialRefWkt)
{
	//����CIDW������
	OGRRegisterAll();
	OGRDataSource *poDS=OGRSFDriverRegistrar::Open(filename,FALSE);
	//OGRDataSource *poDS=OGRSFDriverRegistrar::Open("point.shp",FALSE);
	if( poDS == NULL )
	{
		printf( "[ERROR] zmw Open failed.\n" );
		exit( 1 );
	}

	OGRLayer *poLayer = poDS->GetLayerByName(pLayerName);

	//OGRSpatialReference * sref= poLayer->GetSpatialRef();
	//sref->exportToWkt(pSpatialRefWkt);
	OGRFeature *poFeature;

	poLayer->ResetReading();
	int count = 0;//ͳ��ɢ����
	while((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		count++;
	}
	OGRDataSource::DestroyDataSource( poDS );
	return count;
}

int read_vector(const char* filename,const char* pLayerName, int fieldIdx, char** pSpatialRefWkt,CIDW &idw,double **Sample_Array)
{
	//��λ����Ϣ��������Ϣ���������Sample_Array��
	OGRRegisterAll();
	OGRDataSource *poDS=OGRSFDriverRegistrar::Open(filename,FALSE);
	if( poDS == NULL )
	{
		printf( "[ERROR] Open failed.\n" );
		exit( 1 );
	}	
	OGRLayer *poLayer = poDS->GetLayerByName(pLayerName);

	//OGRSpatialReference * sref= poLayer->GetSpatialRef();
	//sref->exportToWkt(pSpatialRefWkt);

	OGRFeature *poFeature;

	poLayer->ResetReading();
	/*int count = 0;//ͳ��ɢ����
	while((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		count++;
	}
	poLayer->ResetReading();*/

	//����Ҫ����Ĵ洢�ռ䣬��ע�͵�
	//*ptArray = annAllocPts(count, 2);
	//*ptValues = new double[count];

	int idx = 0;
	double x=0.0;
	double y=0.0;
	while((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		//(*ptValues)[idx] = poFeature->GetFieldAsDouble(fieldIdx);
		Sample_Array[idx][2]=poFeature->GetFieldAsDouble(fieldIdx);//��ȡ����ֵ
		//cout<<"value:"<<Sample_Array[idx][2]<<endl;
		OGRGeometry *poGeometry;

		poGeometry = poFeature->GetGeometryRef();
		if( poGeometry != NULL&& wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
		{
			OGRPoint *poPoint=(OGRPoint *)poGeometry;
			x=poPoint->getX();
			y=poPoint->getY();
			//�洢λ����Ϣ
			Sample_Array[idx][0]=x;
			Sample_Array[idx][1]=y;
			//(*ptArray)[idx][0] = x;
			//(*ptArray)[idx][1] = y;
			if(idx==0)
			{
				//extent.minX = extent.maxX = x;
				//extent.minY = extent.maxY = y;
				idw.extent_All.minX=x;
				idw.extent_All.maxX=x;
				idw.extent_All.minY=y;
				idw.extent_All.maxY=y;
			}
			else
			{
				if(x>idw.extent_All.maxX)
					idw.extent_All.maxX = x;
				if(x<idw.extent_All.minX)
					idw.extent_All.minX = x;
				if(y>idw.extent_All.maxY)
					idw.extent_All.maxY = y;
				if(y<idw.extent_All.minY)
					idw.extent_All.minY = y;
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
int create_raster(const char* filename, double left, double top, int nXSize, int nYSize, double pixelSize)
{
	const char *pszFormat = "GTiff";

	GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	if( poDriver == NULL )
	{
		printf("[ERROR] Can't find the driver for writing GeoTiff.\n");
		return 1;
	}

	GDALDataset *poDstDS;
	char **papszOptions = NULL;
	poDstDS = poDriver->Create( filename, nXSize, nYSize, 1, GDT_Float32,
		papszOptions );

	if(poDstDS == NULL)
	{
		printf("[ERROR] Can't create the raster file as output.\n");
		return 1;
	}

	double adfGeoTransform[6] = {left, pixelSize, 0, top, 0, -pixelSize};
	poDstDS->SetGeoTransform(adfGeoTransform);

	GDALClose((GDALDatasetH)poDstDS);
	return 0;
}

int open_raster(const char* filename, GDALDataset ** pDS, GDALRasterBand** pBand)
{
	*pDS = (GDALDataset*)GDALOpen(filename, GA_Update);
	if(*pDS == NULL)
	{
		printf("[ERROR] Can't open the output file.\n");
		return 1;
	}
	*pBand = (*pDS)->GetRasterBand(1);
	return 0;
}

int close_raster(GDALDataset *pDS)
{
	GDALClose((GDALDatasetH)pDS);
	return 0;
}
void Change_data(float *Vecotor,float *ReVecotor,int Nrow,int Ncol,int buffersize)
{
	float **SArray=(float **)malloc(Nrow*sizeof(float *));//Ϊ��������������洢�ռ�
	for (int k=0;k<Nrow;k++)
		SArray[k]=(float *)malloc(Ncol*sizeof(float));
	int allnum=0;
	char buffer[250];
	for (int i=Nrow-1;i>=0;i--)
	{
		for (int j=0;j<Ncol;j++)
		{
			//��������ѭ���Ǽ��������ķ�Χ������Ҫ������ǰ����������С��Χ������������жϣ�����С��Χ�Ĳ����

			SArray[i][j]=Vecotor[allnum];
			allnum++;//�����Ƿ��������Ҫ������
		}
	}

	allnum=0;
	for (int i=buffersize;i<Nrow-buffersize;i++)//����ǰ���buffersize�кͺ����buffersize��
	{
		for (int j=0;j<Ncol;j++)
		{
			//��������ѭ���Ǽ��������ķ�Χ������Ҫ������ǰ����������С��Χ������������жϣ�����С��Χ�Ĳ����

			ReVecotor[allnum]=SArray[i][j];
			allnum++;//�����Ƿ��������Ҫ������
		}
	}
	free(SArray);
}
void Print_time(double time1,double time2)
{
	ofstream o_file;
	o_file.open("log.txt");
	if(o_file.is_open())
	{
		o_file<<"���м�������ʱ�䣺"<<time1<<endl;
		o_file<<"����������ʱ�䣺"<<time2<<endl;
	}
	else
	{
		cout<< "���ļ���"<<"log.txt"<< " ʱ����"; 
	}

	o_file.close();
}

int main(int argc, char** argv) //����Ĳ��������ǣ������ļ���������ļ�������ֵ�ֱ��ʣ�������������������Ȩ���ݣ��뾶1���뾶2
{

	MPI_Init(&argc, &argv);
	int tid, numprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &tid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Status status;
	/*********����Ҫ�ⲿ����ı���*********/
	 float cellsize=0.0;
	 //int sample_num=0;
	 int fldIndex =0;//��ʾ��ʸ�����ݵĵڼ�����������ֵ����
	 int idw_power=0;
	 int idw_rad1=0;
	 int idw_rad2=0;

	 cellsize=std::atof(argv[3]);
	 fldIndex=std::atoi(argv[4]);// 
	 //idw_power=std::atoi(argv[5]); //
	 idw_rad1=std::atoi(argv[5]); //
	 idw_rad2=std::atof(argv[6]); //

        string pLayerName = argv[1];
	 string dbname=argv[7];string hostname=argv[8];string username=argv[9];string pwd=argv[10];string port=argv[11];
        string m_DatasourceConStr  = "PG:host='"+hostname+"' port='"+port+"' dbname='"+dbname+"' user='"+username+"' password='"+pwd+"'";
        cout<<m_DatasourceConStr<<"   LayerName:"<<pLayerName<<endl;


	 const char * dstFile(argv[2]);
	/*********����Ҫ�ⲿ����ı���*********/
	 char* spatialrefWkt;
	 int sample_num=0;
	 sample_num=read_samplenum(m_DatasourceConStr.c_str(),pLayerName.c_str(),&spatialrefWkt);//��ȡɢ����Ŀ



	CIDW idw(cellsize,sample_num);//��һ�������Ǹ����ߴ磬�ڶ��������ǲ��������
	//���������ȡ
	double t1 = MPI_Wtime();
	double **Sample_Array=(double **)malloc(sample_num*sizeof(double *));//Ϊ��������������洢�ռ�
	for (int k=0;k<sample_num;k++)
		Sample_Array[k]=(double *)malloc(3*sizeof(double));
	if (Sample_Array==NULL)
	{
		cout<<"Faliure memory request!"<<endl;
		return 0;
	}
	//idw.Read_data_sample(sampleFile,Sample_Array);
	read_vector(m_DatasourceConStr.c_str(),pLayerName.c_str(),fldIndex,&spatialrefWkt,idw,Sample_Array);//��ȡɢ��λ����Ϣ������ֵ
	//�������������ֵ��Χ�Լ����̺�ȷ����ǰ���̵Ĳ�ֵ��Χ
	//˼·���Ǹ��ݽ����������ʷ�:������һ����0���̸����������·���
	double extentcurr_maxX=0;
	double extentcurr_minX=0;
	double extentcurr_maxY=0;
	double extentcurr_minY=0;
	//���������ʷ�ʱ��X����Χ���䣬ֱ�Ӹ�ֵ����
	extentcurr_maxX=idw.extent_All.maxX;
	extentcurr_minX=idw.extent_All.minX;
	//���ݽ�����ȷ��Y�����ϵķ�Χ
	extentcurr_maxY=idw.extent_All.minY+(numprocs-tid)*(idw.extent_All.maxY-idw.extent_All.minY)/numprocs;//Y���������ֵ
	extentcurr_minY=idw.extent_All.minY+(numprocs-tid-1)*(idw.extent_All.maxY-idw.extent_All.minY)/numprocs;//Y��������Сֵ
	//��ӻ�����:Ϊ����ͳһ������βͳһ�ӻ�����
	int buffersize=(int)(idw_rad1+idw_rad2)/2;
	double Bextentcurr_minY=extentcurr_minY-buffersize*cellsize;
	double Bextentcurr_maxY=extentcurr_maxY+buffersize*cellsize;
	//ȷ��������Ĳ������������洢
	int sub_samplenum=0;//������Ĳ��������

	sub_samplenum=idw.Count_subsample(Bextentcurr_minY,Bextentcurr_maxY,Sample_Array);//���ú��������������в�����ĸ���

	double * Array_X=(double *)malloc(sub_samplenum*sizeof(double));//��Ų�����X����
	if (Array_X==NULL)
	{
		cout<<"Faliure memory request!"<<endl;
		return 0;
	}
	double * Array_Y=(double *)malloc(sub_samplenum*sizeof(double));//��Ų�����Y����
	if (Array_Y==NULL)
	{
		cout<<"Faliure memory request!"<<endl;
		return 0;
	}
	double * Array_Z=(double *)malloc(sub_samplenum*sizeof(double));//��Ų�����Z����
	if (Array_Z==NULL)
	{
		cout<<"Faliure memory request!"<<endl;
		return 0;
	}
	for (int i=0;i<sub_samplenum;i++)
	{
		Array_X[i]=0;
		Array_Y[i]=0;
		Array_Z[i]=0;
	}
	
	idw.Sub_array(sub_samplenum,Bextentcurr_minY,Bextentcurr_maxY,Sample_Array,Array_X,Array_Y,Array_Z);
	

	int AllnXSize = (int)((idw.extent_All.maxX-idw.extent_All.minX) /cellsize); //�����������ĳߴ� 
	int AllnYSize = (int)((idw.extent_All.maxY-idw.extent_All.minY) /cellsize); //�����������ĳߴ�
	int nXSize = (int)((extentcurr_maxX-extentcurr_minX) /cellsize);  
	int nYSize = (int)((extentcurr_maxY-extentcurr_minY) /cellsize);  

	int BnYSize = (int)((Bextentcurr_maxY-Bextentcurr_minY) /cellsize);

	
	MPI_Reduce(&nYSize,&AllnYSize,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	
	MPI_Bcast(&AllnYSize,1,MPI_INT,0,MPI_COMM_WORLD);
	
	GDALAllRegister();  
	// ��ɢ���ڲ巽����ʹ�÷�����Ȩ�ز�ֵ��  
	//GDALGridInverseDistanceToAPowerOptions *poOptions = new GDALGridInverseDistanceToAPowerOptions();  
	//poOptions->dfPower=idw_power;  
	//poOptions->dfRadius1=idw_rad1;  
	//poOptions->dfRadius2=idw_rad2;  
	GDALGridMovingAverageOptions *poOptions=new GDALGridMovingAverageOptions();
	poOptions->dfRadius1=idw_rad1;  
	poOptions->dfRadius2=idw_rad2;

	float *pData  =  new float[nXSize*BnYSize];  
	float *qData=new float[nXSize*nYSize];

	// ��ɢ���ڲ巽����ʹ�÷�����Ȩ�ز�ֵ����ʹ�������Ĳ�ֵ�㷨�����ﻻ�������ģ����������GDALGridCreate�����Ķ�Ӧ����  
	GDALGridCreate(GGA_MovingAverage, poOptions, sub_samplenum, Array_X, Array_Y, Array_Z,   
		extentcurr_minX, extentcurr_maxX, Bextentcurr_minY, Bextentcurr_maxY, nXSize, BnYSize, GDT_Float32, pData, NULL, NULL);  

	Change_data(pData,qData,BnYSize,nXSize,buffersize);//����һ�����ݵ�˳�򣬲�Ȼ����������

	double t2 = MPI_Wtime();
	// ����������ݼ�����ʽΪGeoTiff��ʽ  
	if(tid==0)
	{
		create_raster(dstFile,idw.extent_All.minX,idw.extent_All.maxY, AllnXSize, AllnYSize, cellsize);
	}
	// д��Ӱ��  ���ⲿ�������д���
	GDALDataset * pDS;
	GDALRasterBand* poBand;
	int y_off=0;
	if(tid==0)
		open_raster(dstFile, &pDS, &poBand);
	if (tid==0)
	{
		//0���̣�������������̵ģ�Ȼ�����������̵Ĳ����
		poBand->RasterIO(GF_Write,0,0,nXSize,nYSize,qData,nXSize,nYSize,GDT_Float32,0,0);
		//int other_nXsize=0;//�������������ݿ��С
		int other_nYsize=0;//�������������ݿ��С
		for(int k=1;k<numprocs;k++)
		{
			//MPI_Recv(&other_nXsize, 1, MPI_INT, k, 77, MPI_COMM_WORLD, &status);
			MPI_Recv(&other_nYsize, 1, MPI_INT, k, 88, MPI_COMM_WORLD, &status);
			y_off=y_off+other_nYsize;
			float *buffer = new float[nXSize*other_nYsize];//
			MPI_Recv(buffer, nXSize*other_nYsize, MPI_FLOAT, k, 99, MPI_COMM_WORLD, &status);
			poBand->RasterIO(GF_Write,0,y_off,nXSize, other_nYsize,(void*)buffer, nXSize, other_nYsize, GDT_Float32,0,0);
			delete [] buffer;
		}
	} 
	else
	{
		//MPI_Send(&nXSize,1, MPI_FLOAT, 0, 77, MPI_COMM_WORLD);
		MPI_Send(&nYSize,1, MPI_FLOAT, 0, 88, MPI_COMM_WORLD);
		MPI_Send(qData,nXSize*nYSize, MPI_FLOAT, 0, 99, MPI_COMM_WORLD);
	}
	if (tid==0)
	{
		close_raster(pDS);
	}
	double t3 = MPI_Wtime();
	//Print_time(t2-t1,t3-t2);
	if (tid==0)
	{
		cout<<"[DEBUG][TIMESPAN][IO]"<<t3-t2<<endl;
		cout<<"[DEBUG][TIMESPAN][COMPUTING]"<<t2-t1<<endl;
		cout<<"[DEBUG][TIMESPAN][TOTAL]"<<t3-t1<<endl;
	}
	delete poOptions;
	delete []pData;
	delete []qData;
	free(Array_X);
	free(Array_Y);
	free(Array_Z);
	free(Sample_Array);
	MPI_Finalize();
}