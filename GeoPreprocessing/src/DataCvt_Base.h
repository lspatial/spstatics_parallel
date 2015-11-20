#ifndef _BASEFUNCTION_H
#define _BASEFUNCTION_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <ogrsf_frmts.h>

#ifdef _WIN32

#define  WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#else

#include <unistd.h>
#include <string.h>
#endif

#define MyFile_MAX_PATH  200
#define OneItemMaxLength 16


//���ú���
std::string GetFileExtensionName(std::string pfullPath);
bool IsFileExist(const char* pFileFullPath);
std::string GetFullFilePath(const char* pInputFileName,bool pOnlyFileDir=false);
std::string GetFileNameOnly(const char* pInputFileName);
bool CheckFileDelimChar(std::string pFileFullName);
std::string toString(double);
// �����ǻ�����
class CBaseOperate 
{
public:
	long c_TotalNums;//��������������Ŀ
	long c_TotalItemNums;//�ļ��г�ͷ����Ϣ�⣬����������
	char** c_AllDataItems;double** c_AllNumsGroupByCols;
	int* c_CauculateCols;int c_TopInforRowNum;int c_CalculateColNum;
	long c_RowNums,c_ColNums;//�ļ�������Ϣ����������ͷ����Ϣ

	CBaseOperate(void);
	~CBaseOperate(void);	
	
	double** OpenANSIFile_CPlus_GroupByCol(const char* pFileFullPath,std::vector<int> Cols_Vect,int pSkipLineNum,bool pGetRowColsNumOnly);
	double** OpenFromShp_GroupByCol(const char *pfilefullpath,const char *pfilenameonly,std::vector<int> Cols_Vect);
	void CreateCopyShp(const char* pDatasourceConStr,const char* pPathOrTableName,const char* pResultPathOrTableName);
	int SaveANSIFile_byCol(double** pResultNum,const char* pFileName);
	int SaveShpFile_byCol(double** pResultNum,const char* pOldFileFullPath,const char* pResultFileFullPath,const char* p_LayerName);
	// TODO: �ڴ�������ķ�����
};



/*********************************************************************
���δ�������(long)������(long)��ͷ����Ϣ����(int)�������ִ洢���е���Ŀ(int)���Լ��к�(int)
*********************************************************************/
struct BinaryTopInfor 
{
	long p_RowNums;long p_ColNums;
	int p_TopinforNums;int p_CalculateColNum;
	int p_CauculateCols[20];
};

/*********************************************************************
* ClassName : ReplaceStrFunctor
* Purpose   :
*********************************************************************/

struct ReplaceStrFunctor
{
	void operator() (std::string& strSrc, const std::string& strOlds, const std::string& strNews)
	{
		size_t pos;
		pos = strSrc.find(strOlds, 0);
		while (pos != std::string::npos)
		{
			strSrc.replace(pos, strOlds.size(), strNews);
			pos = strSrc.find(strOlds, pos+strNews.size());
		}
	}
};

/*********************************************************************
* ClassName : String split function class 
* Purpose   : split the string into segments by special delimits.
*********************************************************************/

struct SplitStrFunctor
{
	typedef std::vector<std::string> StrVec;

	enum { DEFAULT_STRVEC_SIZE = 10 };

	void operator() (std::string& strSrc
		, const std::string& strDelims
		, StrVec& result)
	{
		result.reserve(DEFAULT_STRVEC_SIZE);

		size_t pos, start, end;
		start = 0;

		do 
		{
			pos = strSrc.find_first_not_of(strDelims, start);

			if (pos != std::string::npos)
			{
				end = strSrc.find_first_of(strDelims, pos);

				if (end == std::string::npos)
					end = strSrc.size();

				result.push_back(strSrc.substr(pos, end-pos));

				start = end;
			}

		} while(pos != std::string::npos);
	}
};

/*********************************************************************
* ClassName : String joint function class 
* Purpose   : joint the input string vector to a single string.
*********************************************************************/

struct JointStrFunctor
{
	typedef std::vector<std::string> StrVec;
	typedef StrVec::iterator         StrVecItr;

	struct AddStrFunctor    
	{
		std::string& result;
		std::string  midChar;

		AddStrFunctor(std::string& init, const std::string& mid = "")
			: result(init)
			, midChar(mid)
		{
		}

		void operator() (const std::string& toAdd )
		{
			result += toAdd;
			result += midChar;
		}
	};

	void operator() (const StrVecItr& beg
		, const StrVecItr& end
		, std::string& strResult)
	{
		std::for_each(beg, end, AddStrFunctor(strResult, "/"));
	}
};

/*********************************************************************
* ClassName : String filter function class
* Purpose   : filter the special characters in the string.
*********************************************************************/

struct FilterStrFunctor
{
	typedef std::vector<std::string> StrVec;

	struct IsBeFiltered
	{
		const StrVec& selfFilterVec;

		IsBeFiltered(const StrVec& strFilterVec) : selfFilterVec(strFilterVec)
		{
		}

		bool operator() (char input)
		{
			std::string temp("");
			temp += input;
			return (selfFilterVec.end() != 
				std::find(selfFilterVec.begin()
				, selfFilterVec.end()
				, temp)
				);
		}
	};


	void operator() (std::string& strSrc
		, const StrVec& strFilterVec)
	{
		std::string::iterator newEnd = std::remove_if(
			strSrc.begin()
			, strSrc.end()
			, IsBeFiltered(strFilterVec) 
			);
		strSrc.erase(newEnd, strSrc.end());
	}
};

/*********************************************************************
* ClassName : GetExePath
* Purpose   : get the path & name of the execute program.
*********************************************************************/

struct GetExePath
{
	enum  
	{
		E_MAX_PATH = 260, 
	};

	std::string strExeDir;
	std::string strExeName;

	GetExePath()
	{
		char fullPath[E_MAX_PATH];

#ifdef _WIN32
		TCHAR pfullpath[E_MAX_PATH];
		LPWCH path_lpwch = pfullpath;
		::GetModuleFileName(NULL,path_lpwch, E_MAX_PATH);
		//wcstombs(fullPath,pfullpath,E_MAX_PATH);
		size_t   tmp;
		wcstombs_s(&tmp,fullPath,pfullpath,E_MAX_PATH);
#else
		char linkPath[MyFile_MAX_PATH];
		sprintf(linkPath, "/proc/%d/exe", getpid());
		ssize_t count = ::readlink(linkPath, fullPath, E_MAX_PATH);
#endif

		std::string strPath(fullPath);
		ReplaceStrFunctor rsf;
		rsf(strPath, "\\", "/");

		SplitStrFunctor::StrVec svRet;
		SplitStrFunctor ssf;
		ssf(strPath, "/", svRet);

		strExeName = *svRet.rbegin();
		JointStrFunctor jsf;
		jsf(svRet.begin(), svRet.end()-1, strExeDir);

		//�����linux�£���Ҫ����ǰ�����/����Ϊ�������ɾ������ǰ���/
#ifdef _WIN32
#else
		strExeDir = "/"+strExeDir;
#endif

	}

	const std::string& getExeDir()  const { return strExeDir;  }
	const std::string& getExeName() const { return strExeName; }

};



#endif				//_BASEOPERATE_H
