//
// Created by 15291 on 2023/2/1.
//

#ifndef CLUSTERING_CSV_H
#define CLUSTERING_CSV_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using std::string;
using std::vector;
using std::endl;

class Csv {
public:
    string head;
    std::ofstream ofs;
    Csv(string fileName, string _head = "");
    ~Csv();
    static bool Read(string fileName, vector<vector<string>> &words, bool hasHead = true);
};


#endif //CLUSTERING_CSV_H
