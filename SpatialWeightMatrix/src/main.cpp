/* 
 * File:   main.cpp
 * Author: Ricepig
 *
 * Created on 2012年10月21日, 上午12:16
 */


#include "spatial_weight.h"
#include <iostream>
#include "stdlib.h"
using namespace std;

int main (int argc, char * argv[])
{
    int rank, size;

    MPI_Init (&argc, &argv);
    OGRRegisterAll();
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    /*if(argc != 13){
        if(rank==0)
        {
            cout<<"[ERROR] Wrong arguments, there should be 13 args."<<endl;
            cout<<"[ERROR] Args: <input file> <id field index> <output file> <exponent> <distance type> <standarized>"<<endl;
        }
        MPI_Finalize();
        return 1;
    }*/
 
    double start = MPI_Wtime();
    int index = atoi(argv[2]);
    int exp = atoi(argv[4]);
    int type = atoi(argv[5]);
    if(type != 0 && type != 1){
        if(rank==0)
        cout<<"[ERROR] The Argument 'distance type' should be 0 or 1, 0 denotes euclid distance and 1 denotes manhattan distance."<<endl;
        MPI_Finalize();
        return 1; 
    }
    int stan = atoi(argv[6]);
    
	std::string connstr = "PG:host='";
    connstr += argv[7];
    connstr += "' port='";
    connstr += argv[8];
    connstr += "' dbname='";
    connstr += argv[9];
    connstr += "' user='";
    connstr += argv[10];
    connstr += "' password='";
    connstr += argv[11];
    connstr += "'";
    
    if(rank==0){
        cout<<"[DEBUG] [OPTIONS] input file:"<<argv[1]<<endl;
        cout<<"[DEBUG] [OPTIONS] id field index:"<<index<<endl;
        cout<<"[DEBUG] [OPTIONS] out file:"<<argv[3]<<endl;
        cout<<"[DEBUG] [OPTIONS] distance type:"<<type<<endl;
        cout<<"[DEBUG] [OPTIONS] exponent:"<<exp<<endl;
        bool stan2 = stan == 1;
        cout<<"[DEBUG] [OPTIONS] standarized:"<<stan2<<endl;
        cout<<"[DEBUG] [OPTIONS] conn string:"<<connstr<<endl;
    }
    
    double calc_time = calc_weight(connstr.c_str(), argv[1], index, argv[3], exp, type, stan == 1, rank, size); 
    double stop = MPI_Wtime();
    double total_time = stop - start;
    
    if(rank==0){
        cout<<"[DEBUG][TIMESPAN][IO] "<< total_time - calc_time << endl;
        cout<<"[DEBUG][TIMESPAN][COMPUTING] "<< calc_time << endl;
        cout<<"[DEBUG][TIMESPAN][TOTAL]"<< total_time << endl;  
    }
    MPI_Finalize();
    return 0;
}

