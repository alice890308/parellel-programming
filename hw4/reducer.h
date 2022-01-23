#ifndef REDUCER_H
#define REDUCER_H

#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <boost/filesystem.hpp>
#include <iostream>

class Reducer {
    public:
        Reducer();
        Reducer(int reducer, int chunks);
        ~Reducer();
        
        void Sort();
        void GroupKey();
        void Reduce();
        void Output(std::string opName, std::string jobName);
        /* for debug */
        void PrintSortedKey();
        void PrintGroupedKey();
    private:
        int reducerID; // which reduce task to do
        int chunkNum; // number of chunks(maps)
        std::vector<std::pair<std::string, int> > sortedKey;
        std::vector<std::pair<std::string, std::vector<int> > > groupedKey;
        std::vector<std::pair<std::string, int> > reducedKey;
};

#endif