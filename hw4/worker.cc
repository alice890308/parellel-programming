#include "worker.h"
#include <random>

Worker::Worker(int r, std::string path, int chSize, int reducer, std::string opName, int d, std::string jbName, int nodeSize)
{
    /* Map */
    rank = r;
    delay = d;
    inputPath = path;
    chunkSize = chSize;
    numReducer = reducer;
    jobName = jbName;
    pthread_mutex_init(&mapTaskLock, NULL);
    pthread_mutex_init(&availMapThreadLock, NULL);
    cpu_set_t cpuset;
	sched_getaffinity(0, sizeof(cpuset), &cpuset);
	numMapThreads = (int)CPU_COUNT(&cpuset) - 1;
    availableMapThread = numMapThreads;
    mapThreadPool.resize(numMapThreads);

    /* Reduce */
    pthread_mutex_init(&reduceTaskLock, NULL);
    pthread_mutex_init(&availReduceThreadLock, NULL);
    numReduceThreads = 1;
    availableReduceThread = numReduceThreads; // in this project there's one thread for reduce task
    reduceThreadPool.resize(numReduceThreads);
    outputName = opName;

    /* Both */
    nodeNum = nodeSize;
    pthread_mutex_init(&mpiLock, NULL);
}

Worker::~Worker()
{

}

void Worker::MapTask(int rank)
{
    int rc;

    for (int i = 0; i < numMapThreads; i++) {
        rc = pthread_create(&mapThreadPool[i], NULL, Worker::MapThreadHelper, this);
        if (rc) {
            std::cout << "[ERROR] Failed on creating pthread.\n";
            exit(-1);
        }
    }
}

void Worker::NewMapTask(int chunkID, int locality)
{
    pthread_mutex_lock(&mapTaskLock);
    mapTask.push(std::make_pair(chunkID, locality));
    pthread_mutex_unlock(&mapTaskLock);
}

void* Worker::MapThread()
{
    std::pair<int, int> top; // <chunk id, location>
    bool busy = false; // represent whether this thread is working on a task
    double startTime;

    while(1) {
        /* fetch next task */
        pthread_mutex_lock(&mapTaskLock);
        if (!mapTask.empty()) {
            top = mapTask.front();
            if (top.first == -1) {
                pthread_mutex_unlock(&mapTaskLock);
                pthread_exit(NULL);
            }
            mapTask.pop();
            startTime = MPI_Wtime();
            busy = true;
        }
        pthread_mutex_unlock(&mapTaskLock);
        if (busy) {
            pthread_mutex_lock(&availMapThreadLock);
            availableMapThread--;
            pthread_mutex_unlock(&availMapThreadLock);

            Mapper mapper(top.first-1, numReducer);
            if (rank != top.second) {
                sleep(delay);
            }
            mapper.InputSplit(inputPath, top.first-1, chunkSize);
            mapper.Map();
            mapper.Partition();
            int buf[2] = {top.first, (int)(MPI_Wtime() - startTime)};
            pthread_mutex_lock(&mpiLock);
            MPI_Send(buf, 2, MPI_INT, nodeNum-1, 0, MPI_COMM_WORLD); // tag 0 for map task
            pthread_mutex_unlock(&mpiLock);
            
            pthread_mutex_lock(&availMapThreadLock);
            availableMapThread++;
            pthread_mutex_unlock(&availMapThreadLock);
            busy = false;
        }
    }
    pthread_exit(NULL);
}

int Worker::GetAvailableMapNum()
{
    return availableMapThread;
}

void Worker::FinishTask()
{
    for (int i = 0; i < numMapThreads; i++) {
        pthread_join(mapThreadPool[i], NULL);
    }
}

void Worker::ReduceTask()
{
    int rc;

    for (int i = 0; i < numReduceThreads; i++) {
        rc = pthread_create(&reduceThreadPool[i], NULL, Worker::ReduceThreadHelper, this);
        if (rc) {
            std::cout << "[ERROR] Failed on creating pthread.\n";
            exit(-1);
        }
    }
}

void Worker::NewReduceTask(int reducerID, int chunks)
{
    pthread_mutex_lock(&reduceTaskLock);
    reduceTask.push(std::make_pair(reducerID, chunks));
    pthread_mutex_unlock(&reduceTaskLock);
}

void* Worker::ReduceThread(void)
{
    std::pair<int, int> top; // <chunk id, location>
    bool busy = false; // represent whether this thread is working on a task
    double startTime;

    while(1) {
        /* fetch next task */
        pthread_mutex_lock(&reduceTaskLock);
        if (!reduceTask.empty()) {
            top = reduceTask.front();
            if (top.first == -1) {
                pthread_mutex_unlock(&reduceTaskLock);
                pthread_exit(NULL);
            }
            reduceTask.pop();
            startTime = MPI_Wtime();
            busy = true;
        }
        pthread_mutex_unlock(&reduceTaskLock);
        if (busy) {
            pthread_mutex_lock(&availReduceThreadLock);
            availableReduceThread--;
            pthread_mutex_unlock(&availReduceThreadLock);

            Reducer r(top.first, top.second);
            r.Sort();
            r.GroupKey();
            r.Reduce();
            r.Output(outputName, jobName);
            int buf[2] = {top.first, (int)(MPI_Wtime() - startTime)};
            pthread_mutex_lock(&mpiLock);
            MPI_Send(buf, 2, MPI_INT, nodeNum-1, 1, MPI_COMM_WORLD); // tag 1 for reduce task
            pthread_mutex_unlock(&mpiLock);

            pthread_mutex_lock(&availReduceThreadLock);
            availableReduceThread++;
            pthread_mutex_unlock(&availReduceThreadLock);
            busy = false;
        }
    }
    pthread_exit(NULL);
}

int Worker::GetAvailableReduceNum()
{
    return availableReduceThread;
}

void Worker::FinishReduceTask()
{
    for (int i = 0; i < numReduceThreads; i++) {
        pthread_join(reduceThreadPool[i], NULL);
    }
}

void Worker::RequestNewMapTask()
{
    int buf[2] = {-1, -1};

    pthread_mutex_lock(&mpiLock);
    MPI_Send(buf, 2, MPI_INT, nodeNum-1, 0, MPI_COMM_WORLD); // tag 0 for map task
    pthread_mutex_unlock(&mpiLock);
}

void Worker::RequestNewReduceTask()
{
    int buf[2] = {-1, -1};

    pthread_mutex_lock(&mpiLock);
    MPI_Send(buf, 2, MPI_INT,nodeNum-1, 1, MPI_COMM_WORLD); // tag 1 for reduce task
    pthread_mutex_unlock(&mpiLock);
}