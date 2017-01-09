#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>

#define time_mean(a) start=clock(); a; diff = clock() - start; msec = (float)diff * 1000 / CLOCKS_PER_SEC; times[i] = msec; 
#define time(a) start=clock(); a; diff = clock() - start; msec = (float)diff * 1000 / CLOCKS_PER_SEC; printf("time msec: %f\n", msec); 
#define time_mean_create(a) start=clock(); a; diff = clock() - start; msec = (float)diff * 1000 / CLOCKS_PER_SEC; create_mean[i] = msec; 
#define time_mean_remove(a) start=clock(); a; diff = clock() - start; msec = (float)diff * 1000 / CLOCKS_PER_SEC; remove_mean[i] = msec; 

#define FILES_PER_THREAD 10
#define NUM_TESTS 45

void *create_file(void *filename);
void *remove_file(void *filename);
void *threadfile_creator(int num_threads);
void *threadfile_deleter(int num_threads);

clock_t start, diff;
float msec;
volatile int thread_count = 0;

int main(int argc, char *argv[]) {



	if (argc != 2) {
		printf("Usage: %s [number of threads]\n", argv[0]);
		exit(0);
	}

	int num_threads = strtol(argv[1], NULL, 10);



	// GET CREATE/REMOVE AVERAGE OVER 10 TEST RUNS
	float create_mean[NUM_TESTS];
	float remove_mean[NUM_TESTS];
	for (int i = 0; i<NUM_TESTS; i++) {
		time_mean_create(threadfile_creator(num_threads));
		time_mean_remove(threadfile_deleter(num_threads));
	}

	float mean_create = 0;
	for (int i = 0; i<NUM_TESTS; i++) {
		mean_create += create_mean[i];
	}
	printf("mean create time %f\n", mean_create/NUM_TESTS);

	float mean_remove = 0;
	for (int i = 0; i<NUM_TESTS; i++) {
		mean_remove += remove_mean[i];
	}

	printf("mean remove time %f\n", mean_remove/NUM_TESTS);






// SINGLE RUN
/*
	time(threadfile_creator(num_threads));
	time(threadfile_deleter(num_threads));
*/

}

void *threadfile_creator(int num_threads) {
	// create files
	thread_count += num_threads;
	for (int i = 0; i<num_threads; i++) {
		char *filename = malloc(sizeof(char)*10);
		sprintf(filename, "efbefc%d", i);
		pthread_t thread;
		pthread_create(&thread, NULL, &create_file, filename);
	}

	//create_file("efbefc0");

	do {
    	__sync_synchronize();
  	} while (thread_count);

	return NULL;
	
}

void *threadfile_deleter(int num_threads) {
	// remove files
	thread_count += num_threads;
	for (int i = 0; i<num_threads; i++) {
		char *filename = malloc(sizeof(char)*10);
		sprintf(filename, "efbefc%d", i);
		pthread_t thread;
		pthread_create(&thread, NULL, &remove_file, filename);
	}

	do {
    	__sync_synchronize();
  	} while (thread_count);
	
	return NULL;
}



// may only be called from thread
void *create_file(void *filename) {
	for (int i = 0; i<FILES_PER_THREAD; i++) {
		char *fn = malloc(sizeof(char)*strlen(filename)+1);
		sprintf(fn, "%s%d", (char*)filename, i);
		open(fn, O_CREAT | O_EXCL);
		//printf("creating file %s\n", fn);
	}
  	__sync_fetch_and_sub(&thread_count,1);
	return NULL;
}

// may only be called from thread
void *remove_file(void *filename) {
	for (int i = 0; i<FILES_PER_THREAD; i++) {
		char *fn = malloc(sizeof(char)*strlen(filename)+1);
		sprintf(fn, "%s%d", (char*)filename, i);
		remove(fn);
		//printf("removing file %s\n", fn);
	}

  	__sync_fetch_and_sub(&thread_count,1);
	return NULL;

}
