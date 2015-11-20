#pragma once

#include <vector>
#include "IDataBlock.h"
#include "IGeoReader.h"
#include "IMapOperator.h"
#include "cpl_conv.h"
#include "mpi.h"
#include "IReduceOperator.h"
#include "ArrayDataBlock.h"
#include "IMultiplexOperator.h"
#include "IGeodataBlock.h"
#include "ImageDataBlock.h"
#include "VectorDataBlock.h"
#include <iostream>

using namespace std;
class CalculatorBase
{
protected:
	vector<IDataBlock*> * srcData;
	IGeoReader * reader;
	int myid;
	int numprocs;
	size_t max_buffer_size;
	int geotype; //դ����ʸ��

public:
	CalculatorBase(int procsid, int procsnum ){
		srcData = new vector<IDataBlock*>();
		myid = procsid;
		numprocs = procsnum;
		max_buffer_size = 2048*sizeof(float);
	}

	virtual void Run() = 0;
	
	virtual void OpenDataSource(IGeoReader& reader){
		this->reader = &reader;
	}
	
	virtual void RetrieveSizeInfo(size_t& count, size_t& blockcount){		
		size_t * buffer = (size_t*)CPLMalloc(2*sizeof(size_t));
		if(this->myid == 0)
		{
			count = reader->GetSizeInfo(blockcount);
			buffer[0] = count;
			buffer[1] = blockcount;
		}

		MPI_Bcast(buffer, 2*sizeof(size_t),  MPI_CHAR, 0, MPI_COMM_WORLD);
		count = buffer[0];
		blockcount = buffer[1];
        
		CPLFree(buffer);
	}

	virtual vector<IDataBlock*>& Map(vector<IDataBlock*>& src, IMapOperator & op)
	{
		vector<IDataBlock*>* results = new vector<IDataBlock*>();
		for(size_t i=0;i<src.size();i++){
			ImageDataBlock * b = (ImageDataBlock*)src[i];			
			IDataBlock* item = op(src[i]);
			results->push_back(item);			
		}
		return *results;
	}

	virtual vector<IDataBlock*>& Multiplex(vector<IDataBlock*>&src, IMultiplexOperator & op, size_t blockcount)
	{
		int currentblock = 0;
		int maxblock = (blockcount-1)/numprocs + 1;	
        
		int localblocks = 0;

        /*int* blocksizes = (int*)CPLMalloc(numprocs);
		memset(blocksizes, 0, numprocs*sizeof(int));*/

		void * buffer = NULL;		
        void* bf2 = NULL;
        bf2 = malloc(sizeof(size_t));
		IDataBlock * geoBlock = NULL;

		vector<IDataBlock*> * results = new vector<IDataBlock*>();
		size_t size2 = src.size();
        results->resize(size2, NULL);
		for(size_t i=0;i<size2;i++)
		{
			IDataBlock& b = op.CreateDataBlock(*(src[i]));
            (*results)[i] = &b;
//			results->push_back(&b);
		}

		for(size_t i=0;i<blockcount;i++)
		{
			// ��ǰҪ�㲥��������ڵĽ��
			int localid = i % numprocs;
			// ��ݿ��ڽ�����ǵڼ���
			int blockindex = i / numprocs;
			
            if(myid==localid)
            {
                ((size_t*)bf2)[0] = src[blockindex]->Size();
            }

            MPI_Bcast(bf2, sizeof(size_t), MPI_CHAR, localid, MPI_COMM_WORLD);


			size_t size = ((size_t*)bf2)[0];
			
			buffer = CPLMalloc(size);			
			if(myid == localid)
			{
				src[blockindex]->SerializeTo(buffer);
			}

			MPI_Bcast(buffer, size, MPI_CHAR, localid, MPI_COMM_WORLD);

			geoBlock = this->reader->Deserailize(buffer, size);
			
			/*VectorDataBlock * srcblk = (VectorDataBlock *)src[blockindex];
			VectorDataBlock * destblk = (VectorDataBlock*) geoBlock;
			for(size_t j=0;j<srcblk->DataSize();j++)
			{
				if(srcblk->values[j] != destblk->values[j])
					printf("%d %f != %f\r\n",j , srcblk->values[j], destblk->values[j]);
			}*/

			CPLFree(buffer);
			/*if(myid==localid)
			{
				printf("%d block broadcasted\r\n", i);
			}*/
			
			buffer = NULL;

			for(size_t j=0;j<src.size();j++)
			{
				IDataBlock& result = *(results->operator[](j));
				op(*src[j], *geoBlock, result);
			}

			delete geoBlock;
			geoBlock = NULL;
			
		
        }
        CPLFree(bf2);
		return *results;	
	}

	virtual IDataBlock& Reduce(IDataBlock & src)
	{
		ArrayDataBlock* psrc = dynamic_cast<ArrayDataBlock*>(&src);
		if(psrc != NULL){	
			size_t size = psrc->Size();
			void * buffer = CPLMalloc(size);
			void * buffer2 = CPLMalloc(size);
			psrc->SerializeTo(buffer2);
			double p = *((double*)buffer2);
            MPI_Allreduce(buffer2, buffer, psrc->DataSize(), MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);			
            p = *((double*)buffer);
			ArrayDataBlock* result = new ArrayDataBlock();
			result->DeserializeFrom(buffer, psrc->Size());
			CPLFree(buffer2);
			CPLFree(buffer);
			return *result;
		}
		else{
			throw "The only support reduce type is array data block.";
		}
	}

	virtual IDataBlock& Reduce(vector<IDataBlock*>& src, IReduceOperator &op)
	{
		IDataBlock * nodesum = op(src);
		//double avg = ((ArrayDataBlock*) nodesum)->Values->at(0);
		IDataBlock& sum = Reduce(*nodesum);
		delete nodesum;		
		return sum;
	}

	virtual void LoadData(size_t blockcount)
	{
		void* buffer = CPLMalloc(this->max_buffer_size);
		if(this->myid == 0){
			IDataBlock * pblock = this->reader->NextBlock();
			int destPid = 0;
			while(pblock != NULL){
				if(destPid == 0){
					srcData->push_back(pblock);
				}else{
					size_t size = pblock->Size();
					pblock->SerializeTo(buffer);
					MPI_Send(buffer, size, MPI_CHAR, destPid, 0, MPI_COMM_WORLD);				
				}
				pblock = this->reader->NextBlock();

				destPid++;
				if(destPid >= this->numprocs)
					destPid = 0;
			}
		}else{
			MPI_Status status;
			int size;
			for(size_t i=myid;i<blockcount;i+=numprocs)
			{	
                MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_CHAR, &size);
				MPI_Recv(buffer, size, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				IDataBlock * pblock = this->reader->Deserailize(buffer, size);
				srcData->push_back(pblock);				
			}
		}
		CPLFree(buffer);
        MPI_Barrier(MPI_COMM_WORLD);
	}

	virtual void Delete(vector<IDataBlock> *p)
	{
		while(p->size() > 0)
		{
			IDataBlock& item = p->at(p->size()-1);
			p->pop_back();
			delete &item;
		}
		delete p;
	}
};

