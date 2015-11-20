#pragma once
#include "IDataBlock.h"
#include "IMultiplexOperator.h"

#include "ArrayDataBlock.h"
#include "IGeodataBlock.h"
#include "ImageDataBlock.h"
#include "VectorDataBlock.h"
#include "ogrsf_frmts.h"


class GetisGMultiplexor:
	public IMultiplexOperator
{
private:
	
	void CalcOther(IGeoDataBlock& in1, IGeoDataBlock& in2, ArrayDataBlock& out)
	{
		
		double v1v2 = 0;
		double w = 0;
		double v = 0;
		double wv = 0;
		double weight = 0;
		double w2 = 0;
		double w3 = 0;

		size_t size1 = in1.DataSize();
		size_t size2 = in2.DataSize();
		
		double x1, x2, y1, y2, v1, v2;

		vector<double>& values = out.Values;

		for(size_t i=0;i<size1;i++)
		{
			v1 = in1.GetData(i, x1, y1);
			w3=0;
			for(size_t j=0;j<size2;j++)
			{
				v2 = in2.GetData(j, x2, y2);
				weight = 1; //calculator(x1, y1, x2, y2);

				w += weight;
				v1v2 = v1*v2;
				v += v1v2;
				wv += weight*v1v2;				
				w2 += weight * weight;
				w3 += weight;
			}

			values[i+3] += w3;
		}
		values[0] += w;
		values[1] += v;
		values[2] += wv;
		values[3] += w2;
	}

	inline void CalcVector(VectorDataBlock& in1, VectorDataBlock& in2, ArrayDataBlock& out)
	{
		double v1, v2;
		double v1v2 = 0;
		double w = 0;
		double v = 0;
		double wv = 0;
		double weight = 0;
		double w2 = 0;
		double distance =0;
		vector<double>& values = out.Values;
		OGRGeometry * pgeo1 = NULL;
		OGRGeometry * pgeo2 = NULL;
		
		for(size_t i=0;i<in1.DataSize();i++)
		{			
			v1=in1.GetData(i, &pgeo1);
                        OGRPoint *p1 = (OGRPoint*)pgeo1;
			w2=0;
			for(size_t j=0;j<in2.DataSize();j++)
			{
				v2=in2.GetData(j, &pgeo2);
                                OGRPoint *p2 = (OGRPoint*)pgeo2;
				distance = sqrt(
                                        (p1->getX()-p2->getX())*(p1->getX()-p2->getX())+
                                        (p1->getY()-p2->getY())*(p1->getY()-p2->getY())
                                        );
				if(distance != 0)
				{
					weight = 1/distance;
					v1v2 = v1*v2;
					v+=v1v2;
					w+=weight;
					wv+=v1v2*weight;
					w2+=weight*weight;
				}						
			}
			values[i+3] += w2;
		}
		values[0]+= w;
		values[1]+= v;
		values[2]+= wv;
		values[3]+= w2;
	}

	inline void CalcImage(ImageDataBlock& in1, ImageDataBlock& in2, ArrayDataBlock& out)
	{
		
		int i=0;		
		double* p1 = in1.data;
		double* p2 = in2.data;
		double x1, x2, y1, y2, v1, v2;
		double v1v2 = 0;
		double w = 0;
		double v = 0;
		double wv = 0;
		double weight = 0;
		double w2 = 0;

		double deltaX, deltaY;
		vector<double>& values = out.Values;

		y1 = in1.orgY;
		for(size_t r1=0;r1<in1.rows;r1++)
		{			
			x1 = in1.orgX;
			for(size_t c1=0;c1<in1.cols;c1++)
			{				
				v1 = *p1;
				p2 = in2.data;
				y2 = in2.orgY;
				w2=0;
				for(size_t r2=0;r2<in2.rows;r2++)
				{
					deltaY = y1-y2;
					x2 = in2.orgX;
					
					for(size_t c2=0;c2<in2.cols;c2++)
					{
						v2=*p2;		
						deltaX = x1-x2;
						//weight = calculator(x1, y1, x2, y2);
						if(deltaX != 0 || deltaY !=0)
						{
							weight = 1/sqrt(deltaX*deltaX+deltaY*deltaY);
							v1v2 = v1*v2;
							v+=v1v2;
							w+=weight;
							wv+=v1v2*weight;
							w2+=weight*weight;
						}
						x2 +=in2.sizeX;						
						p2++;
					}
					y2+=in2.sizeY;
				}
				values[i+3] += w2;

			    x1 += in1.sizeX;
				i++;
				p1++;
			}
			y1+=in1.sizeY;
		}
		values[0] += w;
		values[1] += v;
		values[2] += wv;
		values[3] += w2;

	}
public:

	virtual void operator ()(IDataBlock& in1, IDataBlock& in2, IDataBlock& out){
		if(dynamic_cast<ImageDataBlock*>(&in1))
		{
			CalcImage(
				dynamic_cast<ImageDataBlock&>(in1),
				dynamic_cast<ImageDataBlock&>(in2),
				dynamic_cast<ArrayDataBlock&>(out));
		}else if(dynamic_cast<VectorDataBlock*>(&in1)){
			CalcVector(
				dynamic_cast<VectorDataBlock&>(in1),
				dynamic_cast<VectorDataBlock&>(in2),
				dynamic_cast<ArrayDataBlock&>(out));
		}else{
			CalcOther(
				dynamic_cast<IGeoDataBlock&>(in1),
				dynamic_cast<IGeoDataBlock&>(in2),
				dynamic_cast<ArrayDataBlock&>(out));			
		}
	}

	virtual IDataBlock& CreateDataBlock(IDataBlock& in)
	{
		IGeoDataBlock& geo1 = dynamic_cast<IGeoDataBlock&>(in);
		IDataBlock* block = new ArrayDataBlock(geo1.DataSize() + 4);
		return *block;
	}
};

