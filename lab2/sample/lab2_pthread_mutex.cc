#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>

unsigned long long total = 0;
pthread_mutex_t mutex;

void* calculate(void* obj) {
	unsigned long long* arr = (unsigned long long*) obj; // arr[id, r, k, num_thread]
	// printf("id = %lld, r = %lld, k = %lld, num_thread = %lld\n", arr[0], arr[1], arr[2], arr[3]);
	unsigned long long pixels = 0;
	unsigned long long part = arr[1] / arr[3];
	unsigned long long rmd = arr[1] % arr[3]; // remainder
	unsigned long long start = (arr[0] < rmd) ? arr[0]*(part+1) : arr[0]*part+rmd;
	unsigned long long end = (arr[0] < rmd) ? start + part + 1 : start + part;
	// printf("id = %lld, start = %lld, end = %lld\n", arr[0], start, end);
	for (unsigned long long x = start; x < end; x++) {
		unsigned long long y = ceil(sqrtl(arr[1]*arr[1] - x*x));
		pixels += y;
	}
    pthread_mutex_lock(&mutex);
    total += pixels;
    pthread_mutex_unlock(&mutex);
	// printf("id = %lld, pixels = %lld\n", arr[0], pixels);
	pthread_exit(NULL);
}

int main(int argc, char** argv) {
	if (argc != 3) {
		fprintf(stderr, "must provide exactly 2 arguments!\n");
		return 1;
	}
	unsigned long long r = atoll(argv[1]);
	unsigned long long k = atoll(argv[2]);
	unsigned long long pixels = 0;
	pthread_mutex_init(&mutex, NULL);

	cpu_set_t cpuset;
	sched_getaffinity(0, sizeof(cpuset), &cpuset);
	unsigned long long ncpus = CPU_COUNT(&cpuset);

	unsigned long long num_threads = ncpus;
	pthread_t threads[num_threads+5];
	int rc;
	void* tmp_rtv;
	unsigned long long* rtv;
	unsigned long long temp[num_threads+5][5];

	for (unsigned long long i = 0; i < num_threads; i++) {
		temp[i][0] = i;
		temp[i][1] = r;
		temp[i][2] = k;
		temp[i][3] = num_threads;
		rc = pthread_create(&threads[i], NULL, calculate, (void*)temp[i]);
		if (rc) {
			printf("[ERROR]: failed to create thread\n");
			exit(-1);
		}
	}
	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], &tmp_rtv);
		// rtv[i] = *tmp_rtv; 
		// rtv = (unsigned long long*) tmp_rtv;
		// printf("id = %d, rtv = %lld\n", i, *rtv);
		// pixels += *rtv;
		// pixels %= k;
	}
	printf("%llu\n", (4*total) % k);

	pthread_exit(NULL);
	return 0;
}
