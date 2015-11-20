#pragma once

#include "IDataBlock.h"
#include "IGeodataBlock.h"
#include "IMapOperator.h"
#include "ArrayDataBlock.h"

class GetisGMapper:
	public IMapOperator{
public:
    
	virtual IDataBlock * operator ()(IDataBlock * in) {
		if(dynamic_cast<IGeoDataBlock* >(in)){
			IGeoDataBlock* p = (IGeoDataBlock*)in;
			double v = 0, v2 = 0, v3 = 0, v4 = 0;
			double s1 = 0, s2 = 0, s3 = 0, s4 = 0;
			size_t size = p->DataSize();

			for(size_t i=0;i<size;i++)
			{
				v = p->GetData(i);
				v2 = v * v;
				v3 = v * v2;
				v4 = v2 * v2;
				s1+=v;
				s2+=v2;
				s3+=v3;
				s4+=v4;
			}

			ArrayDataBlock * block = new ArrayDataBlock(4);
			block->Values[0] = s1;
			block->Values[1] = s2;
			block->Values[2] = s3;
			block->Values[3] = s4;
			return block;			
		} else {
			return new ArrayDataBlock(4);
		}
	};
};

