#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <mpi.h>

class Scheduler {
    public:
        Scheduler(int worker, int reducer);
        ~Scheduler();
        /* Map */
        void ReadIloc(std::string filePath);
        int SendMapTask(int targetNode); // scheduling algorithm with locality
        bool CheckMapFinished(); /* check if all the map task are finished. if so, send request to start reduce task. */
        void UpdateFinishedMapTask();

        /* Shuffle */
        int Shuffle();

        /* Reduce */
        void StartReduceTask();
        int SendReduceTask(int targetNode);
        bool CheckReduceFinished();
        void UpdateFinishedReduceTask();
        
        /* for debug */
        void PrintMapTask();
    private:
        /* Map */
        std::vector<std::pair<int, int> > mapTask; // <chunkID, locality>
        int workerNum;
        int finishedMapTask;
        int mapTaskNum;
        
        /* Reduce */
        std::vector<int> reduceTask;
        int finishedReduceTask;
        int reduceTaskNum;

        /* Both */
        int chunkNum; // chunk number
        int reducerNum;
        int finishedWorkers;
};
// srun -N<NODES> -c<CPUS> ./mapreduce JOB_NAME NUM_REDUCER DELAY INPUT_FILENAME CHUNK_SIZE LOCALITY_CONFIG_FILENAME OUTPUT_DIR

#endif