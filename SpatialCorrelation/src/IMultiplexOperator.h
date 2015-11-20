#pragma once
#include "IDataBlock.h"
#include <vector>
class IMultiplexOperator
{
public:
	virtual IDataBlock& CreateDataBlock(IDataBlock& in) = 0;
	virtual void operator ()(IDataBlock& in1, IDataBlock& in2, IDataBlock& out) = 0;	
};

