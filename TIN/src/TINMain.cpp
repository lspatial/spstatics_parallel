#pragma once
//#include "mpi.h" 
#include <stdio.h>
#include <math.h>
#include "time.h" 
#include <iostream>
#include <string>
#include <cstdlib>
#include "GouTIN.h"
#define MAXLN 4096
using namespace std;



int main(int argc,char **argv)
{
	/*********定义要外部输出的变量*********/

        string pLayerName = argv[1];
       char *resultFile(argv[2]);
	string dbname=argv[3];string hostname=argv[4];string username=argv[5];string pwd=argv[6];string port=argv[7];
        string m_DatasourceConStr  = "PG:host='"+hostname+"' port='"+port+"' dbname='"+dbname+"' user='"+username+"' password='"+pwd+"'";
        
        //cout<<m_DatasourceConStr<<endl;

	CGouTIN gouTIN(0);
	gouTIN.TIN_Main(m_DatasourceConStr.c_str(),pLayerName.c_str(),resultFile);

}