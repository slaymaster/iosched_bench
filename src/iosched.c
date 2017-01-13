#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

#define FILES_PER_THREAD 10
#define NUM_TESTS 40

#define time(a) gettimeofday(&s,NULL); a; gettimeofday(&e,NULL); sec=(e.tv_sec-s.tv_sec); microsec=((sec*1000000) + e.tv_usec)-(s.tv_usec); printf("%ld\n", microsec);


void *create_file(void *filename);
void *remove_file(void *filename);
void *threadfile_creator(int num_threads);
void *threadfile_deleter(int num_threads);
void *thread_master(void *num_threads);
void sig_handler(int signo);

clock_t start, diff;
float msec;

struct timeval s, e;
long microsec, sec;
volatile int thread_count = 0;
volatile int running = 1;

int main(int argc, char *argv[]) {

	signal(SIGINT, sig_handler); 


	if (argc != 2) {
		printf("Usage: %s [number of threads]\n", argv[0]);
		exit(0);
	}

	int num_threads = strtol(argv[1], NULL, 10);


	pthread_t threadmaster;
	pthread_create(&threadmaster, NULL, &thread_master, &num_threads);

	fprintf(stderr, "Master launched threads\n");
	fprintf(stderr, "Master starting massive superwrite\n");
	// begin massive annoying write and time it
	time(system("dd if=/dev/zero of=test bs=64k count=32k conv=fdatasync"));

	if (running == 1) {
		__sync_fetch_and_sub(&running, 1);
	} else {
		fprintf(stderr, "error: running was not %d, something very wrong\n", running);
	}
	fprintf(stderr, "Master waiting for threadmaster\n");
	pthread_join(threadmaster, NULL);


	return 0;

}

void sig_handler(int signo) {

	if (signo == SIGINT) {
		fprintf(stderr, "CAUGHT SIGINT--EXITING\n");
		if (running == 1) {
			__sync_fetch_and_sub(&running, 1);
		}
	}

}

/* Thread that manages smaller threads.
 * Ensures that there is a constant stream of smaller threads
 * working as long as the main thread is busy.
 * usage: num_threads - the amount of threads to be spawned each round
 */
void *thread_master(void *num_threads) {
	while(running) {
		if (thread_count == 0) {
			time(threadfile_creator(*(int*)num_threads));
			time(threadfile_deleter(*(int*)num_threads));

		} 
	}

	return 0;

}

/* Spawns a number of threads and dispatches them
 * to create files. Waits for them all to finish before returning.
 * usage: num_threads - the number of threads
 */
void *threadfile_creator(int num_threads) {
	// create threads
	for (int i = 0; i<num_threads; i++) {
		char *filename = malloc(sizeof(char)*10);
		sprintf(filename, "efbefc%d", i);
		pthread_t thread;
		pthread_attr_t tattr;
		pthread_attr_init(&tattr);
		pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
		__sync_fetch_and_add(&thread_count, 1);
		if (pthread_create(&thread, &tattr, &create_file, filename) != 0) {
			perror("failed to create thread");
			__sync_fetch_and_sub(&thread_count, 1);
		}
	}

	do {
    	__sync_synchronize();
  	} while (thread_count);

	return NULL;
	
}

/* Spawns a number of threads and dispatches them
 * to remove files. Waits for them all to finish before returning.
 * usage: num_threads - the number of threads
 */
void *threadfile_deleter(int num_threads) {
	// remove files
	for (int i = 0; i<num_threads; i++) {
		char *filename = malloc(sizeof(char)*10);
		sprintf(filename, "efbefc%d", i);
		pthread_t thread;
		pthread_attr_t tattr;
		pthread_attr_init(&tattr);
		pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
		__sync_fetch_and_add(&thread_count, 1);
		if(pthread_create(&thread, &tattr, &remove_file, filename)) {
			perror("failed to create thread");
			__sync_fetch_and_sub(&thread_count, 1);
		}
	}

	do {
    	__sync_synchronize();
  	} while (thread_count);
	
	return NULL;
}



/* Single function to create files.
 * May only be called from within a thread
 * usage: filename - the basename of the file to be created
 */
void *create_file(void *filename) {
	for (int i = 0; i<FILES_PER_THREAD; i++) {
		char *fn = malloc(sizeof(char)*strlen(filename)+10);
		sprintf(fn, "%s%d", (char*)filename, i);
		int fd;
		if((fd = open(fn, O_CREAT | O_EXCL) <0)) {
			perror("create file");
		} else {
			close(fd);
		}
		free(fn);
	}
  	free(filename);
  	__sync_fetch_and_sub(&thread_count,1);
	return NULL;
}

/* Single function to remove files.
 * May only be called from within a thread
 * usage: filename - the basename of the file to be removed
 */void *remove_file(void *filename) {
	for (int i = 0; i<FILES_PER_THREAD; i++) {
		char *fn = malloc(sizeof(char)*strlen(filename)+10);
		sprintf(fn, "%s%d", (char*)filename, i);
		if (remove(fn) < 0) {
			perror("remove file");
		}
		free(fn);
	}

  	free(filename);
  	__sync_fetch_and_sub(&thread_count,1);
	return NULL;

}
