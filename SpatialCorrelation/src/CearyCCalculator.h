#pragma once
#include "CalculatorBase.h"
#include "RasterReader.h"
#include <vector>
#include "SumReducer.h"
#include "IReduceOperator.h"
#include "IMapOperator.h"
#include "ValueSumMapper.h"
#include "ArrayDataBlock.h"
#include "CearyCMapper.h"
#include "CearyCMultiplexor.h"
#include "CearyCMultiplexMapper.h"


class CearyCCalculator:
	public CalculatorBase
{
public:
	CearyCCalculator(int procsid, int procsnum )
		:CalculatorBase(procsid, procsnum){}
	
	virtual void Run()
	{
        double start = MPI_Wtime();
		size_t blockcount = 0, count = 0;
		RetrieveSizeInfo(count, blockcount);
		LoadData(blockcount);
		double start2 = MPI_Wtime();
		ValueSumMapper op1;
		
		vector<IDataBlock*>& sums = Map(*srcData, op1);
		SumReducer op2;
		ArrayDataBlock& sum = dynamic_cast<ArrayDataBlock&>(Reduce(sums, op2));

		double avg = sum.Values[0]/count;
		if(myid==0)
			cout << "[DEBUG] [OPTIONS] Average: " << avg << endl;
		
		CearyCMapper op3(avg);
		vector<IDataBlock*>& sums2 = Map(*srcData, op3);
		ArrayDataBlock& sum2 = dynamic_cast<ArrayDataBlock&>(Reduce(sums2, op2));

		CearyCMultiplexor op4;
		
        vector<IDataBlock*>& sum3 = Multiplex(*srcData, op4, blockcount);

		CearyCMultiplexMapper op5;

		vector<IDataBlock*>& sum4 = Map(sum3, op5);

		ArrayDataBlock& sum5 = dynamic_cast<ArrayDataBlock&>(Reduce(sum4, op2));

		double cearyC = (count-1) * sum5.Values[1] / sum5.Values[0] / sum2.Values[1] / 2;

		double s1 = 2 * sum5.Values[2];

		double s2 = sum5.Values[3];

		double b2 = count * sum2.Values[2] / sum2.Values[1] / sum2.Values[1];

		double ws = sum5.Values[0]*sum5.Values[0];

		double var = (count-1)*s1*(count*count - 3*count+3-(count-1)*b2)/count/(count-2)/(count-3)/ws;
		
		var -= (count-1)*s2*(count*count-3*count-6-(count*count-count+2)*b2)/4/count/(count-2)/(count-3)/ws;
		var += (count*count-3-(count-1)*(count-1)*b2)/count/(count-2)/(count-3);
        double stop = MPI_Wtime();
		if(myid==0)
		{
            cout << "[DEBUG][TIMESPAN][IO] " << start2-start << endl;
            cout << "[DEBUG][TIMESPAN][COMPUTING] " << stop-start2 << endl;
            cout << "[DEBUG][TIMESPAN][TOTAL] " << stop-start << endl;
            cout << "[RESULT] Ceary's C: " << cearyC << endl;
            
			//printf("Geary's C=%f, Var(c)=%f\r\n", gearyC, var);
			//printf("vs=%f, vq=%f\r\n", sum2.Values->at(1), sum2.Values->at(2));
			//printf("wij=%f, vij=%f, wsij=%f, wsigma=%f\r\n", sum5.Values->at(0), sum5.Values->at(1), sum5.Values->at(2), sum5.Values->at(3));
		}
		delete &sum3;
		delete &sum2;
		delete &sum;
		
	}
};

