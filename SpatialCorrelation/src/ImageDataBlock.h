#pragma once
#include "IDataBlock.h"
#include "cpl_conv.h" 
#include "IGeodataBlock.h"


class ImageDataBlock:public IGeoDataBlock
{

public:
	double orgX;
	double orgY;	
	double sizeX;
	double sizeY;
	size_t rows;
	size_t cols;
	double* data;

	ImageDataBlock(){
		orgX = 0;
		orgY = 0;
		sizeX = 0;
		sizeY = 0;
		rows = 0;
		cols = 0;
		data = NULL;
	}

	ImageDataBlock(double orgx, double orgy, double sizex, double sizey, int rowcount, int colcount, void * buffer ){
		orgX = orgx;
		orgY = orgy;
		sizeX = sizex;
		sizeY = sizey;
		rows = rowcount;
		cols = colcount;
		data = (double*) buffer;
		/*data = (T*)CPLMalloc(rowcount*colcount*sizeof(T));
		memcpy(data, buffer, rowcount*colcount*sizeof(T));*/
	}

	virtual ~ImageDataBlock(){
		if(data != NULL)
			CPLFree(data);
	}

	virtual void SerializeTo(void * buffer){
		double * p = (double*)buffer;
		p[0] = this->orgX;
		p[1] = this->orgY;
		p[2] = this->sizeX;
		p[3] = this->sizeY;
		p[4] = (double)this->rows;
		p[5] = (double)this->cols;
		if(data != NULL)
			memcpy((unsigned char*)buffer + sizeof(double)*6, this->data, rows * cols * sizeof(double));
	}

	virtual void DeserializeFrom(const void * const buffer, int size) {
		double * p = (double*)buffer;
		this->orgX = p[0];
		this->orgY = p[1];
		this->sizeX = p[2];
		this->sizeY = p[3];
		this->rows = (size_t)p[4];
		this->cols = (size_t)p[5];

		if(this->data != NULL)
		{
			CPLFree(this->data);
			this->data = NULL;
		}
		
		size_t headersize = sizeof(double)*6;
		this->data = (double*) CPLMalloc(size-headersize);
		memcpy(this->data, (unsigned char*)buffer + headersize, size-headersize); 
	}
	
	virtual size_t Size() {
		return rows*cols*sizeof(double)+sizeof(double)*6;
	}

	virtual size_t DataSize() {
		return rows*cols;
	}

	virtual double GetData(int index, double& x, double& y) {
		size_t size = this->Size();
		int row = (int)index/this->cols;
		int col = index % this->cols;
		x = this->orgX + col * this->sizeX;
		y = this->orgY + col * this->sizeY;
		return this->data[index];
	};

	virtual double GetData(int index){
		return this->data[index];
	};

	
};

