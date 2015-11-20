#include "DataCvt_Nml.h"
using namespace std;


int DataCvt_Nml::main_nml(int argc, char* argv[])
{
	MPI_Init(NULL,NULL);
	MPI_Comm_rank(MPI_COMM_WORLD,&rankid);
	MPI_Comm_size(MPI_COMM_WORLD,&numproc);

	Mpi_Start_Moment = MPI_Wtime();
	
	if (argc>1)
	{
		for (int i=1;i<argc;i++)
		{
			//connection string
			m_DatasourceConStr = "PG:host='192.168.5.131' port='5437'  dbname='pdatabase' user='postgres' password='postgres'";
			//path
			if (!strcmp(argv[i],"-p"))
			{
				m_PathOrTableName = argv[i+1];
			}
			//band or col to caculate
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
			//out file
			if (!strcmp(argv[i],"-o"))
			{
				m_ResultPathOrTableName = argv[i+1];
			}
		}
	}

	pFileExtensionName = GetFileExtensionName(m_PathOrTableName);

	if((pFileExtensionName=="tif")||(pFileExtensionName=="img"))
	{
		CalAsImage();
	}
	else
	{
		if (c_CaculateCols.size()==0)
		{
			cout<<"no input columns..."<<endl;
			return 0;
		}
		if(pFileExtensionName=="shp")
			m_DatasourceConStr="";
		CalAsShp();
	}

	Mpi_Finish_Moment = MPI_Wtime();

	if (rankid==0)
	{	
		ShowExcuteInfor();
		if(m_Debug)
		{
			WriteExcuteLog();
		}
	}

	MPI_Finalize();
	if(m_Debug)
		cout<<"Process "<<rankid<<" has finished..."<<endl;

	return 0;
}

void DataCvt_Nml::ShowExcuteInfor()
{
	if(m_Debug)
	{
		cout<<"[DEBUG] [OPTIONS] input file directory: "<<m_PathOrTableName<<endl;
		cout<<"[DEBUG] [OPTIONS] output file directory: "<<m_ResultPathOrTableName<<endl;
		if (pFileExtensionName=="tif")
		{
			cout<<"[DEBUG] [OPTIONS] input file size is : "<<m_RowNum<<"*"<<m_ColNum<<"*"<<m_BandCount<<endl;
		}
		else
		{
			cout<<"[DEBUG] [OPTIONS] input file size is : "<<m_RowNum<<"*"<<c_CaculateCols.size()<<endl;
		}
	}
	cout<<"[DEBUG]  time consuming:"<<endl;
	cout<<"[DEBUG][TIMESPAN][IO] "<<Mpi_Finish_Moment-Mpi_Start_Moment-Mpi_Caculate_Time<<endl;
	cout<<"[DEBUG][TIMESPAN][COMPUTING] "<<Mpi_Caculate_Time<<endl;
	cout<<"[DEBUG][TIMESPAN][TOTAL] "<<Mpi_Finish_Moment-Mpi_Start_Moment<<endl;
}


void DataCvt_Nml::WriteExcuteLog()
{
	string path_log = "log_DataCvt_Nml.txt";
	path_log = GetFullFilePath(path_log.c_str());
	fstream pSaveFile;
	pSaveFile.open(path_log.c_str(),ios::out|ios::app);
	pSaveFile<<"=======================ProcessNum:"<<numproc<<"======================="<<"\n";
	pSaveFile<<"[DEBUG] [TIMESPAN] [IO] "<<Mpi_Finish_Moment-Mpi_Start_Moment-Mpi_Caculate_Time<<"\n";
	pSaveFile<<"[DEBUG] [TIMESPAN] [COMPUTING] "<<Mpi_Caculate_Time<<"\n";
	pSaveFile<<"[DEBUG] [TIMESPAN] [TOTAL] "<<Mpi_Finish_Moment-Mpi_Start_Moment<<"\n";
	pSaveFile.close();
}


void DataCvt_Nml::CalAsImage()
{
	GDALAllRegister();

	const char *pszFormat = "GTiff";
	if(!strcmp(GetFileExtensionName(m_PathOrTableName).c_str(),"img"))
		pszFormat = "HFA";
	else
		pszFormat = "GTiff";

	GDALDriver* podriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

	if(!strcmp(GetFileExtensionName(m_ResultPathOrTableName).c_str(),"img"))
		pszFormat = "HFA";
	else
		pszFormat = "GTiff";

	GDALDriver* podriver_result = GetGDALDriverManager()->GetDriverByName(pszFormat);

	GDALDataset* dataset_result;GDALDataset* dataset_confidence;

	GDALDataset* dataset_gdal = (GDALDataset *) GDALOpen( m_PathOrTableName.c_str(), GA_ReadOnly );
	if (dataset_gdal == NULL )
	{
		cout<<"the file does not exist..."<<endl;
		exit(0);
	}

	m_BandCount = dataset_gdal->GetRasterCount();
	m_ColNum = dataset_gdal->GetRasterXSize();
	m_RowNum = dataset_gdal->GetRasterYSize();
	int width = m_ColNum;int height = m_RowNum;
	const char* pProjectionRef;double pasfGeoTransform[6];
	pProjectionRef = dataset_gdal->GetProjectionRef();
	dataset_gdal->GetGeoTransform(pasfGeoTransform);
	double p_NoDataValue = dataset_gdal->GetRasterBand(1)->GetNoDataValue();

	if(rankid==0)
	{
		//to make sure the create file's block is 1 row
		char **papszMetadata = NULL;
		papszMetadata = CSLSetNameValue(papszMetadata, "BLOCKYSIZE", "1" );
		dataset_result = podriver_result->Create(m_ResultPathOrTableName.c_str(),width,height,1,GDT_Float64,papszMetadata);
		dataset_result->SetProjection(pProjectionRef);
		dataset_result->SetGeoTransform(pasfGeoTransform);
		dataset_result->GetRasterBand(1)->SetNoDataValue(p_NoDataValue);
		GDALClose(dataset_result);
	}

	//use all raster bands
	if(c_CaculateCols.size()==0)
	{
		for(int i=1;i<=m_BandCount;i++)
		{
			c_CaculateCols.push_back(i);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	dataset_result = (GDALDataset *) GDALOpen( m_ResultPathOrTableName.c_str(), GA_Update );
	MPI_Barrier(MPI_COMM_WORLD);

	//caculate and update by row
	//assign block to each process
	int nRow_start,nRow_end;
	int pRowsNum=0; int pRemainder = height%numproc;
	if(rankid<pRemainder)
	{
		pRowsNum = height/numproc+1;
		nRow_start = rankid*pRowsNum;
		nRow_end = nRow_start+pRowsNum-1;
	}
	else
	{
		pRowsNum = height/numproc;
		nRow_start = pRemainder*(pRowsNum+1)+(rankid-pRemainder)*pRowsNum;
		nRow_end = nRow_start+pRowsNum-1;
	}

	if(m_Debug)
		cout<<"Process "<<rankid<<" will caculate from row "<<nRow_start<<" to row "<<nRow_end<<endl;

	double ** p_DataBuffer = new double*[c_CaculateCols.size()];
	double** p_ResultDataBuffer = new double*[c_CaculateCols.size()];

	double * p_SumOfEachRow = new double[c_CaculateCols.size()]();
	double * p_XSquareSumOfEachRow = new double[c_CaculateCols.size()]();
	double * pSum = new double[c_CaculateCols.size()]();
	double * pXSquareSum = new double[c_CaculateCols.size()]();
	double * pSigma = new double[c_CaculateCols.size()]();
	//the num of nodata cell
	double pNodataNum = 0;

	for(int i=0; i< c_CaculateCols.size(); i++ )
	{
		p_DataBuffer[i] = new double[width]();
		p_ResultDataBuffer[i] = new double[width]();
		//set 0
		p_SumOfEachRow[i]=0;p_XSquareSumOfEachRow[i]=0;
		pSum[i]=0;pXSquareSum[i]=0;
	}

	//double ptmptime_start = MPI_Wtime();

	//caculate mean and x^2's mean for each row
	for(int iRow = nRow_start; iRow <= nRow_end; iRow++ )
	{
		double pptmptime_start = MPI_Wtime();
		for(int i=0;i<c_CaculateCols.size();i++)
		{
			GDALRasterBand * poband=dataset_gdal->GetRasterBand(c_CaculateCols[i]);
			poband->RasterIO(GF_Read,0,iRow,width,1,p_DataBuffer[i],width,1,GDT_Float64,0,0);
			//set 0
			p_SumOfEachRow[i]=0;p_XSquareSumOfEachRow[i]=0;
		}
		double pptmptime_end = MPI_Wtime();
		//m_GdalIOInCal_Time=m_GdalIOInCal_Time+pptmptime_end-pptmptime_start;
		for(  int t = 0; t < width; t++  )
		{
			for(int p=0;p<c_CaculateCols.size();p++)
			{
				if(p_DataBuffer[p][t]!=p_NoDataValue)
				{
					p_SumOfEachRow[p]+=p_DataBuffer[p][t];
					p_XSquareSumOfEachRow[p]+=pow(p_DataBuffer[p][t],2);
				}
				else
				{
					double tmp_nodata=0;
					tmp_nodata+=p_DataBuffer[p][t];tmp_nodata+=pow(p_DataBuffer[p][t],2);
					pNodataNum++;
				}
			}
		}			
		for(int p=0;p<c_CaculateCols.size();p++)
		{
			pSum[p]+=p_SumOfEachRow[p];
			pXSquareSum[p]+=p_XSquareSumOfEachRow[p];
		}			
	}

	double * pMean = new double[c_CaculateCols.size()]();
	double * pXSquareMean = new double[c_CaculateCols.size()]();

	for(int p=0;p<c_CaculateCols.size();p++)	
	{
		double rowSumAll=0;
		MPI_Allreduce(&pSum[p],&rowSumAll,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
		pMean[p] = rowSumAll/(width*height-pNodataNum);
		MPI_Allreduce(&pXSquareSum[p],&rowSumAll,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
		pXSquareMean[p] = rowSumAll/(width*height-pNodataNum);
		//sigma=sqrt(E(X^2)-E(X)*E(X))
		pSigma[p] = sqrt(pXSquareMean[p]-pMean[p]*pMean[p]);
	}
	double ptmptime_start = MPI_Wtime();
	//caculate (x-u)/sigma
	for(int iRow = nRow_start; iRow <= nRow_end; iRow++ )
	{
		double pptmptime_start = MPI_Wtime();
		for(int i=0;i<c_CaculateCols.size();i++)
		{
			GDALRasterBand * poband=dataset_gdal->GetRasterBand(c_CaculateCols[i]);
			poband->RasterIO(GF_Read,0,iRow,width,1,p_DataBuffer[i],width,1,GDT_Float64,0,0);
		}
		double pptmptime_end = MPI_Wtime();
		m_GdalIOInCal_Time=m_GdalIOInCal_Time+pptmptime_end-pptmptime_start;
		for(  int t = 0; t < width; t++  )
		{
			for(int p=0;p<c_CaculateCols.size();p++)
			{
				if(p_DataBuffer[p][t]==p_NoDataValue)
				{
					//first caculate nodata
					p_ResultDataBuffer[p][t] = (p_DataBuffer[p][t]-pMean[p])/pSigma[p];
					p_ResultDataBuffer[p][t]  = p_NoDataValue;
				}
				else
				{
					p_ResultDataBuffer[p][t] = (p_DataBuffer[p][t]-pMean[p])/pSigma[p];
				}
			}
		}
		//each process write the result
		double pstart = MPI_Wtime();
		for(int p=0;p<c_CaculateCols.size();p++)
		{
			dataset_result->GetRasterBand(c_CaculateCols[p])->RasterIO(GF_Write,0,iRow,width,1,p_ResultDataBuffer[p],width,1,GDT_Float64,0,0);
		}
		double pend = MPI_Wtime();
		m_GdalIOInCal_Time = m_GdalIOInCal_Time+pend-pstart;
	}
	double ptmptime_endall = MPI_Wtime();
	Mpi_Caculate_Time = ptmptime_endall-ptmptime_start-m_GdalIOInCal_Time;

	//cout<<"Mpi_Caculate_Time of rankid "<<rankid<<" :"<<Mpi_Caculate_Time<<endl;
	
	double tmp=Mpi_Caculate_Time;
	MPI_Reduce(&Mpi_Caculate_Time,&tmp,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

	//free memory
	for(int i=0; i< c_CaculateCols.size(); i++ )
	{
		delete p_DataBuffer[i];
		delete p_ResultDataBuffer[i];
	}
	delete p_DataBuffer;delete p_ResultDataBuffer;
	delete p_SumOfEachRow;delete p_XSquareSumOfEachRow; delete pSum;delete pXSquareSum;
	delete pMean;delete pXSquareMean;delete pSigma;
	delete dataset_gdal;
	delete dataset_result;
}


void DataCvt_Nml::CalAsShp()
{
	double ptmptime_start = MPI_Wtime();
	OGRRegisterAll();
	//create a new shp to caculate
	CBaseOperate pbaseOperate;
	if(rankid==0)
	{
		pbaseOperate.CreateCopyShp(m_DatasourceConStr.c_str(),m_PathOrTableName.c_str(),m_ResultPathOrTableName.c_str());
	}
	MPI_Barrier(MPI_COMM_WORLD);

	string pLayerName = m_ResultPathOrTableName;
	if(strcmp(m_DatasourceConStr.c_str(),"")==0)//input is shp file
	{
		m_DatasourceConStr = m_ResultPathOrTableName;
		string pShpPath = m_ResultPathOrTableName;
		string p_LayerName = GetFileNameOnly(pShpPath.c_str());
		int pindex = p_LayerName.find_first_of('.');
		pLayerName = p_LayerName.erase(pindex,p_LayerName.size()-1);
	}

	OGRDataSource* poDS = OGRSFDriverRegistrar::Open(m_DatasourceConStr.c_str(),TRUE); 
	OGRLayer* poLayer = poDS->GetLayerByName(pLayerName.c_str());
	OGRFeature * pFeature =poLayer->GetNextFeature();

	m_RowNum = poLayer->GetFeatureCount();
	//if -c argv is null, all field will be used
	m_ColNum= pFeature->GetFieldCount();
	if(c_CaculateCols.size()==0)
	{
		if(rankid==0)
			cout<<"!!!!!!!  No input columns ids, all fields in the input file will be used..."<<endl<<endl;
		for(int i=0;i<m_ColNum;i++)
		{
			c_CaculateCols.push_back(i);
		}
	}

	double ptmptime_end = MPI_Wtime();
	m_GdalIOInCal_Time=m_GdalIOInCal_Time+ptmptime_end-ptmptime_start;

	//assign number to each process
	int pnum_start,pnum_end;
	int pMyProcessNum=0; int pRemainder=m_RowNum%numproc;
	if(rankid<pRemainder)
	{
		pMyProcessNum = m_RowNum/numproc+1;
		pnum_start = rankid*pMyProcessNum;
		pnum_end = pnum_start+pMyProcessNum-1;
	}
	else
	{
		pMyProcessNum = m_RowNum/numproc;
		pnum_start = pRemainder*(pMyProcessNum+1)+(rankid-pRemainder)*pMyProcessNum;
		pnum_end = pnum_start+pMyProcessNum-1;
	}

	//postgis: fid begins from 1, not 0
	string pwhere="";
	if(strcmp(m_DatasourceConStr.substr(0,3).c_str(),"PG:")==0)//input is postgis
	{
		pwhere = "gid>="+toString(pnum_start+1)+" and gid<="+toString(pnum_end+1);
	}
	else//shpfile: fid begins from 0, not 1
	{
		pwhere = "fid>="+toString(pnum_start)+" and fid<="+toString(pnum_end);
	}


	//caculate mean and x^2's mean for each col
	double * pMean = new double[c_CaculateCols.size()]();
	double * pXSquareMean = new double[c_CaculateCols.size()]();
	double * pSigma = new double[c_CaculateCols.size()]();
	poLayer->SetAttributeFilter(pwhere.c_str());
	pFeature = poLayer->GetNextFeature();
	while(pFeature!=NULL)
	{		
		for(int i=0;i<c_CaculateCols.size();i++)
		{			
			double tmp =pFeature->GetFieldAsDouble(c_CaculateCols[i]);
			pMean[i]+=tmp;
			pXSquareMean[i]+=tmp*tmp;
		}		
		pFeature = poLayer->GetNextFeature();
	}
	for(int p=0;p<c_CaculateCols.size();p++)	
	{
		double sumAll=0;
		MPI_Allreduce(&pMean[p],&sumAll,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
		pMean[p] = sumAll/m_RowNum;
		MPI_Allreduce(&pXSquareMean[p],&sumAll,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
		pXSquareMean[p] = sumAll/m_RowNum;
		//sigma=sqrt(E(X^2)-E(X)*E(X))
		pSigma[p] = sqrt(pXSquareMean[p]-pMean[p]*pMean[p]);
	}

	//(x-u)/sigma
	poLayer->ResetReading();
	poLayer->SetAttributeFilter(pwhere.c_str());
	pFeature = poLayer->GetNextFeature();
	while(pFeature!=NULL)
	{		
		for(int i=0;i<c_CaculateCols.size();i++)
		{			
			double tmp = (pFeature->GetFieldAsDouble(c_CaculateCols[i])-pMean[i])/pSigma[i];
			double pstart = MPI_Wtime();
			pFeature->SetField(c_CaculateCols[i],tmp);
			poLayer->SetFeature(pFeature);
			double pend = MPI_Wtime();
			m_GdalIOInCal_Time = m_GdalIOInCal_Time+pend-pstart;
		}		
		pFeature = poLayer->GetNextFeature();
	}
	OGRDataSource::DestroyDataSource( poDS );
	delete pMean;delete pXSquareMean;delete pSigma;
	double ptmptime_endall = MPI_Wtime();
	Mpi_Caculate_Time = ptmptime_endall-ptmptime_start-m_GdalIOInCal_Time;

	double tmp=Mpi_Caculate_Time;
	MPI_Reduce(&Mpi_Caculate_Time,&tmp,1,MPI_DOUBLE,MPI_MIN,0,MPI_COMM_WORLD);
	Mpi_Caculate_Time = tmp;
}
