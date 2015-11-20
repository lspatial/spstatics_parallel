#pragma once
#include <iostream>
#include <fstream>
#include "Matrix.h"
using namespace std;

class Logfile
{
public:
	Logfile(char* sfile)
	{
		logfile.open(sfile);
	}

	~Logfile()
	{
		logfile.close();
	}
	
	void Write(char* v) {logfile << v << endl;}
	void Write(unsigned long v) {logfile << v << endl;}
	void Write(long v) {logfile << v << endl;}
	void Write(int v) {logfile << v << endl;}
	void Write(double v) {logfile << v << endl;}
	void Write(Vector &v)
	{
		for(int i=1; i<=v.Length(); i++)
		{
			logfile << v[i];
			if(i != v.Length())
			{
				if(v.bColumn) logfile << endl;
				else logfile << ",";
			}
		}
		logfile << endl;
	}

private:
	ofstream logfile;
};
