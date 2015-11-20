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
#define DIS  -1;//���ڼ����ͬ���ԵĶϵ�.


class DataCvt_Discrete  
{
public:
	struct SCUTINFO
	{
		double m_fCut;        //�ϵ�
		double m_fImportant;  //I�öϵ����Ҫ��
		int  m_nPos;         //�öϵ����ڵľ������	
	};

	//�º������ڰ��վ������ԶԼ�¼��U������
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

	//�洢����K�Ķϵ���Ϣ
	struct CUTKINFO
	{
		vector<SCUTINFO> m_vecCutk;                  //����K�Ķϵ���Ϣ
		int              m_nMid;                     //����Ҫ�Ķϵ���m_vecCutk�е�λ��.
	};	

	struct CUTNODENEW
	{
		int   m_nAttr;          //�öϵ����ڵ�����.
		double m_fValue;         //�ϵ�ֵ.
		int   m_nImportant;     //�öϵ���U�������ֵĶ������
	}; 	 

	//������������Ϣ
	struct CENTERNODE
	{
		double m_fvalue;                              //���������
		int  m_num;                                  //�������������Ķϵ���Ŀ
		double m_fSi;                                 //�����ı�׼��         
	}; 

	//ĳ������е����ϵ����С�ϵ�
	struct CMAXMINCUT
	{		
		double m_fMaxCut;     //�ϵ�ֵ
		double m_fImportant;  //�öϵ����Ҫ��
	};


public:
	DataCvt_Discrete();
	virtual ~DataCvt_Discrete();

	//����ÿ�����̵ľ�����
	struct CLUSTRRESULT
	{
		int m_nAttrNo;     //���Ա��
		vector<double>  m_vecCutCluster;  //��������Ķϵ㼯
	};

public:	
	bool ReadInfoTable();                             //��ȡ��Ϣ��	
	void DiscreteTable();                                               //��������Ϣ����� ��̬������ɢ��
	void ParallelDiscreteTableByCluster();                              //��������Ϣ����в��ж�̬������ɢ��	 
	
	void SaveDiscreteResult();                       //������ɢ�������strPath·����
	void FilterCutByParallel();
	void FilterCutByOneProcess();
private:		
	struct CUTNODE
	{
		int m_nAttr;      //�öϵ����ڵ�����.
		double m_fValue;   //�ϵ�ֵ.
	};			
	vector<vector<int> >      m_vecL;                                  //�洢�ȼ���.
	vector<vector<CUTNODE> >          m_vecCutSlect;                           //�洢������ѡȡ�Ķϵ�
protected: 			
	int GetCutImpotant(const CUTNODE& CutNode);                        //����m_vecCutSlect�е�M���ϵ����Ҫ��	

private:
	//����RIDAS���ݸ�ʽ�洢��Ϣ�������Ϣ           
	int      m_iAttNum;                                            //����������Ŀ
	int      m_iRecordNum;                                         //��¼��Ŀ

	double**  m_ppfInfoTable;	                                   //�洢��Ϣ�� 

	vector<vector<double> > m_vecSelectedCut;                       //�洢����ѡȡ�Ķϵ㼯��.
	vector<double>          m_vecfCutk;                             //��������ѡȡ������K�ϵ�.
private: 
	CUTKINFO m_CutInfok;                                          //����K�Ķϵ���Ϣ
protected: 
	bool GetCandidateCut(const unsigned int k);                   //��������K�Ϻ�ѡ�ϵ㼯��,���������vecCutk��
	bool GetCutImportant_ByAlgebra(const unsigned int k);         //���ô����������ϵ�����ֵ�������Ŀ)��������K�ϵĺ�ѡ�ϵ����Ҫ��
	bool DiscreteByClustering(const unsigned int k);              //���ö�̬Kƽ��������ж�����K������ɢ��.
	bool ClusteringPart(const int k,const int l,const int h);     //���ö�̬Kƽ��������ж�����K��l-h֮��Ķϵ���о���.  
	void TransTable();                                            //����ѡȡ�Ķϵ����Ϣ�����ת��.	

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

	std::string m_PathOrTableName;                           //�����ļ�·�� ��Ӧ-p����
	std::string m_ResultPathOrTableName;           //����ļ�·�� ��Ӧ-o����
    std::string m_DatasourceConStr;
	std::vector <int> c_CaculateCols;                                 //��Ҫ������кŻ򲨶κ� ��Ӧ-c����
	int c_SkipLineNums;                                               //��������������ͷ����Ϣ���� ��Ӧ-t����
	int c_DecideCol;
	bool c_IsUserKMeansFirst;
	bool c_IsCoutClassInfor;
};

#endif // !defined(AFX_DICRETE_H__156F6854_EACB_4FC8_9C89_6D6F1EE68C1F__INCLUDED_)
