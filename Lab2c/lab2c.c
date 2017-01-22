#define _GNU_SOURCE

#include "SortedList.h"
#include <getopt.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

static int numThreads = 1;
static int numLists = 1;
static int iterations = 1;
static char sync_mechanism = 'n';
int keylen = 3;
pthread_mutex_t* mutex;
int opt_yield = 0;
SortedList_t *head;
volatile int *lock;

void* thread_function(void* arg);
void insert(SortedListElement_t *element);
int lookup_and_delete(SortedListElement_t *element);
int getLength();
int hash(SortedListElement_t* element);

int main(int argc, char* argv[]){

  opt_yield = 0;
  
  static struct option longOptions[] = {
    {"threads", required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"yield", required_argument, NULL, 'y'},
    {"sync", required_argument, NULL, 's'},
    {"lists", required_argument, NULL, 'l'},
    {0, 0, 0, 0}
  };
  // Processes the arguments passed in
  while(1){
    int index = 0;
    int c = getopt_long(argc, argv, "", longOptions, &index);
    int i;
    if(c == -1)
      break;
    switch(c){
    case 't':
      numThreads = atoi(optarg); // atoi changes a string to a number if possible
      break;
    case 'i':
      iterations = atoi(optarg); // atoi changes a string to a number if possible
      break;
    case 'l':
      numLists = atoi(optarg); // atoi changes a string to a number if possible
      break;
    case 's':
      // for the case of sync, we find the sync_mechanism by looking at the first character
      sync_mechanism = optarg[0];
      break;
    case 'y':
      // For the case of yield, the arguments can only be ids. Based on the argument, we set opt_yield accordingly
      for(i = 0; i < strlen(optarg); i++){
	if(optarg[i] == 'i'){
	  opt_yield |= 0x01;
	}
	else if(optarg[i] == 'd'){
	  opt_yield |= 0x02;
	}
	else if(optarg[i] == 's'){
	  opt_yield |= 0x04;
	}
	else{
	  exit(-1);
	}
      }
    }
  }

  // Initializes the numLists linked lists. We will be using a circular linked list with a dummy node
  head = (SortedList_t*) malloc(sizeof(SortedList_t) * numLists);
  for(int i = 0; i < numLists; i++){
    head[i].next = &(head[i]);
    head[i].prev = &(head[i]);
    char *key = 0;
    head[i].key = key;
  }

  // Generates random strings of keylen characters
  int n = numThreads*iterations;
  char* keylist = (char*) malloc( (keylen + 1) * n * sizeof(char));
  int nchars = 1 + 'z' - '0';  // number of characters between 0 and z
  int counter = 0;
  for(int i = 0; i < ((keylen + 1) * n); i++){
    counter++;
    if(counter%(keylen + 1) == 0) // on the (keylen+1)th character,
      keylist[i] = 0; // set as nullbyte
    else
      keylist[i] = '0' + rand()%nchars; // random character offset from 0 in ascii
  }

  // Generates numThreads*iterations number of nodes
  SortedListElement_t *nodes = (SortedListElement_t*) malloc( n * sizeof(SortedListElement_t));
  for(int i=0; i<n; i++){
    nodes[i].key = keylist + i*(keylen + 1); // assigns a string to each node
  }

  // Dynamically creates threads based on what we passed into the threads argument
  pthread_t* tid = (pthread_t*) malloc(sizeof(pthread_t)*numThreads);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // Initializes mutexes for each list
  mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t)*numLists);
  for(int i=0; i<numLists; i++){
    pthread_mutex_init(mutex+i, NULL);
  }

  // Initializes the spinlocks for each list
  lock = (volatile int*) calloc(numLists, sizeof(volatile int));
  
  // collect start time
  struct timespec my_start_time;
  clock_gettime(CLOCK_MONOTONIC, &my_start_time);

  // start threads
  for(int i=0; i < numThreads; i++){
    if( pthread_create(&tid[i] , &attr, thread_function, (void*) (nodes+(i*iterations)) ) ){
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

  // calculates the elapsed time in nanoseconds
  long long elapsed_time = (my_end_time.tv_sec - my_start_time.tv_sec) * 1000000000;
  elapsed_time += my_end_time.tv_nsec;
  elapsed_time -= my_start_time.tv_nsec;

  // Report data
  int exit_status = 0;
  int operations = numThreads * iterations * 2;
  long long time_per_operation = elapsed_time / operations;
  fprintf(stdout, "%d threads x %d iterations x (insert + lookup/delete) = %d operations\n", numThreads, iterations, operations);
  if(getLength() != 0){
    fprintf(stderr, "List is not empty at the end\n");
    exit_status = 1;
  }
  fprintf(stdout, "elapsed time: %lld\n", elapsed_time);
  fprintf(stdout, "per operation: %lld\n", time_per_operation);
  
  // Exit
  exit(exit_status);
}

void* thread_function(void* arg){
  SortedListElement_t* node = (SortedListElement_t*) arg;

  // Do the insertion
  for(int i=0; i<iterations; i++){
    insert(node+i);
  }

  // Get the list length
  getLength();

  // Looks up and deletes each of the keys it has previously inserted
  for(int i=0; i<iterations; i++){
    lookup_and_delete(node+i);
  }
  return NULL;
}

int hash(SortedListElement_t* element){
  const char* key = element->key;
  int total = 0;
  for(int i = 0; i < strlen(key); i++){
    total += key[i];
  }
  return total % numLists;
}

void insert(SortedListElement_t *element){
  int index = hash(element);

  // Do the insertion
  switch(sync_mechanism){
  case 'n':
    SortedList_insert( head+index , element);
    break;
  case 'm':
    pthread_mutex_lock(mutex+index);
    SortedList_insert( head+index , element);
    pthread_mutex_unlock(mutex+index);
    break;
  case 's':
    while(__sync_lock_test_and_set( lock+index, 1) == 1);
    SortedList_insert( head+index , element);
    __sync_lock_release(lock+index);
  default:
    break;
  }
}

int lookup_and_delete(SortedListElement_t *element){
  int index = hash(element);
  int ret;
  
  // Deletes each of the keys it has previously inserted
  switch(sync_mechanism){
  case 'n':
    SortedList_lookup(head + index, element->key);
    ret = SortedList_delete(element);
    break;
  case 'm':
    pthread_mutex_lock(mutex+index);
    SortedList_lookup(head + index, element->key);
    ret = SortedList_delete(element);
    pthread_mutex_unlock(mutex+index);
    break;
  case 's':
    while(__sync_lock_test_and_set(lock+index, 1) == 1);
    SortedList_lookup(head + index, element->key);
    ret = SortedList_delete(element);
    __sync_lock_release(lock+index);
  default:
    break;
  }
  
  return ret;
}
int getLength(){
  int total = 0;
  // Get the list length
  for(int i=0; i<numLists; i++){
    switch(sync_mechanism){
    case 'n':
      total += SortedList_length(head+i);
      break;
    case 'm':
      pthread_mutex_lock(mutex+i);
      total += SortedList_length(head+i);
      pthread_mutex_unlock(mutex+i);
      break;
    case 's':
      while(__sync_lock_test_and_set(lock+i, 1) == 1);
      total += SortedList_length(head+i);
      __sync_lock_release(lock+i);
    default:
      break;
    }
  }
  return total;
}
