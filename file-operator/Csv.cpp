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

bool Csv::Read(string fileName, vector<vector<string>> &words, bool hasHead) {
    std::ifstream ifs(fileName, std::ios::in);
    std::string line;

    if (!ifs.is_open()) {
        std::cerr << "[Read] opening file fail. file path: " << fileName << std::endl;
        return false;
    }

    std::istringstream sin;         //�������ַ���line���뵽�ַ���istringstream��
    std::string word;

    // get head
    if(hasHead)
        std::getline(ifs, line);

    while (std::getline(ifs, line)) {
        if (line.empty())
            break;

        sin.clear();
        sin.str(line);
        for (int c = 0; c < words.size(); ++c) {
            std::getline(sin, word, ',');//���ַ�����sin�е��ַ�����field�ַ����У��Զ���Ϊ�ָ���
            words[c].push_back(word);
        }
    }

    ifs.close();
    return true;
}
