#ifndef MAPPER_H
#define MAPPER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <boost/filesystem.hpp>
#include <iostream>

class Mapper {
    public:
        Mapper();
        Mapper(int chunk, int reducer);
        ~Mapper();
        /* when using InputSplit, the startChunk should be deducted by 1 (since it counted from zero). */
        void InputSplit(std::string inputPath, int startChunk, int chunkSize);
        //void Map(int lineNum, std::string &lineText);
        void Map();
        void Partition();

        /* for debug */
        void PrintRawRecords();
        void PrintWordCount();
    private:
        int startChunk;
        int numReducer;
        std::vector<std::pair<std::string, int> > RawRecords;
        std::unordered_map<std::string, int> WordCount;
};

#endif