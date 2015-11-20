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

string toString(double element)
{
	ostringstream oss;
	oss<<element;
	string str(oss.str());

	return str;
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
	if (!IsFileExist(pFileFullPath))
	{
		cout<<pFileFullPath<<" does not exist, please check..."<<endl;
		return 0;
	}
	int ColsCount = Cols_Vect.size();
	ifstream pFileToRead(pFileFullPath);
	//获取文件行数
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
	//获取文件列数
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
   if(ColsCount>pColNums)
   {
	   cout<<"the column num need to caculate is bigger than the max..."<<endl;
	   exit(0);
   }
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
#ifdef _WIN32
				strcpy_s(c_AllDataItems[pAllDataItemIndex],OneItemMaxLength,pEachValue.c_str());
#else
				strcpy(c_AllDataItems[pAllDataItemIndex],pEachValue.c_str());
#endif
				if (tt>=pSkipLineNum)
				{
					for (int i=0;i<ColsCount;i++)
					{				
						if ((pAllDataItemIndex%pColNums)==Cols_Vect[i])
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
			cout<<"read row "<<tt<<" error,please check..."<<endl;
		}
	}
	pFileToRead.close();
	return c_AllNumsGroupByCols;

}


double** CBaseOperate::OpenFromShp_GroupByCol(const char *pDatasourceConStr,const char *pPathOrTableName,std::vector<int> Cols_Vect)
{
	OGRRegisterAll();
	string pLayerName = pPathOrTableName;
    if(strcmp(pDatasourceConStr,"")==0)//input is shp file
    {
    	pDatasourceConStr = pPathOrTableName;
    	string pShpPath = pPathOrTableName;
    	string p_LayerName = GetFileNameOnly(pShpPath.c_str());
    	int pindex = p_LayerName.find_first_of('.');
    	pLayerName = p_LayerName.erase(pindex,p_LayerName.size()-1);
    }

	OGRDataSource* poDS = OGRSFDriverRegistrar::Open(pDatasourceConStr,FALSE);
	OGRLayer* poLayer = poDS->GetLayerByName(pLayerName.c_str());
	OGRFeature * pFeature =poLayer->GetNextFeature();

	c_RowNums = poLayer->GetFeatureCount();
	c_ColNums= pFeature->GetFieldCount();

	int ColsCount = Cols_Vect.size();
	//保存需要计算的列
	c_CauculateCols = new int[ColsCount];
	for (int i=0;i<ColsCount;i++)
	{
		c_CauculateCols[i] = Cols_Vect[i];
	}

	c_TotalNums = c_RowNums*ColsCount;c_TopInforRowNum = 0;c_CalculateColNum = Cols_Vect.size();
	double* c_AllNumsTogether = new double[c_TotalNums]();
	c_AllNumsGroupByCols = new double*[ColsCount];
	for (int i=0;i<ColsCount;i++)
	{
		c_AllNumsGroupByCols[i] = &c_AllNumsTogether[i*c_RowNums];
	}

	int pRowIndex = 0;
	while (pFeature!=NULL)
	{
	  for(int i=0;i<ColsCount;i++)
		{
		   c_AllNumsGroupByCols[i][pRowIndex] = pFeature->GetFieldAsDouble(c_CauculateCols[i]);
		}
	   pRowIndex++;
	   pFeature =poLayer->GetNextFeature();
	}

	OGRDataSource::DestroyDataSource( poDS );

   return c_AllNumsGroupByCols;

}


void CBaseOperate::CreateCopyShp(const char* pDatasourceConStr,const char* pPathOrTableName,const char* pResultPathOrTableName)
{
	/*
	if(strcmp(pDatasourceConStr,"")==0)
	{
		if (IsFileExist(pResultPathOrTableName))
		{
			cout<<pResultPathOrTableName<<" has existed, please enter a new name..."<<endl;
			exit(0);
		}
	}*/

	OGRRegisterAll();
	OGRDataSource* poDS;OGRDataSource* poResultDS;OGRLayer* poSourceLayer;OGRLayer* poResultLayer;
	OGRFeature * pSourceFeature;
	string pSourceLayerName = pPathOrTableName;string pResultLayerName = pResultPathOrTableName;
        cout<<pDatasourceConStr<<endl;
	if(strcmp(pDatasourceConStr,"")==0)//input is shp file
	{
		string p_LayerName = GetFileNameOnly(pPathOrTableName);
		int pindex = p_LayerName.find_first_of('.');
		pSourceLayerName = p_LayerName.erase(pindex,p_LayerName.size()-1);

	    p_LayerName = GetFileNameOnly(pResultPathOrTableName);
		pindex = p_LayerName.find_first_of('.');
		pResultLayerName = p_LayerName.erase(pindex,p_LayerName.size()-1);

	   poDS = OGRSFDriverRegistrar::Open(pPathOrTableName,FALSE);
	   if(OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile")->Open(pResultPathOrTableName)!=NULL)
	   {
		   OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile")->DeleteDataSource(pResultPathOrTableName);
	   }
	   poResultDS= OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile")->CreateDataSource(pResultPathOrTableName,NULL);
	   //poResultDS= OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile")->CopyDataSource(poDS,pResultPathOrTableName);
	}
	else
	{
	   poDS = OGRSFDriverRegistrar::Open(pDatasourceConStr,TRUE);
	   poResultDS = poDS;
	}

	char **papszMetadata = NULL;
	papszMetadata = CSLSetNameValue(papszMetadata, "OVERWRITE", "YES" );

	poSourceLayer = poDS->GetLayerByName(pSourceLayerName.c_str());
	poResultLayer = poResultDS->CopyLayer(poSourceLayer,pResultLayerName.c_str(),papszMetadata);
	/*
	OGRSpatialReference * pSpatialReference = poSourceLayer->GetSpatialRef();
	OGRwkbGeometryType pwkbGeometryType =poSourceLayer->GetGeomType();
	poResultLayer = poResultDS->CreateLayer(pResultLayerName.c_str(),pSpatialReference,pwkbGeometryType,papszMetadata);
	pSourceFeature = poSourceLayer->GetNextFeature();
	if(pSourceFeature!=NULL)
	{
		 int pColNums = pSourceFeature->GetFieldCount();
		 for(int i=0;i<pColNums;i++)
		 {
			 poResultLayer->CreateField(pSourceFeature->GetFieldDefnRef(i));
		 }
	}
	while (pSourceFeature!=NULL)
	{
		poResultLayer->CreateFeature(pSourceFeature);
		pSourceFeature = poSourceLayer->GetNextFeature();
	}*/

	OGRDataSource::DestroyDataSource( poDS );
	if(strcmp(pDatasourceConStr,"")==0)//input is shp file
	{
		OGRDataSource::DestroyDataSource( poResultDS );
	}
}



int CBaseOperate::SaveShpFile_byCol(double** pResultNum,const char* pDatasourceConStr,const char* pPathOrTableName,const char* pResultPathOrTableName)
{
	OGRRegisterAll();
	OGRDataSource* poDS;OGRDataSource* poResultDS;OGRLayer* poResultLayer;
	OGRFeature * pResultFeature;
	string pSourceLayerName = pPathOrTableName;string pResultLayerName = pResultPathOrTableName;
	if(strcmp(pDatasourceConStr,"")==0)//input is shp file
	{
		string p_LayerName = GetFileNameOnly(pPathOrTableName);
		int pindex = p_LayerName.find_first_of('.');
		pSourceLayerName = p_LayerName.erase(pindex,p_LayerName.size()-1);

	    p_LayerName = GetFileNameOnly(pResultPathOrTableName);
		pindex = p_LayerName.find_first_of('.');
		pResultLayerName = p_LayerName.erase(pindex,p_LayerName.size()-1);

	   poDS = OGRSFDriverRegistrar::Open(pPathOrTableName,FALSE);
           poResultDS = OGRSFDriverRegistrar::Open(pResultPathOrTableName,FALSE);
	}
	else
	{
	   poDS = OGRSFDriverRegistrar::Open(pDatasourceConStr,TRUE);
	   poResultDS = poDS;
	}

	poResultLayer = poResultDS->GetLayerByName(pResultLayerName.c_str());
	pResultFeature = poResultLayer->GetNextFeature();
        
        int pRowIndex = 0;
	while(pResultFeature!=NULL)
	{
	    for(int i=0;i<c_CalculateColNum;i++)
		{
	    	pResultFeature->SetField(c_CauculateCols[i],pResultNum[i][pRowIndex]);
		}
            poResultLayer->SetFeature(pResultFeature);
	    pRowIndex++;
	    pResultFeature = poResultLayer->GetNextFeature();
	}

        cout<<"modifed row nums:"<<pRowIndex<<endl;

	OGRDataSource::DestroyDataSource( poDS );
	if(strcmp(pDatasourceConStr,"")==0)//input is shp file
	{
	    OGRDataSource::DestroyDataSource( poResultDS );
	}
	return 1;
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
						if ((tt%c_ColNums)==c_CauculateCols[i])
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
							if ((tt%c_ColNums)==c_CauculateCols[i])
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


