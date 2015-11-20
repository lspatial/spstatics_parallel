#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <mpi.h>
#include "Matrix.h"
#include <ogrsf_frmts.h>
#include "StringLib.h"

using namespace std;

void LoadSamplesFrom(string pdataSource,string pLayerName, unsigned long &nSample, Vector &samples, int nProcSize, int nProcRank)
{
	OGRRegisterAll();
	OGRDataSource* poDS = OGRSFDriverRegistrar::Open(pdataSource.c_str(),FALSE);
	OGRLayer* poLayer = poDS->GetLayerByName(pLayerName.c_str());
	OGRFeature * pFeature =poLayer->GetNextFeature();

	nSample = poLayer->GetFeatureCount();

	unsigned long is, ie; //当前进程计算的起止总体离散点位置
	unsigned long nParts;
	nParts = nSample / nProcSize;
	is = nProcRank * nParts;
	if(nProcRank != nProcSize-1)
		ie = is + nParts;
	else
		ie = nSample;

	samples.Resize(ie-is);

	//set the cursor for each process
	for(int t=0; t<is; t++)
	{
		pFeature = poLayer->GetNextFeature();
	}

	int ns = 1;
	for(int iRow=is; iRow<ie;iRow++)
	{
		//样本取值字段名为value，double型
		samples[ns++] = pFeature->GetFieldAsDouble("WAADT");
		pFeature = poLayer->GetNextFeature();
	}

	OGRDataSource::DestroyDataSource( poDS );
}

void SRS(Vector &samples, unsigned long nAllSample, double &dblMean, double &dblVar)
{
	unsigned long nSample;
	nSample = samples.Length();

	double dblSum, dblSum2;
	double dblSumGlb, dblSum2Glb;
	dblSum = dblSum2 = 0;

	//均值
	for(unsigned long i=1; i<=nSample; i++)
	{
		dblSum += samples[i];
	}
	dblSum /= nAllSample;
	MPI_Allreduce(&dblSum, &dblMean, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	for(unsigned long i=1; i<nSample; i++)
	{
		dblSum2 += (samples[i+1] - dblMean)*(samples[i+1] - dblMean);
	}
	dblSum2 = dblSum2/nAllSample;
	MPI_Reduce(&dblSum2, &dblSum2Glb, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	//MPI_Allreduce(&dblSum2, &dblSum2Glb, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	dblVar = dblSum2Glb/(nAllSample-1);
}

int main(int argc, char *argv[])
{
	/*if(argc != 2)
	{
		printf("参数个数不正确！\n\tSRS 样本文件路径\n");
		return(0);
	}*/
	
        string pLayerName = argv[1];
	string dbname=argv[2];string hostname=argv[3];string username=argv[4];string pwd=argv[5];string port=argv[6];
        string m_DatasourceConStr  = "PG:host='"+hostname+"' port='"+port+"' dbname='"+dbname+"' user='"+username+"' password='"+pwd+"'";
        
//        cout<<m_DatasourceConStr<<endl;


	//MPI环境初始化
	int nProcSize;
	int nProcRank;
	int nProcNameLen;
	char sProcName[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);								//根据传入参数初始化
	MPI_Comm_size(MPI_COMM_WORLD, &nProcSize);          //获取所有参加运算的进程的个数
	MPI_Comm_rank(MPI_COMM_WORLD, &nProcRank);			//获取当前正在运行的进程的标识号
	MPI_Get_processor_name(sProcName, &nProcNameLen);   //获取本进程运行的机器的名称

	double progStarttime;
	progStarttime = MPI_Wtime();

	Vector samples;
	unsigned long nSample;
	LoadSamplesFrom(m_DatasourceConStr,pLayerName, nSample, samples, nProcSize, nProcRank);

	double wtime = 0;
	double start, stop;
	MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();	//计时开始

	double dblMean, dblVar;
	SRS(samples, nSample, dblMean, dblVar);
        MPI_Barrier(MPI_COMM_WORLD);
	stop = MPI_Wtime();		//计时结束
	wtime = stop - start;

	if(nProcRank == 0)
	{
		printf("随机抽样模型统计推断\n");
		printf("总体均值估计: %.2f; \n均值估计方差: %.2f\n", dblMean, dblVar);
		//printf("\n计算耗时: %f秒\n", wtime);		
		
		double timeAll, timeCalc, timeIO;
		timeAll = stop - progStarttime;
		timeCalc = wtime;
		timeIO = start - progStarttime;
		//cout << "计算耗时: " << endtime-calcStarttime << " 秒" << endl;
		cout << "[DEBUG][TIMESPAN][IO] " << timeIO << endl;
		cout << "[DEBUG][TIMESPAN][COMPUTING] " << timeCalc << endl;
		cout << "[DEBUG][TIMESPAN][TOTAL] " << timeAll << endl;
	}

	//结束MPI工作
	MPI_Finalize();
	return 0;
}
