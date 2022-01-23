#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define PNG_NO_SETJMP
#include <mpi.h>
#include <sched.h>
#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <emmintrin.h>
#include <iostream>

pthread_mutex_t height_m;
pthread_mutex_t averagetime_m, highesttime_m, lowesttime_m;
int* image;
int width, height, iters;
double left, right, lower, upper;
int cur_height = 0;
double checkpoint = 0.0;
double iotime = 0.0;
double averagecpu = 0.0, highestcpu = 0.0, lowestcpu = 1000000;

void* calculate(void*obj);

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
    /* detect how many CPUs are available */
    MPI_Init(&argc, &argv);
    cpu_set_t cpu_set;
    sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
    //printf("%d cpus available\n", CPU_COUNT(&cpu_set));
    int num_threads = CPU_COUNT(&cpu_set);

    /* argument parsing */
    assert(argc == 9);
    const char* filename = argv[1];
    iters = strtol(argv[2], 0, 10);
    left = strtod(argv[3], 0);
    right = strtod(argv[4], 0);
    lower = strtod(argv[5], 0);
    upper = strtod(argv[6], 0);
    width = strtol(argv[7], 0, 10);
    height = strtol(argv[8], 0, 10);

    /* allocate memory for image */
    image = (int*)malloc(width * height * sizeof(int));
    assert(image);

    /* init mutex lock */
    pthread_mutex_init(&height_m, NULL);
    pthread_mutex_init(&averagetime_m, NULL);
    pthread_mutex_init(&highesttime_m, NULL);
    pthread_mutex_init(&lowesttime_m, NULL);

    /* create thread */
    pthread_t threads[num_threads+5];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, calculate, NULL);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* draw and cleanup */
    double starttime = MPI_Wtime();
    write_png(filename, iters, width, height, image);
    averagecpu /= num_threads;
    std::cout << "average time: " << averagecpu << '\n';
    std::cout << "highest time: " << highestcpu << '\n';
    std::cout << "lowest time: " << lowestcpu << '\n';
    
    free(image);
}

void* calculate(void*obj) {
    double starttime = MPI_Wtime();
    double endtime = 0.0;
    int i, j, repeats;
    double x, y, temp, length_squared, x_sqr, y_sqr, y0, x0;
    bool flag0, flag1;
    double cnst = (right - left) / width;
    __m128d x_v, y_v, x0_v, y0_v, x_sqr_v, y_sqr_v, length_sqr_v, repeats_v, i_v;

    /* mandelbrot set */
    while(1) {
        flag0 = true;
        flag1 = true;
        pthread_mutex_lock(&height_m);
        j = cur_height;
        cur_height++;
        pthread_mutex_unlock(&height_m);

        if (j >= height)
            break;

        y0 = j * ((upper - lower) / height) + lower;
        y0_v[0] = y0;
        y0_v[1] = y0;
        for (i = 0; i < width;) {
            //printf("in for\n");
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
    endtime = MPI_Wtime()-starttime;
    pthread_mutex_lock(&averagetime_m);
    averagecpu += endtime;
    pthread_mutex_unlock(&averagetime_m);

    pthread_mutex_lock(&highesttime_m);
    highestcpu = (endtime > highestcpu) ? endtime : highestcpu;
    pthread_mutex_unlock(&highesttime_m);

    pthread_mutex_lock(&lowesttime_m);
    lowestcpu = (endtime < lowestcpu) ? endtime : lowestcpu;
    pthread_mutex_unlock(&lowesttime_m);
    pthread_exit(NULL);
}
