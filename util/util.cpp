#include "util.h"
#include <io.h>
#include <direct.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include "float.h"
#include "_const.h"

using namespace std;

int util::getDayOfYear(string fileName) {

}

void util::checkFilePath(string filePath) {
	const char* folder = filePath.c_str();

	if (_access(folder, 0) == -1)	//如果文件夹不存在
		_mkdir(folder);
}

string util::generateFileName(string originFileName, string outputPath, string pre, string type, string date) {
	string folder = outputPath + "\\";
	/*
	string folder = outputPath + "\\" + date + "\\";

	if (_access(folder.c_str(), 0) == -1)	//如果文件夹不存在
	_mkdir(folder.c_str());
	*/
	string mOutFileName = folder + pre + date + "." + type;
	return mOutFileName;
}

string util::generateFileName(string originFileName, string outputPath, string pre, string type) {
	string date = originFileName.substr(originFileName.find_last_of(".") - 8, 8);
	string folder = outputPath + "\\";
	/*
	string folder = outputPath + "\\" + date + "\\";

	if (_access(folder.c_str(), 0) == -1)	//如果文件夹不存在
		_mkdir(folder.c_str());
	*/
	string mOutFileName = folder + pre + date + "." + type;
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

void util::getFileList(string path, vector<string>& files) {
	string empty = "";
	util::getFileList(path, files, empty);
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

string util::getDate(string fileName) {
    static const regex  pattern("19|20[0-9]{2}[0-1][0-9][0-3][0-9]");

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
        cout << "[getDate] failed to match date from file name." <<fileName<< endl;
    }

    return date;
}

int util::GetMonthFromDays(int Days)
{
	int year[2] = { 365,366 };

	int month[2][12] = { 31,28,31,30,31,30,31,31,30,31,30,31,\
		31,29,31,30,31,30,31,31,30,31,30,31 };
	int i;
	for (i = 1800; Days >= year[LeapYear(i)]; i++)
	{
		Days -= year[LeapYear(i)];
	}


	int j;
	for (j = 0; Days >= month[LeapYear(i)][j]; j++)
	{
		Days -= month[LeapYear(i)][j];
	}
	return j + 1;
}

int util::GetYearFromDays(int Days)
{
	int year[2] = { 365,366 };
	int i;
	for (i = 1800; Days >= year[LeapYear(i)]; i++)
	{
		Days -= year[LeapYear(i)];
	}
	return i;
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

double util::STInterpolate(long *pTBuffer, double mScale, long mRows, long mCols, long m, long n, double mMissingValue, long *pBuffer1, long *pBuffer2, long size)
{
	double reValue;
	double dValue[3];
	long valNum = 0;
	double sumValue = 0;
	long tempSize = 3;

	long startRow, endRow, startCol, endCol;

	if (m == 0)
	{
		startRow = m;
		endRow = m + 1;
	}
	else if (m == mRows - 1)
	{
		startRow = m - 1;
		endRow = m;
	}
	else
	{
		startRow = m - 1;
		endRow = m + 1;
	}

	if (n == 0)
	{
		startCol = n;
		endCol = n + 1;
	}
	else if (n == mCols - 1)
	{
		startCol = n - 1;
		endCol = n;
	}
	else
	{
		startCol = n - 1;
		endCol = n + 1;
	}


	//计算空间均值
	for (int i = startRow;i<endRow;i++)
	{
		for (int j = startCol;j<endCol;j++)
		{
			long lValue = pTBuffer[i*mCols + j];

			if (lValue != (long)mMissingValue)
			{
				valNum += 1;
				sumValue += lValue;
			}
		}
	}

	if (valNum != 0)
		dValue[0] = sumValue / valNum * mScale;
	else
		dValue[0] = mMissingValue;

	//计算时刻1的空间均值；
	valNum = 0;
	sumValue = 0;
	for (int i = 0;i<size;i++)
	{
		long lValue = pBuffer1[i];
		if (lValue != (long)mMissingValue)
		{
			valNum += 1;
			sumValue += lValue;
		}
	}

	if (valNum != 0)
		dValue[1] = sumValue / valNum * mScale;
	else
		dValue[1] = mMissingValue;

	//计算时刻2的空间均值；
	valNum = 0;
	sumValue = 0;
	for (int i = 0;i<size;i++)
	{
		long lValue = pBuffer2[i];
		if (lValue != (long)mMissingValue)
		{
			valNum += 1;
			sumValue += lValue;
		}
	}

	if (valNum != 0)
		dValue[2] = sumValue / valNum * mScale;
	else
		dValue[2] = mMissingValue;

	//计算时空均值
	valNum = 0;
	sumValue = 0;
	for (int i = 0;i<3;i++)
	{
		double lValue = dValue[i];
		if (lValue != mMissingValue)
		{
			valNum += 1;
			sumValue += lValue;
		}
	}

	if (valNum != 0)
		reValue = sumValue / valNum;
	else
		reValue = mMissingValue;

	return reValue;
}

double util::CalMeanValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale)
{
	double tValue;
	double mSumValue;
	long valueNum;

	mSumValue = 0.0;
	valueNum = 0;
	for (long i = 0;i<mNum;i++)
	{
		if (pBuffer[i] != mFillValue)
		{
			mSumValue += pBuffer[i] * mScale;
			valueNum += 1;
		}
	}

	if (valueNum != 0)
		tValue = mSumValue / valueNum;
	else
		tValue = -9999.0;

	return tValue;
}

double util::CalMaxValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale)
{
	long tValueNum = 0;
	long tValue = -1000000;
	for (long i = 0;i<mNum;i++)
	{
		if (pBuffer[i] != mFillValue)
		{
			tValueNum += 1;

			if (tValue < pBuffer[i])
				tValue = pBuffer[i];
		}
	}

	if (tValueNum == 0)
		return -9999.0;

	return tValue*mScale;
}

double util::CalMinValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale)
{
	long tValueNum = 0;
	long tValue = 1000000;
	for (long i = 0;i<mNum;i++)
	{
		if (pBuffer[i] != mFillValue)
		{
			tValueNum += 1;
			if (tValue > pBuffer[i])
				tValue = pBuffer[i];
		}
	}

	if (tValueNum == 0)
		return -9999.0;

	return tValue *mScale;
}

//基于纬度不均匀的重采样
void util::SpatialResampleBasedOnUnevenLat(long *pOriBuffer, long *pTarBuffer, double *pOriLatBuffer,
                                           long mOriRows, long mOriCols, long mTarRows, long mTarCols,
                                           double startlog, double endlog, double startlat, double endlat)
{
	//double startlog;
	//double endlog;
	//double startlat;
	//double endlat;
	double *pOriLineLatBuffer = new double[mOriRows + 1];//用来存储栅格边缘纬度值
	//确保纬度没有颠倒
	if (pOriLatBuffer[0] < 0)
	{
		double *temp = new double[mOriRows];
		for (int i = 0; i < mOriRows; i++)
		{
			temp[i] = pOriLatBuffer[i];
		}
		for (int i = 0; i < mOriRows; i++)
		{
			pOriLatBuffer[i] = temp[mOriRows - i - 1];
		}
	}
	////开始计算
	for (int i = 0; i < mOriRows + 1; i++)
	{
		if (i == 0)
		{
			pOriLineLatBuffer[i] = endlat; 
			continue;
		}
		if (i == mOriRows)
		{
			pOriLineLatBuffer[i] = startlat; 
			break;
		}
		pOriLineLatBuffer[i] = pOriLineLatBuffer[i - 1] - (pOriLineLatBuffer[i - 1] - pOriLatBuffer[i - 1]) * 2;
	}
	double mTarResolution = (endlog - startlog) / mTarCols;
	double mOriResolution = (endlog - startlog) / mOriCols;
	if (mOriResolution >= mTarResolution)
	{
		for (int i = 0; i < mTarRows; i++)
		{
			for (int j = 0; j < mTarCols; j++)
			{
				//计算栅格中心经纬度
				double lon = j*mTarResolution + 0.5*mTarResolution + startlog;
				double lat = endlat - (i*mTarResolution + 0.5*mTarResolution);
				//计算经纬度在原文件中的行、列号,注意纬度间隔不均匀
				long col = (lon - startlog) / mOriResolution + 1;
				long row = 0;
				for (int k = 0; k < mOriRows; k++)
				{
					if (lat <= pOriLineLatBuffer[k])
						row = k + 1;
					else
						break;
				}
				if (row > mOriRows || col > mOriCols)
				{
					pTarBuffer[i*mTarCols + j] = -9999;
					continue;
				}
				pTarBuffer[i*mTarCols + j] = pOriBuffer[(row - 1)*mOriCols + (col - 1)];
			}
		}
	}
	else
	{
		for (int i = 0; i < mTarRows; i++)
		{
			for (int j = 0; j < mTarCols; j++)
			{
				//计算栅格中心经纬度
				double lon = j*mTarResolution + 0.5*mTarResolution + startlog;
				double lat = endlat - (i*mTarResolution + 0.5*mTarResolution);
				//计算经纬度在原文件中的行、列号,注意纬度间隔不均匀
				long col = (lon - startlog) / mOriResolution + 1;
				long row = 0;
				for (int k = 0; k < mOriRows; k++)
				{
					if (lat <= pOriLineLatBuffer[k])
						row = k + 1;
					else
						break;
				}
				int count = 0;
				int sum = 0;
				for (int x = row - 2; x < row + 1; x++)
				{
					for (int y = col - 2; y < col + 1; y++)
					{
						if (x == row - 1 && y == col - 1)
							continue;
						if (x > mOriRows || y > mOriCols)
							continue;
						if (pOriBuffer[x*mOriCols + y] != -9999)
						{
							sum += pOriBuffer[x*mOriCols + y]; 
							count++;
						}
					}
				}
				if (count != 0)
					pTarBuffer[i*mTarCols + j] = sum / count;
				else
					pTarBuffer[i*mTarCols + j] = -9999;
			}
		}
	}
}


