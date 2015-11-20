/* 
 * File:   main.cpp
 * Author: Ricepig
 *
 * Created on 2012年10月21日, 下午11:19
 */

#include <cstdlib>
#include "mpi.h"
#include "ogrsf_frmts.h"
#include "gdal.h"
#include "ImageReader.h"
#include "MoranICalculator.h"
#include "CearyCCalculator.h"
#include "GetisGCalculator.h"
#include "VectorReader.h"
using namespace std;

/*
 * 
 */
int main(int argc, char* argv[]) {

    MPI_Init(&argc,&argv);
    
	int myid,numprocs;
    
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    if((argv[0][0] == '0' && argc != 10)||(argv[0][0]=='1' && argc !=5)){
        if(myid==0){
            cout<<"[ERROR] Wrong arguments, there should be 4 args."<<endl;
            cout<<"[ERROR] Args: <input type> <file name> <field index> <type>"<<endl;
        }
        MPI_Finalize();
        return 1;
    }
   
    GDALAllRegister();
	OGRRegisterAll();
    int index = 0;
    int type2=0;
    int type=atoi(argv[1]);
    char * host;
	char* port;
	char* dbname;
	char* user;
	char* password;
	char* layer;
	if(type==0)
	{
		host=argv[2];
		port=argv[3];
		dbname=argv[4];
		user=argv[5];
		password=argv[6];
		layer=argv[7];
		index=atoi(argv[8]);
		type2=atoi(argv[9]);
	}
	else
	{
		layer=argv[2];
		index=atoi(argv[3]);
		type2=atoi(argv[4]);
	}
    if(myid==0){
        cout<<"[DEBUG] [OPTIONS] input type:";
        if(type==0){
		
            cout<<"Vector"<<endl;
		host = argv[2];
		port = argv[3];
		dbname=argv[4];
		user = argv[5];		
		password=argv[6];
		layer=argv[7];
		index = atoi(argv[8]);
		type2 = atoi(argv[9]);

		cout<<"[DEBUG] [OPTIONS] hostname: "<< host<<endl;
		cout<<"[DEBUG] [OPTIONS] port: "<<port<<endl;
		cout<<"[DEBUG] [OPTIONS] dbname: "<<dbname<<endl;
		cout<<"[DEBUG] [OPTIONS] user: "<<user<<endl;
		cout<<"[DEBUG] [OPTIONS] password: "<<password<<endl;


	}
        else{

            cout<<"Raster"<<endl;
		layer=argv[2];
		index=atoi(argv[3]);
		type2=atoi(argv[4]);
		cout<<"[DEBUG] [OPTIONS] input file: "<<layer<<endl;
	}
	cout<<"[DEBUG] [OPTIONS] field index: " << index << endl;
        cout<<"[DEBUG] [OPTIONS] type:";
        switch(type2)
        {
            case 0:
                cout<<"Moran's I"<<endl;
                break;
            case 1:
                cout<<"Ceary's C"<<endl;
                break;
            case 2:
                cout<<"Getis General-Ord G"<<endl;
                break;
        }
        
    }
    
    
    
	VectorReader vreader;
	RasterReader rreader;
	
	MoranICalculator mcalc(myid, numprocs);
	CearyCCalculator ccalc(myid, numprocs);
	GetisGCalculator gcalc(myid, numprocs);

	CalculatorBase* calc;
	
	if(type2 == 0)
		calc = &mcalc;
	else if(type2==1)
		calc = &ccalc;
	else
		calc = &gcalc;
	if(type == 0){
        //vreader.Open( "/home/Ricepig/caseForWS.shp", 1);
        string url("PG:host='");
	url = url + host + "' port='" + port + "' dbname='" + dbname + "' user='" + user + "' password='" + password + "'";
        if(vreader.Open(url.c_str(),layer, index) == 1){
            MPI_Finalize();
            return 1;
        }
		calc->OpenDataSource(vreader);
	}else{
        //rreader.Open("/home/Ricepig/data2.tif", 1);
        if(rreader.Open(layer,index) == 1){
            MPI_Finalize();
            return 1;
        }
		calc->OpenDataSource(rreader);
    }
    if(myid==0)
	cout<<"[DEBUG] [INFO] Start calculating..."<<endl;
	calc->Run();
    MPI_Finalize();
    return 0;
}

