//#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sched.h>


const int INF = ((1 << 30) - 1);
const int V = 50010;
void input(char* inFileName);
void output(char* outFileName);

int ceil(int a, int b);

int n, m; // n: # of vertices, m: # of edges
int start, end;
static int Dist[V][V];

int main(int argc, char* argv[]) {
    input(argv[1]);
    cpu_set_t cpu_set;
    sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
    int size = CPU_COUNT(&cpu_set);
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int part = n / size;
        int remd = n % size;
        int holdSize = (tid < remd) ? part+1 : part;
        
        int start = (tid < remd) ? tid*holdSize : tid*part+remd;
        int end = start+holdSize;
        for(int k = 0; k < n; k++)
        {
            for(int i = start; i < end; i++)
            {
                for(int j = 0; j < n; j++)
                {
                    if (Dist[i][k] == INF || Dist[k][j] == INF)
                        continue;
                    if (Dist[i][k] + Dist[k][j] < Dist[i][j])
                    {
                        Dist[i][j] = Dist[i][k] + Dist[k][j];
                    }
                }
            }
            #pragma omp barrier
        }
    }
    
    output(argv[2]);
    return 0;
}

void input(char* infile) {
    FILE* file = fopen(infile, "rb");
    fread(&n, sizeof(int), 1, file);
    fread(&m, sizeof(int), 1, file);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                Dist[i][j] = 0;
            } else {
                Dist[i][j] = INF;
            }
        }
    }

    int pair[3];
    for (int i = 0; i < m; ++i) {
        fread(pair, sizeof(int), 3, file);
        Dist[pair[0]][pair[1]] = pair[2];
    }
    fclose(file);
}

void output(char* outFileName) {
    FILE* outfile = fopen(outFileName, "w");
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (Dist[i][j] >= INF) Dist[i][j] = INF;
        }
        fwrite(Dist[i], sizeof(int), n, outfile);
    }
    fclose(outfile);
}

int ceil(int a, int b) { return (a + b - 1) / b; }