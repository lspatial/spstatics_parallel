#include "BayesPar.h"


void ShowProbabilityOfEachNode(const CBNet* pBnet)
{
	int nFactors = pBnet->GetNumberOfFactors();
	CFactor *pFactor;
	for(int f = 0; f < nFactors; f++ )
	{
		pFactor = pBnet->GetFactor(f);
		if(pFactor->GetDistributionType()==dtGaussian)
		{
			C2DNumericDenseMatrix<float> *matMeanMatrix;C2DNumericDenseMatrix<float> *matCov;

			int dataLengthM,dataLengthC;const float *pMeanData;const float *pCovData;

			matMeanMatrix = static_cast<C2DNumericDenseMatrix<float>*>(pFactor->GetMatrix(matMean));
			matMeanMatrix->GetRawData(&dataLengthM, &pMeanData);
			matCov = static_cast<C2DNumericDenseMatrix<float>*>(pFactor->GetMatrix(matCovariance));
			matCov->GetRawData(&dataLengthC, &pCovData);

			cout<<"[RESULT] Gaussian Distribution Parameter for node "<<f<<endl;
			cout<<"[RESULT] Mean:";
			for(int i=0;i<dataLengthM;i++)
			{
				cout<<" "<<pMeanData[i];
			}
			cout<<endl;
			cout<<"[RESULT] Sigma*Sigma:";
			for(int i=0;i<dataLengthC;i++)
			{
				cout<<" "<<pCovData[i];
			}
		}
		if(pFactor->GetDistributionType()==dtTabular)
		{
			cout<<"[RESULT] probability distribution for node "<<f<<endl;
			int numOfEl;const float *dataCPD;
			const CNumericDenseMatrix<float> *pMatForCPD = static_cast<CNumericDenseMatrix<float> *>(pFactor->GetMatrix(matTable));
			pMatForCPD->GetRawData( &numOfEl, &dataCPD );
			int j;
			cout<<"[RESULT]";
			for( j = 0; j < numOfEl; j++ )
			{
				cout<<" "<<dataCPD[j];
			}
			cout<<endl;
		}
		cout<<endl;
	}

}

void ShowLearnedStructure(CGraph* pgraph)
{
	int pNumNodes = pgraph->GetNumberOfNodes();
	for(int i=0;i<pNumNodes;i++)
	{
		cout<<"Node "<<i<<" : ";
		intVector pNeighbors;neighborTypeVector pNeighborsType;
		pgraph->GetNeighbors(i,&pNeighbors,&pNeighborsType);
		for(int j=0;j<pgraph->GetNumberOfNeighbors(i);j++)
		{
			char pNeighborsType_s;
			if(pNeighborsType[j]==0)
			{
				pNeighborsType_s = 'p';
			}
			else
			{
				pNeighborsType_s = 'c';
			}
			cout<<pNeighbors[j]<<"_<"<<pNeighborsType_s<<"> ";
		}
		cout<<endl;
	}
	cout<<endl;
}

//0 means structure-learning, 1 means parameter-learning, 2 means infering
void ShowExcuteInfor(int pAlgorithm)
{
	if(pAlgorithm==2)
		cout<<"[DEBUG] [OPTIONS] input file directory: "<<m_ResultPathOrTableName<<endl;
	else
		cout<<"[DEBUG] [OPTIONS] input file directory: "<<m_PathOrTableName<<endl;

	cout<<"[DEBUG]  time consuming:"<<endl;
	cout<<"[DEBUG][TIMESPAN][IO]"<<Mpi_Finish_Moment-Mpi_Start_Moment-Mpi_StructureLearning_Time-Mpi_ParameterLearning_Time-Mpi_Inferring_Time<<endl;
	if(pAlgorithm==0)
	{
		cout<<"[DEBUG][TIMESPAN][COMPUTING-StructLearning] : "<<endl;
		cout<<"[DEBUG][TIMESPAN][COMPUTING]"<<Mpi_StructureLearning_Time<<endl;
	}
	if(pAlgorithm==1)
	{
		cout<<"[DEBUG][TIMESPAN][COMPUTING-ParameterLearning]: "<<endl;
		cout<<"[DEBUG][TIMESPAN][COMPUTING]"<<Mpi_ParameterLearning_Time<<endl;
	}
	if(pAlgorithm==2)
	{
		cout<<"[DEBUG][TIMESPAN][COMPUTING-Inferring:] "<<endl;
		cout<<"[DEBUG][TIMESPAN][COMPUTING]"<<Mpi_Inferring_Time<<endl;
		cout<<"[DEBUG][PD][VALUE]"<<m_PD_value<<endl;
	}
	cout<<"[DEBUG][TIMESPAN][TOTAL]"<<Mpi_Finish_Moment-Mpi_Start_Moment<<endl;
	cout<<endl;
}

void WriteExcuteLog(int pAlgorithm)
{
	string path_log = "log_Bayes.txt";
	GetExePath pexepath;
	path_log = pexepath.getExeDir() + path_log;
	fstream pSaveFile;
	pSaveFile.open(path_log.c_str(),ios::out|ios::app);
	pSaveFile<<"=======ProcessNum:"<<numproc<<"======Algorithm:"<<pAlgorithm<<"============"<<"\n";
	pSaveFile<<"[DEBUG] [TIMESPAN] [IO] "<<Mpi_Finish_Moment-Mpi_Start_Moment-Mpi_StructureLearning_Time-Mpi_ParameterLearning_Time-Mpi_Inferring_Time<<"\n";
	if(pAlgorithm==0)
		pSaveFile<<"[DEBUG] [TIMESPAN] [COMPUTING] StructLearning "<<Mpi_StructureLearning_Time<<"\n";
	if(pAlgorithm==1)
		pSaveFile<<"[DEBUG] [TIMESPAN] [COMPUTING] ParameterLearning "<<Mpi_ParameterLearning_Time<<"\n";
	if(pAlgorithm==2)
		pSaveFile<<"[DEBUG] [TIMESPAN] [COMPUTING] Inferring "<<Mpi_Inferring_Time<<"\n";
	pSaveFile<<"[DEBUG] [TIMESPAN] [TOTAL] "<<Mpi_Finish_Moment-Mpi_Start_Moment<<"\n";
	pSaveFile.close();
}

bool SetFixedNodeSize()
{
	c_CaculateCols.clear();
	for(int i=0;i<18;i++)
	{
		c_CaculateCols.push_back(i);
		if(i!=m_IndexOfDecisionNode)
		{
			m_NodesizeArray.push_back(5);
		}
		else
		{
			m_NodesizeArray.push_back(4);
		}
	}
	return true;
}

bool SetNodeSize()
{
	OGRRegisterAll();
	string p_DatasourceConStr = m_DatasourceConStr;
	string pLayerName = m_PathOrTableName;
	if(strcmp(m_DatasourceConStr.c_str(),"")==0)//input is shp file
	{
		p_DatasourceConStr = m_PathOrTableName;
		string pShpPath = m_PathOrTableName;
		string p_LayerName = GetFileNameOnly(pShpPath.c_str());
		int pindex = p_LayerName.find_first_of('.');
		pLayerName = p_LayerName.erase(pindex,p_LayerName.size()-1);
	}
	OGRDataSource* poDS = OGRSFDriverRegistrar::Open(p_DatasourceConStr.c_str(),FALSE);
	OGRLayer* poLayer = poDS->GetLayerByName(pLayerName.c_str());
	OGRFeature * pFeature =poLayer->GetNextFeature();
	//if -c argv is null, all field will be used
	ColNum= pFeature->GetFieldCount();
	if(c_CaculateCols.size()==0)
	{
		if(rankid==0)
			cout<<"!!!!!!!  No input columns ids, all fields in the input file will be used..."<<endl<<endl;
		for(int i=0;i<ColNum;i++)
		{
			c_CaculateCols.push_back(i);
		}
	}
	m_NodesizeArray.resize(c_CaculateCols.size());

	//cout the field names
	if(rankid==0&&m_Debug)
		cout<<"Fields Names: ";
	for(int i=0;i<c_CaculateCols.size();i++)
	{
		const char* pFielsName = pFeature->GetFieldDefnRef(c_CaculateCols[i])->GetNameRef();
		if(rankid==0)
		{
			if(m_Debug)
			{
				if(i<c_CaculateCols.size()-1)
				{
					cout<<pFielsName<<",";
				}
				else
				{
					cout<<pFielsName<<endl;
				}
			}
		}
		string psql;
		psql.append("select max(");
		psql.append(pFielsName);
		psql.append("),min(");
		psql.append(pFielsName);
		psql.append(") from ");
		psql.append(pLayerName);
		OGRLayer* ptmpLayer = poDS->ExecuteSQL(psql.c_str(),NULL,NULL);
		OGRFeature * pfeature = ptmpLayer->GetNextFeature();
		m_NodesizeArray[i] = pfeature->GetFieldAsInteger(0)-pfeature->GetFieldAsInteger(1)+1;
	}
	//cout the node size
	if(rankid==0&&m_Debug)
	{
		cout << "Node size: ";
		for (int i = 0; i < c_CaculateCols.size(); i++)
		{
			if (i < c_CaculateCols.size() - 1)
			{
				cout << m_NodesizeArray[i] << ",";
			}
			else
			{
				cout << m_NodesizeArray[i] << endl;
			}
		}
		cout<<endl;
	}
	OGRDataSource::DestroyDataSource(poDS);

	return true;

}

bool SetNodeSize_Raster()
{
	GDALAllRegister();
	vector<int> pVIndex4EachImg;
	//statistic the bands num of all input imgs
	for(int i=0;i<m_PathV_LearnSamples.size();i++)
	{
		GDALDataset* dataset_gdal = (GDALDataset *) GDALOpen( m_PathV_LearnSamples[i].c_str(), GA_ReadOnly);

		if (dataset_gdal == NULL )
		{
			cout<<"the file does not exist..."<<endl;
			exit(0);
		}
		int bandcount = dataset_gdal->GetRasterCount();
		double tmpMinMax[2];
		for(int j=0; j< bandcount; j++ )
		{
			dataset_gdal->GetRasterBand(j+1)->ComputeRasterMinMax(1,tmpMinMax);
			int pNodeSize = (int)tmpMinMax[1]-(int)tmpMinMax[0]+1;
			m_NodesizeArray.push_back(pNodeSize);
		}
		delete dataset_gdal;
	}


	//use all raster bands
	//c_CaculateCols of raster stores the index of band   (the start band is 1 in gdal)
	//if there are more than one , it means overlay them , and then the index
	if(c_CaculateCols.size()==0)
	{
		for(int i=1;i<=m_NodesizeArray.size();i++)
		{
			c_CaculateCols.push_back(i);
		}
	}

	//cout the node size
	if(rankid==0)
	{
		cout << "Node size: ";
		for (int i = 0; i < m_NodesizeArray.size(); i++)
		{
			if (i < m_NodesizeArray.size() - 1)
			{
				cout << m_NodesizeArray[i] << ",";
			}
			else
			{
				cout << m_NodesizeArray[i] << endl;
			}
		}
		cout<<endl;
	}
	return true;
}

bool LoadEvidencesFromFile(const char *fname,  pEvidencesVector* evVec, const CModelDomain *pMD)
{
	FILE * stream = fopen(fname, "rt");
	if( !stream )
	{
		return false;
	}
	int next;
	valueVector vls;
	int valInt;
	float valFlt;
	int nnodes = pMD->GetNumberVariables();
	intVector obsNds;
	obsNds.reserve(nnodes);

	boolVector isFloat(nnodes, true);
	intVector nVls;
	nVls.resize(nnodes);
	const CNodeType *nt;

	int numAllVls = 0;
	int i = 0;
	for( i = 0; i < nnodes; i++)
	{
		nt= pMD->GetVariableType(i);
		if( nt->IsDiscrete() )
		{
			isFloat[i] = false;
			nVls[i] = 1;
		}
		else
		{
			nVls[i] = nt->GetNodeSize();
		}
		numAllVls += nVls[i];
	}


	vls.resize(numAllVls);
	valueVector::iterator it;
	valueVector::iterator itEnd;
	it = vls.begin();
	itEnd = vls.end();
	i = 0;
	int ev = 0;

	while (1) {
		next = getc(stream);
		//fread(&next, 1, 1, stream);
		if (feof(stream) || i == nnodes) {
			if (i) {
				if (it != itEnd) {
					vls.erase(it, itEnd);
				}
				ev++;
				CEvidence *pEv = CEvidence::Create(pMD, obsNds, vls);
				evVec->push_back(pEv);
			}

			if (i == nnodes && !feof(stream)) {
				i = 0;
				vls.resize(numAllVls);
				it = vls.begin();
				itEnd = vls.end();
				obsNds.clear();
				obsNds.reserve(nnodes);
				//fread(&next, 1, 1, stream);

			} else {
				break;
			}
		}
		if (next != ',' && next != '\n') {
			if (isalpha(next)) {

				if (next == 'N') {
					char buf[2];
					fread(buf, sizeof(buf) / sizeof(buf[0]), sizeof(buf[0]),
						stream);

					if (buf[0] != '/'
						|| buf[1] != 'A' /*|| !isspace(buf[2])*/) {
							fclose(stream);
							PNL_THROW( CBadArg, "Bad file with cases");
					}
					i++;
				} else {
					fclose(stream);
					PNL_THROW( CBadArg, "Bad file with cases");
				}
			} else {
				obsNds.push_back(i);
				ungetc(next, stream);
				if (isFloat[i]) {
					int j;
					for (j = 0; j < nVls[i]; j++) {

						fscanf(stream, "%e,", &valFlt);
						it->SetFlt(valFlt);
						it++;
					}
				} else {

					fscanf(stream, "%d,", &valInt);
					it->SetInt(valInt);
					it++;
				}
				i++;
			}
		}
	}

	fclose(stream);

	RowNum = evVec->size();
	ColNum = nnodes;

	return true;

}

bool LoadEvidencesFromShp(pEvidencesVector* evVec, const CModelDomain *pMD)
{
	OGRRegisterAll();
	string p_DatasourceConStr = m_DatasourceConStr;
	string pLayerName = m_PathOrTableName;
	if(strcmp(m_DatasourceConStr.c_str(),"")==0)//input is shp file
	{
		p_DatasourceConStr = m_PathOrTableName;
		string pShpPath = m_PathOrTableName;
		string p_LayerName = GetFileNameOnly(pShpPath.c_str());
		int pindex = p_LayerName.find_first_of('.');
		pLayerName = p_LayerName.erase(pindex,p_LayerName.size()-1);
	}
	OGRDataSource* poDS = OGRSFDriverRegistrar::Open(p_DatasourceConStr.c_str(),FALSE);
	OGRLayer* poLayer = poDS->GetLayerByName(pLayerName.c_str());
	OGRFeature * pFeature =poLayer->GetNextFeature();

	RowNum = poLayer->GetFeatureCount();
	ColNum= pFeature->GetFieldCount();

	int nnodes = pMD->GetNumberVariables();
	intVector obsNds;valueVector vls;
	obsNds.reserve(nnodes);vls.resize(nnodes);

	for(int i=0;i<c_CaculateCols.size();i++)
	{
		obsNds.push_back(i);
	}
	int i=0;
	while (pFeature!=NULL)
	{
		for(int i=0;i<c_CaculateCols.size();i++)
		{

			if((pFeature->GetFieldAsInteger(c_CaculateCols[i])>m_NodesizeArray[i])&&(c_CaculateCols[i]==m_IndexOfDecisionNode))
			{
				cout<<"value: "<<pFeature->GetFieldAsInteger(c_CaculateCols[i])<<" in decision column: "<<c_CaculateCols[i]<<" over max, please please reset nodesize..."<<endl;
				exit(1);
			}
			else if((pFeature->GetFieldAsInteger(c_CaculateCols[i])>m_NodesizeArray[i]-1)&&(c_CaculateCols[i]!=m_IndexOfDecisionNode))
			{
				cout<<"value: "<<pFeature->GetFieldAsInteger(c_CaculateCols[i])<<" in normal column: "<<c_CaculateCols[i]<<" over max, please reset nodesize..."<<endl;
				exit(1);
			}
			else
			{
				vls[i].SetInt(pFeature->GetFieldAsInteger(c_CaculateCols[i]));
			}
		}
		CEvidence *pEv = CEvidence::Create(pMD, obsNds, vls);
		evVec->push_back(pEv);

		pFeature =poLayer->GetNextFeature();
	}
	OGRDataSource::DestroyDataSource( poDS );

	return true;

}

bool CheckIfCellValid(int pIndex)
{
	for(int i=0;i<m_AllBandCount;i++)
	{
		if((m_AllBandsData[i][pIndex]==m_NoDataValue)||(m_AllBandsData[i][pIndex]<0)||(m_AllBandsData[i][pIndex]>=m_NodesizeArray[i]))
			//if(m_AllBandsData[i][pIndex]==m_NoDataValue)
		{
			return false;
		}
	}
	return true;
}

bool LoadEvidencesFromRaster(pEvidencesVector* evVec, const CModelDomain *pMD)
{
	GDALAllRegister();
	vector<int> pVIndex4EachImg;
	//statistic the bands num of all input imgs
	for(int i=0;i<m_PathV_LearnSamples.size();i++)
	{
		GDALDataset* dataset_gdal = (GDALDataset *) GDALOpen( m_PathV_LearnSamples[i].c_str(), GA_ReadOnly );
		if (dataset_gdal == NULL )
		{
			cout<<"the file does not exist..."<<endl;
			exit(0);
		}

		int width = dataset_gdal->GetRasterXSize();
		int height = dataset_gdal->GetRasterYSize();
		int bandcount = dataset_gdal->GetRasterCount();
		int p_NoDataValue = dataset_gdal->GetRasterBand(1)->GetNoDataValue();
		if(i==0)
		{
			m_RowNum = height;m_ColNum = width;
			m_NoDataValue = dataset_gdal->GetRasterBand(1)->GetNoDataValue();
		}
		else if((width!=m_ColNum)||(height!=m_RowNum)||(p_NoDataValue!=m_NoDataValue))
		{
			//cout<<"input img Cols Rows NodataValue not same..."<<endl;
			//exit(0);
		}
		pVIndex4EachImg.push_back(m_AllBandCount);
		m_AllBandCount+=bandcount;
		delete dataset_gdal;
	}
	//alloc memory for all image data
	m_AllBandsData = new double*[m_AllBandCount];
	for(int i=0;i<m_PathV_LearnSamples.size();i++)
	{
		GDALDataset* dataset_gdal = (GDALDataset *) GDALOpen( m_PathV_LearnSamples[i].c_str(), GA_ReadOnly );
		int bandcount = dataset_gdal->GetRasterCount();
		//load data
		for(int j=0; j< bandcount; j++ )
		{
			int pindex = pVIndex4EachImg[i]+j;
			m_AllBandsData[pindex] = new double[m_ColNum*m_RowNum];
			GDALRasterBand * poband=dataset_gdal->GetRasterBand(j+1);
			poband->RasterIO(GF_Read,0,0,m_ColNum,m_RowNum,m_AllBandsData[pindex],m_ColNum,m_RowNum,GDT_Float64,0,0);
		}
		delete dataset_gdal;
	}

	int nnodes = pMD->GetNumberVariables();
	intVector obsNds;valueVector vls;
	obsNds.reserve(nnodes);vls.resize(nnodes);

	for(int i=0;i<c_CaculateCols.size();i++)
	{
		obsNds.push_back(i);
	}

	//build the evidence
	for(int i=0;i<m_RowNum*m_ColNum;i++)
	{
		if(CheckIfCellValid(i))
		{
			for(int j=0;j<m_AllBandCount;j++)
			{
				vls[j].SetInt((int)m_AllBandsData[j][i]);
			}

			CEvidence *pEv = CEvidence::Create(pMD, obsNds, vls);
			evVec->push_back(pEv);
		}
	}
	//free memory
	delete m_AllBandsData;
	return true;
}

CBNet * CreateEmptyBNet()
{
	//Create a empty BNet and Get Evidences from file
	int numOfNds = c_CaculateCols.size();//*<-
	CGraph *pGraph = CGraph::Create( numOfNds, NULL, NULL, NULL );
	nodeTypeVector  nodeTypes;intVector nodeAssociation;

	for(int i=0;i<numOfNds;i++)
	{
		if(c_CaculateCols[i]!=m_IndexOfDecisionNode)
		{
			CNodeType nt_Chance(1,m_NodesizeArray[i],nsChance);
			nodeTypes.push_back(nt_Chance);
		}
		else
		{
			CNodeType nt_Decision(1,m_NodesizeArray[i],nsDecision);
			nodeTypes.push_back(nt_Decision);
		}
		nodeAssociation.push_back(i);
	}


	CBNet* pEmptyBNet = CBNet::Create( numOfNds, nodeTypes, nodeAssociation, pGraph );

	pEmptyBNet->AllocFactors();

	for(int i = 0; i < numOfNds; ++i )
	{
		pEmptyBNet->AllocFactor(i);
		CFactor* pFactor = pEmptyBNet->GetFactor(i);
		pFactor->CreateAllNecessaryMatrices();
	}

	return pEmptyBNet;
}

CBNet * StructureLearningOnly_HC(CBNet* pBnet,pEvidencesVector evVec)
{
	//create learning engine
	intVector pvAncestor,pvDescent;
	//Maximum number of parents of a child node
	int p_nMaxFanIn = 2;
	//How many times will hill-climbing search procedure restart with random initial structures
	int p_nRestarts = 1;
	CMlStaticStructLearnHC *pLearn =CMlStaticStructLearnHC::Create(pBnet,itStructLearnML,StructLearnHC,AIC,p_nMaxFanIn,pvAncestor,pvDescent,p_nRestarts);

	//set data for learning
	pLearn->SetData( evVec.size(), &evVec.front() );
	MPI_Barrier(MPI_COMM_WORLD);
	double pSLearningStartMoment = MPI_Wtime();

	pLearn->Learn();

	double pSLearningFinishMoment = MPI_Wtime();
	Mpi_StructureLearning_Time = pSLearningFinishMoment-pSLearningStartMoment;
	double tmp = Mpi_StructureLearning_Time;
	//cout<<"rankid:"<<rankid<<", Mpi_StructureLearning_Time:"<<Mpi_StructureLearning_Time<<endl;
	MPI_Reduce(&Mpi_StructureLearning_Time,&tmp,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
	Mpi_StructureLearning_Time = tmp/numproc;
	//output result dag
	CGraph *pResultGraph = CGraph::Copy(static_cast<const CGraph *>(pLearn->GetResultDAG()));

	//create a BNet for parameter learning by using the result graph of structure learning
	CBNet* pResultBNet = CBNet::Create( pResultGraph,pBnet->GetModelDomain() );
	pResultBNet->AllocFactors();
	for(int i = 0; i < pResultBNet->GetNumberOfNodes(); ++i )
	{
		pResultBNet->AllocFactor(i);
		CFactor* pFactor = pResultBNet->GetFactor(i);
		pFactor->CreateAllNecessaryMatrices();
	}

	delete pLearn;

	return pResultBNet;
}

CBNet* ParameterLearning_EM(const CBNet* pBnet,pEvidencesVector evVec)
{
	CBNet* pLearnBNet = CBNet::Copy( pBnet );

	CParEMLearningEngine * pParLearn = CParEMLearningEngine::Create( pLearnBNet );
	pParLearn->SetData( evVec.size(), &evVec.front() );
	MPI_Barrier(MPI_COMM_WORLD);
	double pPLearningStartMoment = MPI_Wtime();

	pParLearn->Learn();
	//pParLearn->LearnContMPI();

	double pPLearningFinishMoment = MPI_Wtime();
	Mpi_ParameterLearning_Time = pPLearningFinishMoment-pPLearningStartMoment;
	double tmp=Mpi_ParameterLearning_Time;
	//cout<<"rankid:"<<rankid<<", Mpi_ParameterLearning_Time:"<<Mpi_ParameterLearning_Time<<endl;
	MPI_Reduce(&Mpi_ParameterLearning_Time,&tmp,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
	Mpi_ParameterLearning_Time = tmp;
	delete pParLearn;

	return pLearnBNet;
}

int Infer_GibbsSampling(const CBNet* pBnet,CEvidence* pEvidForInf)
{
	CGibbsSamplingInfEngine *pGibbsInf = CGibbsSamplingInfEngine::Create( pBnet );

	pGibbsInf->SetMaxTime( 1000);
	pGibbsInf->SetBurnIn( 100 );

	int VlsToInfer = 0;
	for(int i=0;i<c_CaculateCols.size();i++)
	{
		if(c_CaculateCols[i]==m_IndexOfDecisionNode)
		{
			VlsToInfer = i;
		}
	}

	intVecVector queries(1);queries[0].clear();queries[0].push_back( VlsToInfer );
	pGibbsInf->SetQueries( queries );
	//set the query node
	int numQueryNds = 1;//*<-
	int queryNds[] = { VlsToInfer };//*<-

	pGibbsInf->EnterEvidence( pEvidForInf );
	pGibbsInf->MarginalNodes(queryNds,numQueryNds);

	const CPotential* pMarg = pGibbsInf->GetQueryJPD();

	//display the query node and such value of BNet
	int nnodes;
	const int* domain;
	pMarg->GetDomain(&nnodes, &domain);

	CMatrix<float>* pMat = pMarg->GetMatrix(matTable);
	int nEl;
	const float* data;
	static_cast<CNumericDenseMatrix<float>*>(pMat)->GetRawData(&nEl, &data);

	int pInfValue = 0;
	float data_max = data[0];
	for (int i = 1; i < nEl; i++)
	{
		//cout<<data[i]<<endl;
		if (data[i] > data_max) {
			data_max = data[i];
			pInfValue = i;
		}
	}
	delete pGibbsInf;
	return pInfValue;
}

int Infer_Naive(const CBNet* pBnet,CEvidence* pEvidForInf)
{
	//create Naive inference for BNet
	CNaiveInfEngine* pNaiveInf = CNaiveInfEngine::Create( pBnet );
	//set the query node
	int numQueryNds = 1;//*<-
	int VlsToInfer = 0;
	
	for(int i=0;i<c_CaculateCols.size();i++)
	{
		if(c_CaculateCols[i]==m_IndexOfDecisionNode)
		{
			VlsToInfer = i;
		}
	}
	
	int queryNds[] = { VlsToInfer };//*<-

	pNaiveInf->EnterEvidence(pEvidForInf);
	//get a marginal for query set of nodes
	pNaiveInf->MarginalNodes( queryNds, numQueryNds );
	const CPotential* pMarg = pNaiveInf->GetQueryJPD();

	//display the query node and such value of BNet
	int nnodes;const int* domain;pMarg->GetDomain( &nnodes, &domain );

	CMatrix<float>* pMat = pMarg->GetMatrix(matTable);
	int nEl;
	const float* data;
	static_cast<CNumericDenseMatrix<float>*>(pMat)->GetRawData(&nEl, &data);

	int pInfValue = 0;
	float data_max = data[0];
	for(int i = 1; i < nEl; i++ )
	{
		//cout<<data[i]<<endl;
		if(data[i]>data_max)
		{
			data_max = data[i];
			pInfValue = i;
		}
	}

	delete pNaiveInf;
	return pInfValue;
}

string toString(int element)
{
	ostringstream oss;
	oss << element;
	string str(oss.str());

	return str;
}

void Infer(const CBNet* pBnet,string algorithm2Use)
{
	if(GetFileExtensionName(m_ResultPathOrTableName)!="tif")  //shp file or postgis
	{
		int pCorrectRowNum= 0;
		OGRRegisterAll();
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

		RowNum = poLayer->GetFeatureCount();
		ColNum= pFeature->GetFieldCount();

		//if -c argv is null, all field will be used
		if(c_CaculateCols.size()==0)
		{
			if(rankid==0)
				cout<<"!!!!!!!  No input columns ids, all fields in the input file will be used..."<<endl<<endl;
			for(int i=0;i<ColNum;i++)
			{
				c_CaculateCols.push_back(i);
			}
		}
		//assign number to each process
		int pnum_start,pnum_end;
		int pMyProcessNum=0; int pRemainder=RowNum%numproc;
		if(rankid<pRemainder)
		{
			pMyProcessNum = RowNum/numproc+1;
			pnum_start = rankid*pMyProcessNum;
			pnum_end = pnum_start+pMyProcessNum-1;
		}
		else
		{
			pMyProcessNum = RowNum/numproc;
			pnum_start = pRemainder*(pMyProcessNum+1)+(rankid-pRemainder)*pMyProcessNum;
			pnum_end = pnum_start+pMyProcessNum-1;
		}

		intVector obsNds;valueVector vls;
		for(int i=0;i<c_CaculateCols.size();i++)
		{
			if(c_CaculateCols[i]!=m_IndexOfDecisionNode)
			{
				obsNds.push_back(i);
			}
		}
		//postgis: fid begins from 1, not 0
		string pwhere="";
		if(strcmp(m_DatasourceConStr.substr(0,3).c_str(),"PG:")==0)//input is postgis
		{
			pwhere = "ogc_fid>="+toString(pnum_start+1)+" and ogc_fid<="+toString(pnum_end+1);
		}
		else//shpfile: fid begins from 0, not 1
		{
			pwhere = "fid>="+toString(pnum_start)+" and fid<="+toString(pnum_end);
		}
		poLayer->SetAttributeFilter(pwhere.c_str());
		pFeature = poLayer->GetNextFeature();

		Mpi_Inferring_Time = 0; double pReadTime = 0;double pWriteTime = 0;
		while(pFeature!=NULL)
		{
			double pstart_read = MPI_Wtime();
			vls.clear();
			for(int i=0;i<c_CaculateCols.size();i++)
			{
				if(c_CaculateCols[i]!=m_IndexOfDecisionNode)
				{
					vls.push_back(pFeature->GetFieldAsInteger(c_CaculateCols[i]));
				}
			}
			int oldvalue = pFeature->GetFieldAsInteger(m_IndexOfDecisionNode-1);
			double pend_read = MPI_Wtime();
			pReadTime=pReadTime+pend_read-pstart_read;

			double pstart = MPI_Wtime();
			CEvidence *pEv = CEvidence::Create(pBnet, obsNds.size(), &obsNds[0], vls);
			int pInfValue = 0;
			if (!strcmp(algorithm2Use.c_str(), "to2Infer_Naive"))
			{
				pInfValue = Infer_Naive(pBnet,pEv);
			}
			if (!strcmp(algorithm2Use.c_str(), "to2Infer_Gibbs"))
			{
				pInfValue = Infer_GibbsSampling(pBnet,pEv);
			}
			if(oldvalue==pInfValue)
			{
				pCorrectRowNum++;
			}
			double pend = MPI_Wtime();
			Mpi_Inferring_Time=Mpi_Inferring_Time+pend-pstart;

			double pstart_write = MPI_Wtime();
			delete pEv;
			pFeature->SetField(m_IndexOfDecisionNode,pInfValue);
			poLayer->SetFeature(pFeature);
			pFeature = poLayer->GetNextFeature();
			double pend_write = MPI_Wtime();
			pWriteTime=pWriteTime+pend_write-pstart_write;
		}
		int tmp_correctNum=0;
		MPI_Reduce(&pCorrectRowNum,&tmp_correctNum,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
		m_PD_value = (double)tmp_correctNum/(double)RowNum;

		double tmp=Mpi_Inferring_Time;
	    MPI_Reduce(&Mpi_Inferring_Time,&tmp,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
	    Mpi_Inferring_Time = tmp;

		//cout<<"rankid:"<<rankid<<", pReadTime:"<<pReadTime<<endl;
		//cout<<"rankid:"<<rankid<<", pWriteTime:"<<pWriteTime<<endl;

		OGRDataSource::DestroyDataSource( poDS );
	}
	else//raster
	{
		GDALAllRegister();

		vector<int> pVIndex4EachImg;
		int width,height;int pAllBandCount = 0;

		vector<GDALDataset*> dataset_gdal_list;
		const char *pszFormat = "GTiff";
		GDALDriver* podriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
		GDALDataset* dataset_result;GDALDataset* dataset_confidence;
		//statistic the bands num of all input imgs
		for(int i=0;i<m_PathV_InferSamples.size();i++)
		{
			GDALDataset* dataset_gdal = (GDALDataset *) GDALOpen( m_PathV_InferSamples[i].c_str(), GA_ReadOnly );
			if (dataset_gdal == NULL )
			{
				cout<<"the file does not exist..."<<endl;
				exit(0);
			}
			dataset_gdal_list.push_back(dataset_gdal);

			int bandcount_tmp = dataset_gdal->GetRasterCount();
			pVIndex4EachImg.push_back(pAllBandCount);
			pAllBandCount+=bandcount_tmp;

			if(i==0)
			{
				width = dataset_gdal->GetRasterXSize();
				height = dataset_gdal->GetRasterYSize();
				const char* pProjectionRef;double pasfGeoTransform[6];
				pProjectionRef = dataset_gdal->GetProjectionRef();
				dataset_gdal->GetGeoTransform(pasfGeoTransform);
				int p_InferNoDataValue = dataset_gdal->GetRasterBand(1)->GetNoDataValue();
				if(i==0)
				{
					m_RowNum = height;m_ColNum = width;
					m_InferNoDataValue = dataset_gdal->GetRasterBand(1)->GetNoDataValue();
				}
				else if((width!=m_ColNum)||(height!=m_RowNum)||(p_InferNoDataValue!=m_InferNoDataValue))
				{
					//cout<<"input img Cols Rows NodataValue not same..."<<endl;
					//exit(0);
				}
				if(rankid==0)
				{
					//to make sure the create file's block is 1 row
					char **papszMetadata = NULL;
					papszMetadata = CSLSetNameValue(papszMetadata, "BLOCKYSIZE", "1" );
					dataset_result = podriver->Create(m_ResultPathOrTableName.c_str(),width,height,1,GDT_Byte,papszMetadata);
					dataset_result->SetProjection(pProjectionRef);
					dataset_result->SetGeoTransform(pasfGeoTransform);
					dataset_result->GetRasterBand(1)->SetNoDataValue(255);
					GDALClose(dataset_result);
				}
			}
		}
		if(pAllBandCount!=m_AllBandCount-1)
		{
			cout<<"error:band count not equal as learn sample image..."<<endl;
			exit(0);
		}
		m_AllBandCount = pAllBandCount;

		//use all raster bands
		//c_CaculateCols of raster stores the index of band   (the start band is 1 in gdal)
		//if there are more than one , it means overlay them , and then the index
		if(c_CaculateCols.size()==0)
		{
			for(int i=1;i<=m_NodesizeArray.size();i++)
			{
				c_CaculateCols.push_back(i);
			}
		}

		MPI_Barrier(MPI_COMM_WORLD);
		dataset_result = (GDALDataset *) GDALOpen( m_ResultPathOrTableName.c_str(), GA_Update );

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

		intVector obsNds;valueVector vls;
		for(int i=0;i<c_CaculateCols.size();i++)
		{
			if(c_CaculateCols[i]!=m_IndexOfDecisionNode)
			{
				obsNds.push_back(i);
			}
		}

		cout<<"Process "<<rankid<<" will caculate from row "<<nRow_start<<" to row "<<nRow_end<<endl;

		int pReadWriteBuffer = 1;//read and write 100 rows once

		double ** p_InferDataBuffer = new double*[m_AllBandCount];
		for(int i=0; i< m_AllBandCount; i++ )
		{
			p_InferDataBuffer[i] = new double[width*pReadWriteBuffer]();
		}

		GByte* p_ResultDataBuffer = new GByte[width*pReadWriteBuffer]();
		Mpi_Inferring_Time = 0;
		for(int iRow = nRow_start; iRow <= nRow_end; iRow+=pReadWriteBuffer )
		{
			int pValidReadWriteBuffer = pReadWriteBuffer;
			if(iRow+pReadWriteBuffer-1>nRow_end)
			{
				pValidReadWriteBuffer = nRow_end-iRow+1;
			}
			for(int i=0;i<m_PathV_InferSamples.size();i++)
			{
				int pbandcount = dataset_gdal_list[i]->GetRasterCount();
				for(int j=0; j< pbandcount; j++ )
				{
					int pindex = pVIndex4EachImg[i]+j;
					GDALRasterBand * poband=dataset_gdal_list[i]->GetRasterBand(j+1);
					poband->RasterIO(GF_Read,0,iRow,width,pValidReadWriteBuffer,p_InferDataBuffer[pindex],width,pValidReadWriteBuffer,GDT_Float64,0,0);
				}
			}

			double pstart = MPI_Wtime();
			for(  int t = 0; t < width*pValidReadWriteBuffer; t++  )
			{
				bool pIsValidCell = true;
				for(int p=0;p<m_AllBandCount;p++)
				{
					if((p_InferDataBuffer[p][t]==m_InferNoDataValue)||(p_InferDataBuffer[p][t]<0)||(p_InferDataBuffer[p][t]>=m_NodesizeArray[p]))
					{
						pIsValidCell = false;
						break;
					}
				}

				vls.clear();
				if(pIsValidCell)
				{
					for(int j=0;j<m_AllBandCount;j++)
					{
						vls.push_back((int)p_InferDataBuffer[j][t]);
					}
				}
				else//nodata
				{
					for(int j=0;j<m_AllBandCount;j++)
					{
						vls.push_back(0);
					}				
				}
				CEvidence *pEv = CEvidence::Create(pBnet, obsNds.size(), &obsNds[0], vls);
				int pInfValue = 0;
				if (!strcmp(algorithm2Use.c_str(), "to2Infer_Naive"))
				{
					pInfValue = Infer_Naive(pBnet,pEv);
				}
				if (!strcmp(algorithm2Use.c_str(), "to2Infer_Gibbs"))
				{
					pInfValue = Infer_GibbsSampling(pBnet,pEv);
				}
				delete pEv;
				p_ResultDataBuffer[t] = pInfValue;

				if(!pIsValidCell)//nodata
				{
					p_ResultDataBuffer[t] = 255;//NoData
				}
			}
			double pend = MPI_Wtime();
			Mpi_Inferring_Time = Mpi_Inferring_Time+pend-pstart;

			//each process write the result
			dataset_result->GetRasterBand(1)->RasterIO(GF_Write,0,iRow,width,pValidReadWriteBuffer,p_ResultDataBuffer,width,pValidReadWriteBuffer,GDT_Byte,0,0);
		}

		double tmp=Mpi_Inferring_Time;
	    MPI_Reduce(&Mpi_Inferring_Time,&tmp,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
	    Mpi_Inferring_Time = tmp;

		//free memory
		for(int i=0; i< m_AllBandCount; i++ )
		{
			delete []	p_InferDataBuffer[i];
		}
		delete p_InferDataBuffer;
		delete p_ResultDataBuffer;
		for(int i=0;i<m_PathV_InferSamples.size();i++)
		{
			delete dataset_gdal_list[i];
		}
		if(rankid==0)
		{
			delete dataset_result;
		}
	}
}

int main_startinstep(int argc, char* argv[])
{
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);

	Mpi_Start_Moment = MPI_Wtime();
	string algorithm2Use="";
	std::vector<std::string> p_PathV_LearnSamples;std::vector<std::string> p_PathV_InferSamples;	m_ResultPathOrTableName="";
	//解析程序中输入的各个参数
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			//数据库连接字符串
			//m_DatasourceConStr = "PG:host='192.168.5.131' port='5432'  dbname='postgis' user='postgres' password='postgres'";

			if (!strcmp(argv[i], "-a"))
			{
				//algorithm to use
				algorithm2Use = argv[i + 1];
			}
			//程序路径或者表名
			if (!strcmp(argv[i], "-pl"))
			{
				m_PathOrTableName = argv[i + 1];

				string tmp = argv[i+1];
				SplitStrFunctor::StrVec svRet;
				SplitStrFunctor ssf;
				ssf(tmp, ",", svRet);
				JointStrFunctor::StrVecItr iter;
				for( iter = svRet.begin(); iter != svRet.end(); iter++ )
				{
					string pEachColValue = *iter;
					p_PathV_LearnSamples.push_back(pEachColValue.c_str());
				}
			}
			//参与学习的属性
			if (!strcmp(argv[i], "-c"))
			{
				//optional argv, so check exceed or null first
				if(i+1<argc)
					if(argv[i + 1][0]!='-')
					{
						string pCaculateCols = argv[i + 1];
						SplitStrFunctor::StrVec svRet;
						SplitStrFunctor ssf;
						ssf(pCaculateCols, ",", svRet);
						JointStrFunctor::StrVecItr iter;
						for (iter = svRet.begin(); iter != svRet.end(); iter++)
						{
							string pEachColValue = *iter;
							c_CaculateCols.push_back(atoi(pEachColValue.c_str()));
						}
					}
			}
			//决策属性所在列
			if (!strcmp(argv[i], "-C"))
			{
				m_IndexOfDecisionNode = atoi(argv[i + 1]);
			}
			//程序路径或者表名
			if (!strcmp(argv[i], "-pi"))
			{
				m_ResultPathOrTableName = argv[i + 1];

				string tmp = argv[i+1];
				SplitStrFunctor::StrVec svRet;
				SplitStrFunctor ssf;
				ssf(tmp, ",", svRet);
				JointStrFunctor::StrVecItr iter;
				for( iter = svRet.begin(); iter != svRet.end(); iter++ )
				{
					string pEachColValue = *iter;
					p_PathV_InferSamples.push_back(pEachColValue.c_str());
				}

			}
			//程序路径或者表名
			if (!strcmp(argv[i], "-o"))
			{
				//optional argv, so check exceed or null first
				if(i+1<argc)
					if(argv[i + 1][0]!='-')
					{
						m_ResultPathOrTableName = argv[i + 1];
					}
			}
		}
	}

	//set NodeSize
	if(GetFileExtensionName(m_PathOrTableName)=="tif")
	{
		if(m_PathOrTableName.find_first_of('/')==-1)
		{
			//Get the full path
			GetExePath pexepath;
			m_ResultPathOrTableName = pexepath.getExeDir()+"data/result/"+m_ResultPathOrTableName;
			for(int i=0;i<p_PathV_LearnSamples.size();i++)
			{
				m_PathV_LearnSamples.push_back(pexepath.getExeDir()+"data/learn/"+p_PathV_LearnSamples[i]);
				if(i<p_PathV_InferSamples.size())
				{
					m_PathV_InferSamples.push_back(pexepath.getExeDir()+"data/infer/"+p_PathV_InferSamples[i]);
				}
			}
		}
		if(c_CaculateCols.size()>0)
		{
			if(rankid==0)
			{
				cout<<"******************************"<<endl;
				cout<<"the -c argv will not be used, it will process all input files and all bands"<<endl;
				cout<<"if you need to process specify some ones, you must deal with the image first..."<<endl;
				cout<<"******************************"<<endl;
			}
			c_CaculateCols.clear();
		}
		SetNodeSize_Raster();
	}
	else if(GetFileExtensionName(m_PathOrTableName)=="csv")
	{
		SetFixedNodeSize();
	}
	else
	{
		SetNodeSize();
	}


	pEvidencesVector evVec;
	CBNet* pEmptyBNet = CreateEmptyBNet();

	if(GetFileExtensionName(m_PathOrTableName)=="csv")
	{
		if( ! LoadEvidencesFromFile(m_PathOrTableName.c_str(), &evVec, pEmptyBNet->GetModelDomain()) )
		{
			cout<<"Open file"<<m_PathOrTableName<<" error"<<endl;
			exit(1);getchar();
		}
		cout<<"we will not support  txt,csv file now,please use shp file..."<<endl;
	}
	if((GetFileExtensionName(m_PathOrTableName)=="shp")||strcmp(m_DatasourceConStr.c_str(),""))
	{
		if( ! LoadEvidencesFromShp(&evVec, pEmptyBNet->GetModelDomain()))
		{
			cout<<"Open layer"<<m_PathOrTableName<<" error"<<endl;
			exit(1);getchar();
		}
	}
	if(GetFileExtensionName(m_PathOrTableName)=="tif")
	{
		if( ! LoadEvidencesFromRaster(&evVec, pEmptyBNet->GetModelDomain()))
		{
			cout<<"Open raster"<<m_PathOrTableName<<" error"<<endl;
			exit(1);getchar();
		}
	}


	//Structure Learn Process
	CBNet* pSLearnedBnet = StructureLearningOnly_HC(pEmptyBNet,evVec);
	
	//Show the learned structure
	if(rankid==0)
	{
		ShowLearnedStructure(pSLearnedBnet->GetGraph());
	}
	
	delete pEmptyBNet;

	//Parameter Learn Process
	CBNet* pLearnedBnet=ParameterLearning_EM(pSLearnedBnet,evVec);

	//Show each node's probability after learning
	if(rankid==0)
	{
		ShowProbabilityOfEachNode(pLearnedBnet);
	}

	delete pSLearnedBnet;

	MPI_Barrier(MPI_COMM_WORLD);
	double pPInferringStartMoment = MPI_Wtime();
	Infer(pLearnedBnet,algorithm2Use);
	MPI_Barrier(MPI_COMM_WORLD);
	double pPInferringFinishMoment = MPI_Wtime();
	Mpi_Inferring_Time = pPInferringFinishMoment-pPInferringStartMoment;

	delete pLearnedBnet;
	//free the evidence
	for(int ev = 0; ev < evVec.size(); ev++ )
	{
		delete evVec[ev];
	}
	MPI_Barrier(MPI_COMM_WORLD);
	Mpi_Finish_Moment = MPI_Wtime();

	if(rankid==0)
	{
		ShowExcuteInfor(0);
		ShowExcuteInfor(1);
		ShowExcuteInfor(2);
	}

	MPI_Finalize();


	return 0;
}

int main_stucturelean(int argc, char* argv[])
{
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);

	Mpi_Start_Moment = MPI_Wtime();

	std::vector<std::string> p_PathV_LearnSamples; string poutFileStucture = "";	m_ResultPathOrTableName="";
        string dbname="";string hostname="";string username="";string pwd="";string port="";
	//解析程序中输入的各个参数
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			//数据库连接字符串
			m_DatasourceConStr = "PG:host='192.168.5.131' port='5437'  dbname='pdatabase' user='postgres' password='postgres'";

			//程序路径或者表名
			if (!strcmp(argv[i], "-pl"))
			{
				m_PathOrTableName = argv[i + 1];

				string tmp = argv[i+1];
				SplitStrFunctor::StrVec svRet;
				SplitStrFunctor ssf;
				ssf(tmp, ",", svRet);
				JointStrFunctor::StrVecItr iter;
				for( iter = svRet.begin(); iter != svRet.end(); iter++ )
				{
					string pEachColValue = *iter;
					p_PathV_LearnSamples.push_back(pEachColValue.c_str());
				}
			}
			//参与学习的属性
			if (!strcmp(argv[i], "-c"))
			{
				//optional argv, so check exceed or null first
				if(i+1<argc)
					if(argv[i + 1][0]!='-')
					{
						string pCaculateCols = argv[i + 1];
						SplitStrFunctor::StrVec svRet;
						SplitStrFunctor ssf;
						ssf(pCaculateCols, ",", svRet);
						JointStrFunctor::StrVecItr iter;
						for (iter = svRet.begin(); iter != svRet.end(); iter++)
						{
							string pEachColValue = *iter;
							c_CaculateCols.push_back(atoi(pEachColValue.c_str()));
						}
					}
			}
			//决策属性所在列
			if (!strcmp(argv[i], "-C"))
			{
				m_IndexOfDecisionNode = atoi(argv[i + 1]);
			}
			//the output file of stucture
			if (!strcmp(argv[i], "-os"))
			{
				poutFileStucture = argv[i + 1];
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
		}
	}



         //connection string
	 //m_DatasourceConStr = "PG:host='192.168.5.131' port='5437'  dbname='pdatabase' user='postgres' password='postgres'";           
         m_DatasourceConStr  = "PG:host='"+hostname+"' port='"+port+"' dbname='"+dbname+"' user='"+username+"' password='"+pwd+"'";
         if(rankid==0)
          cout<<m_DatasourceConStr<<endl;



	if(strcmp(GetFileExtensionName(m_PathOrTableName).c_str(),"shp")==0)
	{
		m_DatasourceConStr="";
	}
	//set NodeSize
	if(strcmp(GetFileExtensionName(m_PathOrTableName).c_str(),"tif")==0)
	{
		if(m_PathOrTableName.find_first_of('/')==-1)
		{
			//Get the full path
			GetExePath pexepath;
			poutFileStucture = pexepath.getExeDir()+"data/result/"+poutFileStucture;
			for(int i=0;i<p_PathV_LearnSamples.size();i++)
			{
				m_PathV_LearnSamples.push_back(pexepath.getExeDir()+"data/learn/"+p_PathV_LearnSamples[i]);
			}
		}
		if(c_CaculateCols.size()>0)
		{
			if(rankid==0)
			{
				cout<<"******************************"<<endl;
				cout<<"the -c argv will not be used, it will process all input files and all bands"<<endl;
				cout<<"if you need to process specify some ones, you must deal with the image first..."<<endl;
				cout<<"******************************"<<endl;
			}
			c_CaculateCols.clear();
		}
		SetNodeSize_Raster();
	}
	else if(GetFileExtensionName(m_PathOrTableName)=="csv")
	{
		SetFixedNodeSize();
	}
	else
	{
		SetNodeSize();
	}

	pEvidencesVector evVec;
	CBNet* pEmptyBNet = CreateEmptyBNet();

	if(GetFileExtensionName(m_PathOrTableName)=="csv")
	{
		if( ! LoadEvidencesFromFile(m_PathOrTableName.c_str(), &evVec, pEmptyBNet->GetModelDomain()) )
		{
			cout<<"Open file"<<m_PathOrTableName<<" error"<<endl;
			exit(1);getchar();
		}
		cout<<"we will not support  txt,csv file now,please use shp file..."<<endl;
	}
	if((GetFileExtensionName(m_PathOrTableName)=="shp")||strcmp(m_DatasourceConStr.c_str(),""))
	{
		if( ! LoadEvidencesFromShp(&evVec, pEmptyBNet->GetModelDomain()))
		{
			cout<<"Open layer"<<m_PathOrTableName<<" error"<<endl;
			exit(1);getchar();
		}
	}
	if(GetFileExtensionName(m_PathOrTableName)=="tif")
	{
		if( ! LoadEvidencesFromRaster(&evVec, pEmptyBNet->GetModelDomain()))
		{
			cout<<"Open raster"<<m_PathOrTableName<<" error"<<endl;
			exit(1);getchar();
		}
	}

	//Structure Learn Process
	CBNet* pSLearnedBnet = StructureLearningOnly_HC(pEmptyBNet,evVec);
	//Save and Show the learned structure
	if(rankid==0)
	{
		CContextPersistence xmlContext;
		xmlContext.Put(pSLearnedBnet,"MyBayesModel");
		if(!xmlContext.SaveAsXML(poutFileStucture))
		{
			// something goes wrong – can’t create file, disk full or …
			cout<<"error when store bayes network..."<<endl;
			exit(0);
		}
		if(m_Debug)
		{
		   ShowLearnedStructure(pSLearnedBnet->GetGraph());
		}
	}
	
	delete pEmptyBNet;

	//free the evidence
	for(int ev = 0; ev < evVec.size(); ev++ )
	{
		delete evVec[ev];
	}
	MPI_Barrier(MPI_COMM_WORLD);
	Mpi_Finish_Moment = MPI_Wtime();

	if(rankid==0)
	{
		ShowExcuteInfor(0);
		//WriteExcuteLog(0);
	}

	MPI_Finalize();

	return 0;
}

int main_paralearn(int argc, char* argv[])
{
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);

	Mpi_Start_Moment = MPI_Wtime();

	std::vector<std::string> p_PathV_LearnSamples;string pinputFileStucture="";string poutFileParaStucture="";	m_ResultPathOrTableName="";
        string dbname="";string hostname="";string username="";string pwd="";string port="";
	//解析程序中输入的各个参数
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			//数据库连接字符串
			m_DatasourceConStr = "PG:host='192.168.5.131' port='5437'  dbname='pdatabase' user='postgres' password='postgres'";

			//程序路径或者表名
			if (!strcmp(argv[i], "-pl"))
			{
				m_PathOrTableName = argv[i + 1];

				string tmp = argv[i+1];
				SplitStrFunctor::StrVec svRet;
				SplitStrFunctor ssf;
				ssf(tmp, ",", svRet);
				JointStrFunctor::StrVecItr iter;
				for( iter = svRet.begin(); iter != svRet.end(); iter++ )
				{
					string pEachColValue = *iter;
					p_PathV_LearnSamples.push_back(pEachColValue.c_str());
				}
			}
			//参与学习的属性
			if (!strcmp(argv[i], "-c"))
			{
				//optional argv, so check exceed or null first
				if(i+1<argc)
					if(argv[i + 1][0]!='-')
					{
						string pCaculateCols = argv[i + 1];
						SplitStrFunctor::StrVec svRet;
						SplitStrFunctor ssf;
						ssf(pCaculateCols, ",", svRet);
						JointStrFunctor::StrVecItr iter;
						for (iter = svRet.begin(); iter != svRet.end(); iter++)
						{
							string pEachColValue = *iter;
							c_CaculateCols.push_back(atoi(pEachColValue.c_str()));
						}
					}
			}
			//决策属性所在列
			if (!strcmp(argv[i], "-C"))
			{
				m_IndexOfDecisionNode = atoi(argv[i + 1]);
			}
			if (!strcmp(argv[i], "-is"))
			{
				pinputFileStucture = argv[i + 1];
			}
			if (!strcmp(argv[i], "-ops"))
			{
				poutFileParaStucture = argv[i + 1];
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


		}
	}


         //connection string
	 //m_DatasourceConStr = "PG:host='192.168.5.131' port='5437'  dbname='pdatabase' user='postgres' password='postgres'";           
         m_DatasourceConStr  = "PG:host='"+hostname+"' port='"+port+"' dbname='"+dbname+"' user='"+username+"' password='"+pwd+"'";
         if(rankid==0)
          cout<<m_DatasourceConStr<<endl;

	if(strcmp(GetFileExtensionName(m_PathOrTableName).c_str(),"shp")==0)
	{
		m_DatasourceConStr="";
	}
	//set NodeSize
	if(strcmp(GetFileExtensionName(m_PathOrTableName).c_str(),"tif")==0)
	{
		if(m_PathOrTableName.find_first_of('/')==-1)
		{
			//Get the full path
			GetExePath pexepath;
			pinputFileStucture = pexepath.getExeDir()+"data/result/"+pinputFileStucture;
			poutFileParaStucture = pexepath.getExeDir()+"data/result/"+poutFileParaStucture;
			for(int i=0;i<p_PathV_LearnSamples.size();i++)
			{
				m_PathV_LearnSamples.push_back(pexepath.getExeDir()+"data/learn/"+p_PathV_LearnSamples[i]);
			}
		}
		if(c_CaculateCols.size()>0)
		{
			if(rankid==0)
			{
				cout<<"******************************"<<endl;
				cout<<"the -c argv will not be used, it will process all input files and all bands"<<endl;
				cout<<"if you need to process specify some ones, you must deal with the image first..."<<endl;
				cout<<"******************************"<<endl;
			}
			c_CaculateCols.clear();
		}
		SetNodeSize_Raster();
	}
	else if(GetFileExtensionName(m_PathOrTableName)=="csv")
	{
		SetFixedNodeSize();
	}
	else
	{
		SetNodeSize();
	}

	CContextPersistence xmlContext;
	if(!xmlContext.LoadXML(pinputFileStucture))
	{
		// can’t open file or bad file content
		cout<<"error while loading bayes network..."<<endl;
		exit(0);
	}
	CBNet* pstructureFromFile = static_cast<CBNet*>(xmlContext.Get("MyBayesModel"));

	pEvidencesVector evVec;
	//CBNet* pEmptyBNet = CreateEmptyBNet();

	if(GetFileExtensionName(m_PathOrTableName)=="csv")
	{
		if( ! LoadEvidencesFromFile(m_PathOrTableName.c_str(), &evVec, pstructureFromFile->GetModelDomain()) )
		{
			cout<<"Open file"<<m_PathOrTableName<<" error"<<endl;
			exit(1);getchar();
		}
		cout<<"we will not support  txt,csv file now,please use shp file..."<<endl;
	}
	if((GetFileExtensionName(m_PathOrTableName)=="shp")||strcmp(m_DatasourceConStr.c_str(),""))
	{
		if( ! LoadEvidencesFromShp(&evVec, pstructureFromFile->GetModelDomain()))
		{
			cout<<"Open layer"<<m_PathOrTableName<<" error"<<endl;
			exit(1);getchar();
		}
	}
	if(GetFileExtensionName(m_PathOrTableName)=="tif")
	{
		if( ! LoadEvidencesFromRaster(&evVec, pstructureFromFile->GetModelDomain()))
		{
			cout<<"Open raster"<<m_PathOrTableName<<" error"<<endl;
			exit(1);getchar();
		}
	}


	//Show the learned structure
	if(rankid==0)
	{
		if(m_Debug)
		{
		    ShowLearnedStructure(pstructureFromFile->GetGraph());
		}
	}

	//Parameter Learn Process
	CBNet* pLearnedBnet=ParameterLearning_EM(pstructureFromFile,evVec);

	//Show each node's probability after learning
	if(rankid==0)
	{
		if(m_Debug)
		{
		     ShowProbabilityOfEachNode(pLearnedBnet);
		}
		//store the structure with parameters
		CContextPersistence xmlContext_save;
		xmlContext_save.Put(pLearnedBnet,"MyBayesModel");
		if(!xmlContext_save.SaveAsXML(poutFileParaStucture))
		{
			// something goes wrong – can’t create file, disk full or …
			cout<<"error when store bayes network..."<<endl;
			exit(0);
		}
	}

	delete pstructureFromFile;
	delete pLearnedBnet;
	//free the evidence
	for(int ev = 0; ev < evVec.size(); ev++ )
	{
		delete evVec[ev];
	}
	MPI_Barrier(MPI_COMM_WORLD);
	Mpi_Finish_Moment = MPI_Wtime();

	if(rankid==0)
	{
		ShowExcuteInfor(1);
		//WriteExcuteLog(1);
	}

	MPI_Finalize();


	return 0;
}

int main_infer(int argc, char* argv[])
{
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);

	Mpi_Start_Moment = MPI_Wtime();
	string algorithm2Use="";
	std::vector<std::string> p_PathV_LearnSamples;std::vector<std::string> p_PathV_InferSamples;string pinputFileParaStucture="";
	m_ResultPathOrTableName="";string pOutPutFile = "";
        string dbname="";string hostname="";string username="";string pwd="";string port="";
	//解析程序中输入的各个参数
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			//数据库连接字符串
			m_DatasourceConStr = "PG:host='192.168.5.131' port='5437'  dbname='pdatabase' user='postgres' password='postgres'";

			if (!strcmp(argv[i], "-a"))
			{
				//algorithm to use
				algorithm2Use = argv[i + 1];
			}
			//the var to infer
			if (!strcmp(argv[i], "-c"))
			{
				//optional argv, so check exceed or null first
				if(i+1<argc)
					if(argv[i + 1][0]!='-')
					{
						string pCaculateCols = argv[i + 1];
						SplitStrFunctor::StrVec svRet;
						SplitStrFunctor ssf;
						ssf(pCaculateCols, ",", svRet);
						JointStrFunctor::StrVecItr iter;
						for (iter = svRet.begin(); iter != svRet.end(); iter++)
						{
							string pEachColValue = *iter;
							c_CaculateCols.push_back(atoi(pEachColValue.c_str()));
						}
					}
			}
			//the col index of decision node
			if (!strcmp(argv[i], "-C"))
			{
				m_IndexOfDecisionNode = atoi(argv[i + 1]);
			}
			//input stucture with parameter
			if (!strcmp(argv[i], "-ips"))
			{
				pinputFileParaStucture = argv[i + 1];
			}
			//file to infer
			if (!strcmp(argv[i], "-pi"))
			{
				m_ResultPathOrTableName = argv[i + 1];

				string tmp = argv[i+1];
				SplitStrFunctor::StrVec svRet;
				SplitStrFunctor ssf;
				ssf(tmp, ",", svRet);
				JointStrFunctor::StrVecItr iter;
				for( iter = svRet.begin(); iter != svRet.end(); iter++ )
				{
					string pEachColValue = *iter;
					p_PathV_InferSamples.push_back(pEachColValue.c_str());
				}
			}
			//the result path, only image need
			if (!strcmp(argv[i], "-o"))
			{
				//optional argv, so check exceed or null first
				if(i+1<argc)
					if(argv[i + 1][0]!='-')
					{
						pOutPutFile = argv[i + 1];
					}
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
		}
	}
	//cout<<m_DatasourceConStr<<endl;
	//cout<<m_ResultPathOrTableName<<endl;
	//cout<<GetFileExtensionName(m_ResultPathOrTableName);


         //connection string
	 //m_DatasourceConStr = "PG:host='192.168.5.131' port='5437'  dbname='pdatabase' user='postgres' password='postgres'";           
         m_DatasourceConStr  = "PG:host='"+hostname+"' port='"+port+"' dbname='"+dbname+"' user='"+username+"' password='"+pwd+"'";
         if(rankid==0)
          cout<<m_DatasourceConStr<<endl;


	if(strcmp(GetFileExtensionName(m_ResultPathOrTableName).c_str(),"shp")==0)
	{
		m_DatasourceConStr="";
	}
	//cout<<"New m_DatasourceConStr:"<<m_DatasourceConStr<<endl;
	//complete the path
	if(strcmp(pOutPutFile.c_str(),"")!=0)
	{
		if(strcmp(GetFileExtensionName(pOutPutFile).c_str(),"tif")==0)
		{
			m_ResultPathOrTableName = pOutPutFile;
			if(m_ResultPathOrTableName.find_first_of('/')==-1)
			{
				//Get the full path
				GetExePath pexepath;
				pinputFileParaStucture = pexepath.getExeDir()+"data/result/"+pinputFileParaStucture;
				m_ResultPathOrTableName = pexepath.getExeDir()+"data/result/"+m_ResultPathOrTableName;
				for(int i=0;i<p_PathV_InferSamples.size();i++)
				{
					m_PathV_InferSamples.push_back(pexepath.getExeDir()+"data/infer/"+p_PathV_InferSamples[i]);
				}
			}
			if(c_CaculateCols.size()>0)
			{
				if(rankid==0)
				{
					cout<<"******************************"<<endl;
					cout<<"the -c argv will not be used, it will process all input files and all bands"<<endl;
					cout<<"if you need to process specify some ones, you must deal with the image first..."<<endl;
					cout<<"******************************"<<endl;
				}
				c_CaculateCols.clear();
			}
		}
	}
	CContextPersistence xmlContext;
	if(!xmlContext.LoadXML(pinputFileParaStucture))
	{
		// can’t open file or bad file content
		if(rankid==0)
		cout<<"error while loading bayes network..."<<endl;
		exit(0);
	}
	CBNet* pLearnedBnetFromFile = static_cast<CBNet*>(xmlContext.Get("MyBayesModel"));

	if(rankid==0)
	{
		if(m_Debug)
		{
			//Show the learned structure
			ShowLearnedStructure(pLearnedBnetFromFile->GetGraph());
			//Show each node's probability after learning
			ShowProbabilityOfEachNode(pLearnedBnetFromFile);
		}
	}

	//set m_AllBandCount and m_NodesizeArray
	int pNumNodes = pLearnedBnetFromFile->GetGraph()->GetNumberOfNodes();
	m_AllBandCount = pNumNodes;m_NodesizeArray.clear();
	if(rankid==0&&m_Debug)
	{
		cout << "Node size: ";
	}
	for(int i=0;i<pNumNodes;i++)
	{
		if(rankid==0&&m_Debug)
		{
			if(i<pNumNodes-1)
				cout<<pLearnedBnetFromFile->GetNodeType(i)->GetNodeSize()<<",";
			else
				cout<<pLearnedBnetFromFile->GetNodeType(i)->GetNodeSize()<<endl;
		}
		m_NodesizeArray.push_back(pLearnedBnetFromFile->GetNodeType(i)->GetNodeSize());
	}

	Infer(pLearnedBnetFromFile,algorithm2Use);

	delete pLearnedBnetFromFile;

	Mpi_Finish_Moment = MPI_Wtime();

	if(rankid==0)
	{
		ShowExcuteInfor(2);
		//WriteExcuteLog(2);
	}

	MPI_Finalize();

	return 0;
}

/*
int main_cross_valid(int argc, char* argv[])
{
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);

	Mpi_Start_Moment = MPI_Wtime();
	string algorithm2Use="";
	std::vector<std::string> p_PathV_LearnSamples;std::vector<std::string> p_PathV_InferSamples;string pinputFileParaStucture="";
	m_ResultPathOrTableName="";
	int pCrossValidTime = 3; int pCrossValidPart = 3;
	//解析程序中输入的各个参数
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			//数据库连接字符串
			m_DatasourceConStr = "PG:host='192.168.5.131' port='5437'  dbname='pdatabase' user='postgres' password='postgres'";

			if (!strcmp(argv[i], "-a"))
			{
				//algorithm to use
				algorithm2Use = argv[i + 1];
			}
			//the var to infer
			if (!strcmp(argv[i], "-c"))
			{
				//optional argv, so check exceed or null first
				if(i+1<argc)
					if(argv[i + 1][0]!='-')
					{
						string pCaculateCols = argv[i + 1];
						SplitStrFunctor::StrVec svRet;
						SplitStrFunctor ssf;
						ssf(pCaculateCols, ",", svRet);
						JointStrFunctor::StrVecItr iter;
						for (iter = svRet.begin(); iter != svRet.end(); iter++)
						{
							string pEachColValue = *iter;
							c_CaculateCols.push_back(atoi(pEachColValue.c_str()));
						}
					}
			}
			//the col index of decision node
			if (!strcmp(argv[i], "-C"))
			{
				m_IndexOfDecisionNode = atoi(argv[i + 1]);
			}
			//input stucture with parameter
			if (!strcmp(argv[i], "-ips"))
			{
				pinputFileParaStucture = argv[i + 1];
			}
			//cross valid 3*3, 5*5 ...
			if (!strcmp(argv[i], "-cv"))
			{
				m_ResultPathOrTableName = argv[i + 1];

				string tmp = argv[i+1];
				SplitStrFunctor::StrVec svRet;
				SplitStrFunctor ssf;
				ssf(tmp, "*", svRet);
				JointStrFunctor::StrVecItr iter;

				string tmp = *iter;
				pCrossValidTime=atoi(tmp.c_str());
				iter++;
				tmp=*iter;
				pCrossValidPart = atoi(tmp.c_str());
			}
			//file to infer
			if (!strcmp(argv[i], "-pv"))
			{
				m_ResultPathOrTableName = argv[i + 1];

				string tmp = argv[i+1];
				SplitStrFunctor::StrVec svRet;
				SplitStrFunctor ssf;
				ssf(tmp, ",", svRet);
				JointStrFunctor::StrVecItr iter;
				for( iter = svRet.begin(); iter != svRet.end(); iter++ )
				{
					string pEachColValue = *iter;
					p_PathV_InferSamples.push_back(pEachColValue.c_str());
				}
			}
		}
	}
	if(GetFileExtensionName(m_ResultPathOrTableName)=="shp")
	{
		m_DatasourceConStr="";
	}
	if(GetFileExtensionName(m_ResultPathOrTableName)=="tif")
	{
			if(rankid==0)
			{
				cout<<"********cann't caculate image file, please input vector data ************"<<endl;
			}
			exit(0);
	}

	CContextPersistence xmlContext;
	if(!xmlContext.LoadXML(pinputFileParaStucture))
	{
		// can’t open file or bad file content
		if(rankid==0)
		cout<<"error while loading bayes network..."<<endl;
		exit(0);
	}
	CBNet* pLearnedBnetFromFile = static_cast<CBNet*>(xmlContext.Get("MyBayesModel"));


	if(rankid==0)
	{
		//Show the learned structure
		ShowLearnedStructure(pLearnedBnetFromFile->GetGraph());
		//Show each node's probability after learning
		ShowProbabilityOfEachNode(pLearnedBnetFromFile);
	}

	//set m_AllBandCount and m_NodesizeArray
	int pNumNodes = pLearnedBnetFromFile->GetGraph()->GetNumberOfNodes();
	m_AllBandCount = pNumNodes;m_NodesizeArray.clear();
	if(rankid==0)
	{
		cout << "Node size: ";
	}
	for(int i=0;i<pNumNodes;i++)
	{
		if(rankid==0)
		{
			if(i<pNumNodes-1)
				cout<<pLearnedBnetFromFile->GetNodeType(i)->GetNodeSize()<<",";
			else
				cout<<pLearnedBnetFromFile->GetNodeType(i)->GetNodeSize()<<endl;
		}
		m_NodesizeArray.push_back(pLearnedBnetFromFile->GetNodeType(i)->GetNodeSize());
	}

	MPI_Barrier(MPI_COMM_WORLD);
	double pPInferringStartMoment = MPI_Wtime();
	Infer(pLearnedBnetFromFile,algorithm2Use);
	//MPI_Barrier(MPI_COMM_WORLD);
	double pPInferringFinishMoment = MPI_Wtime();
	Mpi_Inferring_Time = pPInferringFinishMoment-pPInferringStartMoment-m_GdalIOInInfer_Time;
	//cout<<"rankid:"<<rankid<<", Mpi_Inferring_Time:"<<Mpi_Inferring_Time<<endl;
	double tmp=Mpi_Inferring_Time;
	MPI_Reduce(&Mpi_Inferring_Time,&tmp,1,MPI_DOUBLE,MPI_MIN,0,MPI_COMM_WORLD);
	Mpi_Inferring_Time = tmp;

	delete pLearnedBnetFromFile;
	MPI_Barrier(MPI_COMM_WORLD);
	Mpi_Finish_Moment = MPI_Wtime();

	if(rankid==0)
	{
		ShowExcuteInfor(2);
		WriteExcuteLog(2);
	}

	MPI_Finalize();

	return 0;
}
*/
