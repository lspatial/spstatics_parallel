// Dicrete.h: interface for the Dicrete class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DICRETE_H__156F6854_EACB_4FC8_9C89_6D6F1EE68C1F__INCLUDED_)
#define AFX_DICRETE_H__156F6854_EACB_4FC8_9C89_6D6F1EE68C1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<iostream>
#include<vector>
#include<algorithm>
#include<assert.h>
#include<math.h>
#include<list>
#include "mpi.h"
#include "gdal_priv.h"

#include "DataCvt_Base.h"

using namespace std;
#define OUT
#define DIS  -1;//用于间隔不同属性的断点.


class DataCvt_Discrete  
{
public:
	struct SCUTINFO
	{
		double m_fCut;        //断点
		double m_fImportant;  //I该断点的重要性
		int  m_nPos;         //该断点所在的聚类类别	
	};

	//仿函数用于按照决策属性对记录集U的排序
	struct SOPERATION
	{
		SOPERATION(double**& info,int num)
		{
			m_ppfInfo = info;
			m_iAttr   = num;
		}
		double**  m_ppfInfo;
		int m_iAttr;
		bool operator()(int i,int j)
		{
			return m_ppfInfo[i][m_iAttr]<m_ppfInfo[j][m_iAttr];
		}
	};

	//存储属性K的断点信息
	struct CUTKINFO
	{
		vector<SCUTINFO> m_vecCutk;                  //属性K的断点信息
		int              m_nMid;                     //最重要的断点在m_vecCutk中的位置.
	};	

	struct CUTNODENEW
	{
		int   m_nAttr;          //该断点所在的属性.
		double m_fValue;         //断点值.
		int   m_nImportant;     //该断点在U上能区分的对象个数
	}; 	 

	//聚类类别相关信息
	struct CENTERNODE
	{
		double m_fvalue;                              //聚类的中心
		int  m_num;                                  //该类中所包含的断点数目
		double m_fSi;                                 //该类别的标准差         
	}; 

	//某个类别中的最大断点和最小断点
	struct CMAXMINCUT
	{		
		double m_fMaxCut;     //断点值
		double m_fImportant;  //该断点的重要性
	};


public:
	DataCvt_Discrete();
	virtual ~DataCvt_Discrete();

	//保存每个进程的聚类结果
	struct CLUSTRRESULT
	{
		int m_nAttrNo;     //属性标号
		vector<double>  m_vecCutCluster;  //保存聚类后的断点集
	};

public:	
	bool ReadInfoTable();                             //读取信息表	
	void DiscreteTable();                                               //对整个信息表进行 动态聚类离散化
	void ParallelDiscreteTableByCluster();                              //对整个信息表进行并行动态聚类离散化	 
	
	void SaveDiscreteResult();                       //保存离散化结果到strPath路径下
	void FilterCutByParallel();
	void FilterCutByOneProcess();
private:		
	struct CUTNODE
	{
		int m_nAttr;      //该断点所在的属性.
		double m_fValue;   //断点值.
	};			
	vector<vector<int> >      m_vecL;                                  //存储等价类.
	vector<vector<CUTNODE> >          m_vecCutSlect;                           //存储聚类后待选取的断点
protected: 			
	int GetCutImpotant(const CUTNODE& CutNode);                        //计算m_vecCutSlect中第M个断点的重要性	

private:
	//按照RIDAS数据格式存储信息表相关信息           
	int      m_iAttNum;                                            //条件属性数目
	int      m_iRecordNum;                                         //记录数目

	double**  m_ppfInfoTable;	                                   //存储信息表 

	vector<vector<double> > m_vecSelectedCut;                       //存储最终选取的断点集合.
	vector<double>          m_vecfCutk;                             //保存聚类后选取的属性K断点.
private: 
	CUTKINFO m_CutInfok;                                          //属性K的断点信息
protected: 
	bool GetCandidateCut(const unsigned int k);                   //计算属性K上候选断点集合,结果保存在vecCutk。
	bool GetCutImportant_ByAlgebra(const unsigned int k);         //利用代数方法（断点可区分的属性数目)计算属性K上的候选断点的重要性
	bool DiscreteByClustering(const unsigned int k);              //采用动态K平均聚类进行对属性K进行离散化.
	bool ClusteringPart(const int k,const int l,const int h);     //采用动态K平均聚类进行对属性K的l-h之间的断点进行聚类.  
	void TransTable();                                            //根据选取的断点对信息表进行转化.	

public:
	int main_dis(int argc, char* argv[]);
private:
	void ShowExcuteInfor();

	void WriteExcuteLog();

	CBaseOperate pBaseOperate;double ** c_AllNumsGroupByCols;int ** c_AllNumsGroupByBands;

	int m_Numproc,m_rankid;

	std::string pFileExtensionName;int RowNum;int ColNum;int BandCount;

	double Mpi_Start_Moment,Mpi_Finish_Moment;
	double Mpi_Caculate_Time;

	std::string m_PathOrTableName;                           //输入文件路径 对应-p参数
	std::string m_ResultPathOrTableName;           //输出文件路径 对应-o参数
    std::string m_DatasourceConStr;
	std::vector <int> c_CaculateCols;                                 //需要计算的列号或波段号 对应-c参数
	int c_SkipLineNums;                                               //跳过的行数，及头部信息行数 对应-t参数
	int c_DecideCol;
	bool c_IsUserKMeansFirst;
	bool c_IsCoutClassInfor;
};

#endif // !defined(AFX_DICRETE_H__156F6854_EACB_4FC8_9C89_6D6F1EE68C1F__INCLUDED_)
