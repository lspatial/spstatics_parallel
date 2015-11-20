#include <cstring>
#include <iostream>
#include <limits>
#include "Matrix.h"

using namespace std;

//注意：下标从1开始
Vector::Vector(unsigned long nLen, bool bCol)
{
	this->nLen = nLen;
	if(nLen < 1)
		data = NULL;
	else
	{
		data = new double[nLen];
		SetValue(numeric_limits<double>::min());	//初始化为double最小值
	}
	bColumn = bCol;
}

Vector::Vector( Vector &v )
{
	nLen = v.Length();
	data = new double[nLen];

	for(unsigned long i=0; i<nLen; i++)
	{
		data[i] = v.c_data()[i];
	}
	bColumn = v.bColumn;
}

Vector::~Vector(void)
{
	Destroy();
}

unsigned long Vector::Length(void)
{
	return nLen; 
}

void Vector::Resize(unsigned long nLen, bool bReserve/*=false*/)
{
	Vector bak;
	if(bReserve) bak = *this;

	if(this->nLen > 0) delete[] data;
	this->nLen = nLen;
	data = new double[nLen];
	SetValue(numeric_limits<double>::min());	//初始化为double最小值

	if(bReserve)
	{
		for(int i=0; i<min(bak.Length(), nLen); i++)
		{
			data[i] = bak.c_data()[i];
		}
	}
}

void Vector::Destroy(void)
{
	if(nLen == 0) return;
	delete[] data;
	data = NULL;
	nLen = 0;
}

double & Vector::operator[](unsigned long i)
{
	return(data[i-1]);
}

Vector Vector::operator=(Vector &v)
{
	if(nLen != v.Length())
	{
		Destroy();
		nLen = v.Length();
		data = new double[nLen];
	}
	for(unsigned long i=1; i<=nLen; i++)
	{
		data[i-1] = v[i];
	}
	bColumn = v.bColumn;
	
	return *this;
}

void Vector::Print( void )
{
	for(unsigned long i=1; i<=nLen; i++)
	{
		if(bColumn)
			cout << data[i-1] << endl;
		else
			cout << data[i-1] << " ";
	}
	if(!bColumn) cout << endl;
}

void Vector::SetValue( double d )
{
	for(unsigned long i=0; i<nLen; i++)
	{
		data[i] = d;
	}
}

const double * Vector::c_data()
{
	return data;
}

double Vector::Mean()
{
	return(Sum()/nLen);
}

double Vector::Sum()
{
	double sum = 0;
	for(int i=0; i<nLen; i++)
	{
		sum += data[i];
	}

	return(sum);
}

//注意：下标从1开始
VectorI::VectorI(unsigned long nLen, bool bCol)
{
	this->nLen = nLen;
	if(nLen < 1)
		data = NULL;
	else
	{
		data = new int[nLen];
		SetValue(numeric_limits<int>::min());	//初始化为int最小值
	}
	bColumn = bCol;
}

VectorI::VectorI( VectorI &v )
{
	nLen = v.Length();
	data = new int[nLen];

	for(unsigned long i=0; i<nLen; i++)
	{
		data[i] = v.c_data()[i];
	}
	bColumn = v.bColumn;
}

VectorI::~VectorI(void)
{
	Destroy();
}

unsigned long VectorI::Length(void)
{
	return nLen; 
}

void VectorI::Resize(unsigned long nLen, bool bReserve/*=false*/)
{
	VectorI bak;
	if(bReserve) bak = *this;

	if(this->nLen > 0) delete[] data;
	this->nLen = nLen;
	data = new int[nLen];
	SetValue(numeric_limits<int>::min());	//初始化为int最小值

	if(bReserve)
	{
		for(int i=0; i<min(bak.Length(), nLen); i++)
		{
			data[i] = bak.c_data()[i];
		}
	}
}

void VectorI::Destroy(void)
{
	if(nLen == 0) return;
	delete[] data;
	data = NULL;
	nLen = 0;
}

int & VectorI::operator[](unsigned long i)
{
	return(data[i-1]);
}

VectorI VectorI::operator=(VectorI &v)
{
	if(nLen != v.Length())
	{
		Destroy();
		nLen = v.Length();
		data = new int[nLen];
	}
	for(unsigned long i=0; i<nLen; i++)
	{
		data[i] = v.c_data()[i];
	}
	bColumn = v.bColumn;

	return *this;
}

void VectorI::Print( void )
{
	for(unsigned long i=1; i<=nLen; i++)
	{
		if(bColumn)
			cout << data[i-1] << endl;
		else
			cout << data[i-1] << " ";
	}
	if(!bColumn) cout << endl;
}

void VectorI::SetValue( int n )
{
	for(unsigned long i=0; i<nLen; i++)
	{
		data[i] = n;
	}
}

const int * VectorI::c_data()
{
	return data;
}

//注意：矩阵的下标从1开始
Matrix::Matrix(void)
{
	nRowCount = 0;
	nColumnCount = 0;
	data = NULL;
}

Matrix::Matrix(unsigned long nRow, unsigned long nCol)
{
	nRowCount = nRow;
	nColumnCount = nCol;
	data = NULL;
	Allocate();
}

Matrix::Matrix( Matrix &mat )
{
	nRowCount = mat.RowCount();
	nColumnCount = mat.ColCount();
	data = NULL;
	Allocate();

	for(unsigned long i=1; i<=nRowCount; i++)
	{
		for(unsigned long j=1; j<=nColumnCount; j++)
		{
			data[i-1][j] = mat[i][j];
		}
	}
}

Matrix::~Matrix(void)
{
	Destroy();
}

unsigned long Matrix::RowCount(void)
{
	return nRowCount;
}

unsigned long Matrix::ColCount(void)
{
	return nColumnCount;
}

void Matrix::Resize(unsigned long nRow, unsigned long nCol)
{
	if(nRowCount > 0) Destroy();

	nRowCount = nRow;
	nColumnCount = nCol;
	data = NULL;
	Allocate();
}

void Matrix::Allocate(void)
{
	data = new Vector[nRowCount];
	for(unsigned long i=1; i<=nRowCount; i++)
	{
		data[i-1].Resize(nColumnCount);
	}
}

void Matrix::Destroy(void)
{
	for(unsigned long i=1; i<=nRowCount; i++)
	{
		data[i-1].Destroy();
	}
	delete[] data;
}

void Matrix::Print(void)
{
	for(unsigned long i=1; i<=nRowCount; i++)
	{
		for(unsigned long j=1; j<=nColumnCount; j++)
		{
			cout << data[i-1][j];
			if(j < nColumnCount)
				cout << ",";
			else
				cout << ";";
		}
		cout << endl;
	}
}

Vector & Matrix::operator[](unsigned long nRow)
{
	return data[nRow-1];
}

Matrix Matrix::operator=(Matrix &mat)
{
	if(nRowCount != mat.RowCount() && nColumnCount != mat.ColCount())
	{
		Resize(mat.RowCount(), mat.ColCount());
	}

	for(unsigned long i=1; i<=nRowCount; i++)
	{
		for(unsigned long j=1; j<=nColumnCount; j++)
		{
			data[i-1][j] = mat[i][j];
		}
	}

	return *this;
}
