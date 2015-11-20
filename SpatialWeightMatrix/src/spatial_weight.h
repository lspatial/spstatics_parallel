/* 
 * File:   spatial_weight.h
 * Author: Ricepig
 */

#include "ogrsf_frmts.h"
#include <vector>
#include <fstream>
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "mpi.h"

#ifndef SPATIAL_WEIGHT_H
#define	SPATIAL_WEIGHT_H
#define BATCH_SIZE 32
typedef struct point_feature {
	int id;
	float value;
	double x;
	double y;	
} pt_feature;

using namespace std;
double abs2(double x){
    return x>0?x:-x;
}
int readfile(const char * connstr, const char * filename, int idfieldindex, vector<pt_feature> &pts)
{	
	
    
    OGRDataSource *poDS;
    poDS = OGRSFDriverRegistrar::Open( connstr, FALSE );
    if( poDS == NULL )
    {
		printf("[ERROR] Input file '%s' open failed\r\n", connstr );
		return 1;        
    }
    
    OGRLayer  *poLayer;
	poLayer = poDS->GetLayerByName(filename);
	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();

	OGRFieldDefn *fdef = poFDefn->GetFieldDefn(idfieldindex);
	if( fdef->GetType() != OFTInteger)
	{
		printf("[ERROR] Invalid id field, error value type (%d)\r\n", fdef->GetType() );
		return 1;
	}

    OGRFeature *poFeature;	
    poLayer->ResetReading();

    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {	
        pt_feature * ppt = new pt_feature();
        ppt->id = poFeature->GetFieldAsInteger(idfieldindex);
        OGRGeometry *poGeometry;
        poGeometry = poFeature->GetGeometryRef();
        if( poGeometry != NULL 
            && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
        {
            OGRPoint *poPoint = (OGRPoint *) poGeometry;
            ppt->x = poPoint->getX();
            ppt->y = poPoint->getY();
            pts.push_back(*ppt);            
        }
        OGRFeature::DestroyFeature( poFeature );
		
    }
    
    OGRDataSource::DestroyDataSource( poDS );
    
	return 0;
}

inline void manhattan_distance(pt_feature &pt1, vector<pt_feature> &pts, double * p_results, bool square)
{
    size_t size = pts.size();
    double x,y;
    int exp = square?2:1;
    for(size_t i=0;i<size;i++)
    {
        pt_feature & p = pts[i];
        x = abs2(pt1.x - p.x);
        y = abs2(pt1.y - p.y);
        p_results[i] = x==0&&y==0?1: 1/pow((x*x + y*y), exp);
    }
}

inline void euclid_distance(pt_feature &pt1, vector<pt_feature> &pts, double * p_results)
{
    size_t size = pts.size();
    double x,y;
    for(size_t i=0;i<size;i++)
    {
        pt_feature & p = pts[i];
        x = pt1.x - p.x;
        y = pt1.y - p.y;
        p_results[i] = x==0&&y==0?1:1/sqrt(x*x + y*y);
    }
}

inline void square_euclid_distance(pt_feature &pt1, vector<pt_feature> &pts, double * p_results)
{
    size_t size = pts.size();
    double x,y;
    for(size_t i=0;i<size;i++)
    {
        pt_feature & p = pts[i];
        x = pt1.x - p.x;
        y = pt1.y - p.y;
        p_results[i] = x==0&&y==0?1:1/(x*x + y*y);
    }
}

inline void standarize(double * p_weights, size_t count)
{
    double sum = 0;
    for(size_t i=0;i<count;i++){
        sum+= p_weights[i];
    }
    for(size_t i=0;i<count;i++){
        p_weights[i] /= sum;
    }
}

int writefile(ofstream & fout, int from, vector<pt_feature> & p_to, double * p_weights, int count)
{
    for(int i=0;i<count;i++)
    {
        //fout.write( (char*)(&from), sizeof(int)/sizeof(char) );
        //fout.write( (char*)(&(p_to[i].id)), sizeof(int)/sizeof(char) );
        //fout.write( (char*)(p_weights+i), sizeof(double)/sizeof(char) );
        if(i>0)
            fout<<" ";
        fout << p_weights[i];
    }
    fout<<endl;
}

double calc_weight(const char* connstr, const char * inputfile, int idfieldindex, const char * outputfile,
        bool square, int distance_type, bool row_standarized, int nodeid, int numprocs)
{
    
    vector<pt_feature> pts;
    double calc_time = 0;
    double start, stop;
    
    if(readfile(connstr, inputfile, idfieldindex, pts) == 1)
    {
        
        return 1;
    }
   
    size_t size = pts.size();
    char * temp = (char*)malloc((strlen(outputfile)+4)*sizeof(char));
    sprintf(temp, "%s.%d", outputfile, nodeid);
    
    ofstream fout;
    
    fout.open(temp, ios::trunc);
    
    double *p_results = (double*)malloc(size*sizeof(double));
    
    for(size_t i=nodeid;i<size;i+=numprocs)
    {
        start = MPI_Wtime();
        pt_feature &pt = pts[i];
        if(distance_type == 0)
        {
            if(square){
                square_euclid_distance(pt, pts, p_results);
            }else{
                euclid_distance(pt, pts, p_results);
            }
        }
        else if(distance_type == 1)
        {
            manhattan_distance(pt, pts, p_results, square);
        }
        
        if(row_standarized)
           standarize(p_results, size);
        stop = MPI_Wtime();
        calc_time += stop - start;
        
        writefile(fout, pt.id, pts, p_results, size);
       
    }
    fout.close();
    double * dbl = (double*)malloc(sizeof(double));
    *dbl = calc_time;
    double * dbl2 = (double*)malloc(sizeof(double));
    
    MPI_Reduce(dbl, dbl2, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    free(dbl);
    double maxtime = *dbl2;
    free(dbl2);
    if(nodeid==0)
        return maxtime;
    else
        return 0;
}



#endif	/* SPATIAL_WEIGHT_H */

