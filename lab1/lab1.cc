#include <mpi.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

int
main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
		fprintf(stderr, "must provide exactly 2 arguments!\n");
		return 1;
	}
	unsigned long long r = atoll(argv[1]);
	unsigned long long k = atoll(argv[2]);
	unsigned long long pixels = 0;
    unsigned long long part = r / size;
    unsigned long long start = rank * part;
    unsigned long long end = (rank+1) * part;
    unsigned long long ans = 0;
    if (rank == size-1) {
        for (unsigned long long x = start; x < r; x++) {
            unsigned long long y = ceil(sqrtl(r*r - x*x));
            pixels += y;
            pixels %= k;
        }
    }
    else {
        for (unsigned long long x = start; x < end; x++) {
            unsigned long long y = ceil(sqrtl(r*r - x*x));
            pixels += y;
            pixels %= k;
        }
    }
	MPI_Reduce(&pixels, &ans, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	if (rank == 0) {
        printf("%llu\n", (4 * ans) % k);
    }
    MPI_Finalize();
    return 0;
}
