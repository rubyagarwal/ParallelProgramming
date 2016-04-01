#include<stdio.h>
#include<math.h>
#include<mpi.h>

int main(int argc, char* argv[]){
  MPI_Init(NULL,NULL);
  int processCnt;
  int processorCnt;
  
  MPI_Comm_rank(MPI_COMM_WORLD, &processCnt);
  MPI_Comm_size(MPI_COMM_WORLD, &processorCnt);
 
  int r = 99,rounds=2,b,i;
  for(i=1; i<=rounds; i++){
    //printf("Round %d\n",i);
    int batchSize = (int) pow(2, i);
    int half = (int) batchSize>>1;
    int num,j;
    int numBatches = processorCnt/batchSize;
    for(j=0; j<numBatches; j++){
      if(processCnt < (half+(j*batchSize))){
	num = processCnt+i;
        MPI_Send(&processCnt, 1, MPI_INT, num, 2015, MPI_COMM_WORLD);
	MPI_Recv(&r, 1, MPI_INT, num, 2015, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("sent by %d sent to  %d\n",processCnt, num);
      } else {
  	num = processCnt-i;
	MPI_Send(&processCnt, 1, MPI_INT, num, 2015, MPI_COMM_WORLD);
	MPI_Recv(&r, 1, MPI_INT, num, 2015, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	printf("sent by %d sent to  %d\n",processCnt, num);
      }
    }  
  }
  MPI_Finalize();
}
