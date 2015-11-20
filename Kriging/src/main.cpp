/* 
 * File:   main.cpp
 * Author: ricepig
 *
 */

#include <cstdlib>
#include "ANN/ANN.h"
#include "ogrsf_frmts.h"
#include "gdal_priv.h"
#include "mpi.h"

#define EPS 0.00000001

using namespace std;

typedef struct output_info {
	double left;  
	double top;
	double pixelSize;		// pixel width(height)
	int nXSize;		// pixel count on x-axis
	int nYSize;		// pixel count on y-axis
	float * pValues;	// values to fill into the raster
} output_info;

typedef struct extent_info {
	double minX;
	double maxX;
	double minY;
	double maxY;
} extent_info;

int read_vector(const char* connstr, const char* layer, int fieldIdx, ANNpointArray* ptArray, double** ptValues, int& count, extent_info& extent, char** pSpatialRefWkt)
{
	
	OGRDataSource *poDS = OGRSFDriverRegistrar::Open( connstr, FALSE );
    if( poDS == NULL )
    {
        printf( "[ERROR] Open failed.\n" );
        exit( 1 );
    }
	
    OGRLayer *poLayer = poDS->GetLayerByName(layer);
	
	OGRSpatialReference * sref= poLayer->GetSpatialRef();
	sref->exportToWkt(pSpatialRefWkt);

    OGRFeature *poFeature;

    poLayer->ResetReading();
	count = 0;
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
		count++;
	}
	poLayer->ResetReading();
	
	*ptArray = annAllocPts(count, 2);
	*ptValues = new double[count];
	
	int idx = 0;
	double x,y;
	while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
        double value = poFeature->GetFieldAsDouble(fieldIdx);
		(*ptValues)[idx] = value;
		
        OGRGeometry *poGeometry;

        poGeometry = poFeature->GetGeometryRef();
        if( poGeometry != NULL 
            && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
        {
            OGRPoint *poPoint = (OGRPoint *) poGeometry;
			x = poPoint->getX();
			y = poPoint->getY();
			(*ptArray)[idx][0] = x;
			(*ptArray)[idx][1] = y;
			if(idx==0)
			{
				extent.minX = extent.maxX = x;
				extent.minY = extent.maxY = y;
			}
			else
			{
				if(x>extent.maxX) 
					extent.maxX = x;
				if(x<extent.minX)
					extent.minX = x;
				if(y>extent.maxY)
					extent.maxY = y;
				if(y<extent.minY)
					extent.minY = y;
			}
        }
        else
        {
            printf( "[ERROR] No point geometry\n" );
			return 1;
        }       
        OGRFeature::DestroyFeature( poFeature );
		idx++;
    }

    OGRDataSource::DestroyDataSource( poDS );
	return 0;
}

int forward_substitution(double** a, int n) 
{
	int i, j, k, max;
	double t,s;
	for (i = 0; i < n; ++i) 
	{
		// find the row whose i'th element is max in the column
		max = i;
		for (j = i + 1; j < n; ++j)
			if (a[j][i] > a[max][i])
				max = j;
		// swap the max row and the first row;
		if(max != i)
		{
			for (j = 0; j < n + 1; ++j) {
				t = a[max][j];
				a[max][j] = a[i][j];
				a[i][j] = t;
			}
		}
		
		s = a[i][i];
		if(s<EPS && s>-EPS) 
			return 1;
		
		// substitute
		for (j = n; j >= i; --j)
			for (k = i + 1; k < n; ++k)
				a[k][j] -= a[k][i]/s * a[i][j];
	}
	return 0;
}

void reverse_elimination(double** a, int n) 
{
	int i, j;
	for (i = n - 1; i >= 0; --i) {
		a[i][n] = a[i][n] / a[i][i];
		a[i][i] = 1;
		
		for (j = i - 1; j >= 0; --j) {
			a[j][n] -= a[j][i] * a[i][n];
			a[j][i] = 0;
		}
	}
}

int guass_eliminator(double** a, int n)
{
	if(forward_substitution(a, n) == 1) return 1;
	reverse_elimination(a, n);
	return 0;
}

inline double variation_spheroid(double h, double c, double cc, double a)
{
	if(h<EPS)
		return 0;
	if(h>a)
		return c+cc;
	
	double t = h/a;
	return c + cc * (t*3/2 - t*t*t/2);
}

inline double distance(ANNpoint p1, ANNpoint p2)
{
	double x = p1[0] - p2[0];
	double y = p1[1] - p2[1];
	return sqrt(x*x + y*y);
}

void dump_matrix(double** matrix, int xsize, int ysize)
{
	for(int i=0;i<ysize;i++)
	{
		for(int j=0;j<xsize;j++)
		{
			printf("%f;", matrix[i][j]);
		}
		printf("\n");
	}
}


void fill_matrix(int k, double c, double cc, double** matrix)
{
	for(int i=0;i<k;i++)
	{
		matrix[i][i] = c + cc;
	}
	
	for(int i=0;i<k;i++)
	{
		matrix[k][i] = 1;
		matrix[i][k] = 1;
	}
	
	matrix[k][k] = 0;
	matrix[k][k+1] = 1;
}


/*
 * params:
 * k: k-th nearest neighbors			
 * c: nugget
 * cc: 
 * a: range
 * h: lag 
 */
double kriging_spheroid(ANNpointArray ptArray, double* ptValues, ANNkd_tree* tree, 
		ANNpoint queryPoint, int k, double c, double cc, double a, ANNidxArray nnIdx, ANNdistArray dists, double** matrix)
{
	tree->annkSearch(queryPoint, k, nnIdx, dists);
	
	int i,j;
	double value;
	ANNpoint p,q;
	for(i=0;i<k;i++)
	{
		p = ptArray[nnIdx[i]];
		for(j=i+1;j<k;j++)
		{
			q = ptArray[nnIdx[j]];
			value = variation_spheroid(distance(p, q), c, cc, a);
			matrix[i][j] = c+cc-value;
			matrix[j][i] = c+cc-value;
		}
	}
	
	for(i=0;i<k;i++)
	{
		p = ptArray[nnIdx[i]];
		value = variation_spheroid(distance(queryPoint, p), c, cc, a);
		matrix[i][k+1] = c+cc-value;
	}
	
	//dump_matrix(matrix, k+2, k+1);
	
	if(guass_eliminator(matrix, k+1)==1){
		
		//dump_matrix(matrix, k+2, k+1);
		return 0;
	}
		
	
	value = 0;
	for(i=0;i<k;i++)
	{
		value += matrix[i][k+1] * ptValues[nnIdx[i]];
	}
	
	
	return value;
}

void kriging(ANNpointArray ptArray, double* ptValues, ANNkd_tree* tree, int k, double c, double cc, double a, output_info& output)
{
	ANNidxArray nnIdx = new ANNidx[k];
	ANNdistArray dists = new ANNdist[k];
	ANNpoint queryPoint = annAllocPt(2);
	double** matrix = new double*[k+1];
	int i=0;
	for(i = 0;i<k+1;i++)
		matrix[i] = new double[k+2];
	
	double right = output.left + output.pixelSize * output.nXSize;
	double bottom = output.top - output.pixelSize * output.nYSize;
	double halfpixel = output.pixelSize/2;
	int index = 0;
	for(double y = output.top;y>bottom;y-=output.pixelSize)
	{
		for(double x = output.left;x<right;x+=output.pixelSize)
		{
			queryPoint[0] = x + halfpixel;
			queryPoint[1] = y - halfpixel;
			fill_matrix(k, c, cc, matrix);
			double val = kriging_spheroid(ptArray, ptValues, tree, queryPoint, 
					k, c, cc, a, nnIdx, dists, matrix);
			output.pValues[index] = (float)val; 
			index++;
		}
	}

	for(i=0;i<k+1;i++)
		delete[] matrix[i];
	delete[] matrix;
	
	annDeallocPt(queryPoint);
	delete[] nnIdx;
	delete[] dists;
	
}

void Test()
{
	int k = 4;
	ANNpointArray ptArray = annAllocPts(k, 2);
	double * ptValues = new double[k];
	ptArray[0][0] = 50.0;
	ptArray[0][1] = 50.0;
	ptArray[1][0] = 100.0;
	ptArray[1][1] = 0.0;
	ptArray[2][0] = 200.0;
	ptArray[2][1] = 100.0;
	ptArray[3][0] = 0.0;
	ptArray[3][1] = 150.0;
	
	ANNpoint queryPoint = annAllocPt(2);
	queryPoint[0] = 50.0;
	queryPoint[1] = 100.0;
	
	ANNkd_tree* tree = new ANNkd_tree(ptArray, k, 2);
	ANNidxArray nnIdx = new ANNidx[k];
	ANNdistArray dists = new ANNdist[k];
	
	double** matrix = new double*[k+1];
	int i=0;
	for(i = 0;i<k+1;i++)
		matrix[i] = new double[k+2];
	
	fill_matrix(k, 2,20, matrix);
	double value = kriging_spheroid(ptArray, ptValues, tree, queryPoint, k, 2, 20, 200,
			nnIdx, dists, matrix);
	for(i=0;i<k+1;i++)
		delete[] matrix[i];
	delete[] matrix;
	
	delete[] nnIdx;
	delete[] dists;
	delete tree;
	
	annDeallocPt(queryPoint);
	annDeallocPts(ptArray);
	
	printf("%f\n", value);
}

int create_raster(const char* filename, double left, double top, int nXSize, int nYSize, double pixelSize, char* spatialRefWkt )
{
	const char *pszFormat = "GTiff";
	
    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
    if( poDriver == NULL )
	{
		printf("[ERROR] Can't find the driver for writing GeoTiff.\n");
		return 1;
	}
    
	GDALDataset *poDstDS;       
    char **papszOptions = NULL;
    poDstDS = poDriver->Create( filename, nXSize, nYSize, 1, GDT_Float32, 
                                papszOptions );  
	
	if(poDstDS == NULL)
	{
		printf("[ERROR] Can't create the raster file as output.\n");
		return 1;
	}
	
	double adfGeoTransform[6] = {left, pixelSize, 0, top, 0, -pixelSize};
	poDstDS->SetGeoTransform(adfGeoTransform);
	
	poDstDS->SetProjection(spatialRefWkt);
	CPLFree(spatialRefWkt);
	GDALClose((GDALDatasetH)poDstDS);
	return 0;
}

int open_raster(const char* filename, GDALDataset ** pDS, GDALRasterBand** pBand)
{
	*pDS = (GDALDataset*)GDALOpen(filename, GA_Update);
	if(*pDS == NULL)
	{
		printf("[ERROR] Can't open the output file.\n");
		return 1;
	}
	*pBand = (*pDS)->GetRasterBand(1);
	return 0;
}

int close_raster(GDALDataset *pDS)
{
	GDALClose((GDALDatasetH)pDS);
	return 0;
}


int index_of_1(int i)
{
	if(i<0)
		return 0;
	
	int count=0;
	while(i){
		if(i&1)
			return count;
		count++;
		i=i>>1;
	}
	return 0;
}


void print_usage()
{
	printf("Kriging Interpolation Program Usage:\n");
	printf("kriging [hostname] [port] [dbname] [user] [password] [input layer] [output file] [field index] [cellsize] [number of nearest points] [nugget] [height of arc] [range]\n");
}
/*
 * 
 */
int main(int argc, char** argv) {
	
	/*if(argc != 9)
	{
		print_usage();
		return 0;
	}*/
	
	GDALAllRegister();
	OGRRegisterAll();
	MPI_Init(&argc, &argv);
	int tid, numprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &tid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	double pixelSize = std::atof(argv[9]); 
	int k = std::atoi(argv[10]);
	int fldIndex = std::atoi(argv[8]);
	double c = std::atof(argv[11]);
	double cc = std::atof(argv[12]);
	double a = std::atof(argv[13]);
	string hostname(argv[1]);
	string  port(argv[2]);
	string dbname(argv[3]);
	string username(argv[4]);
	string password(argv[5]);
	string layer(argv[6]);
	string dstFile(argv[7]);
	string connstr = "PG:host='" + hostname + "' port='" + port + "' dbname='" + dbname + "' user='" + username + "' password='" + password + "'";

	if(tid==0){
        cout<<"[DEBUG] [OPTIONS] hostname:"<<hostname<<endl;
	cout<<"[DEBUG] [OPTIONS] port:"<<port<<endl;
	cout<<"[DEBUG] [OPTIONS] dbname:"<<dbname<<endl;
	cout<<"[DEBUG] [OPTIONS] user:"<<username<<endl;
	cout<<"[DEBUG] [OPTIONS] password:"<<password<<endl;
        cout<<"[DEBUG] [OPTIONS] output file:"<<dstFile<<endl;
		cout<<"[DEBUG] [OPTIONS] field index:"<<fldIndex<<endl;
		cout<<"[DEBUG] [OPTIONS] cell size:"<<pixelSize<<endl;
        cout<<"[DEBUG] [OPTIONS] number of nearest points:"<<k<<endl;
        cout<<"[DEBUG] [OPTIONS] nugget:"<<c<<endl;
        cout<<"[DEBUG] [OPTIONS] height of arc:"<<cc<<endl;
        cout<<"[DEBUG] [OPTIONS] range:"<<a<<endl;
    }
	
	ANNpointArray pArray;
	
	double* values;
	int count;
	char* spatialrefWkt;
	
	extent_info extent;
	
	// Stage 1: Read data;
	double t1 = MPI_Wtime();
	
	if(read_vector(connstr.c_str(), layer.c_str(), fldIndex, &pArray, &values, count, extent, &spatialrefWkt)==1)
	{
		return 1;
	}
	double t2 = MPI_Wtime();
	
	// Stage 2: Construct kd-tree for search
	ANNkd_tree* tree = new ANNkd_tree(pArray, count, 2);
	
	// Stage 3: Create Raster file
	int nXSize = (int)((extent.maxX - extent.minX)/pixelSize) +1;
	int nYSize = (int)((extent.maxY - extent.minY)/pixelSize) +1;
	if(tid==0)
	{
		create_raster(dstFile.c_str(), extent.minX, extent.maxY, nXSize, nYSize, pixelSize, spatialrefWkt);
	}
	int node = numprocs;
	// if the squre root of node is int (node can be divded by 2)
	int i_1 = index_of_1(node) + 1;
	int col = (i_1 & 1) == 0? node >> ((i_1-1) >> 1):node>>(i_1 >> 1);
	int row = node / col;
	if(nXSize<nYSize) 
	{
		// swap
		col += row ;
		row = col - row;
		col -= row;
	}
	
	int xStride = (nXSize - 1)/col + 1;
	int yStride = (nYSize - 1)/row + 1;
	
	
	GDALDataset * pDS;
	GDALRasterBand* pBand;
	
	if(tid==0)
		open_raster(dstFile.c_str(), &pDS, &pBand);
		
	output_info output;
	output.pixelSize = pixelSize;
    
    int j= tid % col;
	output.left = extent.minX + j * xStride * pixelSize;
	output.nXSize = (j+1)*xStride>nXSize?nXSize-j*xStride:xStride;
    
    int i = tid / col;
	
    double localTop = extent.maxY - i * yStride * pixelSize;
	int localYSize = (i+1)*yStride>nYSize?nYSize-i*yStride:yStride;
	
    output.nYSize = 1;
	
	//cout<<"[DEBUG] Thread " << tid << ", xsize: " << output.nXSize <<", ysize: "<< output.nYSize;
	//cout<<", xStride: "<<xStride<<", yStride: "<<yStride<<endl;
    
    output.pValues = new float[xStride];
    
    float* buffer = new float[xStride];
    
    double t01,t02;
    double atime = 0;
    
    for(int n=0;n<yStride;n++)
    {
        output.top = localTop-n*pixelSize;
        
        if(n<localYSize)
        {
            kriging(pArray, values, tree, k, c, cc, a, output);
        }
        
        t01 = MPI_Wtime();
        if(tid==0)
        {
            pBand->RasterIO(GF_Write, j*xStride, i*yStride+n, output.nXSize, 1, (void*)output.pValues, output.nXSize, 1, GDT_Float32, 0, 0);
 
            int nXSize2;
            
            MPI_Status status;
            for(int k=1;k<numprocs;k++)
            {
                int li = k / col;
                int lj = k % col;
                
                if(li*yStride+n<nYSize)
                {
                    nXSize2 = (lj+1)*xStride>nXSize?nXSize-lj*xStride:xStride;
                    MPI_Recv(buffer, nXSize2, MPI_FLOAT, k, 0, MPI_COMM_WORLD, &status);
					pBand->RasterIO(GF_Write, lj*xStride, li*yStride+n, nXSize2, 1, (void*)buffer, nXSize2, 1, GDT_Float32, 0, 0);
                }
                
            }
        }
        else
        {
			if(n<localYSize){
				MPI_Send((void*)output.pValues, output.nXSize*output.nYSize, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);   
			}
        }
        t02 = MPI_Wtime();
        atime += (t02-t01);
    }
    
	delete[] output.pValues;
    delete[] buffer;
    
    /*
	output.top = extent.maxY - i * yStride * pixelSize;
	output.nYSize = (i+1)*yStride>nYSize?nYSize-i*yStride:yStride;
	output.pValues = new float[output.nXSize*output.nYSize];
			
	kriging(pArray, values, tree, k, c, cc, a, output);
	
	double t3 = MPI_Wtime();
	if(tid==0)
	{
		pBand->RasterIO(GF_Write, j*xStride, i*yStride, output.nXSize, output.nYSize, (void*)output.pValues, output.nXSize, output.nYSize, GDT_Float32, 0, 0);
		delete[] output.pValues;
		
		int nYSize2, nXSize2;
			nXSize2 = (j+1)*xStride>nXSize?nXSize-j*xStride:xStride;
			
			buffer = new float[nXSize2*nYSize2];
			MPI_Recv(buffer, nXSize2*nYSize2, MPI_FLOAT, k, 99, MPI_COMM_WORLD, &status);
			
			pBand->RasterIO(GF_Write, j*xStride, i*yStride, nXSize2, nYSize2, (void*)buffer, nXSize2, nYSize2, GDT_Float32, 0, 0);
		
			delete [] buffer;
		}
	}
	else
	{
		MPI_Send((void*)output.pValues, output.nXSize*output.nYSize, MPI_FLOAT, 0, 99, MPI_COMM_WORLD);
		delete[] output.pValues;
	}
	*/
    
	if(tid==0)
		close_raster(pDS);
	double t4 = MPI_Wtime();
	
	if(tid==0){
        cout<<"[DEBUG][TIMESPAN][IO] "<< t2-t1+atime  << endl;
        cout<<"[DEBUG][TIMESPAN][COMPUTING] "<< t4-t2-atime << endl;
        cout<<"[DEBUG][TIMESPAN][TOTAL]"<< t4-t1 << endl;  
    }
	
	delete tree;
	annDeallocPts(pArray);
	delete[] values;
	
	annClose();
	
	//MPI_Barrier(MPI_COMM_WORLD);
	//cout<<"[DEBUG] Thread " << tid << " finished."<< endl;
	MPI_Finalize();
	//cout<<"[DEBUG] Thread " << tid << " finalized."<< endl;
}


