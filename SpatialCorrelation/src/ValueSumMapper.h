#pragma once

#include "IDataBlock.h"
#include "IGeodataBlock.h"
#include "IMapOperator.h"
#include "ArrayDataBlock.h"


class ValueSumMapper:
	public IMapOperator{
public:
	virtual IDataBlock * operator ()(IDataBlock * in) {
		if(dynamic_cast<IGeoDataBlock* >(in)){
			IGeoDataBlock* p = (IGeoDataBlock*)in;
			double d = 0;
			size_t size = p->DataSize();
			for(size_t i=0;i<size;i++)
			{
				d += p->GetData(i);
			}
			return new ArrayDataBlock(1,d);			
		} else {
			return new ArrayDataBlock();
		}
	};
};

