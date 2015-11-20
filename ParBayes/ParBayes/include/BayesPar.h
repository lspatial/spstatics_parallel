/////////////////////////////////////////////////////////////////////////////

#ifndef _BAYESPAR_H_


#define BUILD_MPI

#include "pnl_dll.hpp"
#include <fstream>
#include "mpi.h"
#include "gdal_priv.h"
#include <ogrsf_frmts.h>
#include "DataCvt_Base.h"

PNL_USING

double Mpi_Start_Moment,Mpi_Finish_Moment;
double Mpi_StructureLearning_Time,Mpi_ParameterLearning_Time,Mpi_Inferring_Time;
int rankid,numproc;

std::string m_DatasourceConStr;std::string m_PathOrTableName;std::string m_ResultPathOrTableName;
std::string pFileExtensionName;int RowNum;int ColNum;
std::vector <int> c_CaculateCols;
int m_IndexOfDecisionNode=-1;

double m_PD_value=0;//only vector can get pd value

bool m_Debug = false;

//raster
std::vector<std::string> m_PathV_LearnSamples;std::vector<std::string> m_PathV_InferSamples;
int m_RowNum;int m_ColNum;int m_AllBandCount;
double m_NoDataValue;int m_ValidCellNums;double m_InferNoDataValue;
double ** m_AllBandsData;GByte * m_AllBandsClassData;

vector<int> m_NodesizeArray;

int main_startinstep(int argc, char* argv[]);
int main_stucturelean(int argc, char* argv[]);
int main_paralearn(int argc, char* argv[]);
int main_infer(int argc, char* argv[]);

#endif //_BAYESPAR_H_
