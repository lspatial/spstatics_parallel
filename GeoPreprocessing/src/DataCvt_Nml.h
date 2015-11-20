//防止重复包含引起link错误
#ifndef _DATACVT_NML_H

#define _DATACVT_NML_H

#include <iostream>
#include <math.h>

#include "gdal_priv.h"
#include "mpi.h"

#include "DataCvt_Base.h"

class DataCvt_Nml
{
private:
	double Mpi_Start_Moment,Mpi_Finish_Moment;
	double Mpi_Caculate_Time;
	double m_GdalIOInCal_Time;

	int rankid,numproc;
	bool m_Debug;

	std::string m_PathOrTableName;std::string m_ResultPathOrTableName;
	std::string m_DatasourceConStr;
	std::string pFileExtensionName;int m_RowNum;int m_ColNum;int m_BandCount;

	std::vector <int> c_CaculateCols;

	void CalAsImage();

	void CalAsShp();

	void ShowExcuteInfor();

	void WriteExcuteLog();

public:
	DataCvt_Nml()
	{
		Mpi_Caculate_Time = 0;m_GdalIOInCal_Time=0;
		m_RowNum = 0;m_ColNum = 0;m_BandCount = 1;
		rankid = 0;numproc=0;
		m_Debug = false;
	}
	int main_nml(int argc, char* argv[]);
};



#endif				//_NORMALIZATION_H
