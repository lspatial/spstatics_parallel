#include "DataCvt_Base.h"
using namespace std;


bool IsFileExist(const char* pFileFullPath)
{
	fstream pFileToCheck;
	pFileToCheck.open(pFileFullPath);
	if (!pFileToCheck)
	{
		return false;
	}
	else
	{
		return true;
	}
}

string GetFileExtensionName(std::string pfullPath)
{
	std::string strPath(pfullPath);

	SplitStrFunctor::StrVec svRet;
	SplitStrFunctor ssf;
	ssf(strPath, ".", svRet);

	std::string FileExtensionName = "";
	FileExtensionName = *svRet.rbegin();

	return FileExtensionName;
}

string GetFullFilePath(const char* pInputFileName,bool pOnlyFileDir)
{
	string pStr_DataFileDir;string pStr_DataFileName;string pStrInputFileName = pInputFileName;
	GetExePath pGetpath;

	ReplaceStrFunctor rsf;
	rsf(pStrInputFileName, "\\", "/");

	SplitStrFunctor::StrVec svRet;
	SplitStrFunctor ssf;
	ssf(pStrInputFileName, "/", svRet);

	pStr_DataFileName = *svRet.rbegin();

	if(svRet.size()!=1)
	{
		JointStrFunctor jsf;
		jsf(svRet.begin(), svRet.end()-1, pStr_DataFileDir);
	    //linux下需要在前面添加一个/
       #ifdef _WIN32
       #else
		pStr_DataFileDir = "/"+pStr_DataFileDir;
       #endif
	}
	else
	{
		pStr_DataFileDir =pGetpath.strExeDir;
	}
	if (!pOnlyFileDir)
	{
		return pStr_DataFileDir+pStr_DataFileName;
	}
	else
	{
		return pStr_DataFileDir;
	}

}


string GetFileNameOnly(const char* pInputFileName)
{
	string pStr_DataFileName;string pStrInputFileName = pInputFileName;
	GetExePath pGetpath;

	ReplaceStrFunctor rsf;
	rsf(pStrInputFileName, "\\", "/");

	SplitStrFunctor::StrVec svRet;
	SplitStrFunctor ssf;
	ssf(pStrInputFileName, "/", svRet);

	pStr_DataFileName = *svRet.rbegin();

	return pStr_DataFileName;
}

bool CheckFileDelimChar(std::string pFileFullName)
{
	ifstream pFileToRead(pFileFullName.c_str());
	char pFirstRow[500];
	pFileToRead.get(pFirstRow,500,'\n');
	int pIndexOfRow = 0;
	while(pIndexOfRow<500)
	{
		if (pFirstRow[pIndexOfRow]==',')
		{
			return true;
		}
		pIndexOfRow++;
	}
	return false;
}


CBaseOperate::CBaseOperate()
{
	c_RowNums = 0;
	c_ColNums = 0;
	c_TopInforRowNum = 2;
	c_CalculateColNum = 0;
	
	c_AllDataItems = NULL;
	c_CauculateCols = NULL;
	c_AllNumsGroupByCols = NULL;
	return;
}


CBaseOperate::~CBaseOperate()
{
	if (c_AllDataItems!=NULL)
	{
		delete []c_AllDataItems;
	}
	if (c_CauculateCols!=NULL)
	{
		delete []c_CauculateCols;
	}
	if (c_AllNumsGroupByCols!=NULL)
	{
		delete []c_AllNumsGroupByCols;
	}

}


double** CBaseOperate::OpenANSIFile_CPlus_GroupByCol(const char* pFileFullPath,vector<int> Cols_Vect,int pSkipLineNum,bool pGetRowColsNumOnly)
{
	int ColsCount = Cols_Vect.size();
	ifstream pFileToRead(pFileFullPath);
	//获取文件行数列数
	int pRowNums(0), pColNums(0);char pFirstRow[500] = {0};
	while(!pFileToRead.eof())
	{		
		char pRowTemp[300]={0};
		pFileToRead.getline(pRowTemp,300,'\n');
		if (pRowTemp[2]!=0)
		{
			pRowNums++;
		}		
	}
	
	pFileToRead.clear();
	pFileToRead.seekg(0, ios_base::beg );
	pFileToRead.get(pFirstRow,500,'\n');
	int pIndexOfRow = 0;
	while(pIndexOfRow<500)
	{
		if (pIndexOfRow>0)
		{
			if ((pFirstRow[pIndexOfRow]==',')&&(pFirstRow[pIndexOfRow-1]!=','))
			{
				pColNums++;
			}
		}
		pIndexOfRow++;		
	}

	pColNums++;

	//保存需要计算的列
	c_CauculateCols = new int[ColsCount];
	for (int i=0;i<ColsCount;i++)
	{
		c_CauculateCols[i] = Cols_Vect[i];
	}

	c_RowNums = pRowNums;c_ColNums = pColNums;c_TopInforRowNum = pSkipLineNum;c_CalculateColNum = ColsCount;
	c_TotalNums = (pRowNums-pSkipLineNum)*ColsCount;c_TotalItemNums = pRowNums*pColNums;

	//只获取行列数
	if (pGetRowColsNumOnly)
	{
		pFileToRead.close();
		return NULL;		
	}
	c_AllDataItems = new char*[c_TotalItemNums];


	double* c_AllNumsTogether = new double[c_TotalNums]();
	c_AllNumsGroupByCols = new double*[ColsCount];
	for (int i=0;i<ColsCount;i++)
	{
		c_AllNumsGroupByCols[i] = &c_AllNumsTogether[i*(pRowNums-pSkipLineNum)];
	}

	pFileToRead.clear();
	pFileToRead.seekg(0, ios_base::beg );

	long pAllNums_index = 0;long pAllDataItemIndex = 0;long pAllNumsGroupByCols_index=0;
	for (long tt=0;tt<pRowNums;tt++)
	{
		string pRowAllValues;
		getline(pFileToRead,pRowAllValues,'\n');
		SplitStrFunctor::StrVec svRet;
		SplitStrFunctor ssf;
		ssf(pRowAllValues, ",", svRet);
		if(svRet.size()==pColNums)
		{
			JointStrFunctor::StrVecItr iter; 
			for( iter = svRet.begin(); iter != svRet.end(); iter++ ) 
			{ 
				string pEachValue = *iter;
				c_AllDataItems[pAllDataItemIndex] = new char[OneItemMaxLength]();
				strcpy(c_AllDataItems[pAllDataItemIndex],pEachValue.c_str());

				if (tt>=pSkipLineNum)
				{
					for (int i=0;i<ColsCount;i++)
					{				
						if ((pAllDataItemIndex%pColNums)+1==Cols_Vect[i])
						{
							c_AllNumsGroupByCols[i][pAllNumsGroupByCols_index] = atof(c_AllDataItems[pAllDataItemIndex]);
							if (i==ColsCount-1)
							{
								pAllNumsGroupByCols_index++;
							}
							break;
						}
					}
				}
				pAllDataItemIndex++;
			} 
		}
		else
		{
			for (int a=0;a<pColNums;a++)
			{
				c_AllDataItems[pAllDataItemIndex] = new char[OneItemMaxLength]();
				c_AllDataItems[pAllDataItemIndex][0] = '0';c_AllDataItems[pAllDataItemIndex][1] = '\0';
				if (tt>=pSkipLineNum)
				{
					for (int i=0;i<ColsCount;i++)
					{				
						if ((pAllDataItemIndex%pColNums)+1==Cols_Vect[i])
						{
							c_AllNumsGroupByCols[i][pAllNumsGroupByCols_index]=0;
							if (i==ColsCount-1)
							{
								pAllNumsGroupByCols_index++;
							}
							break;
						}
					}
				}
				pAllDataItemIndex++;
			}
		}
	}
	pFileToRead.close();
	return c_AllNumsGroupByCols;

}



int CBaseOperate::SaveANSIFile_byCol(double** pResultNum,const char* pFileName)
{
	fstream pSaveFile;
	pSaveFile.open(pFileName,ios::out);
	if(!pSaveFile)
	{
		return 0;
	}
	else
	{
		bool pIsCalculateCol = false;long pAllNumsGroupByCols_index=0;
		for (int tt=0;tt<c_TotalItemNums;tt++)
		{	
			pIsCalculateCol = false;

			if (pResultNum!=NULL)
			{
				if (tt>=c_ColNums*c_TopInforRowNum)
				{
					for (int i=0;i<c_CalculateColNum;i++)
					{
						if ((tt%c_ColNums)+1==c_CauculateCols[i])
						{
							pIsCalculateCol = true;
							pSaveFile<<pResultNum[i][pAllNumsGroupByCols_index];
							if (i==c_CalculateColNum-1)
							{
								pAllNumsGroupByCols_index++;
							}
							break;
						}
					}
				}
				else//头部信息保存
				{
					if(tt<c_ColNums)
					{
						for (int i=0;i<c_CalculateColNum;i++)
						{
							if ((tt%c_ColNums+1)==c_CauculateCols[i])
							{
								pIsCalculateCol = true;
								pSaveFile<<"["<<c_AllDataItems[tt]<<"]";
								break;
							}
						}
					}					
				}
			}
			
			
			if (!pIsCalculateCol)
			{
				pSaveFile<<c_AllDataItems[tt];
			}				
			if ((tt+1)%c_ColNums!=0)
			{
				pSaveFile<<",";
			}
			else
			{
				pSaveFile<<"\n";
			}
		}
	}
	pSaveFile.close();
	return 1;
}


