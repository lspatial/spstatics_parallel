#pragma once

#include "IDataBlock.h"
#include "IMapOperator.h"
#include "ArrayDataBlock.h"


class GetisGMultiplexMapper:
	public IMapOperator{
public:
    
	virtual IDataBlock * operator ()(IDataBlock * in) {
		if(dynamic_cast<ArrayDataBlock* >(in)){
			ArrayDataBlock* block = (ArrayDataBlock*)in;
			
			
			ArrayDataBlock * block2 = new ArrayDataBlock(5);
			block2->Values[0] = block->Values[0];
			block2->Values[1] = block->Values[1];
			block2->Values[2] = block->Values[2];
			block2->Values[3] = block->Values[3];
			double v = 0, sum=0;
			size_t size = block->Values.size();
			for(size_t i=4;i<size;i++)
			{
				v = block->Values[i];
				sum += v * v;
			}
			block2->Values[4] = sum;
			return block2;			
		} else {
			return new ArrayDataBlock(5);
		}
	};
};

