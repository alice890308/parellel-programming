#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

int main(int argc, char** argv) {
	if (argc != 3) {
		fprintf(stderr, "must provide exactly 2 arguments!\n");
		return 1;
	}
	unsigned long long r = atoll(argv[1]);
	unsigned long long k = atoll(argv[2]);
	unsigned long long total = 0;
	omp_lock_t lock;
	omp_init_lock(&lock);

	#pragma omp parallel
	{
		unsigned long long num_threads= omp_get_num_threads();
        unsigned long long id = omp_get_thread_num();
		unsigned long long part = r/num_threads;
		unsigned long long rmd = r%num_threads;
		unsigned long long start = (id < rmd) ? id*(part+1) : id*part+rmd;
		unsigned long long end = (id < rmd) ? start + part + 1 : start + part;
		unsigned long long pixels = 0;
		for (unsigned long long x = start; x < end; x++) {
			unsigned long long y = ceil(sqrtl(r*r - x*x));
			pixels += y;
		}
		omp_set_lock(&lock);
		total += pixels;
		omp_unset_lock(&lock);
	}

	printf("%llu\n", (4 * (total%k)) % k);
	return 0;
}
