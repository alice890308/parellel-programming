#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>

unsigned long long total = 0;
unsigned long long r;
unsigned long long k;
unsigned long long part;
unsigned long long num_threads;
unsigned long long rmd;
pthread_mutex_t mutex;

void* calculate(void* obj) {
	unsigned long long* id = (unsigned long long*) obj; // arr[id, r, k, num_thread]
	unsigned long long pixels = 0;
	unsigned long long start = (*id < rmd) ? *id*(part+1) : *id*part+rmd;
	unsigned long long end = (*id < rmd) ? start + part + 1 : start + part;
	for (unsigned long long x = start; x < end; x++) {
		unsigned long long y = ceil(sqrtl(r*r - x*x));
		pixels += y;
	}
    pthread_mutex_lock(&mutex);
    total += pixels;
    pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}

int main(int argc, char** argv) {
	if (argc != 3) {
		fprintf(stderr, "must provide exactly 2 arguments!\n");
		return 1;
	}
	r = atoll(argv[1]);
	k = atoll(argv[2]);
	pthread_mutex_init(&mutex, NULL);

	cpu_set_t cpuset;
	sched_getaffinity(0, sizeof(cpuset), &cpuset);
	unsigned long long ncpus = CPU_COUNT(&cpuset);

	num_threads = ncpus;
	pthread_t threads[num_threads+5];
    part = r / num_threads;
    rmd = r % num_threads;

	int rc;
	void* tmp_rtv;
	unsigned long long* rtv;
	unsigned long long temp[num_threads+5];

	for (unsigned long long i = 0; i < num_threads; i++) {
		temp[i] = i;
		rc = pthread_create(&threads[i], NULL, calculate, (void*)&temp[i]);
		if (rc) {
			printf("[ERROR]: failed to create thread\n");
			exit(-1);
		}
	}
	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	printf("%llu\n", (4*(total%k)) % k);

	pthread_exit(NULL);
	return 0;
}
