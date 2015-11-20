#include <vector>

#ifndef   _CALCULATOR_FILE_H 
#define   _CALCULATOR_FILE_H 

using namespace std;
template<typename T>
class Calculator
{

public:
	Calculator()
		:blocks(),metas()
	{
		sigmaW = 0.f;
		sigmaSd = 0.f;
		other = 0.f;
		average = 0.f;
		completeCount = 0;
		
	}
	void AddDataBlock(T * const data, float* const meta);
	void CalcAverage();	
	void CalcSelfComponents(int dataIndex);
	
	void CalcCrossComponents(T * const data, float* const meta);
	void GetResults(double& sigmaW, double& sigmaSd, double& other);
	void CalcVariance();
	double GetAverage()
	{
		return this->average;
	}
	void SetAverage(double avg)
	{
		this->average = avg;
	}
	
	vector<T*> blocks;
	vector<float*> metas;
private:
	
	double sigmaW;
	double sigmaSd ;
	double other;
	double average;
	size_t completeCount;
	void CalcSelfComponents(T * const data, float* const meta, double &w, double &o);
	void CalcCrossComponents(T* const data1, float* const meta1, T* const data2, float* const meta2, double& sigmaW, double& other);
	void CalcVariance(T* const data, float* const meta, double &v);
};

template<typename T>
void Calculator<T>::AddDataBlock(T * const data, float * const meta)
{	
	this->blocks.push_back(data);
	this->metas.push_back(meta);
}

template<typename T>
void Calculator<T>::CalcCrossComponents(T* const data1, float* const meta1, T* const data2, float* const meta2, double& sigmaW,double& other)
{	
	int indexI = 0;
	int indexJ = 0;
	T xi,xj;
	double deltaX, deltaY, w, o;

	//if(meta1[0]<meta2[0] && meta1[1]<meta2[1])
	{
		for(int x1=0;x1<meta1[4];x1++)
		{
			for(int y1=0;y1<meta1[5];y1++)
			{
				xi = data1[indexI]; 
				indexJ = 0;
				for(int x2=0;x2<meta2[4];x2++)
				{
					for(int y2=0;y2<meta2[5];y2++)
					{
						xj=data2[indexJ];
						deltaX = meta1[0] + x1* meta1[2] - (meta2[0] + x2*meta2[2]);
						deltaY = meta1[1] + y1* meta1[3] - (meta2[1] + y2*meta2[3]);
						w = 1/sqrt(deltaX * deltaX + deltaY * deltaY);
						o = w * (xi-this->average) * (xj-this->average);
						sigmaW += w;
						other += o;					
						indexJ++;
					}
				}			
				indexI++;
			}
		}
	}
}

template<typename T>
void Calculator<T>::CalcSelfComponents(T * const data, float* const meta, double &w, double &o)
{
	int indexI = 0;
	int indexJ = 0;
	T xi,xj;
	double deltaX, deltaY;
	double lw, lo;

	for(int y1=0;y1<meta[5];y1++)
	{
		for(int x1=0;x1<meta[4];x1++)
		{
			xi = data[indexI]; 
			indexJ = 0;
			for(int y2=0;y2<meta[5];y2++)
			{	
				for(int x2=0;x2<meta[4];x2++)
				{									
					if(indexI != indexJ)
					{
						xj=data[indexJ];
						deltaX = meta[0] + x1* meta[2] - (meta[0] + x2*meta[2]);
						deltaY = meta[1] + y1* meta[3] - (meta[1] + y2*meta[3]);
						lw = 1/sqrt(deltaX * deltaX + deltaY * deltaY);
						lo = lw * (xi-this->average) * (xj-this->average);		
						w+=lw;
						o+=lo;
					}
					indexJ++;
				}
			}			
			indexI++;
		}
	}	
}

template<typename T>
void Calculator<T>::CalcVariance(T* const data, float* meta, double &v)
{
	int index=0;
	double d;
	for(int x=0;x<meta[4];x++)
	{
		for(int y=0;y<meta[5];y++)
		{
			d = data[index]-this->average; 
			v+=(d*d);
		}
	}
}

template<typename T>
void Calculator<T>::CalcVariance()
{
	double v = 0;
	for(size_t i=0;i<this->metas.size();i++)
	{
		float* meta = this->metas[i];
		T* data = this->blocks[i];
		CalcVariance(data, meta, v);
	}
	this->sigmaSd = v;
}

template<typename T>
void Calculator<T>::CalcAverage()
{	
	int index;
	for(size_t i=this->completeCount;i<this->metas.size();i++)
	{
		float* meta = this->metas[i];
		T* data = this->blocks[i];
		index = 0;
		for(int x=0;x<meta[4];x++)
		{
			for(int y=0;y<meta[5];y++)
			{						
				this->average += data[index];
				index++;
			}			
		}
		this->completeCount++;
	}	
	
}

template<typename T>
void Calculator<T>::CalcCrossComponents(T * const data, float* const meta)
{
	double w = 0.0f;	
	double o = 0.0f;
	for(size_t i=0;i<this->metas.size();i++)
	{
		float* meta1 = this->metas[i];
		T* data1 = this->blocks[i];
		this->CalcCrossComponents(data1, meta1, data, meta, w, o);
	}
	this->sigmaW += w;	
	this->other += o;
}

template<typename T>
void Calculator<T>::CalcSelfComponents(int dataIndex)
{
	double w = 0.0f;
	double sd = 0.0f;
	double o = 0.0f;
	int i = dataIndex;
	float* meta1 = this->metas[i];
	T* data1 = this->blocks[i];	

	long start,finish;
	double totaltime;
	start=clock();
	size_t size = metas.size();
	for(size_t j=0;j<this->metas.size();j++)
	{
		if(j != dataIndex)
			this->CalcCrossComponents(data1,meta1, this->blocks[j], this->metas[j], w, o);			
	}

	this->CalcSelfComponents(data1, meta1, w, o);

	this->sigmaW+=w;

	this->other+=o;
	finish=clock();
	totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
	printf("�˳��������ʱ��Ϊ%f�룡", totaltime);
}

template<typename T>
void Calculator<T>::GetResults(double& sigmaW, double& sigmaSd, double& other)
{
	sigmaW = this->sigmaW;
	sigmaSd = this->sigmaSd;
	other = this->other;
}

#endif