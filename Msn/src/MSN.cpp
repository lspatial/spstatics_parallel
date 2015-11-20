#include "MSN.h"
#include "mpi.h"
#include <cmath>
#include <limits>
#include <fstream>
using namespace std;

MSN::MSN(void)
{
}

MSN::~MSN(void)
{
}

double MSN::Cov( double dist, SvModel &model )
{
	double dblCov, ratio;
	dblCov = numeric_limits<double>::max();
	ratio = dist/model.range;

	if(IsEqual(dist,0.0))
		dblCov = model.nugget+model.psill;
	else
	{
		switch (model.type)
		{
		case Sv_Sph:
			if(dist > model.range)
				dblCov = 0.0;
			else
				dblCov = model.psill * (1 - 1.5*ratio + 0.5*pow(ratio,3));
			break;
		case Sv_Gau:
			dblCov = model.psill * exp(-ratio*ratio);
			break;
		case Sv_Exp:
			dblCov = model.psill * exp(-ratio);
			break;
		default:
			break;
		}
	}

	return dblCov;
}

Vector MSN::Cov( Vector &dist, SvModel &model )
{
	Vector vCov = dist;
	for(unsigned long i=1; i<=dist.Length(); i++)
	{
		vCov[i] = Cov(dist[i], model);
	}
	return vCov;
}

bool MSN::IsEqual( double d1, double d2 )
{
	if(abs(d1-d2) < 1e-6) 
		return true;
	else 
		return false;
}

void MSN::Distance( Vector &v0, Matrix &m, Vector &d, unsigned long nDim/* = 0 */ )
{
	double d0;
	if(nDim == 0) nDim = v0.Length();

	d.Resize(m.RowCount());
	for(unsigned long i=1; i<=m.RowCount(); i++)
	{
		d0 = 0;
		for(unsigned long j=1; j<=nDim; j++)
		{
			d0 += (v0[j]-m[i][j])*(v0[j]-m[i][j]);
		}
		d[i] = sqrt(d0);
	}
}

double MSN::Distance( Vector &v0, Vector &v1, unsigned long nDim/* = 0 */ )
{
	double d = 0;
	if(nDim == 0) nDim = v0.Length();
	for(unsigned long i=1; i<=nDim; i++)
	{
		d += (v0[i]-v1[i])*(v0[i]-v1[i]);
	}
	return(sqrt(d));
}

Matrix MSN::FillCMatrix( LIS_MATRIX &C, Matrix &samples, SvModels &models, Config &config, double dblZoomIn )
{
	double d, cov;
	int is, ie;
	Matrix mC;

	lis_matrix_get_range(C, &is, &ie);		//C矩阵在该节点的行序号范围 [is, ie)
	mC.Resize(ie-is, config.nSamples+config.nStrata);

	for(unsigned long i=is; i<ie; i++)
	{
		if(i < config.nSamples)
		{
			Vector v0 = samples[i+1];
			//左上协方差部分
			for(unsigned long j=1; j<=config.nSamples; j++)
			{
				d = Distance(v0, samples[j], 2);
				if(v0[3] == samples[j][3])
					cov = Cov(d, models[v0[3]+1]);
				else
					cov = Cov(d, models[1]);
				//lis_matrix_set_value(LIS_INS_VALUE, i, j-1, cov, C);
				mC[i-is+1][j] = cov;
			}
			//右上0、1部分
			for(unsigned long j=1; j<=config.nStrata; j++)
			{
				if(j == v0[3])
					//lis_matrix_set_value(LIS_INS_VALUE, i, config.nSamples+j-1, 1, C);
					mC[i-is+1][config.nSamples+j] = 1;
				else
					//lis_matrix_set_value(LIS_INS_VALUE, i, config.nSamples+j-1, 0, C);
					mC[i-is+1][config.nSamples+j] = 0;
			}
		}
		else
		{
			int nStratum = i-config.nSamples+1;
			//左下层权重部分
			for(unsigned long j=1; j<=config.nSamples; j++)
			{
				if(samples[j][3] == nStratum)
					//lis_matrix_set_value(LIS_INS_VALUE, i, j-1, 1/config.weight[nStratum], C);
					mC[i-is+1][j] = 1/config.weight[nStratum];
				else
					//lis_matrix_set_value(LIS_INS_VALUE, i, j-1, 0, C);
					mC[i-is+1][j] = 0;
			}
			//右下0部分
			for(unsigned long j=1; j<=config.nStrata; j++)
			{
				//lis_matrix_set_value(LIS_INS_VALUE, i, config.nSamples+j-1, 0, C);
				mC[i-is+1][config.nSamples+j] = 0;
			}
		}
	}

	for(unsigned long i=is; i<ie; i++)
	{
		for(unsigned long j=1; j<=config.nSamples+config.nStrata; j++)
		{
			lis_matrix_set_value(LIS_INS_VALUE, i, j-1, mC[i-is+1][j] / dblZoomIn, C);
		}
	}
	lis_matrix_set_type(C, LIS_MATRIX_DNS);
	lis_matrix_assemble(C);

	return(mC);
}

Vector MSN::FillDVector( LIS_VECTOR &D, Matrix &samples, Matrix &blkpts, SvModels &models, Config &config, double dblZoomIn )
{
	double d, cov;
	int is, ie;

	lis_vector_get_range(D, &is, &ie);		//D向量在该节点的行序号范围 [is, ie)
	Vector dv(ie-is);
	dv.SetValue(0);
	if(is >= config.nSamples)
	{
		for(int i=is; i<ie; i++)
		{
			lis_vector_set_value(LIS_INS_VALUE, i, 1, D);
			dv[i-is+1] = 1;
		}
		return(dv);
	}

	//load block points
	Vector v0(3);
	for(int i=1; i<=config.nBlockPoints; i++)
	{
		v0[1] = blkpts[i][1];
		v0[2] = blkpts[i][2];
		v0[3] = blkpts[i][3];
		for(int j=is; j<ie; j++)
		{
			if(j < config.nSamples)
			{
				d = Distance(v0, samples[j+1], 2);
				if(v0[3] == samples[j+1][3])
					cov = Cov(d, models[v0[3]+1]);
				else
					cov = Cov(d, models[1]);
				//dv[j-is+1] += (cov/config.nBlockPoints);
				dv[j-is+1] += cov;
			}
			else
				dv[j-is+1] = 1;
		}
	}

	for(int i=is; i<ie; i++)
	{
		if(i < config.nSamples) dv[i-is+1] = dv[i-is+1]/config.nBlockPoints;
		lis_vector_set_value(LIS_INS_VALUE, i, dv[i-is+1] * dblZoomIn, D);
	}

	return(dv);
}

void MSN::GetCurProcValue( LIS_VECTOR &v, Vector &d )
{
	int is, ie;
	lis_vector_get_range(v, &is, &ie);		//v向量在该节点的行序号范围 [is, ie)
	
	double d0;
	d.Resize(ie-is);
	for(int i=is; i<ie; i++)
	{
		lis_vector_get_value(v, i, &d0);
		d[i-is+1] = d0;
	}
}

void MSN::Gather( LIS_VECTOR &v0, Vector &v1, int nProcs, int nProcRank, int nRoot/*=0*/ )
{
	int ise[2];
	Vector d;
	lis_vector_get_range(v0, ise, ise+1);		//v0向量在该节点的行序号范围 [ise[0], ise[1])
	GetCurProcValue(v0, d);

	//所有非root进程发送数据
	if(nProcRank != nRoot)
	{
		MPI_Send(&ise, 2, MPI_INT, nRoot, 101, MPI_COMM_WORLD);				//起止位置
		MPI_Send((void*)d.c_data(), ise[1]-ise[0], MPI_DOUBLE, nRoot, 102, MPI_COMM_WORLD);	//数组元素
	}
	else
	//root进程接收其他进程发来的数据
	{
		//root进程自身数据
		for(int i=ise[0]; i<ise[1]; i++)
		{
			v1[i+1] = d[i-ise[0]+1];
		}

		//接收其他进程数据
		double *dRcv;
		MPI_Status status;
		for(int i=1; i<nProcs; i++)
		{
			MPI_Recv(ise, 2, MPI_INT, i, 101, MPI_COMM_WORLD, &status);
			
			dRcv = new double[ise[1]-ise[0]];
			MPI_Recv(dRcv, ise[1]-ise[0], MPI_DOUBLE, i, 102, MPI_COMM_WORLD, &status);
			for(int j=ise[0]; j<ise[1]; j++)
			{
				v1[j+1] = dRcv[j-ise[0]];
			}
			delete[] dRcv;
		}
	}
}

void MSN::Gather( Matrix &mLoc, Matrix &mGlob, int is, int ie, int nProcs, int nProcRank, int nRoot/*=0*/ )
{
	int nCols = mLoc.ColCount();
	int nRows = ie-is;

	if(nProcRank != nRoot)
	{
		int r = 1;
		MPI_Send(&nRows, 1, MPI_INT, nRoot, 101, MPI_COMM_WORLD);	//待发送的行数
		for(int i=is; i<ie; i++)
		{
			MPI_Send(&i, 1, MPI_INT, nRoot, 101+r, MPI_COMM_WORLD);	//行号
			MPI_Send((void*)mLoc[i-is+1].c_data(), nCols, MPI_DOUBLE, nRoot, 1000000+r, MPI_COMM_WORLD);	//数组元素
			r++;
		}
	}
	else
	{
		//root进程自身数据
		for(int i=is; i<ie; i++)
		{
			for(int j=1; j<=nCols; j++)
			{
				mGlob[i+1][j] = mLoc[i-is+1][j];
			}
		}

		//接收其他进程数据
		int nr, r;
		double *dRcv = new double[nCols];
		MPI_Status status;
		for(int i=1; i<nProcs; i++)
		{
			r = 1;
			MPI_Recv(&nRows, 1, MPI_INT, i, 101, MPI_COMM_WORLD, &status);
			for(int j=1; j<=nRows; j++)
			{
				MPI_Recv(&nr, 1, MPI_INT, i, 101+r, MPI_COMM_WORLD, &status);
				MPI_Recv(dRcv, nCols, MPI_DOUBLE, i, 1000000+r, MPI_COMM_WORLD, &status);
				r++;

				for(int j=1; j<=nCols; j++)
				{
					mGlob[nr+1][j] = dRcv[j-1];
				}
			}
		}
		delete[] dRcv;
	}
}

void MSN::AbstractWtLg( Vector &wu, Matrix &samples, Vector &wt, Vector &mu, Config &config )
{
	wt.Resize(config.nSamples);
	mu.Resize(config.nStrata);

	for(int i=1; i<=config.nSamples; i++)
		wt[i] = wu[i]/config.weight[samples[i][3]];

	for(int i=1; i<=config.nStrata; i++)
		mu[i] = wu[i+config.nSamples]*config.weight[i];
}

double MSN::EstPopulationMean( Matrix &samples, Vector &wt )
{
	double mn = 0;
	for(int i=1; i<=samples.RowCount(); i++)
	{
		mn += samples[i][4]*wt[i];
	}

	mn = mn / wt.Sum();

	return mn;
}

double MSN::CAA( Matrix &blkPts, SvModels &models, Config &config, int nProcs, int nProcRank, int nRoot/*=0*/ )
{
	unsigned long is, ie; //当前进程计算的起止总体离散点位置
	unsigned long nParts;
	nParts = config.nBlockPoints / nProcs;
	is = nProcRank * nParts;
	if(nProcRank != nProcs-1)
		ie = is + nParts;
	else
		ie = config.nBlockPoints;

	double d;
	double cov, rowsum, allrowsum;
	allrowsum = 0;
	for(unsigned long i=is; i<ie; i++)
	{
		rowsum = 0;
		Vector v0 = blkPts[i+1];
		for(unsigned long j=1; j<=config.nBlockPoints; j++)
		{
			d = Distance(v0, blkPts[j], 2);
			if(v0[3] == blkPts[j][3])
				cov = Cov(d, models[v0[3]+1]);
			else
				cov = Cov(d, models[1]);
			rowsum += cov;
		}
		allrowsum += (rowsum/config.nBlockPoints);
	}

	//所有进程allrowsum求和
	double caa = 0;
	allrowsum /= config.nBlockPoints;
	MPI_Reduce(&allrowsum, &caa, 1, MPI_DOUBLE, MPI_SUM, nRoot, MPI_COMM_WORLD);
	return caa;
}

double MSN::EstPopulationVar( double caa, Vector &wu, Vector &di, Config &config, double dblZoomIn )
{
	double secondItem = 0;
	for(int i=1; i<=config.nSamples; i++)
	{
		secondItem += (wu[i]*di[i] / (dblZoomIn*dblZoomIn*dblZoomIn));
	}

	Vector mu(config.nStrata);
	for(int i=1; i<=config.nStrata; i++)
		mu[i] = wu[i+config.nSamples]*config.weight[i];
	
	double var = caa - (secondItem + mu.Sum()/(dblZoomIn*dblZoomIn));
	return(var);
}

SvModels::SvModels( unsigned long nLen )
{
	this->nLen = nLen;
	models = NULL;
	if(nLen > 0)
		models = new SvModel[nLen];
}

SvModels::SvModels( SvModels &models )
{
	this->nLen = models.Length();
	this->models = new SvModel[this->nLen];
	for(unsigned long i=1; i<= this->nLen; i++)
	{
		this->models[i-1] = models[i];
	}
}

SvModels::SvModels(void)
{
	nLen = 0;
	models = NULL;
}

unsigned long SvModels::Length(void)
{
	return nLen;
}

SvModel & SvModels::operator[]( unsigned long nIndex )
{
	return(models[nIndex-1]);
}

SvModels::~SvModels(void)
{
	delete[] models;
	models = NULL;
	nLen = 0;
}

SvModels SvModels::operator=( SvModels &models )
{
	delete[] this->models;
	this->models = NULL;
	
	nLen = models.Length();
	this->models = new SvModel[nLen];
	for(unsigned long i=1; i<=nLen; i++)
	{
		this->models[i-1] = models[i];
	}

	return *this;
}

void SvModels::Resize( unsigned long nSize )
{
	if(nLen > 0)
		delete[] models;
	nLen = nSize;
	models = new SvModel[nLen];
}

void SvModels::Print( void )
{
	for(unsigned long i=0; i<nLen; i++)
	{
		cout << models[i].type << ' ' << models[i].nugget << ' ' 
			<< models[i].psill << ' ' << models[i].range << endl;
	}
}
