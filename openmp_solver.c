#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>

#define Tolerance 0.00001
#define TRUE 1
#define FALSE 0

#define N 5000

double ** A;
double ** C;

int initialize (double **A, int n)
{
   int i,j;
   
   #pragma omp parallel shared (A) private (i,j) 
   {
     #pragma omp parallel for 
     for (j=0;j<n+1;j++){
       A[0][j]=1.0;
     }

     #pragma omp parallel for private(j)
     for (i=1;i<n+1;i++){
       A[i][0]=1.0;
       #pragma omp parallel for
       for (j=1;j<n+1;j++) A[i][j]=0.0;
     }
   }
}

void solve(double **A, double **C,  int n)
{
   int convergence=FALSE;
   double diff, tmp;
   int i,j, iters=0;
   int for_iters;

   for (for_iters=1;for_iters<21;for_iters++) 
   { 
     #pragma omp parallel shared (A, C) private (i,j) 
     {
       // save old values of A
       #pragma omp for
       for ( i = 0; i < (n+1); i++ ) 
       {
         for ( j = 0; j < (n+1); j++ )
         {
           C[i][j] = A[i][j];
         }
       }

       #pragma omp for schedule (dynamic)
       for ( i = 1; i < n; i++ )
       {
         for ( j = 1; j < n; j++ )
         {
	     A[i][j] = 0.2*(C[i][j] + C[i][j-1] + C[i-1][j] + C[i][j+1] + C[i+1][j]);
         }
       }
     } // pragma 
 
     diff = 0.0; 
     #pragma omp parallel shared (A, C) private(i,j)
     {   
       #pragma omp for reduction (+:diff)
       for ( i = 1; i < n; i++ )
       {
         for ( j = 1; j < n; j++ )
         {
              diff += fabs(A[i][j] - C[i][j]);
         }
       }      
     } //pragma

     //printf("diff : %f\n",diff);
 
     iters++;
     if (diff/((double)N*(double)N) < Tolerance)
     convergence=TRUE;    
   } /*for*/
   //printf("convergence %d\n", convergence);
}


long usecs (void)
{
  struct timeval t;

  gettimeofday(&t,NULL);
  return t.tv_sec*1000000+t.tv_usec;
}


int main(int argc, char * argv[])
{
 //  printf ("max threads %d\n", omp_get_max_threads());
   int i;
   long t_start,t_end;
   double time;

   A = malloc((N+2) * sizeof(double *));
   C = malloc((N+2) * sizeof(double *));

   #pragma omp parallel for 
   for (i=0; i<N+2; i++) {
	A[i] = malloc((N+2) * sizeof(double)); 
	C[i] = malloc((N+2) * sizeof(double));
   }

   initialize(A, N);

   t_start=usecs();
   solve(A,C,N);
   t_end=usecs();

   time = ((double)(t_end-t_start))/1000000;
   printf("Computation time = %f\n", time);
}
