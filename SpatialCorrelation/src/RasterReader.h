#pragma once

#include "gdal.h"
#include "cpl_conv.h" 
#include "IGeoReader.h"
#include "ImageDataBlock.h"

class RasterReader:
	public IGeoReader
{
public:
	int Open(const char* filename, int band)		
	{
		this->hDataset = GDALOpen(filename, GA_ReadOnly );
		if( this->hDataset == NULL ){			
			std::cout<<"[ERROR] Can't open the specified raster file: "<< filename << std::endl;
            return 1;
		}
	
		double adfGeoTransform[6];
		if( GDALGetGeoTransform( this->hDataset, adfGeoTransform ) == CE_None ){
			this->orgX = adfGeoTransform[0];
			this->orgY = adfGeoTransform[3];
			this->pixelX = adfGeoTransform[1];
			this->pixelY = adfGeoTransform[5];        
		}
		
		this->hBand = GDALGetRasterBand( this->hDataset, band );		
		this->nXSize = GDALGetRasterBandXSize( hBand );
		this->nYSize = GDALGetRasterBandYSize( hBand );
		this->sizeX = this->nXSize;
		this->sizeY = 1;
		this->offsetX = 0;
		this->offsetY = 0;
        return 0;
	}

	void GetDataBlock(int xoffset, int yoffset, int sizex, int sizey, void * pBuffer, GDALDataType type)
	{
		GDALRasterIO( this->hBand, GF_Read, xoffset, yoffset, sizex, sizey,
						pBuffer, sizex, sizey, type,
						0, 0 );
	}

	virtual IDataBlock* NextBlock(){
		int xSize = this->offsetX + this->sizeX > this->nXSize ? this->nXSize - this->offsetX : this->sizeX;
		int ySize = this->offsetY + this->sizeY > this->nYSize ? this->nYSize - this->offsetY : this->sizeY;

		if(xSize <=0 || ySize<=0)
			return NULL;

		void * buffer = CPLMalloc(xSize * ySize * sizeof(double));

		GDALDataType dtype = GDT_Float64;
		/*if(typeid(T) == typeid(float)){
			dtype = GDT_Float32;
		}else{
			if(typeid(T) == typeid(int)){
				dtype = GDT_Int32;
			}else{ 
				throw "This type of data is not supported, it only supports float32 or int32.";
			}
		}*/
		
		GDALRasterIO( this->hBand, GF_Read, this->offsetX, this->offsetY, this->sizeX, this->sizeY, 
						buffer, this->sizeX, this->sizeY, dtype, 0,0);
		
		ImageDataBlock * block = new ImageDataBlock(
			this->orgX + this->offsetX*this->pixelX, 
			this->orgY + this->offsetY*this->pixelY, 
			this->pixelX, this->pixelY, ySize, xSize, buffer);
		

		this->offsetX += xSize;
		// next line
		if(this->offsetX>=this->nXSize)
		{
			this->offsetX = 0;
			this->offsetY += ySize;
		}

		return block;
	}

	virtual IDataBlock* Deserailize(void * buffer, int size){
		ImageDataBlock *block = new ImageDataBlock();
		block->DeserializeFrom(buffer, size); 
		return block;
	}

	virtual size_t GetSizeInfo(size_t & blockcount) {
		blockcount = this->nYSize;
		return this->nXSize * this->nYSize;
	};

	virtual ~RasterReader()
	{
		//GDALClose(hDataset);
	}
	
private:
	int sizeX;
	int sizeY;

	GDALDatasetH  hDataset;	
	int nXSize;
	int nYSize;
	double orgX;
	double orgY;
	double pixelX;
	double pixelY;
	GDALRasterBandH hBand;
	
	int offsetX;
	int offsetY;
};
