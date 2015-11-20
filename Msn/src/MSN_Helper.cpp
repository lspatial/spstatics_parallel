#include "MSN_Helper.h"
#include <fstream>
//#include <iostream>
#include <string>
#include <ogrsf_frmts.h>
#include "StringLib.h"

using namespace std;

void MSN_Helper::LoadSamples(Config &config, Matrix &mat)
{
	if(config.bShapefile)
	{
		OGRRegisterAll();
		string pLayerName = StringGetFileName(config.sSamples);
		OGRDataSource* poDS = OGRSFDriverRegistrar::Open(config.sSamples.c_str(),FALSE);
		OGRLayer* poLayer = poDS->GetLayerByName(pLayerName.c_str());
	
		config.nSamples = poLayer->GetFeatureCount();
		mat.Resize(config.nSamples, 4);

		OGRPoint *poPoint;
		OGRFeature * pFeature =poLayer->GetNextFeature();
		for(unsigned long i=1; i<=config.nSamples; i++)
		{
			//样本取值字段名为value，double型
			poPoint = (OGRPoint *)pFeature->GetGeometryRef();
			mat[i][1] = poPoint->getX();
			mat[i][2] = poPoint->getY();
			mat[i][3] = pFeature->GetFieldAsInteger("stratum");
			mat[i][4] = pFeature->GetFieldAsDouble("value");
			pFeature = poLayer->GetNextFeature();
		}

		OGRDataSource::DestroyDataSource( poDS );
	}
	else
	{
		double x, y, v, stratum;
		string sline;
		ifstream infile(config.sSamples.c_str());
		infile >> config.nSamples;

		mat.Resize(config.nSamples, 4);
		for(unsigned long i=1; i<=config.nSamples; i++)
		{
			infile >> x >> y >> stratum >> v;
			mat[i][1] = x;
			mat[i][2] = y;
			mat[i][3] = stratum;
			mat[i][4] = v;
		}
		infile.close();
	}
}

void MSN_Helper::LoadBlockPoints(Config &config, Matrix &mat)
{
	if(config.bShapefile)
	{
		OGRRegisterAll();
		string pLayerName = StringGetFileName(config.sBlockPoints);
		OGRDataSource* poDS = OGRSFDriverRegistrar::Open(config.sBlockPoints.c_str(),FALSE);
		OGRLayer* poLayer = poDS->GetLayerByName(pLayerName.c_str());
	
		config.nBlockPoints = poLayer->GetFeatureCount();
		mat.Resize(config.nBlockPoints, 3);

		OGRPoint *poPoint;
		OGRFeature * pFeature =poLayer->GetNextFeature();
		for(unsigned long i=1; i<=config.nBlockPoints; i++)
		{
			//样本取值字段名为value，double型
			poPoint = (OGRPoint *)pFeature->GetGeometryRef();
			mat[i][1] = poPoint->getX();
			mat[i][2] = poPoint->getY();
			mat[i][3] = pFeature->GetFieldAsInteger("stratum");
			pFeature = poLayer->GetNextFeature();
		}

		OGRDataSource::DestroyDataSource( poDS );
	}
	else
	{
		double x, y, stratum;
		string sline;
		ifstream infile(config.sBlockPoints.c_str());
		infile >> config.nBlockPoints;

		mat.Resize(config.nBlockPoints, 3);
		for(unsigned long i=1; i<=config.nBlockPoints; i++)
		{
			infile >> x >> y >> stratum;
			mat[i][1] = x;
			mat[i][2] = y;
			mat[i][3] = stratum;
		}
		infile.close();
	}
}

void MSN_Helper::LoadSvParams(const Config &config, SvModels &models)
{
	int nModel;
	double nugget, psill, range;

	ifstream infile(config.sSvParams.c_str());
	models.Resize(config.nStrata+1);

	for(unsigned long i=1; i<=config.nStrata+1; i++)
	{
		infile >> nModel >> nugget >> psill >> range;

		SvModel sm;
		sm.type = (SvModelType)nModel;
		sm.nugget = nugget;
		sm.psill = psill;
		sm.range = range;
		models[i] = sm;
	}
	infile.close();
}

void MSN_Helper::LoadConfig( string sConfig, Config &config )
{
	ifstream infile(sConfig.c_str());
	infile >> config.bShapefile;
	infile >> config.nStrata;

	config.weight.Resize(config.nStrata);
	for(unsigned long i=1; i<=config.nStrata; i++)
	{
		infile >> config.weight[i];
	}

	infile >> config.sSamples;
	infile >> config.sBlockPoints;
	infile >> config.sSvParams;
	infile.close();
}

void MSN_Helper::Save( Vector &v, string sfile )
{
	ofstream outfile(sfile.c_str());
	for(int i=1; i<=v.Length(); i++)
	{
		outfile << v[i];
		if(v.bColumn)
			outfile << endl;
		else
			outfile << ",";
	}
	outfile.close();
}

void MSN_Helper::Save( Matrix &m, string sfile )
{
	ofstream outfile(sfile.c_str());
	for(int i=1; i<=m.RowCount(); i++)
	{
		for(int j=1; j<=m.ColCount(); j++)
		{
			outfile << m[i][j];
			if(j != m.ColCount())
				outfile << ",";
		}
		outfile << endl;
	}
	outfile.close();
}
