#pragma once
#include "CalculatorBase.h"
#include "RasterReader.h"
#include <vector>
#include "SumReducer.h"
#include "IReduceOperator.h"
#include "IMapOperator.h"
#include "ValueSumMapper.h"
#include "ArrayDataBlock.h"
#include "GetisGMapper.h"
#include "GetisGMultiplexor.h"
#include "GetisGMultiplexMapper.h"

class GetisGCalculator:
	public CalculatorBase
{
public:
	GetisGCalculator(int procsid, int procsnum )
		:CalculatorBase(procsid, procsnum){}
	
	virtual void Run()
	{
		double start = MPI_Wtime();
		size_t blockcount = 0, count = 0;
		RetrieveSizeInfo(count, blockcount);
		LoadData(blockcount);
		
        double start2 = MPI_Wtime();
		SumReducer op2;

		GetisGMapper op3;
		vector<IDataBlock*>& sums2 = Map(*srcData, op3);
		ArrayDataBlock& sum2 = dynamic_cast<ArrayDataBlock&>(Reduce(sums2, op2));

		GetisGMultiplexor op4;
		vector<IDataBlock*>& sum3 = Multiplex(*srcData, op4, blockcount);
		GetisGMultiplexMapper op5;

		vector<IDataBlock*>& sum4 = Map(sum3, op5);
		ArrayDataBlock& sum5 = dynamic_cast<ArrayDataBlock&>(Reduce(sum4, op2));

		double getisG = sum5.Values[2]/sum5.Values[1];
		double eg = sum5.Values[0]/count/(count-1);

		double s1 = 2* sum5.Values[3];

		double s2 = 4 * sum5.Values[4];

		double ws = sum5.Values[0] * sum5.Values[0];

		double d0 = (count * count - 3* count+3)*s1 - count * s2 + 3 * ws;

		double d1 = -((count * count -count)*s1 - 2*count*s2 + 6*ws);

		double d2 = -(2*count*s1-(count+3)*s2+6*ws);

		double d3 = 4*(count-1)*s1 - 2*(count+1)*s2+8*ws;

		double d4 = s1 - s2 + ws;

		double a = d0 * sum2.Values[1] * sum2.Values[1] + d1 * sum2.Values[3] + d2*sum2.Values[0] * sum2.Values[0]*sum2.Values[1];

		double b = d3 * sum2.Values[0] * sum2.Values[2] + d4 * pow(sum2.Values[0], 4);

		double c = pow((sum2.Values[0] * sum2.Values[0] - sum2.Values[1]), 2) * count * (count-1) * (count-2) * (count-3);

		double egs = (a+b)/c;

		double var = egs - eg * eg;
        double stop = MPI_Wtime();
		if(myid==0)
		{
            cout << "[DEBUG][TIMESPAN][IO] " << start2-start << endl;
            cout << "[DEBUG][TIMESPAN][COMPUTING] " << stop-start2 << endl;
            cout << "[DEBUG][TIMESPAN][TOTAL] " << stop-start << endl;
            cout << "[RESULT] Getis-Ord General G: " << getisG << endl;
			//printf("Getis-Ord General G=%f, Var(g)=%f\r\n", getisG, var);
			//printf("vs=%f, vq=%f\r\n", sum2.Values->at(1), sum2.Values->at(2));
			//printf("wij=%f, vij=%f, wsij=%f, wsigma=%f\r\n", sum5.Values->at(0), sum5.Values->at(1), sum5.Values->at(2), sum5.Values->at(3));
		}
		delete &sum3;
		delete &sum2;		
		
	}
};

