#pragma once

#include "IDataBlock.h"
#include <vector>
#pragma once

using namespace std;

class ArrayDataBlock
	:public IDataBlock{
public:	
	vector<double> Values;

	ArrayDataBlock(size_t count){
		Values.resize(count, 0);
        /*for(size_t i=0;i<count;i++){
			Values.push_back(0);			
		}*/
	}

	ArrayDataBlock(size_t size, double value){
		Values.resize(size, value);
        /*for(size_t i=0;i<size;i++)
			Values.push_back(value);*/
	}

	ArrayDataBlock(){ }
	
	virtual ~ArrayDataBlock(){
	}

	virtual void Add(ArrayDataBlock* block2){
		size_t size1 = Values.size();
		size_t size2 = block2->Values.size();
		for(size_t i=0;i<size1;i++){
			if(i<size2)
				Values[i] += block2->Values[i];
		}
	}

	virtual void Add(double value){
		size_t size1 = Values.size();
		for(size_t i=0;i<size1;i++){
			Values[i] += value;
		}
	}
	
	virtual void SerializeTo(void * buffer){
		memcpy(buffer, &(Values[0]), Values.size()*sizeof(double));
	}

	virtual void DeserializeFrom(const void * const buffer, int size) {
		Values.clear();
		double * p = (double*)buffer;
		size_t size2 = size/sizeof(double);
        Values.resize(size2, 0);
		
        for(size_t i=0;i<size2;i++){
            Values[i] = p[i];
		}
	}
	
	virtual size_t DataSize() {
		return Values.size();		
	}

	virtual size_t Size() {
		return (Values.size())*sizeof(double);
	}
};

