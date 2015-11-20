
#include <stdio.h>

#include <algorithm>

#include <iostream>
#include <fstream>

#include <string>

#include "stdlib.h"

#include <math.h>
#include <time.h>

#include "mpi.h"

#include "gdal_priv.h"
#include "gdal.h"
#include "ogrsf_frmts.h"


using namespace std;


#define MAX_Xiang    1024                // 处理的乡镇数目的上限
#define MAX_V        9999                // 乡镇序号的上限 表示一个不可能存在的序号
#define MAX_Clu      300                 // 单个候选聚集区域内 最多可包括的乡镇数目
#define MAX_Double   99999999            // 为了便于找MAX_LLR的最大值 预先设定一个极小的 -1 * MAX_Double


                                                     // 一个格网点 与某乡镇点的距离结构体 为了便于排序
struct struTem
{
	int xuhao;                     // 乡镇点的序号
	float dist;                    // 格网与该乡镇点的距离
} ;
                                                     // 比较上面结构体 用于排序函数中
bool compareM (struTem a, struTem b)
{
      return  a.dist < b.dist ;
}
                                                     // 存储某个候选聚集区域所包括的乡镇点的序号 为了便于排序去重
struct StrMultSort
{
	int Data[MAX_Clu+1];        
};
                                                     // 比较上面结构体中数组的某个单元 用于排序函数中
bool compareMS (StrMultSort a, StrMultSort b)
{
      int  tem=a.Data[0];
      return  a.Data[tem] < b.Data[tem] ;
}
                                                     // Cand中保存着最初的所有候选聚集区域 利用多重排序法 去除重复
void MultiSort (int(*Cand)[MAX_Clu+1], int length, int width, bool FirSort, int *ResLength)
{                                                        
	                                                     // Cand二维数组的有效长度length 有效宽度width
	                                                     // 是否第一次排序去重FirSort 去重之后的有效长度ResLength

	StrMultSort *TemStrL;
	TemStrL = (StrMultSort *) malloc ((length+1)*sizeof(StrMultSort));
	for (int k=1; k<=length; k++)                                         // 将需要处理的Cand数据 传输至结构体数组
		for (int t=1; t<=width; t++)  TemStrL[k].Data[t] = Cand[k][t]; 



	if (FirSort)                                                        // 首次排序 每一行的数据需要排序
	{ 
		for (int k=1; k<=length; k++)
  		 stable_sort (TemStrL[k].Data+1, TemStrL[k].Data+(width+1));
	 }



	for (int x=width; x>=1; x--)                                        // 从最后一列开始 依次排序
	{
		for (int y=1; y<=length; y++) TemStrL[y].Data[0]=x;
		stable_sort (TemStrL+1, TemStrL+(length+1), compareMS);
	}



	for (int k=2; k<=length; k++)                             // 对重复的结构体单元 加上记录标志
	{
		bool temBool=true;
		
		for (int x=1; x<=width; x++) 
			if (TemStrL[k].Data[x]!=TemStrL[k-1].Data[x]) { temBool=false; break; }
		
		if (temBool) TemStrL[k].Data[0]=0;        // 表示第k个记录与前一个记录相同 打标记 可删除
	}



	int temNo=1;                                                 // 记录非重复的行 的数目
	
	for (int x=1; x<=width; x++) Cand[1][x]=TemStrL[1].Data[x];  // 结构体数组的第一个单元 写入 Cand数组第一行

	for (int k=2; k<=length; k++)
	{
		if (TemStrL[k].Data[0]==0) continue;                      // 重复的结构体单元 不用写入Cand数组
		temNo++;
        for (int x=1; x<=width; x++) Cand[temNo][x]=TemStrL[k].Data[x];
	}

	*ResLength = temNo;

	free ((StrMultSort *)TemStrL);
}

void MaxLlrFun (int (*Cand)[MAX_Clu+1], int (*Clu_Popu)[2], int *X_Case, int FinaResLength, int MAX_Clu_F, 
	            int CaseSum,         double *maxLLR, int *maxNo)
{
	                                               // Cand 最终的候选聚集区域集合 有效长度FinaResLength 有效宽度MAX_Clu_F
	                                          // Clu_Popu 每个聚集区域内外的病例个数（已经预先计算） X_Case 每个点的病例数
	                                               // CaseSum 病例总数            maxLLR maxNo  找到的最大可能聚集区域序号

	*maxLLR=(-1)*MAX_Double;           // 预先给一个极小值
	double tmpLLR;
	int clu_case;

	for (int k=1; k<=FinaResLength; k++)                        // 依次考察每一个候选聚集区域 
	{
		clu_case=0;

		for (int x=1; x<=MAX_Clu_F; x++)                       // 得到当前候选聚集区域的病例数
            if (Cand[k][x]==MAX_V) break;
		    else clu_case+=X_Case[Cand[k][x]];

		if ((float)clu_case/Clu_Popu[k][0] <= (float)(CaseSum-clu_case)/Clu_Popu[k][1]) continue;

		tmpLLR = clu_case * log( (double)clu_case / Clu_Popu[k][0] );

		if ( CaseSum>clu_case ) tmpLLR+=(CaseSum-clu_case) * log((double)(CaseSum-clu_case) / Clu_Popu[k][1]);

		if (tmpLLR <= *maxLLR) continue;

		*maxLLR = tmpLLR;
		*maxNo = k;
	}

}



int main(int argc, char* argv[])
{

	MPI_Init(&argc,(char***)&argv);

	int ProSize, ProRank;                          // 总的进程数目   当前进程的序号(序号从0开始)
	MPI_Comm_size(MPI_COMM_WORLD, &ProSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProRank);


    // char  Shape_Filename[] = "popu_case.shp";
	// char  Result_Filename[] = "result.txt";

	char Shape_Filename[50]={0};	
	strcpy(Shape_Filename,argv[1]);

	char Result_Filename[50]={0};	
	strcpy(Result_Filename,argv[2]);


	float GridRatio,  CluRatio;     // 格网密度的上限  单个候选聚集区域度量值的最大可能比例
	int   MonteCarlo;              //  蒙特卡罗模拟次数
		
	GridRatio = 0.2;  CluRatio = 0.1;
	MonteCarlo = 200;

	GridRatio  =  atof(argv[3]);
	CluRatio   =  atof(argv[4]);
	MonteCarlo =  atoi(argv[5]);
	
	string dbname=argv[6];string hostname=argv[7];string username=argv[8];string pwd=argv[9];string port=argv[10];
        string m_DatasourceConStr  = "PG:host='"+hostname+"' port='"+port+"' dbname='"+dbname+"' user='"+username+"' password='"+pwd+"'";
        
        //cout<<m_DatasourceConStr<<"    LayerName:"<<Shape_Filename<<endl;

    MPI_Barrier(MPI_COMM_WORLD);
	double time_beg = MPI_Wtime();              // 程序起始时间


	int Number;                                // 读入shape文件属性表的行数（即点的数目） 行数可以动态 但列数是固定的
	int (*xcp)[4+1], (*ca)[2+1];               // 读入的属性表数据 即将写入的二维数组 动态定义


	if (ProRank==0)                           //主进程负责读入shape文件数据                                         
	{

    OGRRegisterAll();

    OGRDataSource       *poDS;
    poDS = OGRSFDriverRegistrar::Open( m_DatasourceConStr.c_str(), FALSE );  // FALSE 表示此shape文件是只读
    if( poDS == NULL )
    {
        printf( "Open failed.\n%s" );
        exit( 1 );
    }

    OGRLayer  *poLayer;
    poLayer = poDS->GetLayerByName(Shape_Filename);  // 用poLayer指向poDS数据源的第一个数据层
		
	OGRFeature *poFeature;      // 用来指向poLayer层中的当前记录

    poLayer->ResetReading();     // 准备下一次读取poLayer层的第一条记录

	Number=0;                                        // 计数shape文件属性表的行数
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
		Number++;
		OGRFeature::DestroyFeature( poFeature );
    }   	                                   // shape文件属性表的行数已经存储在Number中


	xcp = (int(*)[4+1])(malloc((Number+1)*(4+1)*sizeof(int)));    // 得到行数后 才可以为xcp和ca分配存储空间
	ca  = (int(*)[2+1])(malloc((Number+1)*(2+1)*sizeof(int)));  


    poLayer->ResetReading();     // 准备下一次读取poLayer层的第一条记录

	int k=0;
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
		k++;

// shape文件属性表的5列分别为   zonecode  popu  x  y  case
// xcp 的四列分别为   zonecode  x  y  popu
// ca 的两列分别为    zonecode  case

		xcp[k][1] = poFeature->GetFieldAsInteger(0);
		xcp[k][2] = poFeature->GetFieldAsInteger(2);
		xcp[k][3] = poFeature->GetFieldAsInteger(3);
		xcp[k][4] = poFeature->GetFieldAsInteger(1);

		ca[k][1] = poFeature->GetFieldAsInteger(0);
		ca[k][2] = poFeature->GetFieldAsInteger(4);

		OGRFeature::DestroyFeature( poFeature );
    }

    OGRDataSource::DestroyDataSource( poDS );
	
	}
	


	int *X_Case, CaseSum=0;                                // 预先生成 每个乡镇点的病例数大小   以及 总的病例数

	if (ProRank==0)                                        // 只有主进程才需要处理真实病例的分布
	{
	  X_Case = (int *)malloc((Number+1)*sizeof(int));
	  for (int k=1; k<=Number; k++)                        
	   {
	     X_Case[k]=ca[k][2];                            // 此处假设 ca文件与xcp文件的行数相同 第一列的code也一致
	     CaseSum+=X_Case[k];
	    }
	}

	MPI_Bcast (&CaseSum, 1, MPI_INT, 0, MPI_COMM_WORLD);              // 每一个进程被广播了 总的病例数

	MPI_Bcast (&Number, 1, MPI_INT, 0, MPI_COMM_WORLD);                // 每一个进程被广播了Number值 行数
	
	if (ProRank!=0)                                                // 现在其他子进程也可以为xcp分配存储空间了                       
 	    xcp = (int(*)[4+1])(malloc((Number+1)*(4+1)*sizeof(int)));



	int *xcpTrans;                                              // 为了传输xcp而设置的一维数组
	xcpTrans = (int *)malloc((Number*4+1)*sizeof(int));



    if (ProRank==0)                                             // 主进程负责将xcp装进一维数组
	{                                                        
		int tempNo=1;
		for (int tempX=1; tempX<=Number; tempX++)
		    for (int tempY=1; tempY<=4; tempY++)
				xcpTrans[tempNo++]=xcp[tempX][tempY];
	}


	MPI_Bcast( xcpTrans+1, (Number*4), MPI_INT, 0, MPI_COMM_WORLD );     // 每一个进程被广播了xcp二维数组的值
	

    if (ProRank!=0)                                      // 其他子进程 负责将传输的一维数组 还原为二维数组xcp
	{                                                     
		int tempNo=1;
		for (int tempX=1; tempX<=Number; tempX++)
			for (int tempY=1; tempY<=4; tempY++)
				xcp[tempX][tempY] = xcpTrans[tempNo++];
	}


    free ((int *)xcpTrans);         



    MPI_Barrier(MPI_COMM_WORLD);
	double time_mid = MPI_Wtime();              // 程序传输初始数据结束时间



	int grid;                                                  // 网格单边 格网数目 单边的实际点数为grid+2
	float xInterval, yInterval;                                // 网格 x轴 y轴 单位网格长度
	int xZero, yZero;
	
	int xMax=xcp[1][2], xMin=xcp[1][2], yMax=xcp[1][3], yMin=xcp[1][3];

	for (int temp=2; temp<=Number; temp++)
	{
		xMax = max(xMax,xcp[temp][2]);		xMin = min(xMin,xcp[temp][2]);
		yMax = max(yMax,xcp[temp][3]);		yMin = min(yMin,xcp[temp][3]);  
	}

	grid = (int)(Number * GridRatio + 0.5);

	xInterval=(xMax-xMin+1)/(float)grid;  	yInterval=(yMax-yMin+1)/(float)grid;

	xZero=int(xMin-0.5*xInterval+0.5);   	yZero=int(yMin-0.5*yInterval+0.5);





	int RowBeg=0;                                    // 当前进程需要处理的开始行序号   一共grid+2行    ProSize个进程
	int RowN;                                        // 当前进程需要处理的行数目

	int tempA = (grid+2)-((grid+2)/ProSize)*ProSize;    // 前面tempA个进程         每个处理tempB+1行
	int tempB = (grid+2)/ProSize;                       // 后面ProSize-tempA个进程 每个处理tempB行
	if (ProRank<tempA) RowN=tempB+1; else RowN=tempB;                 // 得到了RowN的值  当前进程需处理的格网点行数

	for (int tem=0; tem<ProSize; tem++)
	{	if (ProRank==tem) break;
		if(tem<tempA) RowBeg+=tempB+1; else RowBeg+=tempB;         
	}                                                        // 得到了RowBeg的值  当前进程需处理的格网点 起始行序号




	struTem *XuhDis;
	XuhDis = (struTem *)malloc((Number+1)*sizeof(struTem));                     // 当前格网点 与每一个乡镇点的距离 结构体数组

	int (*CandClu)[MAX_Clu+1];                                                  // 全部格网点的候选聚集区域 汇总
	CandClu = (int(*)[MAX_Clu+1])malloc((RowN*(grid+2)*MAX_Clu+1)*(MAX_Clu+1)*sizeof(int));  


	int PopuSum=0;                                                 // 全部乡镇点 度量值 的总和
	for (int tem=1; tem<=Number; tem++)   PopuSum+=xcp[tem][4];      
	int PopuCritical = (int)(PopuSum * CluRatio);                         // 候选聚集区域 的 度量值 上限






	int r_number, r_popusum, r_casesum;
	float r_inci, r_gridr;
	int r_maxclus, r_mc;

	if (ProRank==0)
	{
		r_number = Number;
		r_popusum = PopuSum;
		r_casesum = CaseSum;
		r_inci = (int)(((float)CaseSum/PopuSum)*10000000+0.5)/(float)100;
		r_gridr = GridRatio;
		r_maxclus = (int)(CluRatio*100);
		r_mc = MonteCarlo;
	}







	int curMAX_Clu=1, curClu_No=0;              // 当前候选聚集区域包含的最多点数目   当前候选聚集区域的数目

	for (int temX=RowBeg; temX<RowBeg+RowN; temX++) 
		for (int temY=0; temY<grid+2; temY++)
		{
			int pointX=(int)(xZero+temX*xInterval+0.5);            // 当前进程需要处理的当前格网点坐标
			int pointY=(int)(yZero+temY*yInterval+0.5);

			for (int temK=1; temK<=Number; temK++)                 // 计算该格网点与每一个乡镇点的距离
			{
				XuhDis[temK].xuhao=temK;

				int tem_1 = pointX-xcp[temK][2];
				int tem_2 = pointY-xcp[temK][3];
				XuhDis[temK].dist=(float)sqrt(((double)tem_1*tem_1+(double)tem_2*tem_2));
			}
			
			stable_sort(XuhDis+1,XuhDis+(Number+1),compareM);

			int PopuTem;                            // 临时存储 候选聚集区域 的 累加 度量值
			int XuhTem[MAX_Clu+1], XuhMax;          // 当前格网点 最多可包含的乡镇点集合 以及数目

			XuhMax=1;                               // 与当前格网点最近的乡镇点 一定纳入
			XuhTem[1]=XuhDis[1].xuhao;
			PopuTem=xcp[XuhDis[1].xuhao][4];


			for (int temK=2; temK<=MAX_Clu; temK++)    // 从第2近的乡镇点开始 每个格网点 最多纳入MAX_Clu个
			{
				if (PopuTem+xcp[XuhDis[temK].xuhao][4]>=PopuCritical)   break;   // 纳入的乡镇点 度量值 超过了限度
				
				PopuTem += xcp[XuhDis[temK].xuhao][4];         // 若没超过限度 该乡镇点纳入
				XuhTem[temK] = XuhDis[temK].xuhao;
				XuhMax++;
			}
			
			curMAX_Clu = max (curMAX_Clu, XuhMax);           // 该格网点一共纳入了XuhMax个乡镇点

			for (int tem=1; tem<=XuhMax; tem++)         // 纳入的XuhMax个乡镇点 可以产生XuhMax个候选聚集区域
			{
				curClu_No++;

				for (int temT=1; temT<=MAX_Clu; temT++)
					{ if (temT<=tem)  CandClu[curClu_No][temT]=XuhTem[temT];
					  else            CandClu[curClu_No][temT]=MAX_V;  
				     }
			}
		
		}

    int ResLength;                                                 // 该进程的所有格网点 产生的所有候选聚集区域
    MultiSort (CandClu, curClu_No, curMAX_Clu, true, &ResLength);   // 多重排序并去除重复行  还剩ResLength行

	free ((struTem *)XuhDis);
	


	int TranCandHead[2];                             // 每一个进程 需要把候选聚集区域数组的行数和列数 发给主进程
	TranCandHead[0]=ResLength;
	TranCandHead[1]=curMAX_Clu;

	int *TranCandHead_All;                           // 主进程用于接收 每一个进程行列数 的数组
	if (ProRank==0) 	TranCandHead_All =(int *)malloc(ProSize*2*sizeof(int));

	MPI_Gather (TranCandHead, 2, MPI_INT, TranCandHead_All, 2, MPI_INT, 0, MPI_COMM_WORLD);



	int *TranCand;                                            // 每一个进程需要把候选聚集区域数组打包
	TranCand = (int *)malloc(ResLength*curMAX_Clu*sizeof(int));       
	int tem=0;
	for (int k=1; k<=ResLength; k++)
		for (int x=1; x<=curMAX_Clu; x++)	TranCand[tem++]=CandClu[k][x];    
	
	free ((int(*)[MAX_Clu+1])CandClu);


	int *recvCount, *displs;                                            // 主进程准备接受数据
	int *TranCand_All;
	
	int MAX_Clu_F=1, SUM_Clu_No=0, temCount=0;             // 最终全部候选聚集区域 的  宽度  区域个数    传输单元个数

	if (ProRank==0)                                                   
	{ 
	  recvCount =(int*)malloc(ProSize*sizeof(int));           // 准备从各个子进程 收集不同数量的数据
	  displs    =(int*)malloc(ProSize*sizeof(int));

	  for (int tmp=0; tmp<ProSize; tmp++)
	  {
		displs[tmp] = temCount;
		recvCount[tmp] = TranCandHead_All[2*tmp] * TranCandHead_All[2*tmp+1];

		temCount+=recvCount[tmp];

		MAX_Clu_F = max(MAX_Clu_F, TranCandHead_All[2*tmp+1]);   // 所有进程 的 最大聚集区域 包含乡镇点的上限
        SUM_Clu_No+=TranCandHead_All[2*tmp];                          // 所有进程 的 候选聚集区域 数目之和
	   }

	  TranCand_All = (int *)malloc(temCount*sizeof(int));       // 主进程用于接收各个进程候选聚集区域的大数组

	}


	MPI_Gatherv (TranCand, ResLength*curMAX_Clu, MPI_INT, TranCand_All, recvCount, displs, MPI_INT, 0, MPI_COMM_WORLD);


	if (ProRank!=0) free ((int *)TranCand);


	int (*Final_Cand)[MAX_Clu+1];            // 主进程从各个进程收集到的候选聚集区域 统一再变为数组 Final_Cand
	int FinaResLength;                       // 所有格网点 产生的所有候选聚集区域 去重复后 最终候选聚集区域个数
	
	if (ProRank==0)
	{ 
	  Final_Cand = (int(*)[MAX_Clu+1])malloc((SUM_Clu_No+1)*(MAX_Clu+1)*sizeof(int));

	  int tmpNo=0;          // TranCand_All数组的游标
	  int tmpR=0;           // Final_Cand二维数组的行标

	  for (int tP=0; tP<ProSize; tP++)
		  for (int tR=1; tR<=TranCandHead_All[2*tP]; tR++)
			  {
				 tmpR++;
				 for (int tC=1; tC<=MAX_Clu; tC++)
			     {
				   if (tC<=TranCandHead_All[2*tP+1])  Final_Cand[tmpR][tC]=TranCand_All[tmpNo++];
				   else                               Final_Cand[tmpR][tC]=MAX_V;
		 	       }
		       }

      free ((int *)TranCand_All);   free ((int *)TranCand);

	  MultiSort (Final_Cand, SUM_Clu_No, MAX_Clu_F, false, &FinaResLength);   // 多重排序并去除重复行  还剩FinaResLength行
	}
	
	
	MPI_Bcast(&FinaResLength, 1, MPI_INT, 0, MPI_COMM_WORLD);        // 最终的候选聚集区域个数
	MPI_Bcast(&MAX_Clu_F,     1, MPI_INT, 0, MPI_COMM_WORLD);        // 最终的最大候选聚集区域包含点数目



	int r_toclu, r_maxclusize;

	if (ProRank==0)
	{
		r_toclu = FinaResLength;
		r_maxclusize = MAX_Clu_F;
	}



	int *Final_Clu_Tran;
	Final_Clu_Tran = (int *)malloc(FinaResLength*MAX_Clu_F*sizeof(int));     // 准备最终候选聚集区域 的 传输数组

	if (ProRank==0)                                                         // 主进程负责打包
	{
		int tem=0;
		for (int x=1; x<=FinaResLength; x++)
			for (int y=1; y<=MAX_Clu_F; y++)
				Final_Clu_Tran[tem++]=Final_Cand[x][y];
	}


	MPI_Bcast(Final_Clu_Tran, FinaResLength*MAX_Clu_F, MPI_INT, 0, MPI_COMM_WORLD);


	if (ProRank!=0)                                                         // 其他子进程负责还原数组
	{
		Final_Cand = (int(*)[MAX_Clu+1])malloc((FinaResLength+1)*(MAX_Clu+1)*sizeof(int));

		int tem=0;
		for (int x=1; x<=FinaResLength; x++)
			for (int y=1; y<=MAX_Clu_F; y++)
				Final_Cand[x][y] = Final_Clu_Tran[tem++];
	}

	free ((int *)Final_Clu_Tran);




	int (*Clu_Popu)[2];                                    // 预先生成 每个候选聚集区域 内外的 度量值大小
	Clu_Popu = (int (*)[2])malloc((FinaResLength+1)*2*sizeof(int));

	for (int k=1; k<=FinaResLength; k++)                        
	{
		Clu_Popu[k][0]=0;

		for (int x=1; x<=MAX_Clu_F; x++)
			{
				if (Final_Cand[k][x]==MAX_V) break;
			    else  Clu_Popu[k][0]+=xcp[Final_Cand[k][x]][4];
		     }
		Clu_Popu[k][1]=PopuSum-Clu_Popu[k][0];
	}




	double maxLLR;                               // 真实的 最大的log似然比值
	int maxNo;                                   // 真实的 最大可能聚集区域的序号


	int *r_id;
	r_id = (int *)malloc((MAX_Clu_F+1)*sizeof(int));
	int r_clupopu, r_clucase;
	float r_expecase, r_cluinci, r_cluratio;


	if (ProRank==0)                   // 只有主进程 才针对真实的病例分布 计算出 最大可能的聚集区域
	{
		MaxLlrFun (Final_Cand, Clu_Popu, X_Case, FinaResLength, MAX_Clu_F, CaseSum,  &maxLLR, &maxNo);

		for (int k=1; k<=MAX_Clu_F; k++)  
			if (Final_Cand[maxNo][k]!=MAX_V) r_id[k] = xcp[Final_Cand[maxNo][k]][1];
			else r_id[k] = 0;

		r_clupopu = Clu_Popu[maxNo][0];

		r_clucase = 0;
		for (int k=1; k<=MAX_Clu_F; k++)  
			if (Final_Cand[maxNo][k]!=MAX_V) r_clucase+=X_Case[Final_Cand[maxNo][k]];
			else break;

		r_expecase = (int)((Clu_Popu[maxNo][0]/(float)PopuSum)*CaseSum*100+0.5)/(float)100;
		r_cluinci = (int)(((float)r_clucase/Clu_Popu[maxNo][0])*10000000+0.5)/(float)100;
		r_cluratio = (int)(r_clucase/r_expecase*100+0.5)/(float)100;

		}

	MPI_Bcast (&maxLLR, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);



	int temMC = MonteCarlo - (MonteCarlo/ProSize) * ProSize;
	if (ProRank<temMC) temMC = MonteCarlo/ProSize + 1;               // 得到当前进程 需要计算的蒙特卡洛模拟次数
	else temMC = MonteCarlo/ProSize;

	srand((unsigned)(time(NULL)*(ProRank+1)));               // 不同的进程 使用不同的随机数种子


	int *PopuTemp;                                          // 各个乡镇度量值 的 阶梯数组 用于随机分配病例
	PopuTemp = (int *)malloc((Number+1)*sizeof(int));
	PopuTemp[1] = xcp[1][4];
	for (int k=2; k<=Number; k++) PopuTemp[k]=PopuTemp[k-1]+xcp[k][4];

	int *R_Case;
	R_Case = (int *)malloc((Number+1)*sizeof(int));         // 存储病例数的某次随机分配


	int Bigger=0;                                         // 比真实病例数产生的maxLLR更大的随机模拟次数
	int Rc;

	// free ((int(*)[4+1])xcp);
	// free ((int(*)[2+1])ca);
	

	int k, t, Random;

	for ( k=0; k<temMC; k++)                                  // 进行temMC次的蒙特卡洛模拟循环
	{
		
		for ( t=1; t<=Number; t++) R_Case[t]=0;               // 每一次新的随机分配前 清零

		for ( t=1; t<=CaseSum; t++)                           // 一共需要随机分配CaseSum个病例
		{
		   Random = (int)(((double)rand()/RAND_MAX)*PopuSum + 0.5);

		   for (Rc=1; Rc<=Number; Rc++)  if (Random<PopuTemp[Rc]) break;
		   R_Case[Rc]++;
		}
		
		double LLRmc;
		MaxLlrFun (Final_Cand, Clu_Popu, R_Case, FinaResLength, MAX_Clu_F, CaseSum,  &LLRmc, &Rc);
		 

		if (maxLLR<=LLRmc) Bigger++;
	}


	free ((int(*)[2])Clu_Popu);
	free ((int(*)[MAX_Clu+1])Final_Cand);



	int *Bigger_all;                                                   // 各个进程得到的Bigger值需要收集到一起
	if (ProRank==0) Bigger_all = (int *)malloc(ProSize*sizeof(int));



	MPI_Gather (&Bigger, 1, MPI_INT, Bigger_all, 1, MPI_INT, 0, MPI_COMM_WORLD);



	int Bigger_all_sum=0;
	
	int r_bigger;
	float r_pv;

	if (ProRank==0) 
	{
		for (int k=0; k<ProSize; k++) Bigger_all_sum+=Bigger_all[k];

		r_bigger = Bigger_all_sum+1;
		r_pv = (float)(r_bigger)/(r_mc+1);
	
	}



	double time_end = MPI_Wtime();              // 程序结束时间



	if (ProRank==0)
	{
	cout << "Number of locations .........: " << r_number << '\n';
	cout << "Density index for Grid ......: " << r_gridr << '\n';
	cout << "Maximum Cluster Size ........: " << r_maxclus << " percent of population" << '\n';
	cout << "Monte Carlo times ...........: " << r_mc << '\n';

	cout << "[DEBUG][TIMESPAN][IO]" << time_mid-time_beg << '\n';
	cout << "[DEBUG][TIMESPAN][COMPUTING]" << time_end-time_mid << '\n';
	cout << "[DEBUG][TIMESPAN][TOTAL]" << time_end-time_beg << '\n'<< '\n';
	}


	if (ProRank==0)
	{
	     ofstream resultFile( Result_Filename, ios::out );

	resultFile << "    ----- Result of spatial scan statistic method -----" << '\n' << '\n';
	
	resultFile << "Number of locations .........: " << r_number << '\n';
	resultFile << "Total population ............: " << r_popusum << '\n';
	resultFile << "Total number of cases .......: " << r_casesum << '\n';
	resultFile << "Incidence /100000 ...........: " << r_inci << '\n' << '\n';

	resultFile << "Density index for Grid ......: " << r_gridr << '\n';
	resultFile << "Maximum Cluster Size ........: " << r_maxclus << " percent of population" << '\n';
	resultFile << "Monte Carlo times ...........: " << r_mc << '\n'<< '\n';


	resultFile << "Total number of candidata clusters ....: " << r_toclu << '\n';
	resultFile << "Maximum Cluster include ...............: " << r_maxclusize << " locations " << '\n' << '\n';


		resultFile << "  --- Detected most likely cluster ---" << '\n';
		
		resultFile << "Location IDs included .....: " << '\n';
		for (int k=1; k<=MAX_Clu_F; k++)  
			if (r_id[k]!=0) resultFile << r_id[k] << "-";
			else break;
		resultFile << '\n';
		
		resultFile << "Population ................: "  << r_clupopu << '\n';

		resultFile << "Number of cases ...........: "  << r_clucase << '\n';

		resultFile << "Expected cases ............: " << r_expecase << '\n';
		
		resultFile << "Incidence /100000 .........: "  << r_cluinci <<'\n';
		resultFile << "Observed / expected .......: "  << r_cluratio <<'\n'<< '\n';
		resultFile << "Relative Risk .............: "  <<((r_clucase*1.0)/r_clupopu)/((r_casesum-r_clucase*1.0)/(r_popusum-r_clupopu))<<'\n'<<'\n';

		resultFile << "Monte Carlo rank ..........: " << r_bigger << "/" << r_mc+1 << '\n';
	    resultFile << "P-value ...................: " << r_pv << '\n'<<'\n';
/*
		resultFile << "Run Time ..................: " << (int)((time_mid - time_beg)*100+0.5);
		resultFile << " + " << (int)((time_end - time_mid)*100+0.5);
		resultFile << " = " << (int)((time_end - time_beg)*100+0.5);
*/

		resultFile.close();
	
	}

	

	MPI_Finalize();

	return 0;
}

