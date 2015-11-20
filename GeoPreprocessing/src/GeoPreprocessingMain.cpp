// GeoStatMain.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "DataCvt_Log.h"
#include "DataCvt_Nml.h"
#include "DataCvt_Discrete.h"

int main(int argc, char* argv[])
{
	int pIndexAlgorithm = 0;//Ĭ�ϵ���log�㷨
	//��������������ĸ�������
	if (argc>1)
	{
		for (int i=1;i<argc;i++)
		{
			//����·��
			if (!strcmp(argv[i],"-a"))
			{
				pIndexAlgorithm = atoi(argv[i+1]);
			}
		}
	}
	switch(pIndexAlgorithm)
	{
		case 0:
			{
				DataCvt_Log pDataCvt_Log;
				pDataCvt_Log.main_log(argc, argv);
				break;
			}
		case 1:
			{
				DataCvt_Nml pDataCvt_Nml;
				pDataCvt_Nml.main_nml(argc, argv);
				break;
			}
		case 2:
			{
				DataCvt_Discrete pDataCvt_Dis;
				pDataCvt_Dis.main_dis(argc, argv);
				break;
			}
		default:break;
	}
	
	
	

	return 0;
}

