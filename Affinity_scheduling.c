/*
* Exam Number: B175662
* Threaded Programming Part 3 Coursework
*/

#include <stdio.h>
#include <math.h>


#define N 1729
#define reps 1000 
#include <omp.h> 

// Flags used to represent whether our scheduling is completed
#define COMPLETED 1
#define UNCOMPLETED 0


double a[N][N], b[N][N], c[N];
int jmax[N];  




void init1(void);
void init2(void);
void runloop(int);
void loop1chunk(int, int);
void loop2chunk(int, int);
void valid1(void);
void valid2(void);
int determineThreadToExecute(int*, int*, int, int, int*, int*);

int main(int argc, char *argv[]) { 

  double start1,start2,end1,end2;
  int r;

  init1(); 

  start1 = omp_get_wtime(); 

  for (r=0; r<reps; r++){ 
    runloop(1);
  } 

  end1  = omp_get_wtime();  

  valid1(); 

  printf("Total time for %d reps of loop 1 = %f\n",reps, (float)(end1-start1)); 


  init2(); 

  start2 = omp_get_wtime(); 

  for (r=0; r<reps; r++){ 
    runloop(2);
  } 

  end2  = omp_get_wtime(); 

  valid2(); 

  printf("Total time for %d reps of loop 2 = %f\n",reps, (float)(end2-start2)); 

} 

void init1(void){
  int i,j; 

  for (i=0; i<N; i++){ 
    for (j=0; j<N; j++){ 
      a[i][j] = 0.0; 
      b[i][j] = 1.618*(i+j); 
    }
  }

}

void init2(void){ 
  int i,j, expr; 

  for (i=0; i<N; i++){ 
    expr =  i%( 4*(i/60) + 1); 
    if ( expr == 0) { 
      jmax[i] = N/2;
    }
    else {
      jmax[i] = 1; 
    }
    c[i] = 0.0;
  }

  for (i=0; i<N; i++){ 
    for (j=0; j<N; j++){ 
      b[i][j] = (double) (i*j+1) / (double) (N*N); 
    }
  }
 
} 

/*********************************************************************************************/
/* The beginning of our implementation of affinity scheduling */


void runloop(int loopid)  {

    // Get the number of threads our code will run on
    int num_of_threads = omp_get_max_threads(); 
    
    // The boundaries of iterations for each thread
    int upper_bound[num_of_threads]; 

    int lower_bound[num_of_threads];


/* Here the number of shared data I use is 4 (2 integers and 2 arrays) */
#pragma omp parallel default(none) shared(num_of_threads, loopid, upper_bound, lower_bound)
  {
    int myid  = omp_get_thread_num();

    // The size of local set
    int set_size = (int) ceil((double)N/(double)num_of_threads); //the size of each local set
    
    //Determine the specific boundaries of iterations for each thread 
    lower_bound[myid] = myid*set_size;
    upper_bound[myid] = (myid+1)*set_size;
    if (upper_bound[myid] > N) upper_bound[myid] = N; 
    

    // Initialize work
    int chunksize = (int) ceil((double)set_size/(double)num_of_threads);

    //Determine the Initialized position to start and stop executing the first chunk
    int chunk_start = lower_bound[myid]; 
    int chunk_stop = lower_bound[myid] + chunksize;
    if (chunk_stop > upper_bound[myid]) chunk_stop = upper_bound[myid];
    
    //Update the lower bounds for each thread after assigning chunks
    lower_bound[myid] = chunk_stop;


    //We use "complete" to represent the situation about our scheduling process 
    int complete = UNCOMPLETED;


//Synchronize all the threads before entering the while-loop to avoid race conditions
#pragma omp barrier

    //Start affinity scheduling
    while (complete != COMPLETED){

        switch (loopid) { 
           case 1: loop1chunk(chunk_start, chunk_stop); break;
           case 2: loop2chunk(chunk_start, chunk_stop); break;
        }
  
// Avoid race conditions
#pragma omp critical
      {
        complete = determineThreadToExecute(lower_bound, upper_bound \
                            , num_of_threads, myid, &chunk_start, &chunk_stop);  
      }
    }
  }
}



/*
 * The following function firstly identifies either executing the chunk in local set or the chunk in the "most loaded" 
 * thread. The function returns COMPLETED if there are no more interations remaining in any thread's local set. 
 * Once the thread's position including the chunk to be executed is confirmed, then update the variables 
 * "chunk_start", "chunk_stop" and "lower_bound".
 */

int determineThreadToExecute(int* lower_bound, int* upper_bound, int num_of_threads, int myid, int* chunk_start, int* chunk_stop)
{
    int position;

    // Firstly determine whether there are any iterations left in thread's local set.
    if (upper_bound[myid] - lower_bound[myid] > 0)
    {
        position = myid;
    }
    // If there are no more iterations left in thread's local set, we then aim to find out the "most loaded" thread.
    else
    {
        int most_loaded = -1; // Since the threadID is from 0, we use -1 to represent "no 'most loaded' thread".
        int value = 0;

        // Look for "most loaded" thread by traversing. 
        for (int i = 0; i < num_of_threads; i++)
        {
            int remaining_iter = upper_bound[i] - lower_bound[i];
            if (remaining_iter > value)
            {
                most_loaded = i;
                value = remaining_iter;
            }
        }
        position = most_loaded;
    }

    // If there are no more iterations in any thread, the function returns our set flag "COMPLETED".
    if (position == -1) return COMPLETED;


    // After having determined the threadID that have the chunk to be executed, we then calculate the chunksize to be executed.
    int loaded_remaining = upper_bound[position] - lower_bound[position];
    int chunk_size = (int) ceil((double)loaded_remaining/(double)num_of_threads);


    // Update the variables 
    *chunk_start = lower_bound[position];
    *chunk_stop = lower_bound[position] + chunk_size; 
    if (*chunk_stop > upper_bound[position]) *chunk_stop = upper_bound[position];   
    lower_bound[position] = *chunk_stop;

    return UNCOMPLETED;
}

/********************************************************************************************************/
/* The end of our modification and implementation of the affinity scheduling */


void loop1chunk(int lo, int hi) { 
  int i,j; 
  
  for (i=lo; i<hi; i++){ 
    for (j=N-1; j>i; j--){
      a[i][j] += cos(b[i][j]);
    } 
  }

} 



void loop2chunk(int lo, int hi) {
  int i,j,k; 
  double rN2; 

  rN2 = 1.0 / (double) (N*N);  

  for (i=lo; i<hi; i++){ 
    for (j=0; j < jmax[i]; j++){
      for (k=0; k<j; k++){ 
    c[i] += (k+1) * log (b[i][j]) * rN2;
      } 
    }
  }

}


void valid1(void) { 
  int i,j; 
  double suma; 
  
  suma= 0.0; 
  for (i=0; i<N; i++){ 
    for (j=0; j<N; j++){ 
      suma += a[i][j];
    }
  }
  printf("Loop 1 check: Sum of a is %lf\n", suma);

} 


void valid2(void) { 
  int i; 
  double sumc; 
  
  sumc= 0.0; 
  for (i=0; i<N; i++){ 
    sumc += c[i];
  }
  printf("Loop 2 check: Sum of c is %f\n", sumc);
} 
 

