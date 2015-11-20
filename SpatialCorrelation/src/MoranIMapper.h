#pragma once

#include "IDataBlock.h"
#include "IGeodataBlock.h"
#include "IMapOperator.h"
#include "ArrayDataBlock.h"


class MoranIMapper:
	public IMapOperator{
private:	
	double average;	
public:
    MoranIMapper(double avg)
	{		
		average = avg;
	}

	virtual IDataBlock * operator ()(IDataBlock * in) {
		if(dynamic_cast<IGeoDataBlock* >(in)){
			IGeoDataBlock* p = (IGeoDataBlock*)in;
			double v = 0, vs = 0, vq = 0;
			double v1, vs1;
		
			size_t size = p->DataSize();

			for(size_t i=0;i<size;i++)
			{
				v1 = p->GetData(i) - average;
				v += v1;
				vs1 = v1 * v1;
				vs += vs1;
				vq += vs1 * vs1;
			}

			ArrayDataBlock * block = new ArrayDataBlock(3);
			block->Values[0] = v;
			block->Values[1] = vs;
			block->Values[2] = vq;
			return block;			
		} else {
			return new ArrayDataBlock(3);
		}
	};
};

