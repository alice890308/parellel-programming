#ifndef WORKER_H
#define WORKER_H

#include <mpi.h>
#include <queue>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include "mapper.h"
#include "reducer.h"



class Worker {
    public:
        Worker(int r, std::string path, int chSize, int reducer, std::string opName, int delay, std::string jobName, int nodeSize);
        ~Worker();

        /* Map */
        void MapTask(int rank);
        void NewMapTask(int chunkID, int locality);
        void* MapThread(void);
        static void* MapThreadHelper(void* obj) { return ((Worker*)obj)->MapThread(); }
        int GetAvailableMapNum(); // for map
        void FinishTask();
        
        /* Reduce */
        void ReduceTask();
        void NewReduceTask(int reducerID, int chunks);
        void* ReduceThread(void);
        static void* ReduceThreadHelper(void* obj) { return ((Worker*)obj)->ReduceThread(); }
        int GetAvailableReduceNum();
        void FinishReduceTask();

        /* Both */
        void RequestNewMapTask();
        void RequestNewReduceTask();
    private:
        /* Map */
        std::vector<pthread_t> mapThreadPool;
        pthread_mutex_t mapTaskLock;
        pthread_mutex_t availMapThreadLock;
        std::queue<std::pair<int, int> > mapTask; // <chunk id, location>
        int numMapThreads;
        int rank;
        int delay;
        std::string inputPath;
        int chunkSize;
        int numReducer; // total reducer task
        int availableMapThread; // for map

        /* Reduce */
        std::vector<pthread_t> reduceThreadPool;
        pthread_mutex_t reduceTaskLock;
        pthread_mutex_t availReduceThreadLock;
        std::queue<std::pair<int, int> > reduceTask; // <reduce id, total map(chunk) num>
        int numReduceThreads;
        int availableReduceThread; // for reduce
        std::string outputName;
        std::string jobName;

        /* Both */
        int nodeNum;
        pthread_mutex_t mpiLock;
};

#endif