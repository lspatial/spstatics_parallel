#pragma once

#ifndef NULL
#define NULL 0
#endif

//ע�⣺�±��1��ʼ
class Vector
{
public:
	bool bColumn;	//Column Vector��true������Row Vector��false��

public:
	Vector(unsigned long nLen=0, bool bCol=false);
	Vector(Vector &v);
	~Vector(void);

	void SetValue(double d);
	unsigned long Length();
	void Resize(unsigned long nLen, bool bReserve=false);
	void Destroy(void);
	void Print(void);
	const double *c_data();	//���س�Ա����data��ָ�루�������޸���ֵ��
	double & operator[](unsigned long i);
	Vector operator=(Vector &v);

	//math
	double Mean();
	double Sum();
private:
	unsigned long nLen;
	double *data;
};

//ע�⣺�±��1��ʼ
class VectorI
{
public:
	bool bColumn;	//Column Vector��true������Row Vector��false��

public:
	VectorI(unsigned long nLen=0, bool bCol=false);
	VectorI(VectorI &v);
	~VectorI(void);

	void SetValue(int n);
	unsigned long Length();
	void Resize(unsigned long nLen, bool bReserve=false);
	void Destroy(void);
	void Print(void);
	const int *c_data(); //���س�Ա����data��ָ�루�������޸���ֵ��
	int & operator[](unsigned long i);
	VectorI operator=(VectorI &v);

private:
	unsigned long nLen;
	int *data;
};

//ע�⣺�����±��1��ʼ
class Matrix
{
public:
	Matrix(void);
	Matrix(unsigned long nRow, unsigned long nCol);
	Matrix(Matrix &mat);
	~Matrix(void);

	unsigned long RowCount(void);
	unsigned long ColCount(void);
	void Resize(unsigned long nRow, unsigned long nCol);
	void Destroy(void);
	void Print(void);
	void CopyTo(Matrix &mat);
	Vector & operator[](unsigned long nRow);
	Matrix operator=(Matrix &mat);
	//Matrix operator=(Matrix mat);

	void GetRow(unsigned long nRow, Vector &v);
	void GetColumn(unsigned long nCol, Vector &v);	
private:
	Vector* data;
	bool bInitialized;
	unsigned long nRowCount, nColumnCount;
	
private:
	void Allocate(void);
};
