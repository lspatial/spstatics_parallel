#pragma once

#include "IDataBlock.h"

class IMapOperator
{
public:
	virtual IDataBlock* operator ()(IDataBlock* in) = 0;
};

