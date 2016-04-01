#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>
#include "mpi.h"

#define Tolerance 0.00001
#define TRUE 1
#define FALSE 0

#define N 5000

double ** A;
double ** C;

int initialize (double **A, int n)
{
   int i,j;
   
   //#pragma omp parallel shared (A) private (i,j) 
   {
     //#pragma omp parallel for 
     for (j=0;j<n+1;j++){
       A[0][j]=1.0;
     }

    // #pragma omp parallel for private(j)
     for (i=1;i<n+1;i++){
       A[i][0]=1.0;
       #pragma omp parallel for
       for (j=1;j<n+1;j++) A[i][j]=0.0;
     }
   }
}

double* solve(double **A,double **C, int n, int s, int e)
{
   //int convergence=FALSE;
   double diff, tmp;
   int i,j, iters=0;
   int for_iters;
   int st = s;
   int et = e;
   double* diff_arr = malloc(sizeof(double) * 20);

   for (for_iters=1;for_iters<21;for_iters++) 
   { 
     //#pragma omp parallel shared (A, C) private (i,j) 
     {
       // save old values of A
       //#pragma omp for
       for ( i = st; i < et; i++ ) 
       {
         for ( j = 0; j < (n+1); j++ )
         {
            C[i][j] = A[i][j];
        } 
       }
      //#pragma omp for
       for ( i = st; i < et; i++ )
       {
         for ( j = 1; j < n; j++ )
         {
	     A[i][j] = 0.2*(C[i][j] + C[i][j-1] + C[i-1][j] + C[i][j+1] + C[i+1][j]);
         }
       }
     } // pragma 
 
     diff = 0.0; 
     //#pragma omp parallel shared( A, C) private(i,j)
     {   
       //#pragma omp parallel for reduction (+:diff)
       for ( i = st; i <et ; i++ )
       {
         for ( j = 1; j < n; j++ )
         {
              diff += fabs(A[i][j] - C[i][j]);
         }
       }
       //printf("%f\t", diff );     
     } //pragma
 
     iters++;
     diff_arr[(for_iters-1)]=diff;
     //if (diff/((double)N*(double)N) < Tolerance)
     //convergence=TRUE;    
    } /*for*/
    return diff_arr;
}


long usecs (void)
{
  struct timeval t;

  gettimeofday(&t,NULL);
  return t.tv_sec*1000000+t.tv_usec;
}


int main(int argc, char * argv[])
{
   int i;
   long t_start,t_end;
   double time;
   double* diff_total = malloc(sizeof(double) * 20); 
   int start, end, start_r, end_r;

   for(i=0;i<20;i++){
	diff_total[i] = 0.0;
   }
   // MPI start
   int numprocs, rank;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   // MPI end

   if(rank == 0){
   t_start=usecs();
   // create batches and send to slave
   int i = 0, batch = 1;
   for(i=1;i<=N;(i=i+(N/(numprocs-1))))
   {
       start = i;
       end = (N/(numprocs-1))*batch;
       if( batch == (numprocs-1) ) {
	  end = N;
	  i = N;
       }
       //printf("Sending start %d , end %d, batch %d \n",start,end,batch);
       MPI_Send(&start, 1, MPI_INT, batch,2015, MPI_COMM_WORLD);
       MPI_Send(&end, 1, MPI_INT, batch,2015, MPI_COMM_WORLD);
       batch++;
   }
   
  double* s = malloc(sizeof(double) * 20);
    for(i=1;i<batch;i++){
          MPI_Recv(s, 20, MPI_DOUBLE, MPI_ANY_SOURCE, 2015, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  int z = 0;
	  for(z=0;z<20;z++){	
	       diff_total[z]+=s[z];	
	  }
    }
 
   int convergence=FALSE;
   for(i=0;i<20;i++){
     //printf("diff [%d} = %f \t",i,diff_total[i]);
     if (diff_total[i]/((double)N*(double)N) < Tolerance)
       convergence=TRUE;  
   }
   t_end=usecs();

   time = ((double)(t_end-t_start))/1000000;
   printf("Computation time = %f\n", time);
   } // if master
   else {
      A = malloc((N+2) * sizeof(double *));
      C = malloc((N+2) * sizeof(double *));

      //#pragma omp parallel for 
      for (i=0; i<N+2; i++) {
          A[i] = malloc((N+2) * sizeof(double));
          C[i] = malloc((N+2) * sizeof(double));
      }
      initialize(A,N);  
  
      MPI_Recv(&start_r, 1, MPI_INT, 0, 2015, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(&end_r, 1, MPI_INT, 0, 2015, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      //printf("I %d received start %d, end %d\n", rank, start_r, end_r);
      double* diff_arr = malloc(sizeof(double) * 20);
      diff_arr = solve(A,C,N, start_r,end_r);      
      MPI_Send(diff_arr, 20, MPI_DOUBLE, 0, 2015, MPI_COMM_WORLD);
   } // slave
   MPI_Finalize();
}
