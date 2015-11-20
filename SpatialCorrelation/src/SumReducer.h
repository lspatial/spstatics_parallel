#pragma once

#include "IDataBlock.h"
#include "IGeodataBlock.h"
#include "IMapOperator.h"
#include "ArrayDataBlock.h"
#include "IReduceOperator.h"
#include <vector>
#include "float.h"

class SumReducer:
	public IReduceOperator{
public:  
	
	virtual IDataBlock * operator ()(vector<IDataBlock *>& in) {
		if(dynamic_cast<ArrayDataBlock*>(in[0])){
			ArrayDataBlock *p = (ArrayDataBlock*)in[0];
			ArrayDataBlock *block = new ArrayDataBlock(p->Values.size());
			for(size_t i=0;i<in.size();i++){
				ArrayDataBlock *p = (ArrayDataBlock*)in[i];	
				block->Add(p);
			}
			return block;
		}else{
			throw "SumReducer only supports op among single value block or array data block.";
		}
		
	}
};

