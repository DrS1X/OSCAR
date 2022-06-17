#pragma once
#include <atlstr.h>
#include <QString>
#include <iostream>
#include <stdio.h>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include <QMap>
#include <QStandardItemModel>
#include <vector>  
#include <utility> 

using namespace std;
class CClsGeneralOperator
{
public:
	static void checkFilePath(string filePath);
	static int getDayOfYear(string fileName);
	static string generateFileName(string originFileName, string outputPath, string pre, string type, string date);
	static string generateFileName(string originFileName, string outputPath, string pre, string type);
	static string generateFileName(string originFilePath, string outputPath, string suffix);
	static void getFileList(string& path, vector<string>& files);
	static void getFileList(string& path, vector<string>& files, string& fileType);
	static string getDate(string fileName);

	static CString QStrToCStr(QString);//ʵ��QString��CString��ת��
	static QString CStrToQStr(CString);//ʵ��CString��QString��ת��
	static void InitializeFileTable(QTableWidget *tableFiles,QStringList strFileList);//��ʼ���ļ��б�
	static void RemoveSelectedFiles(QTableWidget *tableFiles, QStringList &strFileList,QLabel *countLalbe=NULL);//ɾ���ļ���Ĭ�ϲ���Ϊ�գ�ͳ�Ʊ�ǩ�ؼ���
	static CString GetFileName(CString filename);
	static CString GetYMDFromYD(int Year, int Day);
	static CString GetYearMonthFromDays(int Days, long refYear);
	static CString GetYMDFromDaysandReftime(int Days, QString reftimr);
	static CString GetYMDFromDays(double Days, long refYear);
	static int GetMonthFromDays(int Days);
	static int GetYearFromDays(int Days);
	static int LeapYear(int Year);

	static double* GetMin_Max(double *value, long * pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);   //������ͬʱ���������Сֵ ����0Ϊ��Сֵ��1Ϊ���ֵ Ĭ��ȱʡֵ-9999
	static double* GetMin_Max(double *value, double * pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//����
	static double GetMeanValue(long * pBuffer,long m_Rows,long m_Cols, long DefaultValue = -9999);	 //Ĭ��ȱʡֵ-9999
	static double GetMeanValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//����
	static double GetStdValue(long * pBuffer, long m_Rows, long m_Cols, long DefaultValue = -9999);  //Ĭ��ȱʡֵ-9999
	static double GetStdValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue = -9999);//����

	static void split(std::string& s, std::string& delim, std::vector<std::string>* ret);	
	static void addArray(long *arr1, long *arr2,int mRow,int mCol);//arr1+=arr2;
	static bool WriteNakeData(CString mFileName, double *pBuffer, long mRows, long mCols);
	static bool WriteNakeData(CString mFileName, long *pBuffer, long mRows, long mCols);
	static double STInterpolate(long *pTBuffer, double mScale, long mRows, long mCols, long m, long n, double mMissingValue, long *pBuffer1, long *pBuffer2, long size);
	//long GetDsNum(CString Filename);
	static bool DataSpatialConvertByMean(long *pSrcBuffer,/*ԭ���ݼ�*/long *pTarBuffer,/*Ŀ�����ݼ�*/long mSrcRows, long mSrcCols, long mTarRows, long mTarCols, double reSize/*ת���ߴ�*/);
	static double CalMeanValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);
	static double CalMaxValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);
	static double CalMinValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);
	static double CalStdValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale);
	static bool WriteSuferDatFile(CString mFileName, double *pBuffer, double mStartLog, double mStartLat, double LogSize, double LatSize, long mRows, long mCols);
	static int StringToInt(CString tempStr);

	static void SpatialResampleBasedOnUnevenLat(long *pOriBuffer,long *pTarBuffer,double *pOriLatBuffer,long mOriRows,long mOriCols,long mTarRows,long mTarCols,double startlog,double endlog,double startlat,double endlat);
	//�ж��Ƿ�Ϊ���֣��������ţ�
	static bool isNumber(CString str);
};


