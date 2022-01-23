/*
process load balance: data數量 / process數量後，剩下的餘數也要平均分配

1. 每個process先在本地端sort好
2. odd phase: => 使用MPI_Sendrecv()
	2.1 odd process send data to even process
	2.2 even process sort data
	2.3 even process send result to odd process
   even phase: vise versa
3. using boost sort
總時間：166.81
*/
#include <mpi.h>
#include <iostream>
#include<boost/sort/spreadsort/float_sort.hpp>
#define CAST_TYPE int
#define DATA_TYPE float

unsigned long long part;
int rank, remd, holdSize, size, recvNum;
float *data;
float *temp1;
float *combine;
float *t;

using namespace boost::sort::spreadsort;

struct rightshift{
inline CAST_TYPE operator()(const DATA_TYPE &x, const unsigned offset) const {
    return float_mem_cast<DATA_TYPE, CAST_TYPE>(x) >> offset;
  }
};

bool changeSort(bool isLeft);

int main(int argc, char**argv) {
    MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int dataSize = atoll(argv[1]);
	part = dataSize / size;
    remd = dataSize % size;
    holdSize = (rank < remd) ? part+1 : part;
    int start = (rank < remd) ? rank*holdSize : rank*part+remd;
	data = new float[holdSize+5];
	temp1 = new float[holdSize+5];
	combine = new float[holdSize+5];
	bool oddExchange = false;

	MPI_File f;
	MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
	MPI_File_read_at(f, sizeof(float) * start, data, holdSize, MPI_FLOAT, MPI_STATUS_IGNORE);
	MPI_File_close(&f);
	float_sort(data, data+holdSize, rightshift()); // sort local data, optimizes

    for (int i = 0; i < size+1; i++) {
		if (oddExchange) {
			if (rank%2 == 1) {
				if (rank != size-1)
                    changeSort(true);
			}
			else if (rank%2 == 0) {
				if (rank != 0)
                    changeSort(false);
			}
		}
		else {
			if (rank%2 == 0) {
				if (rank != size-1)
                    changeSort(true);
			}
			else if (rank%2 == 1) {
                changeSort(false);
			}
		}
		oddExchange = !oddExchange;
	}
	MPI_File_open(MPI_COMM_WORLD, argv[3], MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &f);
	MPI_File_write_at(f, sizeof(float) * start, data, holdSize, MPI_FLOAT, MPI_STATUS_IGNORE);
	MPI_File_close(&f);
	MPI_Finalize();
	return 0;
}

bool changeSort(bool isLeft)
{
	if (isLeft) { // receive data from right and sort data
        recvNum = (rank+1 < remd) ? part+1 : part;
        if (holdSize <= 0 || recvNum <= 0) return true;
        MPI_Sendrecv(data, holdSize, MPI_FLOAT, rank+1, 1, temp1, recvNum, MPI_FLOAT, rank+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if (temp1[0] >= data[holdSize-1]) // already sorted
			return true;
        for (int i = 0, cur1 = 0, cur2 = 0; i < holdSize; i++) {
            combine[i] = (cur2 < recvNum && data[cur1] > temp1[cur2]) ? temp1[cur2++] : data[cur1++];
        }
        std::swap(data, combine);
	}
	else { // receive data from left and sort data
        recvNum = (rank-1 < remd) ? part+1 : part;
        if (holdSize <= 0) return true;
        MPI_Sendrecv(data, holdSize, MPI_FLOAT, rank-1, 2, temp1, recvNum, MPI_FLOAT, rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if (data[0] >= temp1[recvNum-1]) // already sorted
			return true;
		for (int i = holdSize-1, cur1 = holdSize-1, cur2 = recvNum-1; i >= 0; i--) {
            combine[i] = (cur2 >= 0 && temp1[cur2] > data[cur1]) ? temp1[cur2--] : data[cur1--];
        }
        std::swap(data, combine);
	}

	return false;
}