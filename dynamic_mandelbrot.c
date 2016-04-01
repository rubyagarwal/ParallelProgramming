#include<stdio.h>
#include<mpi.h>

void color(int red, int green, int blue)  {
       fputc((char)red,stdout);
       fputc((char)green,stdout);
       fputc((char)blue,stdout);
}

int main(int argc, char *argv[]){

  MPI_Init(NULL,NULL);
  int image[500][500];
  int processCnt, processorCnt, start, end=0, start_r, end_r;
  MPI_Comm_rank(MPI_COMM_WORLD, &processCnt);
  MPI_Comm_size(MPI_COMM_WORLD, &processorCnt);

  if(processCnt == 0){
      // master
      double startT = MPI_Wtime();
      printf("P6\n# CREATOR: Eric R Weeks / mandel program\n");
      printf("%d %d\n255\n",500,500);
      int i = 0, max = 500, batch = 1, batchCnt = 2;
      for(i=1;i<processorCnt;i++){
        start = end+1;
        end = (max/batchCnt)*batch;
        MPI_Send(&start, 1, MPI_INT, i,2015, MPI_COMM_WORLD);
        MPI_Send(&end, 1, MPI_INT, i,2015, MPI_COMM_WORLD);    	
        batch++;
     }

     int iter[end-start+1][500], hy, hx;
     MPI_Status status;
     int s,e;
     for(i=0;i<batchCnt;i++){ 
       	  MPI_Recv(&s, 1, MPI_INT, MPI_ANY_SOURCE, 2015, MPI_COMM_WORLD, &status );
	  MPI_Recv(&e, 1, MPI_INT, status.MPI_SOURCE, 2015, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  MPI_Recv(iter, ((e-s+1)*500), MPI_INT, status.MPI_SOURCE, 2015, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  start = end+1;
          end = (max/batchCnt)*batch;
   	  if(end<=500){
	     MPI_Send(&start, 1, MPI_INT, status.MPI_SOURCE,2015, MPI_COMM_WORLD);
             MPI_Send(&end, 1, MPI_INT, status.MPI_SOURCE,2015, MPI_COMM_WORLD);
	     batch++;
	  }
	  int a , b;
          for(a=s;a<=e;a++){
		for(b=1;b<=500;b++){
			image[a-1][b-1] = iter[a-s][b-1];
		}
	  }
     }
     int p ;
     for(p=1;p<processorCnt;p++){
	MPI_Send(&p, 1, MPI_INT,p,2016, MPI_COMM_WORLD);	
     }
     int m,n;
     for(m=0;m<500;m++){
	for(n=0;n<500;n++){
	   if (image[m][n]<99999)  color(0,255,255);
	   else color(180,0,0);
	}
     }
     printf("time taken %f\n",((MPI_Wtime()-startT)/MPI_Wtick()));
  } else {
      double x,xx,y,cx,cy;
      int iteration,hx,hy;
      int itermax = 100;              /* how many iterations to do    */
      double magnify=1.0; 
      MPI_Status status;
      while(1){
      MPI_Recv(&start_r, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if(status.MPI_TAG == 2016){
	break;
      }
      MPI_Recv(&end_r, 1, MPI_INT, 0, 2015, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      //printf("Process %d received start %d , end %d\n", processCnt, start_r, end_r);


      int iter[end_r-start_r+1][500];
      int hyres=500, hxres=500;
      for (hy=start_r;hy<=end_r;hy++)  {
        for (hx=1;hx<=hxres;hx++)  {
          cx = (((float)hx)/((float)hxres)-0.5)/magnify*3.0-0.7;
          cy = (((float)hy)/((float)hyres)-0.5)/magnify*3.0;
          x = 0.0; y = 0.0;
          for (iteration=1;iteration<itermax;iteration++)  {
            xx = x*x-y*y+cx;
            y = 2.0*x*y+cy;
            x = xx;
            if (x*x+y*y>100.0)  iteration = 999999;
          }
	  iter[hy-start_r][hx-1] = iteration;
         }
      }
      MPI_Send(&start_r, 1, MPI_INT, 0, 2015, MPI_COMM_WORLD);
      MPI_Send(&end_r, 1, MPI_INT, 0, 2015, MPI_COMM_WORLD);
      MPI_Send(iter, ((end_r-start_r+1)*500), MPI_INT, 0, 2015, MPI_COMM_WORLD);
      }   
  }
  MPI_Finalize();
}
