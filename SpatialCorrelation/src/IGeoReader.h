#pragma once

#include "IDataBlock.h"

class IGeoReader{
public:
	virtual IDataBlock* NextBlock() = 0;
	virtual size_t GetSizeInfo(size_t & blockcount) = 0;
	virtual IDataBlock* Deserailize(void * buffer, int size) = 0;
};

