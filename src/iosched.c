#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define time_mean(a) start=clock(); a; diff = clock() - start; msec = (float)diff * 1000 / CLOCKS_PER_SEC; times[i] = msec; 
#define time(a) start=clock(); a; diff = clock() - start; msec = (float)diff * 1000 / CLOCKS_PER_SEC; printf("time msec: %f\n", msec); 

void *create_file(void *filename);
void *remove_file(void *filename);
//void *file_factory(int num_files);
void *threadfile_creator(int num_files);
void *threadfile_deleter(int num_files);

clock_t start, diff;
float msec;
volatile int thread_count = 0;

int main(int argc, char *argv[]) {



	if (argc != 2) {
		printf("Usage: %s [number of files]\n", argv[0]);
		exit(0);
	}

	int num_files = strtol(argv[1], NULL, 10);


/*
	// GET CREATE AVERAGE
	float times[10];
	for (int i = 0; i<10; i++) {
		time_mean(threadfile_creator(num_files));
	}
	float mean = 0;
	for (int i = 0; i<10; i++) {
		mean += times[i];
	}
	printf("mean create time %f\n", mean/10);
*/

/*
	// GET DELETE AVERAGE
	memset(times, 0, 10*sizeof(*times));
	for (int i = 0; i<10; i++) {
		time_mean(threadfile_deleter(num_files));
	}
	mean = 0;
	for (int i = 0; i<10; i++) {
		mean += times[i];
	}
	printf("mean create time %f\n", mean/10);
*/

	time(threadfile_creator(num_files));
	time(threadfile_deleter(num_files));


}

void *threadfile_creator(int num_files) {
	// create files
	thread_count += num_files;
	for (int i = 0; i<num_files; i++) {
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

void *threadfile_deleter(int num_files) {
	// remove files
	thread_count += num_files;
	for (int i = 0; i<num_files; i++) {
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

	//printf("creating file %s\n", (char *)filename);
  	__sync_fetch_and_sub(&thread_count,1);
	return NULL;
}

// may only be called from thread
void *remove_file(void *filename) {
	//printf("removing file %s\n", (char *)filename);
  	__sync_fetch_and_sub(&thread_count,1);
	return NULL;

}
