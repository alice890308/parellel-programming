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
#include <emmintrin.h>
#include <iostream>

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
    //printf("%d cpus available\n", CPU_COUNT(&cpu_set));

    /* calculating time */
    double cputime = 0.0;
    double iotime = 0.0;
    double commtime = 0.0;
    double tmptime = 0.0, tmp2time = 0.0;

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
    tmptime = MPI_Wtime();
    #pragma omp parallel
	{
        int i, j, repeats;
        double x, y, temp, length_squared, x_sqr, y_sqr, y0, x0;
        bool flag0, flag1;
        double cnst = (right - left) / width;
        __m128d x_v, y_v, x0_v, y0_v, x_sqr_v, y_sqr_v, length_sqr_v, repeats_v, i_v;
        
        /* mandelbrot set */
        while(1) {
            flag0 = true;
            flag1 = true;
            omp_set_lock(&lock);
            j = cur_height;
            cur_height++;
            omp_unset_lock(&lock);
            j = j*size+rank; // 每個process分配到的row是1, 2, 3, 4, 1, 2, 3, 4...這樣的順序
            if (j >= height)
                break;

            y0 = j * ((upper - lower) / height) + lower;
            y0_v[0] = y0;
            y0_v[1] = y0;
            for (int i = 0; i < width;) {
                if (flag0) {
                    flag0 = false;
                    x0_v[0] = i * cnst + left;
                    repeats_v[0] = 0;
                    x_v[0] = 0;
                    y_v[0] = 0;
                    length_sqr_v[0] = 0;
                    i_v[0] = i;
                    x_sqr_v[0] = 0;
                    y_sqr_v[0] = 0;
                    i++;
                }
                if (flag1) {
                    flag1 = false;
                    x0_v[1] = i * cnst + left;
                    repeats_v[1] = 0;
                    x_v[1] = 0;
                    y_v[1] = 0;
                    x_sqr_v[1] = 0;
                    y_sqr_v[1] = 0;
                    length_sqr_v[1] = 0;
                    i_v[1] = i;
                    i++;
                }

                while (repeats_v[0] < iters && repeats_v[1] < iters && length_sqr_v[0] < 4 && length_sqr_v[1] < 4) {
                    y_v = 2 * x_v * y_v + y0_v;
                    x_v = x_sqr_v - y_sqr_v + x0_v;
                    x_sqr_v = x_v*x_v;
                    y_sqr_v = y_v*y_v;
                    length_sqr_v = x_sqr_v + y_sqr_v;
                    ++repeats_v;
                }
                if (repeats_v[0] >= iters || length_sqr_v[0] >= 4) {
                    int tmp = i_v[0];
                    //printf("write img\n");
                    repeats = repeats_v[0];
                    image[j * width + tmp] = repeats;
                    flag0 = true;
                }
                if (repeats_v[1] >= iters || length_sqr_v[1] >= 4) {
                    int tmp = i_v[1];
                    //printf("write img\n");
                    repeats = repeats_v[1];
                    image[j * width + tmp] = repeats;
                    flag1 = true;
                }
            }
            if (!flag0) {
                repeats = repeats_v[0];
                x = x_v[0];
                y = y_v[0];
                x_sqr = x_sqr_v[0];
                y_sqr = y_sqr_v[0];
                y0 = y0_v[0];
                x0 = x0_v[0];
                i = i_v[0];
                length_squared = length_sqr_v[0];
                while (repeats < iters && length_squared < 4) {
                    y = 2 * x * y + y0;
                    x = x_sqr - y_sqr + x0;
                    x_sqr = x * x;
                    y_sqr = y * y;
                    length_squared = x_sqr + y_sqr;
                    repeats++;
                }
                image[j * width + i] = repeats;
            }
            if (!flag1) {
                repeats = repeats_v[1];
                x = x_v[1];
                y = y_v[1];
                x_sqr = x_sqr_v[1];
                y_sqr = y_sqr_v[1];
                y0 = y0_v[1];
                x0 = x0_v[1];
                i = i_v[1];
                length_squared = length_sqr_v[1];
                while (repeats < iters && length_squared < 4) {
                    y = 2 * x * y + y0;
                    x = x_sqr - y_sqr + x0;
                    x_sqr = x * x;
                    y_sqr = y * y;
                    length_squared = x_sqr + y_sqr;
                    repeats++;
                }
                image[j * width + i] = repeats;
            }
        }
    }
    cputime = MPI_Wtime() - tmptime;

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
        tmptime = MPI_Wtime();
        for (int i = 1; i < size; i++) {
            MPI_Recv(buffer, width * height, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j*size+i < height; j++) {
                int k = j*size+i;
                for (int l = 0; l < width; l++) {
                    image[k*width+l] = buffer[k*width+l];
                }
            }
        }
        commtime = MPI_Wtime() - tmptime;
        tmptime = MPI_Wtime();
        write_png(filename, iters, width, height, image);
        iotime = MPI_Wtime() - tmptime;
        free(image);
        std::cout << cputime << '\n';
        std::cout << commtime << '\n';
        std::cout << iotime << '\n';
    }
    
    MPI_Finalize();
    return 0;
}
