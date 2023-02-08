//
// Created by 15291 on 2023/2/1.
//

#include "Csv.h"

Csv::Csv(string fileName, string _head) : head(_head) {
    ofs.open(fileName, std::ios::out | std::ios::trunc);
    if (head != "")
        ofs << head << endl;
}

Csv::~Csv() {
    ofs.close();
}

bool Csv::Read(string fileName, vector<vector<string>> &words, int nRow, int nCol) {
    std::ifstream ifs(fileName, std::ios::in);
    std::string line;

    if (!ifs.is_open()) {
        std::cerr << "[Read] opening file fail. file path: " << fileName << std::endl;
        return false;
    }

    std::istringstream sin;         //将整行字符串line读入到字符串istringstream中
    std::string word;

    // get head
    std::getline(ifs, line);

    for (int r = 0; r < nRow; ++r) {
        std::getline(ifs, line);
        if (line.empty())
            break;

        sin.clear();
        sin.str(line);
        for (int c = 0; c < nCol; ++c) {
            std::getline(sin, word, ',');//将字符串流sin中的字符读到field字符串中，以逗号为分隔符
            words[r][c] = word;
        }
    }

    ifs.close();
    return true;
}
