#include "util.h"
#include <io.h>
#include <direct.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include "float.h"
#include "Cst.h"

using namespace std;

void util::checkFilePath(string filePath) {
	const char* folder = filePath.c_str();

	if (_access(folder, 0) == -1)	//如果文件夹不存在
		_mkdir(folder);
}


string util::generateFileName(string originFileName, string outputPath, string pre, string type) {
	string date = GetDate(originFileName);
	string folder = outputPath + "\\";
	/*
	string folder = outputPath + "\\" + date + "\\";

	if (_access(folder.c_str(), 0) == -1)	//如果文件夹不存在
		_mkdir(folder.c_str());
	*/
	string mOutFileName = folder + pre + date + type;
	return mOutFileName;
}

string util::generateFileName(string originFilePath, string outputPath, string suffix) {
	string fileName = originFilePath.substr(originFilePath.find_last_of("\\"),
		originFilePath.find_last_of(".") - originFilePath.find_last_of("\\"));
	string folder = outputPath + "\\";
	/*
	string folder = outputPath + "\\" + date + "\\";

	if (_access(folder.c_str(), 0) == -1)	//如果文件夹不存在
	_mkdir(folder.c_str());
	*/
	string outFileName = folder + fileName + suffix;
	return outFileName;
}

bool CheckFolderExist(string folder){
    error_code err;
    if (filesystem::exists(folder)){
        if(!filesystem::remove_all(folder,err)){
            std::cerr << "[CheckFolderExist] fail to remove the old folder: " << folder
            << ", err code: " << err.message() << endl;
            return false;
        }
    }

    if(!filesystem::create_directories(folder, err)) {
        std::cerr << "[CheckFolderExist] fail to create new folder " << folder
                << ", err code: " << err.message() << endl;
        return false;
    }
    return true;
}

void util::getFileList(string path, vector<string>& files, string fileType)
{
	//文件句柄
	intptr_t   hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (fileType == "" && strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
				files.push_back((p.assign(path).append("\\").append(fileinfo.name)));
			}
			else {
				//如果是目录,迭代之
				//如果不是,加入列表
				if ((fileinfo.attrib &  _A_SUBDIR))
				{
					if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
						getFileList(p.assign(path).append("\\").append(fileinfo.name), files, fileType);
					}
				}
				else
				{
					string fileName = fileinfo.name;
					string type = fileName.substr(fileName.find_last_of('.'));
					//cout << p.assign(path).append("\\").append(fileinfo.name) << endl;
					if (fileType == type)
						files.push_back((p.assign(path).append("\\").append(fileinfo.name)));
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}

	if (files.size() == 0)
		cout << "No file-operator is found" << endl;
}

string GetDate(string fileName) {
    static const regex  pattern("19|20[0-9]{2}[0-1][0-9]([0-3][0-9]|)");

    string date = "";
    int idx = fileName.find_last_of("\\");
    if(idx == -1)
        idx = fileName.find_last_of("/");
    if(idx != -1)
        fileName = fileName.substr(idx + 1);

    smatch result;
    string::const_iterator iter_begin = fileName.cbegin();
    string::const_iterator iter_end = fileName.cend();
    if (regex_search(iter_begin, iter_end,result, pattern)){
        date = fileName.substr(result[0].first - iter_begin,result[0].second - result[0].first);
    }else{
        cout << "[GetDate] failed to match date from file name: " <<fileName<< endl;
    }

    return date;
}

int util::LeapYear(int Year)
{
	if (Year % 4 != 0 || (Year % 100 == 0 && Year % 400 != 0))
		return 0;  //不是闰年
	else
		return 1;   //闰年
}

double* util::GetMin_Max(double *value, long * pBuffer, long m_Rows, long m_Cols, long DefaultValue)
{
	bool isGet = false;
	for (int i = 0; i < m_Rows; i++)
	{
		for (int j = 0; j < m_Cols; j++)
		{
			if (pBuffer[i*m_Cols + j] != DefaultValue)
			{
				if (!isGet)
				{
					isGet = true;
					value[0] = value[1] = pBuffer[i*m_Cols + j];
				}
				if (value[0] >= pBuffer[i*m_Cols + j])
					value[0] = pBuffer[i*m_Cols + j];
				if (value[1] <= pBuffer[i*m_Cols + j])
					value[1] = pBuffer[i*m_Cols + j];
			}
		}
	}
	return value;
}

double *util::GetMin_Max(double *value, double * pBuffer, long m_Rows, long m_Cols, double DefaultValue)
{
	bool isGet = false;
	for (int i = 0; i < m_Rows; i++)
	{
		for (int j = 0; j < m_Cols; j++)
		{
			if (pBuffer[i*m_Cols + j] != DefaultValue)
			{
				if (!isGet)
				{
					isGet = true;
					value[0] = value[1] = pBuffer[i*m_Cols + j];
				}
				if (value[0] > pBuffer[i*m_Cols + j])
					value[0] = pBuffer[i*m_Cols + j];
				if (value[1] < pBuffer[i*m_Cols + j])
					value[1] = pBuffer[i*m_Cols + j];
			}
		}
	}
	return value;
}

double util::GetMeanValue(long * pBuffer, long m_Rows, long m_Cols, long DefaultValue)
{
	long count = 0;
	double sum = 0;
	double MeanValue = 0;
	for (int i = 0; i < m_Rows*m_Cols; i++)
	{
		if (pBuffer[i] != DefaultValue)
		{
			++count;
			sum += pBuffer[i] * 1.0;
		}
	}
	if (count == 0)
	{
		return DefaultValue;
	}
	return sum / count;

}

double util::GetMeanValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue)
{
	long count = 0;
	double sum = 0;
	double MeanValue = 0;
	for (int i = 0; i < m_Rows*m_Cols; i++)
	{
		if (pBuffer[i] != DefaultValue)
		{
			++count;
			sum += pBuffer[i];
		}
	}
	if (count == 0)
	{
		return DefaultValue;
	}
	return sum / count;

}

double util::GetStdValue(long * pBuffer, long m_Rows, long m_Cols, long DefaultValue)
{
	long count = 0;
	double sum = 0;
	double MeanValue = GetMeanValue(pBuffer, m_Rows, m_Cols, DefaultValue);
	for (int i = 0; i < m_Rows*m_Cols; i++)
	{
		if (pBuffer[i] != DefaultValue)
		{
			++count;
			sum += (pBuffer[i]*1.0 - MeanValue)*(pBuffer[i]*1.0 - MeanValue);
		}
	}
	if (count == 0)
	{
		return DefaultValue;
	}
	return sqrt(sum / count);
}

double util::GetStdValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue)
{
	long count = 0;
	double sum = 0;
	double MeanValue = GetMeanValue(pBuffer, m_Rows, m_Cols, DefaultValue);
	for (int i = 0; i < m_Rows*m_Cols; i++)
	{
		double a = pBuffer[i];
		if (pBuffer[i] != DefaultValue)
		{
			++count;
			sum += (pBuffer[i] * 1.0 - MeanValue)*(pBuffer[i] * 1.0 - MeanValue);
		}
	}
	if (count == 0)
	{
		return DefaultValue;
	}
	return sqrt(sum / count);
}

void util::split(std::string & s, std::string & delim, std::vector<std::string>* ret)
{
	{
		size_t last = 0;
		size_t index = s.find_first_of(delim, last);
		while (index != string::npos)
		{
			ret->push_back(s.substr(last, index - last));
			last = index + 1;
			index = s.find_first_of(delim, last);
		}
		if (index - last > 0)
		{
			ret->push_back(s.substr(last, index - last));
		}
	}
}
