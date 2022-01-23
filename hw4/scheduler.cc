#include "scheduler.h"
#include <fstream>

Scheduler::Scheduler(int worker, int reducer)
{
    workerNum = worker;
    reducerNum = reducer;
    finishedWorkers = 0;
}

Scheduler::~Scheduler()
{

}

void Scheduler::ReadIloc(std::string filePath)
{
    std::ifstream locality_file(filePath);
    std::string line;
    int cur = 0, chunk, loc;

    while(locality_file >> chunk >> loc) {
        mapTask.push_back(std::make_pair(chunk, loc % workerNum));
    }
    chunkNum = mapTask.size();
    finishedMapTask = 0;
    mapTaskNum = mapTask.size();
    locality_file.close();
    PrintMapTask();
}

/* return dispatched task number */
int Scheduler::SendMapTask(int targetNode)
{
    int buf[2] = {-1, -1};
    if (mapTask.empty()) { // no more task to do
        MPI_Send(buf, 2, MPI_INT, targetNode, 0, MPI_COMM_WORLD);
        finishedWorkers++;
        return -1;
    }
    else { // distribute task
        
        std::vector<std::pair<int, int> >::iterator it = mapTask.begin();
        for (; it != mapTask.end(); it++) {
            if (it->second == targetNode) {
                buf[0] = it->first;
                buf[1] = it->second;
                MPI_Send(buf, 2, MPI_INT, targetNode, 0, MPI_COMM_WORLD);
                it = mapTask.erase(it);
                return buf[0];
            }
        }
        if (it == mapTask.end()) {
            it = mapTask.begin();
            buf[0] = it->first;
            buf[1] = it->second;
            MPI_Send(buf, 2, MPI_INT, targetNode, 0, MPI_COMM_WORLD);
            it = mapTask.erase(it);
        }
        return buf[0];
    }
}

bool Scheduler::CheckMapFinished()
{
    return mapTask.empty() && (finishedMapTask >= mapTaskNum);
}

int Scheduler::Shuffle()
{
    int totalPairs = 0;
    std::string word;
    int count;

    for (int i = 1; i <= reducerNum; i++) {
        std::ofstream shuffled("./interFile/shuffled_reducer" + std::to_string(i) + ".txt");
        for (int j = 0; j < chunkNum; j++) {
            std::ifstream inter("./interFile/chunk" + std::to_string(j) + "_reducer" + std::to_string(i) + ".txt");
            while(inter >> word >> count) {
                totalPairs++;
                shuffled << word << " " << count << '\n';
            }
            inter.close();
        }
        shuffled.close();
    }
    return totalPairs;
}

void Scheduler::StartReduceTask()
{
    finishedReduceTask = 0;

    for (int i = 1; i <= reducerNum; i++) {
        reduceTask.push_back(i);
    }
    reduceTaskNum = reduceTask.size();
}

int Scheduler::SendReduceTask(int targetNode)
{
    int buf[2] = {-1, -1};
    if (reduceTask.empty()) { // no more task to do
        std::cout << "[SCHEDULER]: send reduce finish signal\n";
        MPI_Send(buf, 2, MPI_INT, targetNode, 1, MPI_COMM_WORLD);
        return -1;
    }
    else { // distribute task
        std::vector<int>::iterator it = reduceTask.begin();
        buf[0] = *it;
        buf[1] = chunkNum;
        MPI_Send(buf, 2, MPI_INT, targetNode, 1, MPI_COMM_WORLD);
        it = reduceTask.erase(it);
    }
    return buf[0];
}

void Scheduler::UpdateFinishedMapTask()
{
    finishedMapTask += 1;
}

void Scheduler::UpdateFinishedReduceTask()
{
    finishedReduceTask += 1;
}

bool Scheduler::CheckReduceFinished()
{
    return reduceTask.empty() && (finishedReduceTask >= reduceTaskNum);
}

void Scheduler::PrintMapTask()
{
    std::cout << "----------scheduler task--------------\n";
    for (auto i : mapTask) {
        std::cout << i.first << " " << i.second << '\n';
    }
}