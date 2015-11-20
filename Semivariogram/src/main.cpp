/* 
 * File:   main.cpp
 * Author: ricepig
 *
 */

#include <cstdlib>
#include <string>
#include <iostream>
#include "ogrsf_frmts.h"
#include "ElementContainer.h"
#include "LagContainer.h"
#include "GuassianElimination.h"
#include "math.h"
#include "mpi.h"

#define EPSILON 0.0000001

using namespace std;

void fatal(const char* msg)
{
    printf("Fatal error: %s\n", msg);
    exit(0);
}

void print_usage()
{
	printf("Semivariogram Fitting Program Usage:\n");
	printf("Semivariogram [input file] [field index] [lag] [lag count]\n");
}

struct ElementContainer loadData(const char * connstr, const char* layer, int fieldIndex, double samplingRate, int rankSize, int myRank, int *pcount)
{
	OGRDataSource* poDS = OGRSFDriverRegistrar::Open(connstr, FALSE );
    if(poDS == NULL)
        fatal("Can't open the data source.");
    OGRLayer  *poLayer = poDS->GetLayerByName(layer);
    if(poLayer == NULL)
        fatal("Can't open the data source.");
    
    struct ElementContainer ec;
    ECInit(&ec);
    
    OGRFeature *poFeature;
    int count = 0;
	int localcount = 0;
	int stride = (int)(1 / samplingRate);
    poLayer->ResetReading();
	
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
		if(count % stride == 0){        
			if(localcount % rankSize == myRank){
				OGRGeometry *poGeometry = poFeature->GetGeometryRef();
				if(poGeometry != NULL){

					if(wkbFlatten(poGeometry->getGeometryType()) != wkbPoint)
						fatal("The layer type is restricted to point.");

					OGRPoint *poPoint = (OGRPoint*)poGeometry;
					float value = (float)poFeature->GetFieldAsDouble(fieldIndex);
					//if(value>0)
						ECAdd(&ec, poPoint->getX(), poPoint->getY(), value);
				}
			}
			localcount++;            
        }
        OGRFeature::DestroyFeature( poFeature );       
		count++;
    }
	
	int * counts = new int[2];
	counts[0] = ec.Length;
	MPI_Allreduce(counts, counts+1, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	
    OGRDataSource::DestroyDataSource( poDS );
    *pcount = counts[1];
//	cout << "Node " << myRank << ":"<<ec.Length<<","<<*pcount<<endl;
    return ec;
}

inline void inner_classify(struct Element* poE1, struct Element* poE2, struct LagContainer *lc, double lag, double range)
{
    double dist = sqrt((poE1->X - poE2->X) * (poE1->X - poE2->X) + (poE1->Y - poE2->Y) * (poE1->Y - poE2->Y));
    if(dist < range)
    {
        int index = dist/lag;
        (lc->counts[index])++;
        (lc->sums[index]) += fabs(poE1->Value - poE2->Value);
    }
}

void classify_cross(struct ElementContainer* ec1, struct ElementContainer* ec2, struct LagContainer *lc, double lag, int lagCount)
{
    double range = lag * lagCount;
    struct Element* poE1 = ec1->Head;
    
    for(size_t i=0;i<ec1->Length;i++, poE1++){
        struct Element* poE2 = ec2->Head;
        for(size_t j=0;j<ec2->Length;j++, poE2++)
            inner_classify(poE1, poE2, lc, lag, range);
    }
	
    //printf("cross: %d*%d=%d\n", ec1->Length, ec2->Length, ec1->Length * ec2->Length);
}

void classify_self(struct ElementContainer* ec, struct LagContainer *lc, double lag, int lagCount)
{
    double range = lag * lagCount;
    struct Element* poE1 = ec->Head;
    for(size_t i=0;i<ec->Length;i++, poE1++){
        struct Element* poE2 = ec->Head;
        for(size_t j=0;j<ec->Length;j++, poE2++){
			if(i!=j)
				inner_classify(poE1, poE2, lc, lag, range);
		}
    }
	
	//printf("self: %d*%d=%d\n", ec->Length, ec->Length, ec->Length * ec->Length - ec->Length);
}

double ** new_matrix(int row, int col)
{
    double ** matrix = new double*[row];
    for(int i=0;i<row;i++){
        matrix[i] = new double[col];
    }
	return matrix;
}

void delete_matrix(double ** matrix, int row)
{
    for(int i=0;i<row;i++)
        delete[] matrix[i];
    delete[] matrix;
}

#ifdef DEBUG

void print_matrix(double** matrix, int sizex, int sizey)
{
    printf("\nMatrix: %d x %d\n", sizey,sizex);
    for(int i=0;i<sizey;i++) 
    {
        for(int j=0;j<sizex;j++) 
            printf("v:%.4f\t",matrix[i][j]);
        printf("\n"); 
    }
}

void print_lags(struct LagContainer * lc){
	for(int i=0;i<lc->size;i++)
	{
		printf("lag %d: sum: %f, count: %d\n", i+1, lc->sums[i], lc->counts[i]);
	}
		
}
#endif

void OLS_spheroid(struct LagContainer * lc, double lag, int rankSize, int myRank, double * c0, double *c, double *a)
{
#ifdef DEBUG
	print_lags(lc);
#endif
    if(myRank == 0){
		
        double ** matrix = new_matrix(3, 4);
        for(int i=0;i<lc->size;i++)
        {
			if(lc->counts[i]>0){
				double x1 = lag * i + lag / 2;
				double d2 = x1 * x1;
				double x2 = - d2 * x1;
				matrix[0][0]++;
				matrix[0][1] += x1;
				matrix[0][2] += x2;
				matrix[0][3] += lc->sums[i];
				matrix[1][1] += d2;
				matrix[1][2] += x1 * x2;
				matrix[1][3] += lc->sums[i] * x1;
				matrix[2][2] += x2 * x2;
				matrix[2][3] += lc->sums[i] * x2;
			}
        }
        matrix[1][0] = matrix[0][1];
        matrix[2][0] = matrix[0][2];
        matrix[2][1] = matrix[1][2];
        
#ifdef DEBUG
        print_matrix(matrix, 4, 3);
#endif        
        guass_eliminator(matrix, 3);
        *c0 = matrix[0][3];
        *a = sqrt(matrix[1][3]/matrix[2][3]/3);
        *c = matrix[1][3] * (*a) * 2 / 3;
        delete_matrix(matrix, 3);
    }
    
}

int variogram(const char* connstr, const char* layer, int fieldIndex, double lag, int lagCount, double samplingRate, int rankSize, int myRank)
{
    if(myRank == 0){
	cout<<"[DEBUG] [OPTIONS] conn string:"<<connstr<<endl;
        cout<<"[DEBUG] [OPTIONS] input layer:"<<layer<<endl;
        cout<<"[DEBUG] [OPTIONS] field index:"<<fieldIndex<<endl;
		cout<<"[DEBUG] [OPTIONS] lag:"<<lag<<endl;
		cout<<"[DEBUG] [OPTIONS] lag count:"<<lagCount<<endl;
    }
    int count;
    
    double t1 = MPI_Wtime();
    struct ElementContainer ec = loadData(connstr, layer, fieldIndex, samplingRate, rankSize, myRank, &count);
    double t2 = MPI_Wtime();
    struct LagContainer lc;
    LCInit(&lc, lagCount);
    
    int blockSize = (count + rankSize - 1)/rankSize;
    struct ElementContainer ec2;
	ECInitWithSize(&ec2, blockSize);
	
    for(int i=0;i<rankSize;i++)
    {
        int currentSize = (blockSize-1)*rankSize + i < count?blockSize:blockSize-1;
		if(i==myRank){
			MPI_Bcast(ec.Head, currentSize*sizeof(struct Element), MPI_BYTE, i, MPI_COMM_WORLD);
			//cout<<"[DEBUG] node "<<myRank<<", sent size："<<currentSize<<endl;
            classify_self(&ec, &lc, lag, lagCount);
        }else{
			MPI_Bcast(ec2.Head, currentSize*sizeof(struct Element), MPI_BYTE, i, MPI_COMM_WORLD);
			ec2.Length = currentSize;
			//cout<<"[DEBUG] node "<<myRank<<", calc size："<<currentSize<<endl;
			classify_cross(&ec, &ec2, &lc, lag, lagCount);
			
        }
    }
	
	struct LagContainer lc2;
    LCInit(&lc2, lagCount);
    MPI_Allreduce(lc.counts, lc2.counts, lc.size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(lc.sums, lc2.sums, lc.size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	
    LCCalcAverage(&lc2);
    double c, c0, a;
    OLS_spheroid(&lc2, lag, rankSize, myRank, &c0, &c, &a);
    double t3 = MPI_Wtime();
    LCDestory(&lc2);
    ECDestory(&ec2);
    LCDestory(&lc);
    ECDestory(&ec);
    
    if(myRank==0){
        cout<<"[DEBUG][TIMESPAN][IO] "<< t2-t1  << endl;
        cout<<"[DEBUG][TIMESPAN][COMPUTING] "<< t3-t2 << endl;
        cout<<"[DEBUG][TIMESPAN][TOTAL]"<< t3-t1 << endl;
        cout << "[OUTPUT] C0: " << c0 << endl;
        cout << "[OUTPUT] C: " << c << endl;
        cout << "[OUTPUT] a: " << a << endl;
    }  
	return 0;
}

int main(int argc, char** argv) {
    
    if(argc <11){
        print_usage();
        return 0;
    }
    
    OGRRegisterAll();
	MPI_Init(&argc, &argv);
	int tid, numprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &tid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    
    int fieldIndex = atoi(argv[2]);
    double lag = atof(argv[3]);
    int lagcount = atoi(argv[4]);
	double percentage = argc==5? 1.0:atof(argv[5]);
	string host(argv[6]);
	string port(argv[7]);
	string dbname(argv[8]);
	string user(argv[9]);
	string password(argv[10]);

	string connstr = "PG:host='" + host + "' port='" + port + "' dbname='" + dbname + "' user='" + user + "' password='" + password + "'";

    if(lag < EPS || lagcount == 0){
        print_usage();
        MPI_Finalize();
        return 0;
    }
    
    variogram(connstr.c_str(), argv[1], fieldIndex, lag, lagcount, percentage, numprocs, tid);
    MPI_Finalize();
    return 0;
}



