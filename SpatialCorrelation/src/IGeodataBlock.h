#include "IDataBlock.h"
#pragma once


class IGeoDataBlock
	:public IDataBlock{
public:
	virtual ~IGeoDataBlock(){}
	virtual void SerializeTo(void * buffer) = 0;
	virtual void DeserializeFrom(const void * const buffer,int size) = 0;
	virtual size_t Size() = 0;
	virtual size_t DataSize() = 0;
	virtual double GetData(int index, double& x, double& y) = 0;
	virtual double GetData(int index) = 0;
};
