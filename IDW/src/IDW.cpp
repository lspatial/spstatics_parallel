#include "IDW.h"


CIDW::CIDW(void)
{
}


CIDW::~CIDW(void)
{
}
CIDW::CIDW(float cellsize,int sample)
{
	h=cellsize;
	Nsample=sample;
}
void CIDW::Read_data_sample(char *filename2,double **Array)
{
	//读取采样点信息的过程，同时确定整个插值区域的范围
	//string filename=filename2;
	ifstream i_file;
	int count=0;
	double *result=new double[Nsample*3];//数组大小应大于采样点数目

	i_file.open(filename2);
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


			result[count]=out_test;
			count++;

		}
	}
	else
		cout << "打开文件：" << filename2 << " 时出错！"; 
	i_file.close();
	int ii=0;
	//转换成矩阵，同时确定插值范围
	//赋初值
	extent_All.maxX=-999999999;
	extent_All.maxY=-999999999;
	extent_All.minX=999999999;
	extent_All.minY=999999999;
	for(int i=0;i<Nsample;i++)
	{
		Array[i][0]=result[ii];
		Array[i][1]=result[ii+1];
		if (Array[i][0]<extent_All.minX)
		{
			extent_All.minX=Array[i][0];
		}
		if (Array[i][0]>extent_All.maxX)
		{
			extent_All.maxX=Array[i][0];
		}
		if (Array[i][1]<extent_All.minY)
		{
			extent_All.minY=Array[i][1];
		}
		if (Array[i][1]>extent_All.maxY)
		{
			extent_All.maxY=Array[i][1];
		}
		Array[i][2]=result[ii+2];
		ii=ii+3;
	}
	//cout<<Array[Nsample-1][0]<<"   "<<Array[Nsample-1][1]<<"    "<<Array[Nsample-1][2]<<endl;
	delete []result;

}
int CIDW::Count_subsample(double minY,double maxY,double **Array)
{
	int sub_samplenum=0;
	for (int i=0;i<Nsample;i++)
	{
		if ((Array[i][1]>=minY)&&(Array[i][1]<=maxY))
		{
			sub_samplenum++;
		}
	}
	return sub_samplenum;
}
void CIDW::Sub_array(int sub_sample,float MinY,float MaxY,double **Array,double *Array_X,double *Array_Y,double *Array_Z)
{
	int t=0;
	for (int i=0;i<Nsample;i++)
	{
		if((Array[i][1]>=MinY)&&(Array[i][1]<=MaxY))
		{
			Array_X[t]=Array[i][0];
			Array_Y[t]=Array[i][1];
			Array_Z[t++]=Array[i][2];
		}
		if (t>=sub_sample)
		{
			break;
		}
	}
	//cout<<"0000:"<<t<<"   "<<sub_sample<<endl;
}