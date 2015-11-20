#pragma once
#include "CalculatorBase.h"
#include "RasterReader.h"
#include <vector>
#include "SumReducer.h"
#include "IReduceOperator.h"
#include "IMapOperator.h"
#include "ValueSumMapper.h"
#include "ArrayDataBlock.h"
#include "MoranIMapper.h"
#include "MoranIMultiplexor.h"
#include "MoranIMultiplexMapper.h"

class MoranICalculator:
	public CalculatorBase
{
public:
	MoranICalculator(int procsid, int procsnum )
		:CalculatorBase(procsid, procsnum){}
	
	virtual void Run()
	{				
		double start = MPI_Wtime();
        size_t blockcount = 0, count = 0;
		RetrieveSizeInfo(count, blockcount);
		LoadData(blockcount);
        if(myid==0)
            cout<<"[DEBUG] [OPTIONS] Calucating"<<endl;
        double start2 = MPI_Wtime();
		ValueSumMapper op1;
		
		vector<IDataBlock*>& sums = Map(*srcData, op1);
        if(myid==0)
            cout<<"[DEBUG] [OPTIONS] Calculating Average"<<endl;
		SumReducer op2;
		ArrayDataBlock& sum = dynamic_cast<ArrayDataBlock&>(Reduce(sums, op2));

		double avg = sum.Values[0]/count;
        if(myid==0)
            cout << "[DEBUG] [OPTIONS] Average: " << avg << endl;
		
		MoranIMapper op3(avg);
		vector<IDataBlock*>& sums2 = Map(*srcData, op3);
		ArrayDataBlock& sum2 = dynamic_cast<ArrayDataBlock&>(Reduce(sums2, op2));

		MoranIMultiplexor op4(avg);
		vector<IDataBlock*>& sum3 = Multiplex(*srcData, op4, blockcount);		

		MoranIMultiplexMapper op5;

		vector<IDataBlock*>& sum4 = Map(sum3, op5);

		ArrayDataBlock& sum5 = dynamic_cast<ArrayDataBlock&>(Reduce(sum4, op2));

		double moranI = count * sum5.Values[1] / sum5.Values[0] /sum2.Values[1];

		double s1 = 2 * sum5.Values[2];

		double s2 = sum5.Values[3];

		double s3 = (sum2.Values[2]/count)/(sum2.Values[1]/count)/(sum2.Values[1]/count);

		double s4 = (count*count-3*count+3)*s1 - count*s2 + 3* sum5.Values[0]*sum5.Values[0];

		double s5 = s1 - 2 * count * s1 + 6 * sum5.Values[0]*sum5.Values[0];

		double var = (count * s4 - s3 * s5)/(count-1)/(count-2)/(count-3)/(sum5.Values[0]*sum5.Values[0]);
		double stop = MPI_Wtime();
		if(myid==0)
		{
            cout << "[DEBUG][TIMESPAN][IO] " << start2-start << endl;
            cout << "[DEBUG][TIMESPAN][COMPUTING] " << stop-start2 << endl;
            cout << "[DEBUG][TIMESPAN][TOTAL] " << stop-start << endl;
            cout << "[RESULT] Moran's I: " << moranI << endl;
            //printf("MoranI=%f, Var(I)=%f, sum2(1)=%f, sum5(0)=%f, sum5(1)=%f\r\n", moranI, var, sum2.Values[1], sum5.Values[0], sum5.Values[1]);			
		}
		delete &sum3;
		delete &sum2;
		delete &sum;
	}
};

