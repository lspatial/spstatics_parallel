#pragma once
#include "IDataBlock.h"
#include "cpl_conv.h" 
#include "IGeodataBlock.h"
#include "ogrsf_frmts.h"


class VectorDataBlock:public IGeoDataBlock
{

public:
	vector<OGRGeometry*> geos;
	vector<double> values;

	virtual void AddData(OGRGeometry& geo, double value)
	{
		geos.push_back(&geo);
		values.push_back(value);
	}
	
	virtual ~VectorDataBlock(){
	}

	virtual void SerializeTo(void * buffer){
		size_t size = geos.size();
		size_t * p1 = (size_t*)buffer;
		p1[0] = size;

		size_t * p2 = p1+1;

		unsigned char * p3 = (unsigned char*) (p2 + geos.size());

		for(size_t i=0;i<geos.size();i++)
		{			
			p2[i] = geos[i]->WkbSize();
			geos[i]->exportToWkb(wkbNDR, p3);
			p3+=p2[i];
		}

		double* p4 = (double*)p3;

		for(size_t i=0;i<values.size();i++)
		{
			p4[i] = values[i];
		}
	}

	virtual void DeserializeFrom(const void * const buffer, int size) {
		size_t * p1 = (size_t*)buffer;

		size = p1[0];

		size_t* p2 = p1+1;

		unsigned char * p3 = (unsigned char*) (p2+size);

		OGRGeometry * pGeo;

		for(int i=0;i<size;i++)
		{
			OGRGeometryFactory::createFromWkb(p3, NULL, &pGeo, p2[i]);
			geos.push_back(pGeo);
			p3+=p2[i];
		}
		
		double* p4 = (double*)p3;
		for(int i=0;i<size;i++)
		{
			values.push_back(p4[i]);
		}
	}
	
	virtual size_t Size() {
		size_t size = (1+geos.size())*sizeof(size_t);
		size += values.size()* sizeof(double);
		for(size_t i=0;i<geos.size();i++)
		{
			size+=geos[i]->WkbSize();
		}
		return size;
	}

	virtual size_t DataSize() {
		return geos.size();
	}

	virtual double GetData(int index, double& x, double& y) {
		return values[index];
	}

	virtual double GetData(int index, OGRGeometry **geo)
	{
		*geo = (geos[index]);
		return values[index];
	}

	virtual double GetData(int index){
		return values[index];
	}
};

