#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define PNG_NO_SETJMP
#include <sched.h>
#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>

void write_png(const char* filename, int iters, int width, int height, const int* buffer) {
    FILE* fp = fopen(filename, "wb");
    assert(fp);
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png_ptr);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr);
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_filter(png_ptr, 0, PNG_NO_FILTERS);
    png_write_info(png_ptr, info_ptr);
    png_set_compression_level(png_ptr, 1);
    size_t row_size = 3 * width * sizeof(png_byte);
    png_bytep row = (png_bytep)malloc(row_size);
    for (int y = 0; y < height; ++y) {
        memset(row, 0, row_size);
        for (int x = 0; x < width; ++x) {
            int p = buffer[(height - 1 - y) * width + x];
            png_bytep color = row + x * 3;
            if (p != iters) {
                if (p & 16) {
                    color[0] = 240;
                    color[1] = color[2] = p % 16 * 16;
                } else {
                    color[0] = p % 16 * 16;
                }
            }
        }
        png_write_row(png_ptr, row);
    }
    free(row);
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

int main(int argc, char** argv) {
    /* MPI init */
    MPI_Init(&argc, &argv);
    int rank, size;
    int cur_height = 0;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* detect how many CPUs are available */
    cpu_set_t cpu_set;
    sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
    printf("%d cpus available\n", CPU_COUNT(&cpu_set));

    /* argument parsing */
    assert(argc == 9);
    const char* filename = argv[1];
    int iters = strtol(argv[2], 0, 10);
    double left = strtod(argv[3], 0);
    double right = strtod(argv[4], 0);
    double lower = strtod(argv[5], 0);
    double upper = strtod(argv[6], 0);
    int width = strtol(argv[7], 0, 10);
    int height = strtol(argv[8], 0, 10);

    /* allocate memory for image */
    int* image = (int*)malloc(width * height * sizeof(int));
    int* buffer = (int*)malloc(width * height * sizeof(int));
    assert(image);

    /* lock */
    omp_lock_t lock;
	omp_init_lock(&lock);

    #pragma omp parallel
	{
        int j;

        /* mandelbrot set */
        while(1) {
            omp_set_lock(&lock);
            j = cur_height;
            cur_height++;
            omp_unset_lock(&lock);
            j = j*size+rank; // ??????process????????????row???1, 2, 3, 4, 1, 2, 3, 4...???????????????
            if (j >= height)
                break;

            double y0 = j * ((upper - lower) / height) + lower;
            for (int i = 0; i < width; ++i) {
                double x0 = i * ((right - left) / width) + left;

                int repeats = 0;
                double x = 0;
                double y = 0;
                double length_squared = 0;
                while (repeats < iters && length_squared < 4) {
                    double temp = x * x - y * y + x0;
                    y = 2 * x * y + y0;
                    x = temp;
                    length_squared = x * x + y * y;
                    ++repeats;
                }
                image[j * width + i] = repeats;
            }
        }
    }

    if (rank != 0) {
        // char text[20];
        // sprintf(text, "rank%d.png", rank);
        // write_png(text, iters, width, height, image);
        MPI_Send(image, width * height, MPI_INT, 0, 1, MPI_COMM_WORLD);
        free(image);
    }

    /* draw and cleanup */
    //printf("size = %d\n", size);
    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            MPI_Recv(buffer, width * height, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("receive!\n");
            //printf("k = %d\n", size+i);
            for (int j = 0; j*size+i < height; j++) {
                int k = j*size+i;
                for (int l = 0; l < width; l++) {
                    image[k*width+l] = buffer[k*width+l];
                }
            }
        }
        write_png(filename, iters, width, height, image);
        free(image); 
    }
    
    MPI_Finalize();
    return 0;
}
