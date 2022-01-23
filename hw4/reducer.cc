#include "reducer.h"

Reducer::Reducer()
{

}

Reducer::Reducer(int reducer, int chunks)
{
    reducerID = reducer;
    chunkNum = chunks;
}

Reducer::~Reducer()
{

}

bool cmp(std::pair<std::string, int> &a, std::pair<std::string, int> &b)
{
    return a.first < b.first;
}

void Reducer::Sort()
{
    std::string str;
    int count;
    std::ifstream inter_file("./interFile/shuffled_reducer" + std::to_string(reducerID) + ".txt");
    while(inter_file >> str >> count) {
        sortedKey.push_back(make_pair(str, count));
    }
    sort(sortedKey.begin(), sortedKey.end(), cmp);
    PrintSortedKey();
}

void Reducer::GroupKey()
{
    if (sortedKey.empty())
        return;
    groupedKey.push_back(make_pair(sortedKey[0].first, std::vector<int>(1, sortedKey[0].second)));
    for (int i = 1; i < sortedKey.size(); i++) {
        if (sortedKey[i].first != groupedKey.back().first) {
            groupedKey.push_back(make_pair(sortedKey[i].first, std::vector<int>(1, sortedKey[i].second)));
        }
        else {
            groupedKey.back().second.push_back(sortedKey[i].second);
        }
    }
    PrintGroupedKey();
}

void Reducer::Reduce()
{
    int count;

    for (auto p : groupedKey) {
        count = 0;
        for (auto i : p.second) {
            count += i;
        }
        reducedKey.push_back(make_pair(p.first, count));
    }
}

void Reducer::Output(std::string opName, std::string jobName)
{
    boost::filesystem::create_directories("./" + opName);
    std::ofstream out("./" + opName + "/" + jobName + "-" + std::to_string(reducerID) + ".out");
    for (auto pair : reducedKey) {
        out << pair.first << " " << pair.second << "\n";
    }
    out.close();
}

void Reducer::PrintSortedKey()
{
    std::cout << "--------sorted key---------\n";
    for (auto i : sortedKey) {
        std::cout << i.first << " " << i.second << '\n';
    }
}

void Reducer::PrintGroupedKey()
{
    std::cout << "--------grouped key---------\n";
    for (auto i : groupedKey) {
        std::cout << i.first << ": ";
        for (auto c : i.second) {
            std::cout << c << " ";
        }
        std::cout << '\n';
    }
}