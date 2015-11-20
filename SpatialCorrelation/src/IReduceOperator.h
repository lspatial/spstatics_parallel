#pragma once
#include "IDataBlock.h"
#include <vector>

using namespace std;

class IReduceOperator
{
public:
	virtual IDataBlock * operator ()(vector<IDataBlock *>& in) = 0;
};

