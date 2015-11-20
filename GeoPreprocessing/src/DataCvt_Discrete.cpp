// DataCvt_Discrete.cpp
// This algorithm requires data is on each node 
//////////////////////////////////////////////////////////////////////

#include "DataCvt_Discrete.h"

int DataCvt_Discrete::main_dis(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);   
	MPI_Comm_size(MPI_COMM_WORLD,&m_Numproc);
	MPI_Comm_rank(MPI_COMM_WORLD,&m_rankid);
	Mpi_Start_Moment = MPI_Wtime(); 
       string dbname="";string hostname="";string username="";string pwd="";string port="";
	if (argc>1)
	{
		for (int i=1;i<argc;i++)
		{
			
			if (!strcmp(argv[i],"-p"))
			{
				m_PathOrTableName = argv[i+1];
			}
			//band or column
			if (!strcmp(argv[i],"-c"))
			{
				string pCaculateCols = argv[i+1];
				SplitStrFunctor::StrVec svRet;
				SplitStrFunctor ssf;
				ssf(pCaculateCols, ",", svRet);
				JointStrFunctor::StrVecItr iter; 
				for( iter = svRet.begin(); iter != svRet.end(); iter++ )
				{
					string pEachColValue = *iter;
					c_CaculateCols.push_back(atoi(pEachColValue.c_str()));
				}
			}
			//decision column
			if (!strcmp(argv[i],"-C"))
			{
				string pCaculateCols = argv[i+1];
				c_DecideCol = atoi(pCaculateCols.c_str());
				c_CaculateCols.push_back(c_DecideCol);
			}
			//kmeans
			if (!strcmp(argv[i],"-k"))
			{
				if(atoi(argv[i+1])==1)
				{
				    c_IsUserKMeansFirst = true;
				}
				else
				{
					c_IsUserKMeansFirst = false;
				}
			}
			//cout class information
			if (!strcmp(argv[i],"-co"))
			{
				if(atoi(argv[i+1])==1)
				{
					c_IsCoutClassInfor = true;
				}
				else
				{
					c_IsCoutClassInfor = false;
				}
			}
			//out path
                     if (!strcmp(argv[i],"-o"))
			{
				m_ResultPathOrTableName = argv[i+1];
			}

			if (!strcmp(argv[i],"-dbname"))
			{
				dbname= argv[i+1];
			}
                     if (!strcmp(argv[i],"-hostname"))
			{
				hostname= argv[i+1];
			}
                     if (!strcmp(argv[i],"-username"))
			{
				username= argv[i+1];
			}
                     if (!strcmp(argv[i],"-pwd"))
			{
				pwd= argv[i+1];
			}
                     if (!strcmp(argv[i],"-port"))
			{
				port= argv[i+1];
			}
                     //connection string
			//m_DatasourceConStr = "PG:host='192.168.5.131' port='5437'  dbname='pdatabase' user='postgres' password='postgres'";
                     
		}
	}
         m_DatasourceConStr  = "PG:host='"+hostname+"' port='"+port+"' dbname='"+dbname+"' user='"+username+"' password='"+pwd+"'";
         //if(m_rankid==0)
         // cout<<m_DatasourceConStr<<endl;

	FilterCutByParallel();
    Mpi_Finish_Moment =  MPI_Wtime();

	if (0 == m_rankid)
	{		
		ShowExcuteInfor();
		//WriteExcuteLog();
	} 

	//cout<<"Process "<<m_rankid<<" finish..."<<endl;
	MPI_Finalize(); 

	return 0;
}


void DataCvt_Discrete::ShowExcuteInfor()
{
	cout<<"[DEBUG] [OPTIONS] input file directory: "<<m_PathOrTableName<<endl;
	cout<<"[DEBUG] [OPTIONS] output file directory: "<<m_ResultPathOrTableName<<endl;

	cout<<"[DEBUG] [OPTIONS] input file size is : "<<RowNum<<"*"<<ColNum<<"*"<<BandCount<<endl;

	cout<<"[DEBUG]  time consuming:"<<endl;
	cout<<"[DEBUG][TIMESPAN][IO] "<<Mpi_Finish_Moment-Mpi_Start_Moment-Mpi_Caculate_Time<<endl;
	cout<<"[DEBUG][TIMESPAN][COMPUTING] "<<Mpi_Caculate_Time<<endl;
	cout<<"[DEBUG][TIMESPAN][TOTAL] "<<Mpi_Finish_Moment-Mpi_Start_Moment<<endl;
}


void DataCvt_Discrete::WriteExcuteLog()
{
	string path_log = "log_DataCvt_Descrete.txt";
	path_log = GetFullFilePath(path_log.c_str());
	fstream pSaveFile;
	pSaveFile.open(path_log.c_str(),ios::out|ios::app);
	pSaveFile<<"=======================ProcessNum:"<<m_Numproc<<"======================="<<endl;
	pSaveFile<<"[DEBUG] [TIMESPAN] [IO] "<<Mpi_Finish_Moment-Mpi_Start_Moment-Mpi_Caculate_Time<<"\n";
	pSaveFile<<"[DEBUG] [TIMESPAN] [COMPUTING] "<<Mpi_Caculate_Time<<"\n";
	pSaveFile<<"[DEBUG] [TIMESPAN] [TOTAL] "<<Mpi_Finish_Moment-Mpi_Start_Moment<<"\n";
	pSaveFile.close();
}


DataCvt_Discrete::DataCvt_Discrete()
{
	//指针类型变量初始化
	this->m_ppfInfoTable      = NULL;

	c_AllNumsGroupByCols = NULL;
	c_AllNumsGroupByBands = NULL;

	//基本数据类型初始化
	this->m_iAttNum           =  0;
	this->m_iRecordNum        =  0;

	this->c_SkipLineNums = 1;
	Mpi_Caculate_Time = 0;
	c_DecideCol = -1;

	RowNum = 0;ColNum = 0;BandCount = 1;
	pFileExtensionName = "csv";

	m_DatasourceConStr = "";

	c_IsUserKMeansFirst = false;
	c_IsCoutClassInfor = false;
}


DataCvt_Discrete::~DataCvt_Discrete()
{
	//释放this->m_ppfInfoTable的存储空间 
	for (int i=0; i<this->m_iRecordNum; i++)
	{
		if (this->m_ppfInfoTable[i] != NULL)
		{
			delete[]this->m_ppfInfoTable[i];
			this->m_ppfInfoTable[i] = NULL; 
		}
	}
	if (this->m_ppfInfoTable != NULL)
	{
		delete[] this->m_ppfInfoTable;
		this->m_ppfInfoTable = NULL;
	}	
}


bool DataCvt_Discrete::ReadInfoTable()
{
	pFileExtensionName = GetFileExtensionName(m_PathOrTableName);

	if (pFileExtensionName=="tif")
	{
		GDALAllRegister();
		GDALDataset* dataset_gdal = (GDALDataset *) GDALOpen( m_PathOrTableName.c_str(), GA_ReadOnly );
		if (dataset_gdal == NULL )
		{
			cout<<"the file does not exist..."<<endl;
		}
		int width = dataset_gdal->GetRasterXSize();
		int height = dataset_gdal->GetRasterYSize();
		int bandcount = dataset_gdal->GetRasterCount();

		RowNum = height;ColNum = width;BandCount = bandcount;

		GDALDriver *driver = dataset_gdal->GetDriver();	

		c_AllNumsGroupByBands = new int*[bandcount];
			
		for(int t=0;t<bandcount;t++)
		{
			c_AllNumsGroupByBands[t] = new int[height*width]();
		}

		//use all bands
		if (c_CaculateCols.size()==0)
		{
			for (int r=1;r<=bandcount;r++)
			{
				c_CaculateCols.push_back(r);
			}
		}

		for(int t=0;t<c_CaculateCols.size();t++)
		{
			GDALRasterBand * poband=dataset_gdal->GetRasterBand(c_CaculateCols[t]);
			poband->RasterIO(GF_Read,0,0,width,height,c_AllNumsGroupByBands[t],width,height,GDT_CInt16,0,0);
		}

		m_iRecordNum = height*width;
		m_iAttNum = bandcount;
	}

	if ((pFileExtensionName=="shp")||strcmp(m_DatasourceConStr.c_str(),""))
	{
		if(pFileExtensionName=="shp")
			m_DatasourceConStr="";
		c_AllNumsGroupByCols = pBaseOperate.OpenFromShp_GroupByCol(m_DatasourceConStr.c_str(),m_PathOrTableName.c_str(),c_CaculateCols);
		m_iRecordNum = pBaseOperate.c_RowNums - pBaseOperate.c_TopInforRowNum;
		m_iAttNum = this->c_CaculateCols.size();

		RowNum = pBaseOperate.c_RowNums-pBaseOperate.c_TopInforRowNum;
	   ColNum = pBaseOperate.c_ColNums;
	}


	if (c_DecideCol!=-1)//如果输入-C参数，则需要将m_iAttNum减去1
	{
		m_iAttNum--;
	}
	

	if (this->m_ppfInfoTable == NULL)
	{			
		this->m_ppfInfoTable = new double*[this->m_iRecordNum]();
	}		
	for(int i=0; i<this->m_iRecordNum; i++)
	{		
		this->m_ppfInfoTable[i]=new double[this->m_iAttNum+1]();
	}

	for (int i=0; i<this->m_iRecordNum; i++)
	{
		for (int j=0; j<this->m_iAttNum; j++)
		{		
			if (c_AllNumsGroupByCols!=NULL)
			{
				this->m_ppfInfoTable[i][j]=c_AllNumsGroupByCols[j][i];
			}
			else
			{
				this->m_ppfInfoTable[i][j]=(double)c_AllNumsGroupByBands[j][i];
			}
		}
		
		if (c_DecideCol==-1)//没有选择决策属性所在列或者波段
		{
			int pDecideColNum = i/(m_iRecordNum/3);
			if (pDecideColNum>3)
			{
				pDecideColNum = 3;
			}
		    this->m_ppfInfoTable[i][m_iAttNum] = pDecideColNum;//决策属性共分为3类，按顺序分别为0,1,2
		}
		else
		{
			this->m_ppfInfoTable[i][m_iAttNum] = c_AllNumsGroupByCols[m_iAttNum][i];
		}
	}   
	return true;			
}


//保存离散化后的信息表到strPath下
void DataCvt_Discrete::SaveDiscreteResult()
{
	//*********************0进程将数据进行存储*****************
	if (m_rankid==0)
	{
		this->TransTable();
		string pFileExtensionName = GetFileExtensionName(m_ResultPathOrTableName);

		if ((pFileExtensionName=="shp")||strcmp(m_DatasourceConStr.c_str(),""))
		{
			/*
			if(strcmp(m_DatasourceConStr.c_str(),"")==0)
			{
			   if (IsFileExist(this->m_ResultPathOrTableName.c_str()))
			   {
				  cout<<this->m_ResultPathOrTableName<<" has existed, please enter a new name..."<<endl;
				  exit(0);
			   }
			}*/
			//将计算后的m_ppfInfoTable保存至c_AllNumsGroupByCols
			for (int i=0; i<this->m_iRecordNum; i++)
			{
				for (int j=0; j<this->m_iAttNum; j++)
				{		
					c_AllNumsGroupByCols[j][i] = this->m_ppfInfoTable[i][j];			
				}
			}  
                        pBaseOperate.CreateCopyShp(m_DatasourceConStr.c_str(),m_PathOrTableName.c_str(),m_ResultPathOrTableName.c_str());
			pBaseOperate.SaveShpFile_byCol(c_AllNumsGroupByCols,this->m_DatasourceConStr.c_str(),this->m_PathOrTableName.c_str(),this->m_ResultPathOrTableName.c_str());
		}
		if (pFileExtensionName=="tif")
		{
			//将计算后的m_ppfInfoTable保存至c_AllNumsGroupByBands
			for (int i=0; i<this->m_iRecordNum; i++)
			{
				for (int j=0; j<this->m_iAttNum; j++)
				{		
					c_AllNumsGroupByBands[j][i] = this->m_ppfInfoTable[i][j];			
				}
			}  
			GDALAllRegister();
			GDALDataset* dataset_gdal = (GDALDataset *) GDALOpen( m_PathOrTableName.c_str(), GA_ReadOnly );
			if (dataset_gdal == NULL )
			{
				cout<<"the file does not exist..."<<endl;
			}
			int width = dataset_gdal->GetRasterXSize();int height = dataset_gdal->GetRasterYSize();

			GDALDriver *driver = dataset_gdal->GetDriver();	GDALDataset *dataset_result;

			dataset_result = driver->CreateCopy(m_ResultPathOrTableName.c_str(),dataset_gdal,0,NULL,NULL,NULL);

			for(int t=0;t<c_CaculateCols.size();t++)
			{
				GDALRasterBand * poband=dataset_result->GetRasterBand(t+1);

				//把结果数据变换到0-255之间，主要是为列了显示美观，否则全是黑色
				int Value_min = c_AllNumsGroupByBands[t][0];int Value_max=c_AllNumsGroupByBands[t][0];
				for (int qq=1;qq<m_iRecordNum;qq++)
				{
					if (c_AllNumsGroupByBands[t][qq]>Value_max)
					{
						Value_max = c_AllNumsGroupByBands[t][qq];
					}
					if (c_AllNumsGroupByBands[t][qq]<Value_min)
					{
						Value_min = c_AllNumsGroupByBands[t][qq];
					}
				}
				for (int qq=0;qq<m_iRecordNum;qq++)
				{
					c_AllNumsGroupByBands[t][qq] = (c_AllNumsGroupByBands[t][qq]-Value_min)*255/(Value_max-Value_min);
				}

				poband->RasterIO(GF_Write,0,0,width,height,c_AllNumsGroupByBands[t],width,height,GDT_CInt16,0,0);
			}
		}
       if(c_IsCoutClassInfor)
       {
		cout<<"Condition attributes number:"<<this->m_iAttNum<<endl;
		cout<<"Records number:"<<this->m_iRecordNum<<endl;

		//保存断点信息
		 int n;
		 cout<<"[Cuts]"<<endl;
		 for(int j=0;j<this->m_iAttNum;j++)
		 {
			cout<<"Column: "<<c_CaculateCols[j]<<endl;
			cout<<"Num of class: "<<this->m_vecSelectedCut[j].size()+1<<endl;
			cout<<"[*,";
			n = this->m_vecSelectedCut[j].size();int i = 0;
			for(i=0;i<n;i++)
			{
				cout<<this->m_vecSelectedCut[j][i]<<") ";
				cout<<i<<endl;
				cout<<"["<<this->m_vecSelectedCut[j][i]<<",";
			}
			cout<<"*] ";
			cout<<i<<endl;
		 }
       }
	}	
}


//计算属性K上的候选断点
bool DataCvt_Discrete::GetCandidateCut(const unsigned int k)
{
	//判断输入参数K的合法性
	if (k<0 || k>=this->m_iAttNum)
	{
		return false;
	}

	//把属性K上的属性数值拷贝到临时的向量vecTemp，然后排序
	this->m_CutInfok.m_vecCutk.clear();
	this->m_CutInfok.m_nMid = -1; 
	int i=0;
	vector<double> vecTemp;vecTemp.reserve(this->m_iRecordNum);	
	int step = m_iRecordNum/100000+1;
	while(i<this->m_iRecordNum)
	{
		vecTemp.push_back(this->m_ppfInfoTable[i][k]);
		//数据量很大只取部分数据
		i+=step;
	}
	sort(vecTemp.begin(),vecTemp.end());

	vector<double>::iterator end_unique = unique(vecTemp.begin(), vecTemp.end());//筛选出无重复的序列
	vecTemp.erase(end_unique,vecTemp.end());//擦除剩余重复元素

	
	//取相邻属性值的平均值作为候选断点
	int n = vecTemp.size();
	SCUTINFO cutinfoTemp;
	/*
	if(n<=4)
	{
		for(int i=0;i<n-1;i++)
		{
			cutinfoTemp.m_fCut =(vecTemp[i]+vecTemp[i+1])/2.0;	
			cutinfoTemp.m_fImportant = 0.0f; 
			cutinfoTemp.m_nPos = -1; 
			this->m_CutInfok.m_vecCutk.push_back(cutinfoTemp);
		}
	}
	for (i=0; i+4<n; i+=5)
	{
		cutinfoTemp.m_fCut = (vecTemp[i]+vecTemp[i+1]+vecTemp[i+2]+vecTemp[i+3]+vecTemp[i+4])/5.0;	
		cutinfoTemp.m_fImportant = 0.0f; 
		cutinfoTemp.m_nPos = -1; 
		this->m_CutInfok.m_vecCutk.push_back(cutinfoTemp);
	}
	*/

	for (i=0; i<n-1; i++)
	{
		cutinfoTemp.m_fCut = (vecTemp[i]+vecTemp[i+1])/2.0;	
		cutinfoTemp.m_fImportant = 0.0f; 
		cutinfoTemp.m_nPos = -1; 
		this->m_CutInfok.m_vecCutk.push_back(cutinfoTemp);
	}
	
	return true;
}


//采用K平均聚类，对属性K进行离散化.
bool DataCvt_Discrete::DiscreteByClustering(const unsigned int k)
{
	//判断输入参数K的合法性
	if (k<0 || k>=this->m_iAttNum)
	{
		return false;
	}	

	//计算属性K的候选断点.
	this->GetCandidateCut(k);

	//计算属性K上的断点的重要性	
	this->GetCutImportant_ByAlgebra(k);	

	//对左半部分进行聚类.
	this->ClusteringPart(k,0,this->m_CutInfok.m_nMid);

	//对右半部分进行聚类
	this->ClusteringPart(k,this->m_CutInfok.m_nMid+1,this->m_CutInfok.m_vecCutk.size()-1);

	return true;

}



//对于向量this->m_CutInfok.m_vecCutk中l-h之间的断点进行聚类.
bool DataCvt_Discrete::ClusteringPart(const int k,const int l,const int h)
{
	//检验输入参数的合法性.
	if (l<0 || l>=this->m_CutInfok.m_vecCutk.size())
	{
		return false;
	}
	if (h<0 || h>=this->m_CutInfok.m_vecCutk.size())
	{
		return false;
	}
	if(l >= h)
	{
		return false;
	}

	//计算阈值
	const int low = l;
	const int high = h;
	double fEx = 0.0f;     //均值
	const int n = high-low+1;
	int i;
	for (i=low; i<=high; i++)
	{
		fEx += this->m_CutInfok.m_vecCutk[i].m_fImportant;
	}
	fEx /= n;
	double fE = 0.0f;      //阈值
	for (i=low; i<=high; i++)
	{
		fE +=  pow((this->m_CutInfok.m_vecCutk[i].m_fImportant-fEx),2);	
	}
	fE = sqrt(fE)/n;//sqrt(fE/n)/sqrt(n);

	//初始化循环变量,进行聚类.
	double fV = fE+1;
	int nK = 1;	
	vector<CENTERNODE> vecCenter;        //存储聚类中心.
	CENTERNODE         centerTemp; 	     //临时变量	
	int j;
	double fMinDist;
	double fe1 = 0.0f;
	int d;    
	int m;
	int nKk;

	//选取合适的聚类类别数目.
	while(fV>fE)
	{
		//初始化聚类类别
		//AfxMessageBox("here");
		vecCenter.clear();
		for (j=low; j<=high; j++)
		{
			this->m_CutInfok.m_vecCutk[j].m_nPos = -1;
		}

		//选取初始的K个聚类中心点.
		d = (high-low+1)/nK;
		for (j=1; j<=nK; j++)
		{
			centerTemp.m_fvalue = this->m_CutInfok.m_vecCutk[low+j*d-1].m_fImportant;
			centerTemp.m_num = 1;
			vecCenter.push_back(centerTemp); 
			this->m_CutInfok.m_vecCutk[low+j*d-1].m_nPos = j-1;               
		}		

		//循环达到平衡状态.
		fe1 = 0.0f;		
		while(fe1 != fV)
		{
			fe1 = fV;
			nKk = 0;

			//计算各点到聚类中心的距离,并把该点归入距离最近的类中。
			for (j=low; j<=high; j++)
			{
				fMinDist = -1;				
				this->m_CutInfok.m_vecCutk[j].m_nPos = -1;
				for (m=0; m<nK; m++)
					if (vecCenter[m].m_num>0)
					{
						if ((-1==fMinDist) 
							||(-1!=fMinDist
							&&fabs(this->m_CutInfok.m_vecCutk[j].m_fImportant-vecCenter[m].m_fvalue)<fMinDist))
						{
							fMinDist = fabs(this->m_CutInfok.m_vecCutk[j].m_fImportant-vecCenter[m].m_fvalue);
							this->m_CutInfok.m_vecCutk[j].m_nPos = m;	 
						}

					}				
			}

			//更新聚类中心.
			for (j=0; j<nK; j++)
			{
				vecCenter[j].m_fvalue  = 0.0f;
				vecCenter[j].m_num     =0;
				vecCenter[j].m_fSi     =0.0f;
			}
			for (j=low; j<=high; j++)
			{
				//每个断点的重要性都加到所属类中心的m_fvalue
				vecCenter[this->m_CutInfok.m_vecCutk[j].m_nPos].m_fvalue += this->m_CutInfok.m_vecCutk[j].m_fImportant;
				vecCenter[this->m_CutInfok.m_vecCutk[j].m_nPos].m_num++;					
			}
			for (j=0; j<nK; j++)
			{
				//assert(0 != vecCenter[j].m_num);
				if (0 != vecCenter[j].m_num)
				{
					vecCenter[j].m_fvalue /= vecCenter[j].m_num;
					nKk++;
				}
			}

			//计算每个类别的标准差,更新V.
			for (j=low; j<=high; j++)
			{
				vecCenter[this->m_CutInfok.m_vecCutk[j].m_nPos].m_fSi += pow((this->m_CutInfok.m_vecCutk[j].m_fImportant-vecCenter[this->m_CutInfok.m_vecCutk[j].m_nPos].m_fvalue),2);
			}
			fV = 0.0f; 
			for (j=0; j<nK; j++)
			{
				if (vecCenter[j].m_num>0)
				{
					//此处m_fSi才为标准差
					vecCenter[j].m_fSi = sqrt(vecCenter[j].m_fSi/vecCenter[j].m_num);
					fV += vecCenter[j].m_fSi; 
				}
			}
			fV /= nKk; 			
		}   //结束while(fe1 != fV)
		nK += 1;	
	}      //结束while(fV>fE)

	//从每个类别中选取最大的断点作为候选的断点集	
	vector<CMAXMINCUT> vecMaxMin;	
	vecMaxMin.reserve(nK-1);
	CMAXMINCUT  MaxMinTemp;
	MaxMinTemp.m_fMaxCut = this->m_CutInfok.m_vecCutk[0].m_fCut-1;
	MaxMinTemp.m_fImportant = -0.1f;
	for (i=0; i<nK-1; i++)
	{
		vecMaxMin.push_back(MaxMinTemp);	  
	}
	for (i=low; i<= high; i++)
	{
		if (this->m_CutInfok.m_vecCutk[i].m_fCut
	>vecMaxMin[this->m_CutInfok.m_vecCutk[i].m_nPos].m_fMaxCut)
		{
			vecMaxMin[this->m_CutInfok.m_vecCutk[i].m_nPos].m_fMaxCut    = this->m_CutInfok.m_vecCutk[i].m_fCut;	
			vecMaxMin[this->m_CutInfok.m_vecCutk[i].m_nPos].m_fImportant = this->m_CutInfok.m_vecCutk[i].m_fImportant;
		}	
	}
	//保存vecMaxMin到this->m_vecfCutk
	for (i=0; i<nK-1; i++)
	{
		if ((this->m_CutInfok.m_vecCutk[0].m_fCut-1) != vecMaxMin[i].m_fMaxCut)
		{
			this->m_vecfCutk.push_back(vecMaxMin[i].m_fMaxCut);			
		}		
	}    
	return true;	
}


//根据选取的断点转化信息表.
void DataCvt_Discrete::TransTable()
{
	int i,j,k;
	int n;
	for(i=0; i<this->m_iAttNum; i++)
	{
		n = this->m_vecSelectedCut[i].size();
		for (j=0; j<this->m_iRecordNum; j++)
		{
			for (k=0; k<n; k++)
			{
				if (this->m_vecSelectedCut[i][k]>this->m_ppfInfoTable[j][i])
				{
					this->m_ppfInfoTable[j][i] = k;
					break;
				}				
			}
			if(k == n)
			{
				this->m_ppfInfoTable[j][i] = n;			
			}
		}
	}

}


//利用断点可区分的属性数目计算属性K上的断点的重要性
bool DataCvt_Discrete::GetCutImportant_ByAlgebra(const unsigned int k)
{
	//判断输入参数K的合法性
	if (k<0 || k>=this->m_iAttNum)
	{
		return false;
	}

	//分别计算属性K上每个断点能区分的记录个数
	int i;
	int n = this->m_CutInfok.m_vecCutk.size();
	CUTNODE cutnodeTemp;
	for (i=0; i<n; i++)
	{		
		cutnodeTemp.m_fValue = this->m_CutInfok.m_vecCutk[i].m_fCut;
		cutnodeTemp.m_nAttr  = k;
		this->m_CutInfok.m_vecCutk[i].m_fImportant = (double)this->GetCutImpotant(cutnodeTemp);
	}  

	//查找属性K的断点中最重要的断点.
	int nTemp = this->m_CutInfok.m_vecCutk[0].m_fImportant;
	this->m_CutInfok.m_nMid = 0;
	for (i=1; i<n; i++)
	{
		if (this->m_CutInfok.m_vecCutk[i].m_fImportant>nTemp)
		{
			nTemp = this->m_CutInfok.m_vecCutk[i].m_fImportant;
			this->m_CutInfok.m_nMid = i; //记录的是最重要断点的序号
		}
	}     
	//对属性K的断点重要性进行归一化.
	for (i=0; i<n; i++)
	{
		if (nTemp==0)
		{
			this->m_CutInfok.m_vecCutk[i].m_fImportant = 0;
		}
		else
		{
			this->m_CutInfok.m_vecCutk[i].m_fImportant /= nTemp;	
		}	  
	}
	return true;	
}


//计算某个断点的重要性，使用之前必须对断点按照决策属性排序
int DataCvt_Discrete::GetCutImpotant(const CUTNODE& CutNode)
{
	int fCutImportant = 0;	
	int n = this->m_vecL.size();
	int i,j;
	int iCol = CutNode.m_nAttr;
	double fValue = CutNode.m_fValue;
	int nLessj,nLargej,nlessALL,nLargeAll;
	vector<int>  vecLess;vector<int>  vecLarge;

	//在每个等价类上进行计算
	double npre;
	int m;
	int nL = 0; 
	for (i=0; i<n; i++)
	{	
		npre = this->m_ppfInfoTable[this->m_vecL[i][0]][this->m_iAttNum];
		nLessj  = 0; 
		nLargej = 0;
		if (this->m_ppfInfoTable[this->m_vecL[i][0]][iCol]<fValue)
		{
			nLessj = 1;
		}
		else
		{
			nLargej = 1;
		}
		assert(nLessj==1 || 1==nLargej);
		nlessALL  = 0;
		nLargeAll = 0;
		vecLess.clear();
		vecLarge.clear();

		//扫描按决策属性排序后的记录集，计算断点i能区分的对象个数
		m = this->m_vecL[i].size();
		for (j=1; j<m; j++)
		{
			int aa = m_vecL[i][j];
			int bb = this->m_ppfInfoTable[this->m_vecL[i][j]][this->m_iAttNum];
			if (this->m_ppfInfoTable[this->m_vecL[i][j]][this->m_iAttNum] != npre)
			{
				vecLess.push_back(nLessj);			
				vecLarge.push_back(nLargej);
				nlessALL  += nLessj;
				nLargeAll += nLargej;		
				npre = this->m_ppfInfoTable[this->m_vecL[i][j]][this->m_iAttNum]; 			
				nLessj  = 0;nLargej = 0;
			}
			if (this->m_ppfInfoTable[this->m_vecL[i][j]][iCol] <fValue)
			{
				nLessj++;				
			}
			else
			{
				nLargej++;				   
			}
		} //结束for(j).

		//保存最后一个决策类的nLessj，nLargej.
		vecLess.push_back(nLessj);
		vecLarge.push_back(nLargej);
		nlessALL  += nLessj;
		nLargeAll += nLargej;

		//根据nlessALL，nLargeAll，向量vecLess，vecLarge代入计算公式计算断点i能区分的对象个数.
		m = vecLess.size();	
		nL = 0;  
		for (j=0; j<m; j++)
		{
			nL += (double)(vecLess[j]*vecLarge[j]);			
		}
		if(nlessALL>10000&&nLargeAll>10000)
		{
			nlessALL=10000;nLargeAll=10000;
		}
		fCutImportant += nlessALL*nLargeAll-nL;			
	}//结束for(i);

	//返回该断点的属性重要性.
	return fCutImportant;	
}


//并行筛选断点
void DataCvt_Discrete::FilterCutByParallel()
{
	const int tag1=0,tag2=1; //消息类型.
	MPI_Status stat;
	double startwtime, endwtime;
	CUTNODE cutnodeTemp;
	ReadInfoTable();	

	//清空this->m_vecL
	for (int i=0; i<this->m_vecL.size(); i++)
	{
		this->m_vecL[i].clear();
	}
	this->m_vecL.clear();
	//初始化等价类，并根据决策属性排序
	vector<int> vecZ;
	vecZ.reserve(this->m_iRecordNum); 	
	for (int i=0; i<this->m_iRecordNum; i++)
	{
		vecZ.push_back(i);
	}
	this->m_vecL.push_back(vecZ);
	vecZ.clear();

	//(A)使用K均值聚类进行初步筛选
	if(c_IsUserKMeansFirst)
	{
		for (int i=0; i<this->m_iAttNum; i++)
		{
			this->m_vecfCutk.clear();
			//分别对每一个属性采用K均值聚类筛选
			this->DiscreteByClustering(i);

			//为this->m_vecCutSlect开辟空间
			vector<CUTNODE> vecCutSlectOfOneCol;
			m_vecCutSlect.push_back(vecCutSlectOfOneCol);
			for (int j=0; j<this->m_vecfCutk.size(); j++)
			{
				cutnodeTemp.m_fValue = this->m_vecfCutk[j];
				cutnodeTemp.m_nAttr  = i;
				this->m_vecCutSlect[i].push_back(cutnodeTemp);
			}

			//为this->m_vecSelectedCut开辟空间
			vector<double> ptemp;
			this->m_vecSelectedCut.push_back(ptemp);
		}
	}
	else//(B)不筛选直接使用初始断点集
	{
		for (int i=0 ;i<this->m_iAttNum; i++)
		{
			this->GetCandidateCut(i);
			//为this->m_vecCutSlect开辟空间
			vector<CUTNODE> vecCutSlectOfOneCol;
			m_vecCutSlect.push_back(vecCutSlectOfOneCol);
			for (int j=0; j<this->m_CutInfok.m_vecCutk.size(); j++)
			{
				cutnodeTemp.m_fValue = this->m_CutInfok.m_vecCutk[j].m_fCut;
				cutnodeTemp.m_nAttr  = i;
				this->m_vecCutSlect[i].push_back(cutnodeTemp);	
			}
			//为this->m_vecSelectedCut开辟空间
			vector<double> ptemp;
			this->m_vecSelectedCut.push_back(ptemp);  
		}
	}


	int nMax = -1;
	int nTemp;
	vector<int> vecless;vector<int> veclarge; 
	int nFlag = 1;
	double fDesi;
	int nSelect = -1;

	MPI_Barrier(MPI_COMM_WORLD);
	startwtime = MPI_Wtime(); //计算开始计时

	double pstarttime1 = 0;double pendtime1 = 0;double caculate_p = 0;

	for(int pColIndex=0;pColIndex<this->m_iAttNum;pColIndex++)
	{
		int nLeftNum = this->m_vecCutSlect[pColIndex].size();
	    int nBlock;
		//筛选断点直到所有等价类的决策属性相同，得到结果断点
		while (1)
		{
			pstarttime1 = MPI_Wtime();

			//所有断点被选取则终止
			if (0 == nLeftNum)
			{
				break;
			}
			nFlag = 0; nMax = -1; nSelect = -1; 

			//每个进程依次读取计算，不断重复
			for(int i=m_rankid;i<nLeftNum;i+=m_Numproc)
			{
				nTemp = this->GetCutImpotant(this->m_vecCutSlect[pColIndex][i]);
				if (nTemp>nMax)
				{
					cutnodeTemp = this->m_vecCutSlect[pColIndex][i]; 
					nSelect = i;
					nMax = nTemp;				  
				}  
			}

			pendtime1 = MPI_Wtime();
			caculate_p = caculate_p+pendtime1-pstarttime1;

			if(m_rankid!=0)//从进程向主进程发送选取的断点
			{
				double sendata[2];			
				sendata[0] = (double)nSelect;
				sendata[1] = nMax;
				MPI_Send(&sendata,2,MPI_DOUBLE,0,tag1,MPI_COMM_WORLD);//向主进程发送数据;		   
			}	
			else//主进程接收从进程结果，然后进行得到最大断点编号与重要性
			{
				for (int i=1; i<m_Numproc; i++)
				{
					double recv[2];
					MPI_Recv (&recv,2,MPI_DOUBLE,i,tag1,MPI_COMM_WORLD,&stat);
					if (recv[1]>nMax)
					{
						nSelect = (int)recv[0];
						nMax = recv[1];
					}				 
				} 
				//将选择的断点加入结果断点集this->m_vecSelectedCut
				this->m_vecSelectedCut[this->m_vecCutSlect[pColIndex][nSelect].m_nAttr].push_back(this->m_vecCutSlect[pColIndex][nSelect].m_fValue);			
			}

			//广播归约结果
			MPI_Bcast(&nSelect, 1, MPI_INT, 0, MPI_COMM_WORLD);	

			//从进程更新计算结果
			if (m_rankid != 0)
			{
				this->m_vecSelectedCut[this->m_vecCutSlect[pColIndex][nSelect].m_nAttr].push_back(this->m_vecCutSlect[pColIndex][nSelect].m_fValue);
			}

			nLeftNum--;
			cutnodeTemp = this->m_vecCutSlect[pColIndex][nSelect];  


			//更新等价类，并判断所有等价类的决策属性是否已经相等.	
			if(m_vecSelectedCut[pColIndex].size()<1)//只取一个断点，及只进行0、1分类
			{
				int pSize_m_vecL = this->m_vecL.size();
				for (int i=0; i<pSize_m_vecL; i++)
				{
					vecless.clear();
					veclarge.clear();
					int pSize_m_vecLi = this->m_vecL[i].size();
					for (int j=0; j<pSize_m_vecLi; j++)
					{
						if (this->m_ppfInfoTable[this->m_vecL[i][j]][cutnodeTemp.m_nAttr]>cutnodeTemp.m_fValue)
						{
							veclarge.push_back(this->m_vecL[i][j]);
						}
						else
						{
							vecless.push_back(this->m_vecL[i][j]);
						}
					}
					assert((veclarge.size()+vecless.size()) == pSize_m_vecLi);

					//更新等价类
					if (veclarge.size()>0 && vecless.size()>0)
					{
						this->m_vecL[i].clear();
						int pp = vecless.size();
						for (int j=0; j<pp; j++)
						{
							this->m_vecL[i].push_back(vecless[j]);
						}
						sort(this->m_vecL[i].begin(),this->m_vecL[i].end(),SOPERATION(this->m_ppfInfoTable,this->m_iAttNum)); 
						this->m_vecL.push_back(veclarge);			
						sort(this->m_vecL[this->m_vecL.size()-1].begin(),this->m_vecL[this->m_vecL.size()-1].end(),SOPERATION(this->m_ppfInfoTable,this->m_iAttNum)); 
					}            

					assert(vecless.size()>0 || veclarge.size()>0);

					//判断vecless的决策属性是否都相等
					if (vecless.size()>0)
					{
						int pp = vecless.size();int pSameDecCount = 0;
						fDesi = this->m_ppfInfoTable[vecless[0]][this->m_iAttNum];				
						for (int j=1; j<pp; j++)
						{		
							if (this->m_ppfInfoTable[vecless[j]][this->m_iAttNum] != fDesi)
							{
								nFlag = 1;
								break;
							}					
						}//for (int j=1; j<pp; j++)
					}//if (vecless.size()>0)

					//判断veclarge的决策属性是否都相等 
					if (veclarge.size()>0)
					{
						int pp = veclarge.size();int pSameDecCount = 0;
						fDesi = this->m_ppfInfoTable[veclarge[0]][this->m_iAttNum];
						for (int j=1; j<pp; j++)
						{				
							if (this->m_ppfInfoTable[veclarge[j]][this->m_iAttNum] != fDesi)
							{
								nFlag = 1;
								break;
							}
						}//for (int j=1; j<pp; j++)
					}//if (veclarge.size()>0)

				}  //结束for(int i=0; i<pSize_m_vecL; i++)

			}//if(m_vecSelectedCut.size()<1)


			//判断是否满足结束条件
			if (0 == nFlag)
			{	
				break;
			}

			for (int i=nSelect+1; i<this->m_vecCutSlect[pColIndex].size(); i++)//从Cut_Select中删除选取的断点;
			{
				this->m_vecCutSlect[pColIndex][i-1] = this->m_vecCutSlect[pColIndex][i];
			}
			this->m_vecCutSlect[pColIndex].pop_back(); 

		}  // 结束while(1).
	}
	

	//对筛选的断点排序
	for (int i=0; i<this->m_iAttNum; i++)
	{
		sort(this->m_vecSelectedCut[i].begin(),this->m_vecSelectedCut[i].end());
	}
	endwtime = MPI_Wtime();
	Mpi_Caculate_Time = endwtime-startwtime;

	//cout<<"Mpi_Caculate_Timerankid "<<m_rankid<<" : "<<Mpi_Caculate_Time<<endl;
	//cout<<"caculate_p rankid "<<m_rankid<<" : "<<caculate_p<<endl;

	double tmp=caculate_p;
	MPI_Reduce(&caculate_p,&tmp,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
	Mpi_Caculate_Time = tmp/m_Numproc;


	SaveDiscreteResult();	 


}

