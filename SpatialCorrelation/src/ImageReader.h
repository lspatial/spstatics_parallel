#include "gdal.h"
#include "cpl_conv.h" 


#ifndef   _IMAGEREADER_FILE_H 
#define   _IMAGEREADER_FILE_H 

class ImageReader
{
public:
	ImageReader(const char* filename, int band)		
	{
		this->hDataset = GDALOpen(filename, GA_ReadOnly );
		if( this->hDataset == NULL ){			
			throw 1;
		}
	
		double adfGeoTransform[6];
		this->xSize = GDALGetRasterXSize( this->hDataset );
		this->ySize = GDALGetRasterCount( this->hDataset );

		if( GDALGetGeoTransform( this->hDataset, adfGeoTransform ) == CE_None ){
			this->orgX = (float)adfGeoTransform[0];
			this->orgY = (float)adfGeoTransform[3];
			this->pixelX = (float)adfGeoTransform[1];
			this->pixelY = (float)adfGeoTransform[5];        
		}
		
		this->hBand = GDALGetRasterBand( this->hDataset, band );
		
		this->nXSize = GDALGetRasterBandXSize( hBand );
		this->nYSize = GDALGetRasterBandYSize( hBand );
	}

	void GetMetadata(int& sizex, int& sizey, float& orgx, float& orgy, float& pixelsizex, float& pixelsizey)
	{
		sizex = this->nXSize;
		sizey = this->nYSize;
		orgx = this->orgX;
		orgy = this->orgY;
		pixelsizex = this->pixelX;
		pixelsizey = this->pixelY;
	}

	void GetDataBlock(int xoffset, int yoffset, int sizex, int sizey, void * pBuffer, GDALDataType type)
	{
		GDALRasterIO( this->hBand, GF_Read, xoffset, yoffset, sizex, sizey,
						pBuffer, sizex, sizey, type,
						0, 0 );
	}

	virtual ~ImageReader()
	{
	}
	
private:
	GDALDatasetH  hDataset;
	int xSize;
	int ySize;
	int nXSize;
	int nYSize;
	float orgX;
	float orgY;
	float pixelX;
	float pixelY;
	GDALRasterBandH hBand;
};

#endif

