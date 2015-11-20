#include <cstdlib>
#include "mpi.h"
#include "ParallelInversion.h"
#define a(x,y) a[x*M+y]
#define A(x,y) matrix[x*M+y] 
#define intsize sizeof(int)

void doublecopy(double* dest, double* src, int size)
{
    for(int i=0;i<size;i++)
        dest[i] = src[i];
}

void printMatrix(double* matrix, int sizex, int sizey)
{
    printf("\nMatrix: %d x %d\n", sizey,sizex);
    for(int i=0;i<sizey;i++) 
    {
        for(int j=0;j<sizex;j++) 
            printf("v:%.4f\t",matrix[i*sizex+j]);
        printf("\n"); 
    }
    printf("ok");   
}

void printMatrix(double* matrix, int size)
{
    printMatrix(matrix, size, size);
}

void invert(double* matrix, int order, int group_size, int my_rank)
{
    MPI_Status status;
    int i,j,k;
    int v,w;
    int M = order;
    int m,localm;
    double *f; double *a; 
    
    int p=group_size;
    
    /*0 号处理器将 M 广播给所有处理器*/ 
    MPI_Bcast(&M,1,MPI_INT,0,MPI_COMM_WORLD); 
    m=M/p;
    if (M%p!=0) m++;
    localm = (m-1)*p + my_rank<M?m:m-1;
    
    /*各处理器为主行元素建立发送和接收缓冲区*/ 
    f=(double*)malloc(sizeof(double)*M); 
    /*分配至各处理器的子矩阵大小为 m*M*/ 
    a=(double*)malloc(sizeof(double)*m*M);
    
    /*0 号处理器采用行交叉划分将矩阵 A 划分为 m*M 的 p 块子矩阵,依次发送给 1 至 p-1 号处理器*/
    if (my_rank==0) {
        for(i=0;i<M;i++)
        {
            if ((i%p)!=0) {                
                MPI_Send(&A(i,0),M,MPI_FLOAT,i%p,i/p+1, MPI_COMM_WORLD);
            }else{      
                doublecopy(&a(i/p,0), &A(i,0), M);                
            }
        }
    }else{
        for(i=my_rank;i<M;i+=p)
            MPI_Recv(&a(i/p,0),M,MPI_FLOAT,0,i/p+1,MPI_COMM_WORLD,&status); 
    }
    
    /*发送主行数据的处理器利用主行 对其主行之外的 m-1 行行向量做行变换*/
    for(i=0;i<m;i++) {
        for(j=0;j<p&&i*p+j<M;j++) {
            /*j 号处理器负责广播主行元素*/ 
            v=i*p+j;     
            if (my_rank==j){
                a(i,v)=1/a(i,v); 
                for(k=0;k<M;k++){                
                    if (k!=v) 
                        a(i,k)=a(i,k)*a(i,v);
                    f[k]=a(i,k);
                }
                MPI_Bcast(f,M,MPI_FLOAT, my_rank,MPI_COMM_WORLD);
            }else{                
                MPI_Bcast(f,M,MPI_FLOAT,j , MPI_COMM_WORLD);
            }
        
        
            /*其余处理器则利用主行对其 m 行 行向量做行变换*/
            if (my_rank!=j) {
                for(k=0;k<localm;k++) 
                    for(w=0;w<M;w++)
                        if (w!=v) 
                            a(k,w)=a(k,w)-f[w]*a(k,v); 
                for(k=0;k<localm;k++)
                    a(k,v)=-f[v]* a(k,v);
            }else{            
                for(k=0;k<localm;k++) 
                    if (k!=i)
                        for(w=0;w<M;w++) 
                            if (w!=v)
                                a(k,w)=a(k,w)- f[w]*a(k,v);

                for(k=0;k<localm;k++)
                    if (k!=i) 
                        a(k,v)=-f[v]*a(k,v);
            }
            
        }
    }
    
    /*0 号处理器从其余各处理器中接收子矩阵 a, 得到经过变换的逆矩阵 A*/
    if(my_rank==0){
        for(i=0;i<m;i++) 
            doublecopy(&A(i*p,0), &a(i,0), M);            
    }
    
    if(my_rank!=0){
        MPI_Send(a, localm*M, MPI_FLOAT, 0, my_rank, MPI_COMM_WORLD);        
    }else{        
        for(i=1;i<p;i++)
        {
            int rows = (m-1)*p + i<M?m:m-1;
            MPI_Recv(a, rows*M, MPI_FLOAT, i, i, MPI_COMM_WORLD, &status);
            for(j=0;j<rows;j++)
            {
                doublecopy(&A(j*p+i,0), &a(j,0), M);
            }
        }            
    }
  
    free(a);
    free(f);
}