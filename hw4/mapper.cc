#include "mapper.h"

Mapper::Mapper()
{

}

Mapper::Mapper(int chunk, int reducer)
{
    startChunk = chunk;
    numReducer = reducer;
}

Mapper::~Mapper()
{

}

void Mapper::InputSplit(std::string inputPath, int startChunk, int chunkSize)
{
    std::ifstream input_file(inputPath);
    int cur = 1;  // array index counts from 0, but we want line index from 1.
    int start = startChunk * chunkSize;
    std::string line;

    for (; cur < start+1 && getline(input_file, line); cur++); // go through to the start line.
    for (int i = 0; i < chunkSize && getline(input_file, line); i++, cur++) {
        RawRecords.push_back(std::make_pair(line, cur));
    }
    input_file.close();
}

void Mapper::Map()
{
    std::vector<std::string> words;
    for (auto &line : RawRecords) {
        std::string word;
        

        int len = line.first.size();
        int pos = 0;
        while((pos = line.first.find(" ")) != std::string::npos)
        {
            word = line.first.substr(0, pos); // 以空格作為斷點，把空格前的一個完整的字抓出來
            if (WordCount.count(word) == 0)
            {
                WordCount[word] = 1;
                words.push_back(word);
            }
            else
            {
                WordCount[word]++;
            }
            line.first.erase(0, pos+1);
        }
        if (!line.first.empty()) {
            if (WordCount.count(line.first) == 0) {
                WordCount[line.first] = 1;
                words.push_back(line.first);
            }
            else {
                WordCount[line.first]++;
            }
        }
    }
}

void Mapper::Partition()
{
    std::vector<std::vector<std::pair<std::string, int> > > result(numReducer);

    for (auto kv : WordCount) {
        int target = kv.first.size() % numReducer;
        result[target].push_back(kv);
    }
    boost::filesystem::create_directories("./interFile");
    for (int i = 1; i <= numReducer; i++) {
        std::ofstream out("./interFile/chunk" + std::to_string(startChunk) + "_reducer" + std::to_string(i) + ".txt");
        for (auto pair : result[i-1]) {
            out << pair.first << " " << pair.second << "\n";
        }
    } 
}

void Mapper::PrintRawRecords()
{
    std::cout << "------RawRecords------\n";
    std::cout << "Start chunk: " << startChunk << '\n';
    for (auto i : RawRecords) {
        std::cout << i.first << ' ' << i.second << '\n';
    }
}

void Mapper::PrintWordCount()
{
    std::cout << "-----MappedRecords---------\n";
    for (auto kv : WordCount) {
        std::cout << kv.first << ' ' << kv.second << '\n';
    }
}