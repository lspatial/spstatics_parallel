#pragma once

#include "gdal.h"
#include "cpl_conv.h" 
#include "IGeoReader.h"
#include "ogrsf_frmts.h"
#include "VectorDataBlock.h"

class VectorReader:
	public IGeoReader
{
public:
	int Open(const char* connstr, const char* layer, int fieldIndex)			
	{
		blocksize = 64;
		
		std::string mainname(layer);

		poDS = OGRSFDriverRegistrar::Open( connstr  , FALSE );		
		if( poDS == NULL ){	
            std::cout<<"[ERROR] Can't open the vector file:"<< layer<<std::endl;
            return 1;
		}

		std::string sql = "SELECT count(*) FROM ";
		sql+=mainname;
		OGRLayer* lyr = poDS->ExecuteSQL(sql.c_str(), NULL, NULL);
		
		OGRFeature* f = lyr->GetNextFeature();
		
		if(f != NULL)
		{
			itemcount = f->GetFieldAsInteger(0);
			OGRFeature::DestroyFeature( f );
		}
		else
		{
			itemcount = 0;
		}
		poDS->ReleaseResultSet(lyr);
		poLayer = poDS->GetLayerByName( mainname.c_str() );		
		poFDefn = poLayer->GetLayerDefn();
		
		fldidx = fieldIndex;
		if(fieldIndex<poFDefn->GetFieldCount())
        {
            OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( fldidx );
			OGRFieldType type = poFieldDefn->GetType();
            if(type == OFTInteger || type == OFTReal)
                return 0;
        }
        
        std::cout<<"[ERROR] The specifed field ("<<fieldIndex<<") has wrong type."<<std::endl;
        return 1;
	}

	
	virtual IDataBlock* NextBlock(){
	    VectorDataBlock* block = new VectorDataBlock();		
		OGRFeature *poFeature;
		OGRGeometry *poGeometry;
		size_t count = 0;
		while ( (poFeature = poLayer->GetNextFeature()) != NULL)
		{
			double data = 0;
			if(fldidx>-1){
				data = poFeature->GetFieldAsDouble(fldidx);									
			}

			//��ȡfeature
			poGeometry = poFeature->StealGeometry();
			
			block->AddData(*poGeometry, data);
			//itemcount++;
			
			OGRFeature::DestroyFeature( poFeature );
			count++;
			if(count>=blocksize)
				break;
		}
		
		return count>0?block:NULL;
		
	}

	virtual IDataBlock* Deserailize(void * buffer, int size){
		VectorDataBlock * block = new VectorDataBlock();
		block->DeserializeFrom(buffer, size);
		return block;				
	}

	virtual size_t GetSizeInfo(size_t & blockcount) {
		blockcount = (itemcount + blocksize - 1)/blocksize;
		return itemcount;		
	}

	virtual ~VectorReader()
	{
		//OGRDataSource::DestroyDataSource( poDS );
	}
	
private:
	
	OGRDataSource *poDS;
	OGRLayer *poLayer;
	OGRFeatureDefn *poFDefn;
	int fldidx;
	size_t itemcount;	
	size_t blocksize;	
};
