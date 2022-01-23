#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
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

	MPI_Init(&argc, &argv);
    int rank, size;
	unsigned long long ans = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	#pragma omp parallel
	{
		unsigned long long num_threads= omp_get_num_threads();
        unsigned long long ompID = omp_get_thread_num();
		unsigned long long total_threads = num_threads * size;
		unsigned long long id = rank * num_threads + ompID;
		unsigned long long part = r/total_threads;
		unsigned long long rmd = r%total_threads;
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
	
	MPI_Reduce(&total, &ans, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	if (rank == 0) {
        printf("%llu\n", (4 * (ans%k)) % k);
    }
    MPI_Finalize();
	return 0;
}
