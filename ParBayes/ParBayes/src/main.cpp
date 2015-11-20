/////////////////////////////////////////////////////////////////////////////
#include "DataCvt_Base.h"
extern int main_startinstep(int argc, char* argv[]);
extern int main_stucturelean(int argc, char* argv[]);
extern int main_paralearn(int argc, char* argv[]);
extern int main_infer(int argc, char* argv[]);

using namespace std;

int main(int argc, char* argv[])
{
	//-a strLearn_HC -pl E:/skydrive/Dev/ParallelPrograms/ParBayes-wind/Debug/data/vector/gps_people_s.shp -c 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17 -C 17
	//-is E:/skydrive/Dev/ParallelPrograms/ParBayes-wind/Debug/stucture.xml -ops E:/skydrive/Dev/ParallelPrograms/ParBayes-wind/Debug/bnet.xml
	//	-os E:/skydrive/Dev/ParallelPrograms/ParBayes-wind/Debug/stucture.xml
	//-pi E:/skydrive/Dev/ParallelPrograms/ParBayes-wind/Debug/data/vector/gps_people_m.shp -ips E:/skydrive/Dev/ParallelPrograms/ParBayes-wind/Debug/bnet.xml


	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (!strcmp(argv[i], "-a"))
			{
				//strLearn_* means struct learn, parLearn_* means parameter learn, to2Infer_* means infer
				//strLearn_HC parLearn_EM
				//infer algorithm:  to2Infer_Naive   to2Infer_Gibbs
				string pCalculateInfo = argv[i + 1];
				string pCalculateType=pCalculateInfo.substr(0,8);
				if (!strcmp(pCalculateType.c_str(), "strLearn"))
				{
					main_stucturelean(argc, argv);
				}
				if (!strcmp(pCalculateType.c_str(), "parLearn"))
				{
					main_paralearn(argc, argv);
				}
				if (!strcmp(pCalculateType.c_str(), "to2Infer"))
				{
					main_infer(argc, argv);
				}
			}
		}
	}
	

	return 0;
}
