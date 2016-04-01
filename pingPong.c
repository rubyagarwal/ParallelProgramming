#include<stdio.h>
#include<math.h>
#include<mpi.h>

int main(int argc, char* argv[]){
  MPI_Init(NULL,NULL);
  int processCnt;
  int processorCnt;
  int tag = 0;
  int i = 0;
  int rounds=50;
  double start, timeTaken;
  long fact = 1;
 
  MPI_Comm_rank(MPI_COMM_WORLD, &processCnt);
  MPI_Comm_size(MPI_COMM_WORLD, &processorCnt);
  
  int sizeCnt=20;
  int sizeArr[sizeCnt+2];
  double avgCommTime[sizeCnt+2];
  sizeArr[0] = 0;
  for(i=0; i<=sizeCnt; i++){
     sizeArr[i+1]=pow(2, i);
  }

  if(processCnt == 0){
   printf("index	size		avg_communication_time\n"); 
   for(i=0;i<=(sizeCnt+1);i++){
      // message size changes
      char message[sizeArr[i]];
      int k;
      for(k=0; k<sizeArr[i]; k++){
	message[k] = k+'a';
      }
 	
      int j=0;
      start = MPI_Wtime();  
      //printf("size %d\n",sizeArr[i]);
      for(j=0; j<rounds; j++){
        //double s = MPI_Wtime();
	MPI_Send(message, sizeArr[i], MPI_CHAR, 1, tag, MPI_COMM_WORLD);
        MPI_Recv(message, sizeArr[i],  MPI_CHAR,  1,  tag,  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	//printf("%f,",((MPI_Wtime()-s)/(MPI_Wtick()*2)));
      }
      timeTaken = (MPI_Wtime()-start)/MPI_Wtick();  
      avgCommTime[i] = (timeTaken/(rounds*2));
      printf("%d	%d		%f\n", i, sizeArr[i], avgCommTime[i]) ;
    }
        
    int numArrSize = 10000;
    int numArr[numArrSize];
    for(i=0; i<numArrSize; i++){
	numArr[i]= (i+1);
    }

    start = MPI_Wtime();
    for(i=0;i<numArrSize;i++){
	fact = i * numArr[i];		
    }
    printf("Average computation time %f\n",((MPI_Wtime()-start)/(MPI_Wtick()*numArrSize)));
  } else if(processCnt == 1){
    char message[sizeArr[i]];
    int j;
    for(i=0;i<=(sizeCnt+1);i++){
      for(j=0; j<rounds; j++){   
         MPI_Recv(message, sizeArr[i], MPI_CHAR, 0, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         MPI_Send(message, sizeArr[i], MPI_CHAR, 0, tag, MPI_COMM_WORLD);
      }
    }
  }
 
  MPI_Finalize();
}

// least fit square line method
