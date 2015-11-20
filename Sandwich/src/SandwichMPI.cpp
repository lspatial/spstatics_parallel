// SandwichMPI.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <iostream>
#include <string.h>
#include <vector>
#include <list>
#include<iterator>
#include <stdio.h>
#include "mpi.h"
#include "time.h" 
#include <stdlib.h>
//#include "sstream.h"
#include <fstream>
using namespace std;


struct tPoint{
	char* id;
	double x;
	double y;
	double val;
};

typedef list<tPoint> tPoints;

struct cSample{
	char* id;
	//char* zoneID;
	//char* reportID;
	tPoint pnt;
};
typedef vector<cSample> tSamples;

struct cReport{
	char* id;
	double mean;
	double variance;
	tPoints pnts;
};
typedef vector<cReport> tReports;

struct cZone{
	char* id;
	double mean;
	double variance;
	tPoints pnts;
	tSamples samples;
};
typedef vector<cZone> tZones;

struct cOverlay{
	char* reportID;
	char* zoneID;
	double weight;
	double mean;
	double variance;
	//tPoints pnts;
};
typedef vector<cOverlay> tOverlays;

struct sRecv{ 
	int id; 
	double mean;
	double variance;
};
struct sRecv_Weight{
	int id;
	double weight;
};

#define MSG_EXIT 1
#define MSG_SENT_SAMPLE_LENGTH 2 
#define MSG_SENT_ZONE_LENGTH 3
#define MSG_SENT_REPORT_LENGTH 4
#define MSG_SENT_SAMPLE_DATA 5
#define MSG_SENT_ZONE_DATA 6
#define MSG_SENT_REPORT_DATA 7
#define MSG_STAT_ZONE 8 
#define MSG_STAT_OVERLAY 9 
#define MSG_STAT_REPORT 10 
#define MSG_SENT_ZONE_COUNT 11


int GetTotalLineCount(FILE* fp)
{   
    int i = 0;
    char strLine[255];
    fseek(fp,0,SEEK_SET);
    while (fgets(strLine, 255, fp))
        i++;
    fseek(fp,0,SEEK_SET);
    return i;
}

int substring_index(const char *s1,const char *s2, int pos){
    int i,j,k;
    for( i = pos ; s1[i] ; i++ ) {
        for( j = i, k = 0 ; s1[j] == s2[k]; j++,k++ ){
            if (! s2[k + 1]) {
                return i;
            }
        }
    }
    return -1;
}

int strstr_cnt(const char *string, const char *substring) {
 int i,j,k,count = 0;
 for (i = 0; string[i]; i++){
	for (j = i, k = 0; (string[j] == substring[k] && (j < strlen(string))); j++,k++) {
		if (! substring[k + 1]) {
			count++;
		}
	}
 }
 return count;
}

tSamples readSamples(char *sampleFilePath){

	char charLine[255];
	int pos;
	int startpos;
	
	string strBuff;
	string strLine;
	
	FILE* fCsv;
	//sampleFilePath = "D:\\sampledata\\xam\\samples.csv";
	fCsv = fopen( sampleFilePath, "r" );
	int sampleCount=GetTotalLineCount(fCsv)-1;
	tSamples samples;

	fseek(fCsv,0,SEEK_SET);
	fgets(charLine,255,fCsv);
	for (int i=0;i<sampleCount;i++)
	{
		if(fgets(charLine,255,fCsv))
		{
			cSample sample;
			startpos=0;
			pos = substring_index(charLine,",",0);
			strLine=(string)charLine;
			strBuff=strLine.substr(startpos,pos-startpos);
			sample.id=(char*)new char(255);
			strcpy(sample.id,strBuff.c_str());
			startpos=pos+1;
			pos=substring_index(charLine,",",pos+1);
			strBuff=strLine.substr(startpos,pos-startpos);			
			sample.pnt.val=atof(strBuff.c_str());
			startpos=pos+1;
			pos=substring_index(charLine,",",pos+1);
			strBuff=strLine.substr(startpos,pos-startpos);			
			sample.pnt.x=atof(strBuff.c_str());
			startpos=pos+1;
			//pos=substring_index(charLine,",",pos+1);
			strBuff=strLine.substr(startpos,strLine.length()-startpos);			
			sample.pnt.y=atof(strBuff.c_str());
			samples.push_back(sample);
		}
	}
	fclose(fCsv);
	return samples;
}

tPoint* readZonePop(char* zoneFilePath){
	char charLine[255];
	
	
	int pos;
	int startpos;
	
	string strBuff;
	string strLine;
	
	FILE* fCsv;
	fCsv = fopen(zoneFilePath, "r" );
	int zonePopCount=GetTotalLineCount(fCsv)-1;
	tPoint* zonePop=(tPoint*)malloc(zonePopCount*sizeof(tPoint));
	fseek(fCsv,0,SEEK_SET);
	fgets(charLine,255,fCsv);
	for (int i=0;i<zonePopCount;i++)
	{
		if(fgets(charLine,255,fCsv))
		{
			startpos=0;
			pos = substring_index(charLine,",",0);			
			strLine=(string)charLine;
			startpos=pos+1;
			pos=substring_index(charLine,",",pos+1);
			strBuff=strLine.substr(startpos,pos-startpos);
			zonePop[i].id=(char*)new char(strBuff.length());
			strcpy(zonePop[i].id,strBuff.c_str());
			startpos=pos+1;
			pos=substring_index(charLine,",",pos+1);
			strBuff=strLine.substr(startpos,pos-startpos);			
			zonePop[i].x=atof(strBuff.c_str());
			startpos=pos+1;			
			strBuff=strLine.substr(startpos,strLine.length()-startpos);			
			zonePop[i].y=atof(strBuff.c_str());
		}
	}
	fclose(fCsv);
	return zonePop;
}

tPoints readZonePopList(char* zoneFilePath){
	char charLine[255];
	
	
	int pos;
	int startpos;
	
	string strBuff;
	string strLine;
	
	FILE* fCsv;
	fCsv = fopen( zoneFilePath, "r" );
	int zonePopCount=GetTotalLineCount(fCsv)-1;
	tPoints zonePop;//=(tPoint*)malloc(zonePopCount*sizeof(tPoint));
	fseek(fCsv,0,SEEK_SET);
	fgets(charLine,255,fCsv);
	for (int i=0;i<zonePopCount;i++)
	{		
		if(fgets(charLine,255,fCsv))
		{
			tPoint pnt;
			startpos=0;
			pos = substring_index(charLine,",",0);			
			strLine=(string)charLine;
			startpos=pos+1;
			pos=substring_index(charLine,",",pos+1);
			strBuff=strLine.substr(startpos,pos-startpos);
			pnt.id=(char*)new char(strBuff.length());
			strcpy(pnt.id,strBuff.c_str());
			startpos=pos+1;
			pos=substring_index(charLine,",",pos+1);
			strBuff=strLine.substr(startpos,pos-startpos);			
			pnt.x=atof(strBuff.c_str());
			startpos=pos+1;			
			strBuff=strLine.substr(startpos,strLine.length()-startpos);			
			pnt.y=atof(strBuff.c_str());
			zonePop.push_back(pnt);
		}
	}
	fclose(fCsv);
	return zonePop;
}

tPoints readReportPopList(char* reportFilePath){
	char charLine[255];
	
	int pos;
	int startpos;
	
	string strBuff;
	string strLine;
	
	FILE* fCsv;
	fCsv = fopen( reportFilePath, "r" );
	int zonePopCount=GetTotalLineCount(fCsv)-1;
	tPoints zonePop;//=(tPoint*)malloc(zonePopCount*sizeof(tPoint));
	fseek(fCsv,0,SEEK_SET);
	fgets(charLine,255,fCsv);
	for (int i=0;i<zonePopCount;i++)
	{		
		if(fgets(charLine,255,fCsv))
		{
			tPoint pnt;
			startpos=0;
			pos = substring_index(charLine,",",0);			
			strLine=(string)charLine;
			startpos=pos+1;
			pos=substring_index(charLine,",",pos+1);
			strBuff=strLine.substr(startpos,pos-startpos);
			pnt.id=(char*)new char(strBuff.length());
			strcpy(pnt.id,strBuff.c_str());
			startpos=pos+1;
			pos=substring_index(charLine,",",pos+1);
			strBuff=strLine.substr(startpos,pos-startpos);			
			pnt.x=atof(strBuff.c_str());
			startpos=pos+1;			
			strBuff=strLine.substr(startpos,strLine.length()-startpos);			
			pnt.y=atof(strBuff.c_str());
			zonePop.push_back(pnt);
		}
	}
	fclose(fCsv);
	return zonePop;
}
bool comp(const tPoint &lhs, const tPoint &rhs)
{
	int i=strcmp(lhs.id,rhs.id);
	if (i<0)
		return true;
	else
		return false;
	//return lhs.id < rhs.id;
}

bool compCnt(const cZone &lzone,const cZone &rzone)
{
	return false;
}

void statZone(tZones &zones,tSamples samples){
	double mean=0;
	double variance =0;
	tPoint pnt;
	cZone zone;
	cSample sample;
	int cnt=zones.size();
	double d;

	list<double> vals;
	double** zoneVals=new double*[cnt];
	int* valCnts=new int[cnt];
	for (int i=0;i<cnt;i++)
	{
		valCnts[i]=0;
		zoneVals[i]=new double[samples.size()];
	}
	tZones::iterator z;
	
	bool flag;//标记sample是否找到对应的zone
	//tSamples smpsBuff;
	for (tSamples::iterator i=samples.begin();i!=samples.end();i++)
	{
		sample=*i;
		z=zones.begin();
		flag=false;
		for(int j=0;j<cnt;j++)
		{
			zone=*z;	
			//zone.samples.clear();
			for (tPoints::iterator k= zone.pnts.begin();k!=zone.pnts.end();k++)
			{
				pnt=*k;
				if (sample.pnt.x==pnt.x && sample.pnt.y==pnt.y)
				{
					zone.samples.push_back(sample);
					//valCnts[j]=valCnts[j]+1;					
					//zoneVals[j][valCnts[j]-1]=sample.pnt.val;					
					flag=true;
					*z=zone;
					break;
				}				
			}
			if (flag) break;
			z++;
		}
	}

	z=zones.begin();
	for (int j=0;j<cnt;j++)
	{
		zone=*z;
		zone.mean=0;
		zone.variance=0;
		
		for (tSamples::iterator k= zone.samples.begin();k!=zone.samples.end();k++)
		{
			sample=*k;
			zone.mean+=sample.pnt.val;
		}
		zone.mean=zone.mean/zone.samples.size();

		for (tSamples::iterator k= zone.samples.begin();k!=zone.samples.end();k++)
		{
			sample=*k;
			d=zone.mean-sample.pnt.val;
			zone.variance+=d*d;
		}
		zone.variance=zone.variance/zone.samples.size();

		*z=zone;
		z++;
	}
	
}

void statZone(cZone &zone, tSamples samples){
	double mean=0;
	double variance =0;
	tPoint pnt;
	
	cSample sample;
	
	double d;
	
	for (tSamples::iterator i=samples.begin();i!=samples.end();i++)
	{
		sample=*i;		
		for (tPoints::iterator k= zone.pnts.begin();k!=zone.pnts.end();k++)
		{
			pnt=*k;
			if (sample.pnt.x==pnt.x && sample.pnt.y==pnt.y)
			{
				zone.samples.push_back(sample);													
				break;
			}				
		}		
	}
		
	zone.mean=0;
	zone.variance=0;
		
	for (tSamples::iterator k= zone.samples.begin();k!=zone.samples.end();k++)
	{
		sample=*k;
		zone.mean+=sample.pnt.val;
	}
	zone.mean=zone.mean/zone.samples.size();

	for (tSamples::iterator k= zone.samples.begin();k!=zone.samples.end();k++)
	{
		sample=*k;
		d=zone.mean-sample.pnt.val;
		zone.variance+=d*d;
	}
	zone.variance=zone.variance/zone.samples.size();

}

tZones getZonesFromPop(tPoints zonePop){
	tZones zones; //要返回的值
	
	zonePop.sort(comp);//对zonePop集合中的元素排序

	tPoint pnt;//zonePop集合中的元素

	cZone zone;//zones中的元素
	
	//首先将zonePop中的第一个zoneID加入到zones中
	pnt=zonePop.front();
	zone.id=pnt.id;
	zones.push_back(zone);

	//找到zones最后一个元素的指针z
	tZones::iterator z;//zones中的元素指针
	z=zones.end();
	z--;
	zone=*z;//获取zones中的最后一个元素

	tPoints pntsBuff;//每个zone中的pntlist

	for (tPoints::iterator i=zonePop.begin();i!=zonePop.end();i++){				
		
		pnt=*i;

		if (strcmp(zone.id,pnt.id)!=0){
			//如果出现新的ID，则将临时zone对象更新到zones中的最后一项
			zone.pnts=pntsBuff;
			//statZone(zone);
			z=zones.end();
			z--;
			*z=zone;
			//然后创建新的zone对象，并加入到zones的最后一项上
			pntsBuff.clear();//=tPoints();//创建一个新的zone中的pntlist						
			zone.id=pnt.id;
			pntsBuff.push_back(pnt);
			zone.pnts=pntsBuff;
			zones.push_back(zone);		
		}
		else{
			pntsBuff.push_back(pnt);			
		}
	}
	zone.pnts=pntsBuff;
	//statZone(zone);
	z=zones.end();
	z--;
	*z=zone;
	return zones;
}

tReports getReportsFromPop(tPoints reportPop)
{
	tReports reports; //要返回的值
	
	reportPop.sort(comp);//对zonePop集合中的元素排序

	tPoint pnt;//zonePop集合中的元素

	cReport report;//zones中的元素
	
	//首先将zonePop中的第一个zoneID加入到zones中
	pnt=reportPop.front();
	report.id=pnt.id;
	reports.push_back(report);

	//找到zones最后一个元素的指针z
	tReports::iterator z;//zones中的元素指针
	z=reports.end();
	z--;
	report=*z;//获取zones中的最后一个元素

	tPoints pntsBuff;//每个zone中的pntlist

	for (tPoints::iterator i=reportPop.begin();i!=reportPop.end();i++){				
		
		pnt=*i;

		if (strcmp(report.id,pnt.id)!=0){
			//如果出现新的ID，则将临时zone对象更新到zones中的最后一项
			report.pnts=pntsBuff;
			report.mean=0;
			report.variance=0;
			//statZone(zone);
			z=reports.end();
			z--;
			*z=report;
			//然后创建新的zone对象，并加入到zones的最后一项上
			pntsBuff.clear();//=tPoints();//创建一个新的zone中的pntlist						
			report.id=pnt.id;
			pntsBuff.push_back(pnt);
			report.pnts=pntsBuff;
			reports.push_back(report);		
		}
		else{
			pntsBuff.push_back(pnt);			
		}
	}
	report.pnts=pntsBuff;
	report.mean=0;
	report.variance=0;
	//statZone(zone);
	z=reports.end();
	z--;
	*z=report;
	return reports;
}

double getWeightOfOverlay(int oid, tZones zones, tReports reports)
{
	int rid,zid,pnum;
	double weight;
	tPoint rpnt,zpnt;

	rid = oid/zones.size();
	zid = oid - rid*zones.size();
	cZone zone = zones[zid];
	cReport report = reports[rid];
	pnum = 0;
	for (tPoints::iterator rp=report.pnts.begin();rp!=report.pnts.end();rp++)
	{
		rpnt=*rp;
		for (tPoints::iterator zp=zone.pnts.begin();zp!=zone.pnts.end();zp++)
		{
			zpnt=*zp;
			if (zpnt.x==rpnt.x && zpnt.y==rpnt.y)
			{
				pnum++;
			}
		}
	}
	weight=(double)pnum/(double)report.pnts.size();
	return weight;
}

tOverlays getOverlays(tZones zones,tReports reports)
{
	tOverlays overlays;
	cOverlay overlay;
	cReport report;
	cZone zone;
	tPoint zpnt,rpnt;
	int pntNum;

	for (tReports::iterator i=reports.begin();i!=reports.end();i++)
	{
		report=*i;
		for (tZones::iterator j=zones.begin();j!=zones.end();j++)
		{
			zone=*j;
			overlay.reportID=report.id;
			overlay.zoneID=zone.id;
			overlay.mean=zone.mean;
			overlay.variance=zone.variance;
			overlay.weight=0;
			pntNum=0;
			for (tPoints::iterator rp=report.pnts.begin();rp!=report.pnts.end();rp++)
			{
				rpnt=*rp;
				for (tPoints::iterator zp=zone.pnts.begin();zp!=zone.pnts.end();zp++)
				{
					zpnt=*zp;
					if (zpnt.x==rpnt.x && zpnt.y==rpnt.y)
					{
						pntNum++;
					}
				}
			}
			overlay.weight=(double)pntNum/(double)report.pnts.size();
			overlays.push_back(overlay);
		}
	}
	return overlays;
}

void statReport(tReports &reports,tOverlays overlays)
{
	cOverlay overlay;
	cReport report;
	for (tReports::iterator i=reports.begin();i!=reports.end();i++)
	{
		report=*i;
		report.mean=0;
		report.variance=0;
		for (tOverlays::iterator j=overlays.begin();j!=overlays.end();j++)
		{
			overlay=*j;
			if (strcmp(overlay.reportID,report.id)==0)
			{
				report.mean=report.mean+overlay.mean*overlay.weight;
				report.variance=report.variance+overlay.variance*overlay.weight*overlay.weight;
			}
		}
		*i=report;
	}
}

char* deSampleList(tSamples samples, int &length)
{
	char *msamples = (char *)malloc((unsigned int)10000000);
	cSample sample;
	int size=sizeof(int);//size of each of block
	length=0; //comulated size of all blocks
	int count=samples.size(); //count of elements in lists
		
	memcpy(msamples, &count,size);
	length=length+size;

	for (tSamples::iterator i=samples.begin();i!=samples.end();i++)
	{
		sample=*i;
		size=strlen(sample.id)+1;
		count = strlen(sample.id)+1;
		memcpy(&msamples[length],&count,sizeof(int));
		length = length + sizeof(int);
		memcpy(&msamples[length],sample.id,size);
		length=length+size;

		size=sizeof(double);				
		memcpy(&msamples[length],&sample.pnt.x,size);
		length = length + size;
		memcpy(&msamples[length],&sample.pnt.y,size);
		length = length + size;
		memcpy(&msamples[length],&sample.pnt.val,size);
		length = length + size;				
	}
	return msamples;
}

tSamples conSampleList(char* mSamples)
{
	tSamples samples;
	int sampleCount;
	std::size_t size;
	int csize;
	int count;
	int length=0;

	memcpy(&sampleCount,&mSamples[0],sizeof(int));
	length = length + sizeof(int);
	for (int i=0;i<sampleCount;i++)
	{
		cSample sample;
		sample.id=new char[255];
		size = sizeof(int);
		memcpy(&count,&mSamples[length],size);		
		length = length + size;
		memcpy(sample.id,&mSamples[length],count);
		length=length+count;

		size = sizeof(double);
		memcpy(&sample.pnt.x,&mSamples[length],size);
		length=length+size;

		memcpy(&sample.pnt.y,&mSamples[length],size);
		length=length+size;

		memcpy(&sample.pnt.val,&mSamples[length],size);
		length=length+size;

		samples.push_back(sample);
	}
	return samples;
}

char* deZoneList(tZones zones,int &length)
{
	char *mzones = (char *)malloc((unsigned int)10000000);
	cZone zone;
	int size=sizeof(int);//size of each of block
	length=0; //comulated size of all blocks
	int count=zones.size(); //count of elements in lists
		
	memcpy(mzones, &count,size);
	length=length+size;

	for (tZones::iterator i=zones.begin();i!=zones.end();i++)
	{
		zone=*i;
		size=strlen(zone.id)+1;
		count = strlen(zone.id)+1;
		memcpy(&mzones[length],&count,sizeof(int));
		length = length + sizeof(int);
		memcpy(&mzones[length],zone.id,size);
		length=length+size;
		
		size=sizeof(double);				
		memcpy(&mzones[length],&zone.mean,size);
		length = length + size;
		memcpy(&mzones[length],&zone.variance,size);
		length = length + size;
		
		count = zone.pnts.size();
		size=sizeof(int);				
		memcpy(&mzones[length],&count,size);
		length = length + size;

		for (tPoints::iterator j=zone.pnts.begin();j!=zone.pnts.end();j++)
		{
			tPoint pnt = *j;
			size=sizeof(double);				
			memcpy(&mzones[length],&pnt.x,size);
			length = length + size;
			memcpy(&mzones[length],&pnt.y,size);
			length = length + size;
			/*memcpy(&mzones[length],&pnt.val,size);
			length = length + size;	*/
		}		
	}
	return mzones;
}

tZones conZoneList(char* mzones)
{
	tZones zones;
	
	int size=sizeof(int);//size of each of block
	int length=0; //comulated size of all blocks
	int zoneCount=zones.size(); //count of elements in lists
	int count=0;
	int pntCount=0;

	memcpy(&zoneCount,mzones, size);
	length=length+size;

	for (int i = 0; i< zoneCount; i++)
	{
		cZone zone;
		zone.id = new char[255];
		size = sizeof(int);
		memcpy(&count, &mzones[length], size);
		length = length + size;	
		memcpy(zone.id, &mzones[length], count);
		length=length+count;
		
		size=sizeof(double);				
		memcpy(&zone.mean,&mzones[length],size);
		length = length + size;
		memcpy(&zone.variance,&mzones[length],size);
		length = length + size;
		
		size=sizeof(int);
		memcpy(&pntCount,&mzones[length],size);
		length = length + size;
		for (int j=0;j<pntCount;j++)
		{			
			tPoint pnt;
			size=sizeof(double);				
			memcpy(&pnt.x,&mzones[length],size);
			length = length + size;
			memcpy(&pnt.y,&mzones[length],size);
			length = length + size;
			/*memcpy(&pnt.val,&mzones[length],size);
			length = length + size;	*/
			zone.pnts.push_back(pnt);
		}	
		zones.push_back(zone);
	}
	return zones;
}

char* deReportList(tReports reports,int &length)
{
	char *mreports = (char *)malloc((unsigned int)10000000);
	cReport report;
	int size=sizeof(int);//size of each of block
	length=0; //comulated size of all blocks
	int count=reports.size(); //count of elements in lists
		
	memcpy(mreports, &count,size);
	length=length+size;

	for (tReports::iterator i=reports.begin();i!=reports.end();i++)
	{
		report=*i;
		size=strlen(report.id)+1;
		count = strlen(report.id)+1;
		memcpy(&mreports[length],&count,sizeof(int));
		length = length + sizeof(int);
		memcpy(&mreports[length],report.id,size);
		length=length+size;
		
		size=sizeof(double);				
		memcpy(&mreports[length],&report.mean,size);
		length = length + size;
		memcpy(&mreports[length],&report.variance,size);
		length = length + size;
		
		count = report.pnts.size();
		size=sizeof(int);				
		memcpy(&mreports[length],&count,size);
		length = length + size;

		for (tPoints::iterator j=report.pnts.begin();j!=report.pnts.end();j++)
		{
			tPoint pnt = *j;
			size=sizeof(double);				
			memcpy(&mreports[length],&pnt.x,size);
			length = length + size;
			memcpy(&mreports[length],&pnt.y,size);
			length = length + size;
			/*memcpy(&mreports[length],&pnt.val,size);
			length = length + size;	*/
		}		
	}
	return mreports;
}

tReports conReportList(char* mreports)
{
	tReports reports;
	
	int size=sizeof(int);//size of each of block
	int length=0; //comulated size of all blocks
	int reportCount=reports.size(); //count of elements in lists
	int count=0;
	int pntCount=0;

	memcpy(&reportCount,mreports, size);
	length=length+size;

	for (int i = 0; i< reportCount; i++)
	{
		cReport report;
		report.id = new char[255];
		size = sizeof(int);
		memcpy(&count, &mreports[length], size);
		length = length + size;	
		memcpy(report.id, &mreports[length], count);
		length=length+count;
		
		size=sizeof(double);				
		memcpy(&report.mean,&mreports[length],size);
		length = length + size;
		memcpy(&report.variance,&mreports[length],size);
		length = length + size;
		
		size=sizeof(int);
		memcpy(&pntCount,&mreports[length],size);
		length = length + size;
		for (int j=0;j<pntCount;j++)
		{			
			tPoint pnt;
			size=sizeof(double);				
			memcpy(&pnt.x,&mreports[length],size);
			length = length + size;
			memcpy(&pnt.y,&mreports[length],size);
			length = length + size;
			/*memcpy(&pnt.val,&mreports[length],size);
			length = length + size;	*/
			report.pnts.push_back(pnt);
		}	
		reports.push_back(report);
	}
	return reports;
}

void readParameters(char* &sampleFile, char* &zoneFile, char* &reportFile, const char* parameterFile)
{
	char charLine[255];
	int pos;
	int startpos;
	
	sampleFile = new char[255];
	zoneFile = new char[255];
	reportFile = new char[255];

	string strBuff;
	string strLine;
	
	FILE* fCsv;
	fCsv = fopen( parameterFile, "r" );
	
	fseek(fCsv,0,SEEK_SET);
	fgets(sampleFile,255,fCsv);	
	fgets(zoneFile,255,fCsv);
	fgets(reportFile,255,fCsv);
	fclose(fCsv);
}

int main(int argc, char* argv[])
{
	int myid,numprocs;
	int namelen;
	char* csvFilePath;
	char* zoneFilePath;
	char* reportFilePath;
	
	char* msamples ;
	char* mzones;
	char* mreports;
	char* mblock;

	int length=0;
	int mSmpLen=0;
	int mZonLen=0;
	int mRepLen=0;
	int idx=0;
	int flag=0;

	
	tSamples samples;
	tPoints zonePop;
	tZones zones;
	tPoints reportPop;
	tReports reports;
	tOverlays overlays;
	cZone zone;
	cReport report;
	cOverlay overlay;	

	sRecv* zoneVals;

	double start , finish, ioSpan, comSpan;	

	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Init(&argc,&argv);/**//*程序初始化*/
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);/**//*得到当前进程号*/
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);/**//*得到总的进程数*/
	MPI_Get_processor_name(processor_name,&namelen);/**//*得到机器名*/

	MPI_Status status;

	sRecv value;
	 
	MPI_Datatype rst;
	int blocklens[3];
	MPI_Aint indices[3];
	MPI_Datatype old_types[3];

	blocklens[0] = 1;
	blocklens[1] = 1;
	blocklens[2] = 1;

	old_types[0] = MPI_INT;
	old_types[1] = MPI_DOUBLE;
	old_types[2] = MPI_DOUBLE;

	MPI_Address( &value.id, &indices[0] );
	MPI_Address( &value.mean, &indices[1] );
	MPI_Address( &value.variance, &indices[2] );

	indices[2] = indices[2] - indices[0];
	indices[1] = indices[1] - indices[0];
	indices[0] = 0;

	MPI_Type_struct(3,blocklens,indices,old_types,&rst);
	MPI_Type_commit( &rst );

	// define overlay structure for mpi
	sRecv_Weight oWeight;
	MPI_Datatype rstOverlay;
	int oblocklens[2];
	MPI_Aint oindices[2];
	MPI_Datatype oold_types[2];

	oblocklens[0] = 1;
	oblocklens[1] = 1;
	
	oold_types[0] = MPI_INT;
	oold_types[1] = MPI_DOUBLE;

	MPI_Address( &oWeight.id, &oindices[0] );
	MPI_Address( &oWeight.weight, &oindices[1] );	

	oindices[1] = oindices[1] - oindices[0];
	oindices[0] = 0;

	MPI_Type_struct(2,oblocklens,oindices,oold_types,&rstOverlay);
	MPI_Type_commit( &rstOverlay );
	
	if(myid == 0)
	{		
		csvFilePath=argv[1];
		zoneFilePath=argv[2];
		reportFilePath=argv[3];

              ofstream resultFile( argv[4], ios::out );
              //cout<<"argv[4]"<<endl;
		start = MPI_Wtime();

		samples=readSamples(csvFilePath);
		zonePop=readZonePopList(zoneFilePath);		
		reportPop=readReportPopList(reportFilePath);

		zones=getZonesFromPop(zonePop);				
		reports=getReportsFromPop(reportPop);
		
		finish = MPI_Wtime();		
		ioSpan = finish - start;		

		msamples = deSampleList(samples,mSmpLen);
		mzones = deZoneList(zones,mZonLen);
		mreports = deReportList(reports,mRepLen);
		
		for (int i=1;i<numprocs;i++)
		{
			MPI_Send(&mSmpLen, 1, MPI_INT, i, MSG_SENT_SAMPLE_LENGTH, MPI_COMM_WORLD);
			MPI_Send(msamples, mSmpLen, MPI_CHAR, i, MSG_SENT_SAMPLE_DATA, MPI_COMM_WORLD);
			MPI_Send(&mZonLen, 1, MPI_INT, i, MSG_SENT_ZONE_LENGTH, MPI_COMM_WORLD);
			MPI_Send(mzones, mZonLen, MPI_CHAR, i, MSG_SENT_ZONE_DATA, MPI_COMM_WORLD);
			MPI_Send(&mRepLen, 1, MPI_INT, i, MSG_SENT_REPORT_LENGTH, MPI_COMM_WORLD);
			MPI_Send(mreports, mRepLen, MPI_CHAR, i, MSG_SENT_REPORT_DATA, MPI_COMM_WORLD);
			//fprintf(stderr," sent over\n");
		}		
		delete []msamples;
		delete []mzones;
		delete []mreports;
		int nrecv = numprocs-1;
		
		for (int j = 0; j<zones.size(); j=j+numprocs)
		{				
			statZone(zones[j],samples);				
		}

		while(nrecv > 0)
		{
			MPI_Recv(&value, 1, rst, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );

			switch (status.MPI_TAG) 
			{
				case MSG_EXIT: 
					nrecv--;
					break;
				case MSG_STAT_ZONE:
					zones[value.id].mean=value.mean;
					zones[value.id].variance=value.variance;
					break;
			}
		}		
		int n = zones.size();
		zoneVals = new sRecv[n];//(sRecv*)malloc(sizeof(sRecv)*zones.size());
		for (int i=0;i<n;i++)
		{
			zoneVals[i].id=i;
			zoneVals[i].mean=zones[i].mean;
			zoneVals[i].variance=zones[i].variance;
		}

		for (int i=1;i<numprocs;i++)
		{
			MPI_Send(zoneVals, n, rst, i, MSG_SENT_ZONE_COUNT, MPI_COMM_WORLD);
		}
		
		for (int i=0;i<reports.size();i++)
		{
			for (int j=0;j<zones.size();j++)
			{
				cOverlay temp;
				temp.reportID=reports[i].id;
				temp.zoneID,zones[j].id;
				temp.mean=zones[j].mean;
				temp.variance=zones[j].variance;
				temp.weight=0;
				overlays.push_back(temp);
			}
		}

		nrecv = numprocs-1;
		for (int i = 0;i<overlays.size();i=i+numprocs)
		{
			overlays[i].weight = getWeightOfOverlay(i,zones,reports);
		}
		while (nrecv > 0)
		{
			MPI_Recv(&oWeight, 1, rstOverlay, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
			switch (status.MPI_TAG) 
			{
				case MSG_EXIT: 
					nrecv--;
					break;
				case MSG_STAT_OVERLAY:
					overlays[oWeight.id].weight=oWeight.weight;					
					break;
			}			
		}
		//tOverlays overlays=getOverlays(zones,reports);
		//
		statReport(reports,overlays);

		for (tReports::iterator i=reports.begin();i!=reports.end();i++)
		{
			report=*i;
			//cout<<report.id<<" , "<<report.mean<<" , "<<report.variance<<"\n" ;
			//fprintf(stderr,"report id: %s, mean: %f, variance: %f \n",report.id,report.mean,report.variance);
                     resultFile<<"report id:"<<report.id<<" , report mean:"<<report.mean<<" , report variance"<<report.variance<<"\n" ;
		}
		comSpan = MPI_Wtime() - finish;
		//fprintf(stderr,"[DEBUG][TIMESPAN][IO] %f \n", ioSpan);
		//fprintf(stderr,"[DEBUG][TIMESPAN][COMPUTING] %f \n", comSpan);
		//fprintf(stderr,"[DEBUG][TIMESPAN][TOTAL] %f \n",ioSpan+comSpan);
                cout<<endl;
                cout<<"[DEBUG]  time consuming:"<<endl;
		cout<<"[DEBUG][TIMESPAN][IO]"<<ioSpan<<endl;
		cout<<"[DEBUG][TIMESPAN][COMPUTING]"<<comSpan<<endl;
		cout<<"[DEBUG][TIMESPAN][TOTAL]"<<ioSpan+comSpan<<endl;
	}
	else
	{		
		//fprintf(stderr," recive s\n");
		MPI_Recv(&mSmpLen, 1, MPI_INT, 0, MSG_SENT_SAMPLE_LENGTH, MPI_COMM_WORLD, &status );					
		msamples = (char *)malloc(mSmpLen);				
		MPI_Recv(msamples, mSmpLen, MPI_CHAR, 0, MSG_SENT_SAMPLE_DATA, MPI_COMM_WORLD, &status );			
		samples = conSampleList(msamples);						
				
		MPI_Recv(&mZonLen, 1, MPI_INT, 0, MSG_SENT_ZONE_LENGTH, MPI_COMM_WORLD, &status );
		mzones = (char*) malloc(mZonLen);
		MPI_Recv(mzones, mZonLen, MPI_CHAR, 0, MSG_SENT_ZONE_DATA, MPI_COMM_WORLD, &status );					
		zones = conZoneList(mzones);

		MPI_Recv(&mRepLen, 1, MPI_INT, 0, MSG_SENT_REPORT_LENGTH, MPI_COMM_WORLD, &status );
		mreports = (char*) malloc(mRepLen);
		MPI_Recv(mreports, mRepLen, MPI_CHAR, 0, MSG_SENT_REPORT_DATA, MPI_COMM_WORLD, &status );					
		reports = conReportList(mreports);
					
		//reports = conReportList(mreports);
		tZones::iterator z=zones.begin();
		/*finish = MPI_Wtime();
		fprintf(stderr," con time: %f \n", finish - start);*/

		for (int j = myid; j<zones.size(); j=j+numprocs)
		{

			z=zones.begin()+j;			
			zone = *z;
			statZone(zone,samples);
			value.id = j;
			value.mean = zone.mean;
			value.variance = zone.variance;
			MPI_Send(&value, 1, rst, 0, MSG_STAT_ZONE,MPI_COMM_WORLD );
			
		}
		MPI_Send(&value, 1, rst, 0, MSG_EXIT,MPI_COMM_WORLD );
		
		int n=zones.size();
		sRecv* zoneVals = new sRecv[n];
		MPI_Recv(zoneVals, n, rst, 0, MSG_SENT_ZONE_COUNT, MPI_COMM_WORLD, &status );
		for (int i=0;i<zones.size();i++)
		{
			zones[i].mean=zoneVals[i].mean;
			zones[i].variance=zoneVals[i].variance;
		}

		for (int i=0;i<reports.size();i++)
		{
			for (int j=0;j<zones.size();j++)
			{
				cOverlay temp;
				temp.reportID=reports[i].id;
				temp.zoneID,zones[j].id;
				temp.mean=zones[j].mean;
				temp.variance=zones[j].variance;
				temp.weight=0;
				overlays.push_back(temp);
			}
		}
		
		for (int i = myid; i<overlays.size(); i=i+numprocs)
		{
			oWeight.id = i;
			oWeight.weight = getWeightOfOverlay(i,zones,reports);
			MPI_Send(&oWeight, 1, rstOverlay, 0, MSG_STAT_OVERLAY,MPI_COMM_WORLD );
		}
		MPI_Send(&oWeight, 1, rstOverlay, 0, MSG_EXIT,MPI_COMM_WORLD );
	}
	
	MPI_Finalize();/**//*结束*/
	//fprintf(stderr," Process %d exit ok \n",myid);
	/*int n;	
	scanf("%d", &n);*/
	return 0;
	
}




//#define MSG_EXIT 1
#define MSG_PRINT_ORDERED 2 /*定义按续打印标识*/
#define MSG_PRINT_UNORDERED 3 /*定义乱续打印标识*/
/* 下面的函数为主进程执行的部分 */
int master_io( void )
{
	int i,j, size, nslave, firstmsg;
	char buf[256], buf2[256];
	MPI_Status status;
	MPI_Comm_size( MPI_COMM_WORLD, &size );/*得到总的进程数*/
	nslave = size - 1;/*得到从进程的进程数*/
	while (nslave > 0) 
	{/* 只要还有从进程则执行下面的接收与打印*/
		MPI_Recv( buf, 256, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );/*从任意从进程接收任意标识的消息*/
		switch (status.MPI_TAG) 
		{
			case MSG_EXIT: 
				nslave--; 
				break;/*若该从进程要求退出则将总的从进程个数减1*/
			case MSG_PRINT_UNORDERED:/*若该从进程要求乱续打印则直接将该消息打印*/
				fputs( buf, stdout );
				break;
			case MSG_PRINT_ORDERED:/*从进程要求按续打印这种打印方式处理起来复杂一
						些首先需要对收到的消息进行排序若有些消息还没有收到则需要调用接收语句接收相
						应的有序消息*/
				firstmsg = status.MPI_SOURCE;
				for (i=1; i<size; i++) 
				{/*标识号从小到大开始打印*/
					if (i == firstmsg)
						fputs( buf, stdout );/*若接收到的消息恰巧是需要打印的消息则直接打印*/
					else 
					{/* 否则先接收需要打印的消息然后再打印*/
						MPI_Recv( buf2, 256, MPI_CHAR, i, MSG_PRINT_ORDERED, MPI_COMM_WORLD, &status );/*注意这一接收语句指定了源进程和消息标
									识而不是象一开始的那样用任意源和任意标识*/
						fputs( buf2, stdout );
					}
				}
				break;
		}
	}
	return 0;
}

/*下面的函数为从进程执行的部分*/
int slave_io( void )
{
	char buf[256];
	int rank;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );/*得到自己的标识*/
	
	sprintf( buf, "Goodbye from slave %d, ordered print\n", rank );	
	MPI_Send( buf, strlen(buf) + 1, MPI_CHAR, 0, MSG_PRINT_ORDERED,	MPI_COMM_WORLD );/*再向主进程发送一个有序打印消息*/
	
	sprintf( buf, "Hello from slave %d ordered print\n", rank );	
	MPI_Send( buf, strlen(buf) + 1, MPI_CHAR, 0, MSG_PRINT_ORDERED,MPI_COMM_WORLD );/*先向主进程发送一个有序打印消息*/
	
	sprintf( buf, "I'm exiting (%d),unordered print\n", rank );
	MPI_Send( buf, strlen(buf) + 1, MPI_CHAR, 0, MSG_PRINT_UNORDERED,MPI_COMM_WORLD );/*最后向主进程发送一个乱续打印消息*/
	
	MPI_Send( buf, 0, MPI_CHAR, 0, MSG_EXIT, MPI_COMM_WORLD );/*向主进程发送退出执行的消息*/
	return 0;
}
int _tmain1(int argc, char* argv[])
{
	int rank, size;
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	if (rank == 0)
		master_io(); /*进程0作为主进程*/
	else
		slave_io(); /*其它进程作为从进程*/
	MPI_Finalize( );
	return 0;
}
//length = sizeof(int) *3 + mSmpLen + mZonLen + 1;
//		mblock = (char *) malloc(length);
//		memcpy(mblock,&mSmpLen,sizeof(int));
//		memcpy(&mblock[sizeof(int)], &mZonLen,sizeof(int));
//		//memcpy(&mblock[sizeof(int)*2], &mRepLen,sizeof(int));
//		memcpy(&mblock[sizeof(int)*2], msamples, mSmpLen);
//		memcpy(&mblock[sizeof(int)*2 + mSmpLen], mzones,mZonLen);
//		//memcpy(&mblock[sizeof(int)*2 + mSmpLen + mZonLen], mreports, mRepLen);
//		//MPI_Bcast(&length, 1, MPI_INT, 0, MPI_COMM_WORLD );
//		//MPI_Bcast(mblock, length, MPI_CHAR, 0, MPI_COMM_WORLD );
//int _tmain(int argc, char* argv[])
//{
//	int myid,numprocs;
//	int namelen;
//	char* csvFilePath;
//	char* zoneFilePath;
//	char* reportFilePath;
//	
//	int length=0;
//	int mSmpLen=0;
//	int mZonLen=0;
//	int mRepLen=0;
//
//
//	tSamples samples;
//	tPoints zonePop;
//	tZones zones;
//
//	char processor_name[MPI_MAX_PROCESSOR_NAME];
//	MPI_Init(&argc,&argv);/**//*程序初始化*/
//	MPI_Comm_rank(MPI_COMM_WORLD,&myid);/**//*得到当前进程号*/
//	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);/**//*得到总的进程数*/
//	MPI_Get_processor_name(processor_name,&namelen);/**//*得到机器名*/
//	fprintf(stderr," Process %d of %d SAY HELLO TO MPI on %s\n",
//	myid, numprocs, processor_name);
//	//std::cin>>zoneLayerFile[255];
//	
//	if(myid == 0)
//	{
//		csvFilePath="D:\\地理所\\mpiProgram\\mytest\\data\\xam\\samples.csv";
//		zoneFilePath="D:\\地理所\\mpiProgram\\mytest\\data\\xam\\zonation.csv";
//		reportFilePath="D:\\地理所\\mpiProgram\\mytest\\data\\xam\\rep_adm.csv";
//		samples=readSamples(csvFilePath);
//		zonePop=readZonePopList(zoneFilePath);
//		zones=getZonesFromPop(zonePop);
//		tPoints reportPop=readReportPopList(reportFilePath);
//		tReports reports=getReportsFromPop(reportPop);
//
//		char* smps = deSampleList(samples,mSmpLen);
//		char* mzones = deZoneList(zones,mZonLen);
//		char* mreports = deReportList(reports,mRepLen);
//
//		length = mSmpLen + mZonLen + mRepLen + 1;
//		char* mblock = (char *) malloc(length);
//
//		//MPI_Bcast(mblock, length, MPI_CHAR, 0, MPI_COMM_WORLD )
//
//		
//		samples = conSampleList(smps);
//		zones = conZoneList(mzones);
//		reports = conReportList(mreports);		
//		
//		statZone(zones,samples);
//		cZone zone;
//		for (tZones::iterator i=zones.begin();i!=zones.end();i++)
//		{
//			zone=*i;
//			cout<<zone.id<<" , "<<zone.mean<<" , "<<zone.variance<<"\n";		
//		}
//		
//		
//
//		
//		
//		tOverlays overlays=getOverlays(zones,reports);
//		
//		statReport(reports,overlays);
//
//		cReport report;
//		for (tReports::iterator i=reports.begin();i!=reports.end();i++)
//		{
//			report=*i;
//			cout<<report.id<<" , "<<report.mean<<" , "<<report.variance<<"\n" ;
//		}
//
//		
//
//		
//
//	}
//	MPI_Finalize();/**//*结束*/
//	int n;
//	
//	scanf("%d", &n);
//	return 0;
//	
//}
