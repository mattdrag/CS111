#define _GNU_SOURCE

#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

long long counter = 0;
static int numThreads = 1;
static int iterations = 1;
static int sync_mechanism = 'n';
static int opt_yield = 0;
volatile int lock = 0;
pthread_mutex_t mutex;

void add(long long* pointer, long long value);
void* thread_function(void* arg);

int main(int argc, char* argv[]){
  // using while loop and getopt_long() to process options
  static struct option longOptions[] = {
    {"threads", required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"sync", required_argument, NULL, 's'},
    {"yield", no_argument, &opt_yield, 1},
    {0, 0, 0, 0}
  };
  while(1){
    int index = 0;
    int c = getopt_long(argc, argv, "", longOptions, &index);
    if(c == -1)
      break;
    switch(c){
    case 't':
      numThreads = atoi(optarg);
      break;
    case 'i':
      iterations = atoi(optarg);
      break;
    case 's':
      sync_mechanism = optarg[0];
      break;
    case 0:
    default:
      break;
    }
  }

  // Dynamically creates threads based on what we passed into the threads argument
  pthread_t* tid = (pthread_t*) malloc(sizeof(pthread_t)*numThreads);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_mutex_init(&mutex, NULL);
  
  // collect start time
  struct timespec my_start_time;
  clock_gettime(CLOCK_MONOTONIC, &my_start_time);
  
  // start threads
  for(int i=0; i < numThreads; i++){
    if( pthread_create(&tid[i] , &attr, thread_function, NULL) ){
      //if( pthread_create(&tid[i] , NULL, thread_function, NULL) ){
      fprintf(stderr, "Error creating threads");
      exit(1);
    }
  }

  pthread_attr_destroy(&attr);

  // wait for all the threads to finish
  for(int i=0; i < numThreads; i++){
    if( pthread_join(tid[i], NULL) ){
      fprintf(stderr, "Error joining threads");
      exit(1);
    }
  }
  
  // collect end time
  struct timespec my_end_time;
  clock_gettime(CLOCK_MONOTONIC, &my_end_time);

  free(tid);
  pthread_mutex_destroy(&mutex);
  
  // calculates the elapsed time in nanoseconds
  long long elapsed_time = (my_end_time.tv_sec - my_start_time.tv_sec) * 1000000000;
  elapsed_time += my_end_time.tv_nsec;
  elapsed_time -= my_start_time.tv_nsec;

  // Report data
  int operations = numThreads * iterations * 2;
  long long time_per_operation = elapsed_time / operations;
  fprintf(stdout, "%d threads x %d iterations x (add + subtract) = %d operations\n", numThreads, iterations, operations);
  if(counter != 0)
    fprintf(stderr, "ERROR: final count = %lld\n", counter);
  fprintf(stdout, "elapsed time: %lld\n", elapsed_time);
  fprintf(stdout, "per operation: %lld\n", time_per_operation);
  
  // Exit
  exit(0);
}

//thread_function_to_run_test() performs the additions (with locking)
void* thread_function(void* arg){  // declare whatever parameters you want
  //first perform num_of_iterations + 1
  for(int i = 0; i < iterations; i++){
    long long temp;
    long long sum;
    switch(sync_mechanism){
    case 'n':             // No lock
      add(&counter, 1);
      break;
    case 'm':             // Mutex lock
      // Implement mutex lock
      pthread_mutex_lock(&mutex);
      add(&counter, 1);
      pthread_mutex_unlock(&mutex);
      break;
    case 's':             // Spin lock
      // Implement spin lock
      while(__sync_lock_test_and_set(&lock, 1) == 1);
      add(&counter, 1);
      __sync_lock_release(&lock);
      break;
    case 'c':             // Compare and swap
      // Implement compare and swap
      // use __sync_cal_compare and swap to ensure atomic updates to the shared counter
      // the textbookgives an example using compare and swap to implement spin lock
      //apparently that is not what you should do here
      //good example: http://web.cs.ucla.edu/classes/spring16/cs111/slides/08mutex.pdf page 4
      do{
	temp = counter;
	sum = temp + 1;
	if(opt_yield)
	  pthread_yield();
      }while( __sync_val_compare_and_swap(&counter, temp, sum) != temp);
      break;
    }
  }
  
  //second perform num_of_iterations times -1
  for(int i = 0; i < iterations; i++){
    long long temp;
    long long diff;
    switch(sync_mechanism){
    case 'n':             // No lock
      add(&counter, -1);
      break;
    case 'm':             // Mutex lock
      // Implement mutex lock
      pthread_mutex_lock(&mutex);
      add(&counter, -1);
      pthread_mutex_unlock(&mutex);
      break;
    case 's':             // Spin lock
      // Implement spin lock
      while(__sync_lock_test_and_set(&lock, 1) == 1);
      add(&counter, -1);
      __sync_lock_release(&lock);
      break;
    case 'c':             // Compare and swap
      do{
	temp = counter;
	diff = temp - 1;
	if(opt_yield)
	  pthread_yield();
      }while( __sync_val_compare_and_swap(&counter, temp, diff) != temp);
      break;
    }
  }
  return NULL;
}

void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if (opt_yield)
    pthread_yield();
  *pointer = sum;
}
