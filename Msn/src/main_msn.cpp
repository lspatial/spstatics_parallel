#include "lis.h"
#include <iostream>
#include "mpi.h"
#include "Matrix.h"
#include "MSN_Helper.h"
#include <math.h>
#include <string.h>
using namespace std;

void MSN(Config &config)
{
	LIS_INT nprocs, my_rank;
#ifdef USE_MPI
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
#else
	nprocs  = 1;
	my_rank = 0;
#endif

	//Watch time
	double calcStarttime, calcEndtime;
	double totStarttime;

	totStarttime = MPI_Wtime();

	//Load samples, block discrete points, semi-variogram models
	Matrix samples;
	Matrix blockpts;
	SvModels models;
	MSN_Helper::LoadSamples(config, samples);
	MSN_Helper::LoadBlockPoints(config, blockpts);
	MSN_Helper::LoadSvParams(config, models);
	
        MPI_Barrier(MPI_COMM_WORLD);
	if(my_rank == 0)
		calcStarttime = MPI_Wtime();

	LIS_MATRIX C;
	LIS_VECTOR D, x;
	LIS_SOLVER solver;
	LIS_INT is, ie, gn;
	
	//����C����
	gn = config.nSamples+config.nStrata;
	lis_matrix_create(LIS_COMM_WORLD, &C);	//��������C���ڸ��ڵ�ֲ�ʽ�洢��
	lis_matrix_set_size(C, 0, gn);			//C����Ĵ�С��ȫ�֣� gn��gn
	lis_matrix_get_range(C, &is, &ie);		//C�����ڸýڵ������ŷ�Χ [is, ie)

	//��������
	char sn[128];
	sprintf(sn, "%d", config.nSamples);
	double dblZoomIn = sqrt(pow(10.0, (int)strlen(sn)-1));
	//cout << "Zoom factor: " << dblZoomIn << endl;
	if(dblZoomIn > 500) dblZoomIn = 500;

#ifdef _DEBUG
	Matrix mC;
	MSN::FillCMatrix(C, samples, models, config, dblZoomIn).CopyTo(mC);
#else
	MSN::FillCMatrix(C, samples, models, config, dblZoomIn);
#endif

	//����D����
	lis_vector_create(LIS_COMM_WORLD, &D);
	lis_vector_duplicate(C, &D);
	MSN::FillDVector(D, samples, blockpts, models, config, dblZoomIn);
	
	//����x����
	lis_vector_create(LIS_COMM_WORLD, &x);
	lis_vector_duplicate(C, &x);
	
	//��ⷽ��
	lis_solver_create(&solver);	
	//lis_solver_set_option("-i bicg", solver);
	//lis_solver_set_option("-tol 1.0e-15", solver);
	//lis_solver_set_option("-maxiter 2000", solver);
	lis_solver_set_optionC(solver);
	lis_solve(C, D, x, solver);
	
	//gather
	Vector wu(gn, true);
	MSN::Gather(x, wu, nprocs, my_rank, 0);
	
#ifdef _DEBUG
	Matrix Ca(gn, gn);
	MSN::Gather(mC, Ca, is, ie, nprocs, my_rank, 0);
#endif

	Vector di(gn, true);
	MSN::Gather(D, di, nprocs, my_rank, 0);
	
	double caa = MSN::CAA(blockpts, models, config, nprocs, my_rank, 0);
	

        MPI_Barrier(MPI_COMM_WORLD);

	if (my_rank == 0)
	{
		//printf("w\n");
		//wu.Print();
		
		//LIS_INT iter;
		//double times;
		//lis_solver_get_iters(solver,&iter);
		//lis_solver_get_time(solver,&times);
		//printf("iter= %d time = %e\n",iter,times);

		Vector wt, mu;
		MSN::AbstractWtLg(wu, samples, wt, mu, config);
		
#ifdef _DEBUG
		MSN_Helper::Save(Ca, "./C.csv");
		MSN_Helper::Save(di, "./D.csv");
		MSN_Helper::Save(wu, "./w.csv");
#endif

		double mn = MSN::EstPopulationMean(samples, wt);
		double var = MSN::EstPopulationVar(caa, wu, di, config, dblZoomIn);
		calcEndtime = MPI_Wtime();		

		cout << "�����ֵ: " << mn << endl;
		cout << "��ֵ���Ʒ���: " << var << endl;
		//cout << "�ܺ�ʱ: " << calcEndtime - totStarttime << " ��" << endl;
		//cout << "�����ʱ: " << calcEndtime - calcStarttime << " ��" << endl;
		
		double timeAll, timeCalc, timeIO;
		timeAll = calcEndtime - totStarttime;
		timeCalc = calcEndtime - calcStarttime;
		timeIO = calcStarttime - totStarttime;
		cout << "[DEBUG][TIMESPAN][IO]" << timeIO << endl;
		cout << "[DEBUG][TIMESPAN][COMPUTING]" << timeCalc << endl;
		cout << "[DEBUG][TIMESPAN][TOTAL]" << timeAll << endl;
	}
	
	lis_matrix_destroy(C);
	lis_vector_destroy(D);
	lis_vector_destroy(x);
	lis_solver_destroy(solver);
}

LIS_INT main(LIS_INT argc, char* argv[])
{
	lis_initialize(&argc, &argv);
//	if(argc != 2) return(0);

	Config config;
	MSN_Helper::LoadConfig(argv[1], config);
	MSN(config);

	lis_finalize();
	return 0;
}
