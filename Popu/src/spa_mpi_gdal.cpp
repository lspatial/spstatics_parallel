
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


#define MAX_Xiang    1024                // �����������Ŀ������
#define MAX_V        9999                // ������ŵ����� ��ʾһ�������ܴ��ڵ����
#define MAX_Clu      300                 // ������ѡ�ۼ������� ���ɰ�����������Ŀ
#define MAX_Double   99999999            // Ϊ�˱�����MAX_LLR�����ֵ Ԥ���趨һ����С�� -1 * MAX_Double


                                                     // һ�������� ��ĳ�����ľ���ṹ�� Ϊ�˱�������
struct struTem
{
	int xuhao;                     // ���������
	float dist;                    // ������������ľ���
} ;
                                                     // �Ƚ�����ṹ�� ������������
bool compareM (struTem a, struTem b)
{
      return  a.dist < b.dist ;
}
                                                     // �洢ĳ����ѡ�ۼ���������������������� Ϊ�˱�������ȥ��
struct StrMultSort
{
	int Data[MAX_Clu+1];        
};
                                                     // �Ƚ�����ṹ���������ĳ����Ԫ ������������
bool compareMS (StrMultSort a, StrMultSort b)
{
      int  tem=a.Data[0];
      return  a.Data[tem] < b.Data[tem] ;
}
                                                     // Cand�б�������������к�ѡ�ۼ����� ���ö������� ȥ���ظ�
void MultiSort (int(*Cand)[MAX_Clu+1], int length, int width, bool FirSort, int *ResLength)
{                                                        
	                                                     // Cand��ά�������Ч����length ��Ч���width
	                                                     // �Ƿ��һ������ȥ��FirSort ȥ��֮�����Ч����ResLength

	StrMultSort *TemStrL;
	TemStrL = (StrMultSort *) malloc ((length+1)*sizeof(StrMultSort));
	for (int k=1; k<=length; k++)                                         // ����Ҫ�����Cand���� �������ṹ������
		for (int t=1; t<=width; t++)  TemStrL[k].Data[t] = Cand[k][t]; 



	if (FirSort)                                                        // �״����� ÿһ�е�������Ҫ����
	{ 
		for (int k=1; k<=length; k++)
  		 stable_sort (TemStrL[k].Data+1, TemStrL[k].Data+(width+1));
	 }



	for (int x=width; x>=1; x--)                                        // �����һ�п�ʼ ��������
	{
		for (int y=1; y<=length; y++) TemStrL[y].Data[0]=x;
		stable_sort (TemStrL+1, TemStrL+(length+1), compareMS);
	}



	for (int k=2; k<=length; k++)                             // ���ظ��Ľṹ�嵥Ԫ ���ϼ�¼��־
	{
		bool temBool=true;
		
		for (int x=1; x<=width; x++) 
			if (TemStrL[k].Data[x]!=TemStrL[k-1].Data[x]) { temBool=false; break; }
		
		if (temBool) TemStrL[k].Data[0]=0;        // ��ʾ��k����¼��ǰһ����¼��ͬ ���� ��ɾ��
	}



	int temNo=1;                                                 // ��¼���ظ����� ����Ŀ
	
	for (int x=1; x<=width; x++) Cand[1][x]=TemStrL[1].Data[x];  // �ṹ������ĵ�һ����Ԫ д�� Cand�����һ��

	for (int k=2; k<=length; k++)
	{
		if (TemStrL[k].Data[0]==0) continue;                      // �ظ��Ľṹ�嵥Ԫ ����д��Cand����
		temNo++;
        for (int x=1; x<=width; x++) Cand[temNo][x]=TemStrL[k].Data[x];
	}

	*ResLength = temNo;

	free ((StrMultSort *)TemStrL);
}

void MaxLlrFun (int (*Cand)[MAX_Clu+1], int (*Clu_Popu)[2], int *X_Case, int FinaResLength, int MAX_Clu_F, 
	            int CaseSum,         double *maxLLR, int *maxNo)
{
	                                               // Cand ���յĺ�ѡ�ۼ����򼯺� ��Ч����FinaResLength ��Ч���MAX_Clu_F
	                                          // Clu_Popu ÿ���ۼ���������Ĳ����������Ѿ�Ԥ�ȼ��㣩 X_Case ÿ����Ĳ�����
	                                               // CaseSum ��������            maxLLR maxNo  �ҵ��������ܾۼ��������

	*maxLLR=(-1)*MAX_Double;           // Ԥ�ȸ�һ����Сֵ
	double tmpLLR;
	int clu_case;

	for (int k=1; k<=FinaResLength; k++)                        // ���ο���ÿһ����ѡ�ۼ����� 
	{
		clu_case=0;

		for (int x=1; x<=MAX_Clu_F; x++)                       // �õ���ǰ��ѡ�ۼ�����Ĳ�����
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

	int ProSize, ProRank;                          // �ܵĽ�����Ŀ   ��ǰ���̵����(��Ŵ�0��ʼ)
	MPI_Comm_size(MPI_COMM_WORLD, &ProSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProRank);


    // char  Shape_Filename[] = "popu_case.shp";
	// char  Result_Filename[] = "result.txt";

	char Shape_Filename[50]={0};	
	strcpy(Shape_Filename,argv[1]);

	char Result_Filename[50]={0};	
	strcpy(Result_Filename,argv[2]);


	float GridRatio,  CluRatio;     // �����ܶȵ�����  ������ѡ�ۼ��������ֵ�������ܱ���
	int   MonteCarlo;              //  ���ؿ���ģ�����
		
	GridRatio = 0.2;  CluRatio = 0.1;
	MonteCarlo = 200;

	GridRatio  =  atof(argv[3]);
	CluRatio   =  atof(argv[4]);
	MonteCarlo =  atoi(argv[5]);
	
	string dbname=argv[6];string hostname=argv[7];string username=argv[8];string pwd=argv[9];string port=argv[10];
        string m_DatasourceConStr  = "PG:host='"+hostname+"' port='"+port+"' dbname='"+dbname+"' user='"+username+"' password='"+pwd+"'";
        
        //cout<<m_DatasourceConStr<<"    LayerName:"<<Shape_Filename<<endl;

    MPI_Barrier(MPI_COMM_WORLD);
	double time_beg = MPI_Wtime();              // ������ʼʱ��


	int Number;                                // ����shape�ļ����Ա���������������Ŀ�� �������Զ�̬ �������ǹ̶���
	int (*xcp)[4+1], (*ca)[2+1];               // ��������Ա����� ����д��Ķ�ά���� ��̬����


	if (ProRank==0)                           //�����̸������shape�ļ�����                                         
	{

    OGRRegisterAll();

    OGRDataSource       *poDS;
    poDS = OGRSFDriverRegistrar::Open( m_DatasourceConStr.c_str(), FALSE );  // FALSE ��ʾ��shape�ļ���ֻ��
    if( poDS == NULL )
    {
        printf( "Open failed.\n%s" );
        exit( 1 );
    }

    OGRLayer  *poLayer;
    poLayer = poDS->GetLayerByName(Shape_Filename);  // ��poLayerָ��poDS����Դ�ĵ�һ�����ݲ�
		
	OGRFeature *poFeature;      // ����ָ��poLayer���еĵ�ǰ��¼

    poLayer->ResetReading();     // ׼����һ�ζ�ȡpoLayer��ĵ�һ����¼

	Number=0;                                        // ����shape�ļ����Ա������
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
		Number++;
		OGRFeature::DestroyFeature( poFeature );
    }   	                                   // shape�ļ����Ա�������Ѿ��洢��Number��


	xcp = (int(*)[4+1])(malloc((Number+1)*(4+1)*sizeof(int)));    // �õ������� �ſ���Ϊxcp��ca����洢�ռ�
	ca  = (int(*)[2+1])(malloc((Number+1)*(2+1)*sizeof(int)));  


    poLayer->ResetReading();     // ׼����һ�ζ�ȡpoLayer��ĵ�һ����¼

	int k=0;
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
		k++;

// shape�ļ����Ա��5�зֱ�Ϊ   zonecode  popu  x  y  case
// xcp �����зֱ�Ϊ   zonecode  x  y  popu
// ca �����зֱ�Ϊ    zonecode  case

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
	


	int *X_Case, CaseSum=0;                                // Ԥ������ ÿ�������Ĳ�������С   �Լ� �ܵĲ�����

	if (ProRank==0)                                        // ֻ�������̲���Ҫ������ʵ�����ķֲ�
	{
	  X_Case = (int *)malloc((Number+1)*sizeof(int));
	  for (int k=1; k<=Number; k++)                        
	   {
	     X_Case[k]=ca[k][2];                            // �˴����� ca�ļ���xcp�ļ���������ͬ ��һ�е�codeҲһ��
	     CaseSum+=X_Case[k];
	    }
	}

	MPI_Bcast (&CaseSum, 1, MPI_INT, 0, MPI_COMM_WORLD);              // ÿһ�����̱��㲥�� �ܵĲ�����

	MPI_Bcast (&Number, 1, MPI_INT, 0, MPI_COMM_WORLD);                // ÿһ�����̱��㲥��Numberֵ ����
	
	if (ProRank!=0)                                                // ���������ӽ���Ҳ����Ϊxcp����洢�ռ���                       
 	    xcp = (int(*)[4+1])(malloc((Number+1)*(4+1)*sizeof(int)));



	int *xcpTrans;                                              // Ϊ�˴���xcp�����õ�һά����
	xcpTrans = (int *)malloc((Number*4+1)*sizeof(int));



    if (ProRank==0)                                             // �����̸���xcpװ��һά����
	{                                                        
		int tempNo=1;
		for (int tempX=1; tempX<=Number; tempX++)
		    for (int tempY=1; tempY<=4; tempY++)
				xcpTrans[tempNo++]=xcp[tempX][tempY];
	}


	MPI_Bcast( xcpTrans+1, (Number*4), MPI_INT, 0, MPI_COMM_WORLD );     // ÿһ�����̱��㲥��xcp��ά�����ֵ
	

    if (ProRank!=0)                                      // �����ӽ��� ���𽫴����һά���� ��ԭΪ��ά����xcp
	{                                                     
		int tempNo=1;
		for (int tempX=1; tempX<=Number; tempX++)
			for (int tempY=1; tempY<=4; tempY++)
				xcp[tempX][tempY] = xcpTrans[tempNo++];
	}


    free ((int *)xcpTrans);         



    MPI_Barrier(MPI_COMM_WORLD);
	double time_mid = MPI_Wtime();              // �������ʼ���ݽ���ʱ��



	int grid;                                                  // ���񵥱� ������Ŀ ���ߵ�ʵ�ʵ���Ϊgrid+2
	float xInterval, yInterval;                                // ���� x�� y�� ��λ���񳤶�
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





	int RowBeg=0;                                    // ��ǰ������Ҫ����Ŀ�ʼ�����   һ��grid+2��    ProSize������
	int RowN;                                        // ��ǰ������Ҫ���������Ŀ

	int tempA = (grid+2)-((grid+2)/ProSize)*ProSize;    // ǰ��tempA������         ÿ������tempB+1��
	int tempB = (grid+2)/ProSize;                       // ����ProSize-tempA������ ÿ������tempB��
	if (ProRank<tempA) RowN=tempB+1; else RowN=tempB;                 // �õ���RowN��ֵ  ��ǰ�����账��ĸ���������

	for (int tem=0; tem<ProSize; tem++)
	{	if (ProRank==tem) break;
		if(tem<tempA) RowBeg+=tempB+1; else RowBeg+=tempB;         
	}                                                        // �õ���RowBeg��ֵ  ��ǰ�����账��ĸ����� ��ʼ�����




	struTem *XuhDis;
	XuhDis = (struTem *)malloc((Number+1)*sizeof(struTem));                     // ��ǰ������ ��ÿһ�������ľ��� �ṹ������

	int (*CandClu)[MAX_Clu+1];                                                  // ȫ��������ĺ�ѡ�ۼ����� ����
	CandClu = (int(*)[MAX_Clu+1])malloc((RowN*(grid+2)*MAX_Clu+1)*(MAX_Clu+1)*sizeof(int));  


	int PopuSum=0;                                                 // ȫ������� ����ֵ ���ܺ�
	for (int tem=1; tem<=Number; tem++)   PopuSum+=xcp[tem][4];      
	int PopuCritical = (int)(PopuSum * CluRatio);                         // ��ѡ�ۼ����� �� ����ֵ ����






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







	int curMAX_Clu=1, curClu_No=0;              // ��ǰ��ѡ�ۼ����������������Ŀ   ��ǰ��ѡ�ۼ��������Ŀ

	for (int temX=RowBeg; temX<RowBeg+RowN; temX++) 
		for (int temY=0; temY<grid+2; temY++)
		{
			int pointX=(int)(xZero+temX*xInterval+0.5);            // ��ǰ������Ҫ����ĵ�ǰ����������
			int pointY=(int)(yZero+temY*yInterval+0.5);

			for (int temK=1; temK<=Number; temK++)                 // ����ø�������ÿһ�������ľ���
			{
				XuhDis[temK].xuhao=temK;

				int tem_1 = pointX-xcp[temK][2];
				int tem_2 = pointY-xcp[temK][3];
				XuhDis[temK].dist=(float)sqrt(((double)tem_1*tem_1+(double)tem_2*tem_2));
			}
			
			stable_sort(XuhDis+1,XuhDis+(Number+1),compareM);

			int PopuTem;                            // ��ʱ�洢 ��ѡ�ۼ����� �� �ۼ� ����ֵ
			int XuhTem[MAX_Clu+1], XuhMax;          // ��ǰ������ ���ɰ���������㼯�� �Լ���Ŀ

			XuhMax=1;                               // �뵱ǰ���������������� һ������
			XuhTem[1]=XuhDis[1].xuhao;
			PopuTem=xcp[XuhDis[1].xuhao][4];


			for (int temK=2; temK<=MAX_Clu; temK++)    // �ӵ�2��������㿪ʼ ÿ�������� �������MAX_Clu��
			{
				if (PopuTem+xcp[XuhDis[temK].xuhao][4]>=PopuCritical)   break;   // ���������� ����ֵ �������޶�
				
				PopuTem += xcp[XuhDis[temK].xuhao][4];         // ��û�����޶� �����������
				XuhTem[temK] = XuhDis[temK].xuhao;
				XuhMax++;
			}
			
			curMAX_Clu = max (curMAX_Clu, XuhMax);           // �ø�����һ��������XuhMax�������

			for (int tem=1; tem<=XuhMax; tem++)         // �����XuhMax������� ���Բ���XuhMax����ѡ�ۼ�����
			{
				curClu_No++;

				for (int temT=1; temT<=MAX_Clu; temT++)
					{ if (temT<=tem)  CandClu[curClu_No][temT]=XuhTem[temT];
					  else            CandClu[curClu_No][temT]=MAX_V;  
				     }
			}
		
		}

    int ResLength;                                                 // �ý��̵����и����� ���������к�ѡ�ۼ�����
    MultiSort (CandClu, curClu_No, curMAX_Clu, true, &ResLength);   // ��������ȥ���ظ���  ��ʣResLength��

	free ((struTem *)XuhDis);
	


	int TranCandHead[2];                             // ÿһ������ ��Ҫ�Ѻ�ѡ�ۼ�������������������� ����������
	TranCandHead[0]=ResLength;
	TranCandHead[1]=curMAX_Clu;

	int *TranCandHead_All;                           // ���������ڽ��� ÿһ������������ ������
	if (ProRank==0) 	TranCandHead_All =(int *)malloc(ProSize*2*sizeof(int));

	MPI_Gather (TranCandHead, 2, MPI_INT, TranCandHead_All, 2, MPI_INT, 0, MPI_COMM_WORLD);



	int *TranCand;                                            // ÿһ��������Ҫ�Ѻ�ѡ�ۼ�����������
	TranCand = (int *)malloc(ResLength*curMAX_Clu*sizeof(int));       
	int tem=0;
	for (int k=1; k<=ResLength; k++)
		for (int x=1; x<=curMAX_Clu; x++)	TranCand[tem++]=CandClu[k][x];    
	
	free ((int(*)[MAX_Clu+1])CandClu);


	int *recvCount, *displs;                                            // ������׼����������
	int *TranCand_All;
	
	int MAX_Clu_F=1, SUM_Clu_No=0, temCount=0;             // ����ȫ����ѡ�ۼ����� ��  ���  �������    ���䵥Ԫ����

	if (ProRank==0)                                                   
	{ 
	  recvCount =(int*)malloc(ProSize*sizeof(int));           // ׼���Ӹ����ӽ��� �ռ���ͬ����������
	  displs    =(int*)malloc(ProSize*sizeof(int));

	  for (int tmp=0; tmp<ProSize; tmp++)
	  {
		displs[tmp] = temCount;
		recvCount[tmp] = TranCandHead_All[2*tmp] * TranCandHead_All[2*tmp+1];

		temCount+=recvCount[tmp];

		MAX_Clu_F = max(MAX_Clu_F, TranCandHead_All[2*tmp+1]);   // ���н��� �� ���ۼ����� ��������������
        SUM_Clu_No+=TranCandHead_All[2*tmp];                          // ���н��� �� ��ѡ�ۼ����� ��Ŀ֮��
	   }

	  TranCand_All = (int *)malloc(temCount*sizeof(int));       // ���������ڽ��ո������̺�ѡ�ۼ�����Ĵ�����

	}


	MPI_Gatherv (TranCand, ResLength*curMAX_Clu, MPI_INT, TranCand_All, recvCount, displs, MPI_INT, 0, MPI_COMM_WORLD);


	if (ProRank!=0) free ((int *)TranCand);


	int (*Final_Cand)[MAX_Clu+1];            // �����̴Ӹ��������ռ����ĺ�ѡ�ۼ����� ͳһ�ٱ�Ϊ���� Final_Cand
	int FinaResLength;                       // ���и����� ���������к�ѡ�ۼ����� ȥ�ظ��� ���պ�ѡ�ۼ��������
	
	if (ProRank==0)
	{ 
	  Final_Cand = (int(*)[MAX_Clu+1])malloc((SUM_Clu_No+1)*(MAX_Clu+1)*sizeof(int));

	  int tmpNo=0;          // TranCand_All������α�
	  int tmpR=0;           // Final_Cand��ά������б�

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

	  MultiSort (Final_Cand, SUM_Clu_No, MAX_Clu_F, false, &FinaResLength);   // ��������ȥ���ظ���  ��ʣFinaResLength��
	}
	
	
	MPI_Bcast(&FinaResLength, 1, MPI_INT, 0, MPI_COMM_WORLD);        // ���յĺ�ѡ�ۼ��������
	MPI_Bcast(&MAX_Clu_F,     1, MPI_INT, 0, MPI_COMM_WORLD);        // ���յ�����ѡ�ۼ������������Ŀ



	int r_toclu, r_maxclusize;

	if (ProRank==0)
	{
		r_toclu = FinaResLength;
		r_maxclusize = MAX_Clu_F;
	}



	int *Final_Clu_Tran;
	Final_Clu_Tran = (int *)malloc(FinaResLength*MAX_Clu_F*sizeof(int));     // ׼�����պ�ѡ�ۼ����� �� ��������

	if (ProRank==0)                                                         // �����̸�����
	{
		int tem=0;
		for (int x=1; x<=FinaResLength; x++)
			for (int y=1; y<=MAX_Clu_F; y++)
				Final_Clu_Tran[tem++]=Final_Cand[x][y];
	}


	MPI_Bcast(Final_Clu_Tran, FinaResLength*MAX_Clu_F, MPI_INT, 0, MPI_COMM_WORLD);


	if (ProRank!=0)                                                         // �����ӽ��̸���ԭ����
	{
		Final_Cand = (int(*)[MAX_Clu+1])malloc((FinaResLength+1)*(MAX_Clu+1)*sizeof(int));

		int tem=0;
		for (int x=1; x<=FinaResLength; x++)
			for (int y=1; y<=MAX_Clu_F; y++)
				Final_Cand[x][y] = Final_Clu_Tran[tem++];
	}

	free ((int *)Final_Clu_Tran);




	int (*Clu_Popu)[2];                                    // Ԥ������ ÿ����ѡ�ۼ����� ����� ����ֵ��С
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




	double maxLLR;                               // ��ʵ�� ����log��Ȼ��ֵ
	int maxNo;                                   // ��ʵ�� �����ܾۼ���������


	int *r_id;
	r_id = (int *)malloc((MAX_Clu_F+1)*sizeof(int));
	int r_clupopu, r_clucase;
	float r_expecase, r_cluinci, r_cluratio;


	if (ProRank==0)                   // ֻ�������� �������ʵ�Ĳ����ֲ� ����� �����ܵľۼ�����
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
	if (ProRank<temMC) temMC = MonteCarlo/ProSize + 1;               // �õ���ǰ���� ��Ҫ��������ؿ���ģ�����
	else temMC = MonteCarlo/ProSize;

	srand((unsigned)(time(NULL)*(ProRank+1)));               // ��ͬ�Ľ��� ʹ�ò�ͬ�����������


	int *PopuTemp;                                          // �����������ֵ �� �������� ����������䲡��
	PopuTemp = (int *)malloc((Number+1)*sizeof(int));
	PopuTemp[1] = xcp[1][4];
	for (int k=2; k<=Number; k++) PopuTemp[k]=PopuTemp[k-1]+xcp[k][4];

	int *R_Case;
	R_Case = (int *)malloc((Number+1)*sizeof(int));         // �洢��������ĳ���������


	int Bigger=0;                                         // ����ʵ������������maxLLR��������ģ�����
	int Rc;

	// free ((int(*)[4+1])xcp);
	// free ((int(*)[2+1])ca);
	

	int k, t, Random;

	for ( k=0; k<temMC; k++)                                  // ����temMC�ε����ؿ���ģ��ѭ��
	{
		
		for ( t=1; t<=Number; t++) R_Case[t]=0;               // ÿһ���µ��������ǰ ����

		for ( t=1; t<=CaseSum; t++)                           // һ����Ҫ�������CaseSum������
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



	int *Bigger_all;                                                   // �������̵õ���Biggerֵ��Ҫ�ռ���һ��
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



	double time_end = MPI_Wtime();              // �������ʱ��



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

