#include "ClsGeneralOperator.h"
#include <QMessageBox>
#include <qdatetime.h>
#include <io.h>
#include <direct.h>

int CClsGeneralOperator::getDayOfYear(string fileName) {
	string dateStr = fileName.substr(fileName.find_last_of("\\") + 26, 8);
	int year = stoi(dateStr.substr(0, 4));
	int month = stoi(dateStr.substr(4, 2));
	int day = stoi(dateStr.substr(6, 2));

	if (month == 2 && day == 29) return -1;
	int DayNumber[12] = { 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
	if (month == 1)
		return day - 1;
	else
		return DayNumber[month - 2] + day - 1;
}

void CClsGeneralOperator::checkFilePath(string filePath) {
	const char* folder = filePath.c_str();

	if (_access(folder, 0) == -1)	//����ļ��в�����
		_mkdir(folder);
}

string CClsGeneralOperator::generateFileName(string originFileName, string outputPath, string pre, string type, string date) {
	string folder = outputPath + "\\";
	/*
	string folder = outputPath + "\\" + date + "\\";

	if (_access(folder.c_str(), 0) == -1)	//����ļ��в�����
	_mkdir(folder.c_str());
	*/
	string mOutFileName = folder + pre + date + "." + type;
	return mOutFileName;
}


string CClsGeneralOperator::generateFileName(string originFileName, string outputPath, string pre, string type) {
	string date = originFileName.substr(originFileName.find_last_of(".") - 8, 8);
	string folder = outputPath + "\\";
	/*
	string folder = outputPath + "\\" + date + "\\";

	if (_access(folder.c_str(), 0) == -1)	//����ļ��в�����
		_mkdir(folder.c_str());
	*/
	string mOutFileName = folder + pre + date + "." + type;
	return mOutFileName;
}

string CClsGeneralOperator::generateFileName(string originFilePath, string outputPath, string suffix) {
	string fileName = originFilePath.substr(originFilePath.find_last_of("\\"),
		originFilePath.find_last_of(".") - originFilePath.find_last_of("\\"));
	string folder = outputPath + "\\";
	/*
	string folder = outputPath + "\\" + date + "\\";

	if (_access(folder.c_str(), 0) == -1)	//����ļ��в�����
	_mkdir(folder.c_str());
	*/
	string outFileName = folder + fileName + suffix;
	return outFileName;
}

void CClsGeneralOperator::getFileList(string& path, vector<string>& files) {
	string empty = "";
	CClsGeneralOperator::getFileList(path, files, empty);
}

void CClsGeneralOperator::getFileList(string& path, vector<string>& files, string& fileType)
{
	//�ļ����
	intptr_t   hFile = 0;
	//�ļ���Ϣ
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
				//�����Ŀ¼,����֮
				//�������,�����б�
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
		cout << "No file is found" << endl;
}

string CClsGeneralOperator::getDate(string fileName) {
	return fileName.substr(fileName.find_last_of("\\") + 26, 8);
}

CString CClsGeneralOperator::QStrToCStr(QString QStr)
{
	string strTemp = QStr.toLocal8Bit().data();
	return strTemp.c_str();
}

QString CClsGeneralOperator::CStrToQStr(CString CStr)
{
	//string strTemp = CT2A(CStr);//CString->string
	//return QString::fromStdString(strTemp);
	return QString::fromWCharArray((LPCTSTR)CStr, CStr.GetLength());
}

void CClsGeneralOperator::InitializeFileTable(QTableWidget *tableFiles, QStringList strFileList)
{
	tableFiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableFiles->setRowCount(strFileList.length());//�������ļ�����ͬ
	tableFiles->setColumnCount(1);//1��������ʾ�ļ���
	for (int i = 0; i < strFileList.length(); i++)
	{
		tableFiles->setItem(i, 0, new QTableWidgetItem(strFileList.at(i)));
	}
	tableFiles->show();
}

CString CClsGeneralOperator::GetFileName(CString mFilePathName)
{
	int index1 = mFilePathName.ReverseFind('.');
	int index2 = mFilePathName.ReverseFind('\/') + 1;
	CString tempStr = mFilePathName.Mid(index2, index1 - index2);
	return tempStr;
}

CString CClsGeneralOperator::GetYMDFromYD(int Year, int Day)
{
	CString YYYYMMDDD;
	int DayArray[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };   //ƽ��ÿһ���µ�����
	BOOL IsYear;
	IsYear = false;
	//�����ж��Ƿ�����
	if ((Year % 4 == 0 && Year % 100 != 0) || Year % 400 == 0)
		IsYear = true;

	if (IsYear)
	{
		DayArray[1] = 29;          //һ�������꣬���·ݵ�������1
	}

	int Month, Dayth;
	for (int i = 0; i < 12; i++)
	{
		if (Day > DayArray[i])
		{
			Day -= DayArray[i];
		}
		else
		{
			Month = i + 1;
			Dayth = Day;

			CString SYear, SMonth, SDay;
			SYear.Format(L"%d", Year);
			SMonth.Format(L"%d", Month);
			SDay.Format(L"%d", Day);
			if (SMonth.GetLength() == 1)
				SMonth = _T("0") + SMonth;
			if (SDay.GetLength() == 1)
				SDay = _T("0") + SDay;
			YYYYMMDDD = SYear + SMonth + SDay;

			return YYYYMMDDD;

		}
	}
	return YYYYMMDDD;
}

CString CClsGeneralOperator::GetYearMonthFromDays(int Days, long refYear)
{
	CString sYear, Month;

	int year[2] = { 365,366 };
	int month[2][12] = { 31,28,31,30,31,30,31,31,30,31,30,31,\
		31,29,31,30,31,30,31,31,30,31,30,31 };
	if (Days >= 0)
	{
		int i;
		for (i = refYear; Days >= year[LeapYear(i)]; i++)
		{
			Days -= year[LeapYear(i)];
		}

		sYear.Format(_T("%d"), i);
		int j;
		for (j = 0; Days >= month[LeapYear(i)][j]; j++)
		{
			Days -= month[LeapYear(i)][j];
		}
		CString temp;
		if (j + 1 <= 9)
		{
			temp.Format(_T("%d"), j + 1);
			Month = _T("0") + temp;
		}
		else
		{
			temp.Format(_T("%d"), j + 1);
			Month = temp;
		}
	}
	else
	{
		Days = (-1)*Days;
		int i;
		for (i = refYear - 1; Days >= year[LeapYear(i)]; i--)
		{
			Days -= year[LeapYear(i)];
		}
		sYear.Format(_T("%d"), i);
		int j;
		for (j = 0; Days >= month[LeapYear(i)][j]; j++)
		{
			Days -= month[LeapYear(i)][j];
		}
		CString temp;
		if (j + 1 <= 9)
		{
			temp.Format(_T("%d"), j + 1);
			Month = _T("0") + temp;
		}
		else
		{
			temp.Format(_T("%d"), j + 1);
			Month = temp;
		}
	}

	return sYear + Month;
}

CString CClsGeneralOperator::GetYMDFromDaysandReftime(int Days, QString reftime)
{
	CString sYear, Month;
	QDateTime start = QDateTime::fromString(reftime, "yyyy-MM-dd");
	QDateTime end = start.addDays(Days);

	QString time = end.toString("yyyyMMdd");
	//QString time2 = end.toString("yyyy-MM-dd");

	return CClsGeneralOperator::QStrToCStr(time);
}


CString CClsGeneralOperator::GetYMDFromDays(double Days, long refYear)
{
	CString sYear, Month, Day;

	int year[2] = { 365,366 };
	int month[2][12] = { 31,28,31,30,31,30,31,31,30,31,30,31,\
		31,29,31,30,31,30,31,31,30,31,30,31 };
	if (Days >= 0)
	{
		int i;
		for (i = refYear; Days >= year[LeapYear(i)]; i++)
		{
			Days -= year[LeapYear(i)];
		}

		sYear.Format(_T("%d"), i);
		int j;
		for (j = 0; Days >= month[LeapYear(i)][j]; j++)
		{
			Days -= month[LeapYear(i)][j];
		}
		CString temp;
		if (j + 1 <= 9)
		{
			temp.Format(_T("%d"), j + 1);
			Month = _T("0") + temp;
		}
		else
		{
			temp.Format(_T("%d"), j + 1);
			Month = temp;
		}
	}
	else
	{
		Days = (-1)*Days;
		int i;
		for (i = refYear - 1; Days >= year[LeapYear(i)]; i--)
		{
			Days -= year[LeapYear(i)];
		}
		sYear.Format(_T("%d"), i);
		int j;
		for (j = 0; Days >= month[LeapYear(i)][j]; j++)
		{
			Days -= month[LeapYear(i)][j];
		}
		CString temp;
		if (j + 1 <= 9)
		{
			temp.Format(_T("%d"), j + 1);
			Month = _T("0") + temp;
		}
		else
		{
			temp.Format(_T("%d"), j + 1);
			Month = temp;
		}
	}

	
	if (Days <= 9)
	{
		CString tempday;
		int ch = ceil(Days);
		tempday.Format(_T("%d"), ch);
		Day = _T("0") + tempday;
	}
	else
	{
		int ch = ceil(Days);
		Day.Format(_T("%d"), ch);
	}
	
	return sYear + Month + Day;
}

int CClsGeneralOperator::GetMonthFromDays(int Days)
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

int CClsGeneralOperator::GetYearFromDays(int Days)
{
	int year[2] = { 365,366 };
	int i;
	for (i = 1800; Days >= year[LeapYear(i)]; i++)
	{
		Days -= year[LeapYear(i)];
	}
	return i;
}

int CClsGeneralOperator::LeapYear(int Year)
{
	if (Year % 4 != 0 || (Year % 100 == 0 && Year % 400 != 0))
		return 0;  //��������
	else
		return 1;   //����
}

double* CClsGeneralOperator::GetMin_Max(double *value, long * pBuffer, long m_Rows, long m_Cols,long DefaultValue)
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

double *CClsGeneralOperator::GetMin_Max(double *value, double * pBuffer, long m_Rows, long m_Cols, double DefaultValue)
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

double CClsGeneralOperator::GetMeanValue(long * pBuffer, long m_Rows, long m_Cols, long DefaultValue)
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

double CClsGeneralOperator::GetMeanValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue)
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

double CClsGeneralOperator::GetStdValue(long * pBuffer, long m_Rows, long m_Cols, long DefaultValue)
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

double CClsGeneralOperator::GetStdValue(double * pBuffer, long m_Rows, long m_Cols, double DefaultValue)
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

void CClsGeneralOperator::split(std::string & s, std::string & delim, std::vector<std::string>* ret)
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

void CClsGeneralOperator::addArray(long *arr1, long *arr2, int mRow, int mCol)
{
	for (int i = 0; i < mRow*mCol; i++)
	{
		if (arr2[i] == -9999 || arr1[i] == -9999)
		{
			arr1[i] = -9999;
			continue;
		}
		arr1[i] += arr2[i];
		//if (arr2[i] == -9999)
		//{
		//	continue;
		//}
		//arr1[i] += arr2[i];
	}
}

void CClsGeneralOperator::RemoveSelectedFiles(QTableWidget * tableFiles, QStringList & strFileList, QLabel * countLalbel)
{
	//ͨ��Map����ɾ�����У�����ɾ�����ҵ�����
	QItemSelectionModel *selections = tableFiles->selectionModel();
	QModelIndexList selected = selections->selectedIndexes();
	QMap<int, int> rowMap;
	foreach(QModelIndex index, selected)
	{
		rowMap.insert(index.row(), 0);
	}
	int rowToDel;
	QMapIterator<int, int>rowMapInterator(rowMap);
	rowMapInterator.toBack();
	while (rowMapInterator.hasPrevious())
	{
		rowMapInterator.previous();
		rowToDel = rowMapInterator.key();
		tableFiles->removeRow(rowToDel);
	}
	//
	//������֯ѡ���ļ�
	int INT_TABLE_COUNT = tableFiles->rowCount();
	strFileList.clear();

	for (int i = 0; i < INT_TABLE_COUNT; i++)
	{
		strFileList.append(tableFiles->item(i, 0)->text());
	}
	if (countLalbel != NULL)
	{
		countLalbel->setText(QStringLiteral("����:") + QString::number(strFileList.length(), 10));
	}
	
}

bool CClsGeneralOperator::WriteNakeData(CString mFileName, double *pBuffer, long mRows, long mCols)
{
	//д���ļ�
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)mFileName.GetBuffer(mFileName.GetLength()));

	FILE *outFile;
	errno_t err;

	if ((err = fopen_s(&outFile, lpsz, "wb")) != 0)
	{
		//QMessageBox(_T("���ļ�ʧ��!"));
		QMessageBox::information(NULL, QStringLiteral("����"), QStringLiteral("���ļ�ʧ��!"));
		return  false;
	}

	////д���ļ���ʾ��sample Sequence data sets
	/*fprintf(outFile,"SSDS\n");

	fprintf(outFile,"���� %d\n", mRows);
	fprintf(outFile,"���� %d\n", mCols);
	fprintf(outFile,"ֵ \n");*/

	for (long i = 0;i<mRows;i++)
	{
		for (long j = 0;j<mCols;j++)
		{
			fprintf(outFile, "%lf ", pBuffer[i*mCols + j]);
		}
		fprintf(outFile, "\n");
	}

	//�ر��ļ�
	fclose(outFile);

	return true;
}

bool CClsGeneralOperator::WriteNakeData(CString mFileName, long *pBuffer, long mRows, long mCols)
{
	//д���ļ�
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)mFileName.GetBuffer(mFileName.GetLength()));

	FILE *outFile;
	errno_t err;

	if ((err = fopen_s(&outFile, lpsz, "wb")) != 0)
	{
		//QMessageBox(_T("���ļ�ʧ��!"));
		QMessageBox::information(NULL, QStringLiteral("����"), QStringLiteral("���ļ�ʧ��!"));
		return  false;
	}

	////д���ļ���ʾ��sample Sequence data sets
	/*fprintf(outFile,"SSDS\n");

	fprintf(outFile,"���� %d\n", mRows);
	fprintf(outFile,"���� %d\n", mCols);
	fprintf(outFile,"ֵ \n");*/

	for (long i = 0; i<mRows; i++)
	{
		for (long j = 0; j<mCols; j++)
		{
			fprintf(outFile, "%d ", pBuffer[i*mCols + j]);
		}
		fprintf(outFile, "\n");
	}

	//�ر��ļ�
	fclose(outFile);

	return true;
}

double CClsGeneralOperator::STInterpolate(long *pTBuffer, double mScale, long mRows, long mCols, long m, long n, double mMissingValue, long *pBuffer1, long *pBuffer2, long size)
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


	//����ռ��ֵ
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

	//����ʱ��1�Ŀռ��ֵ��
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

	//����ʱ��2�Ŀռ��ֵ��
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

	//����ʱ�վ�ֵ
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

bool CClsGeneralOperator::DataSpatialConvertByMean(long *pSrcBuffer,/*ԭ���ݼ�*/long *pTarBuffer,/*Ŀ�����ݼ�*/long mSrcRows, long mSrcCols, long mTarRows, long mTarCols, double reSize/*ת���ߴ�*/)
{
	double resampleSize;
	//��ȡ�����ߴ�
	if (reSize >1)
		resampleSize = (int)(reSize + 0.5);
	else
		resampleSize = (int)(1 / reSize + 0.5);

	resampleSize = reSize;

	//ת��
	if (reSize >1)    //�ۺ�
	{
		for (int i = 0;i<mTarRows;i++)
		{
			for (int j = 0;j<mTarCols;j++)
			{
				//�ۺϼ���
				long meanValue;
				long sumValue = 0;
				long num = 0;
				long sumNum = 0;
				for (int k = (long)(-resampleSize / 2 - 0.5);k<(long)(resampleSize / 2 + 0.5);k++)
				{
					for (int l = (long)(-resampleSize / 2 - 0.5);l<(long)(resampleSize / 2 + 0.5);l++)
					{
						sumNum += 1;
						if ((int)(i*resampleSize + k) >= 0 && (int)(i*resampleSize + k) < mSrcRows && (int)(j*resampleSize + l) >= 0 && (int)(j*resampleSize + l) < mSrcCols && pSrcBuffer[(int)(i*resampleSize + k)*mSrcCols + (int)(j*resampleSize + l)] != -9999)
						{
							sumValue += pSrcBuffer[(int)(i*resampleSize + k)*mSrcCols + (int)(j*resampleSize + l)];
							num += 1;
						}
					}
				}

				if (num != 0 )//&& num >= sumNum / 3)
					meanValue = (long)(sumValue / num + 0.5);
				else
					meanValue = -9999;

				pTarBuffer[i*mTarCols + j] = meanValue;

			}
		}
	}
	else           //��ֵ
	{
		for (int i = 0;i<mTarRows;i++)
		{
			int k = (int)(i / resampleSize + 0.5);

			for (int j = 0;j<mTarCols;j++)
			{
				int l = (int)(j / resampleSize + 0.5);

				if (k<mSrcRows && l<mSrcCols)
					pTarBuffer[i*mTarCols + j] = pSrcBuffer[k*mSrcCols + l];
				else
					pTarBuffer[i*mTarCols + j] = -9999;
			}
		}

	}

	return true;
}

double CClsGeneralOperator::CalMeanValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale)
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

double CClsGeneralOperator::CalMaxValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale)
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

double CClsGeneralOperator::CalMinValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale)
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

double CClsGeneralOperator::CalStdValueFromLongSeq(long *pBuffer, long mNum, long mFillValue, double mScale)
{
	double tValue;
	double sumValue;
	long mValueNum;

	sumValue = 0.0;
	mValueNum = 0;

	//��ƽ����
	for (long i = 0;i<mNum;i++)
	{
		if (pBuffer[i] != mFillValue)
		{
			sumValue += pow(pBuffer[i] * mScale, 2.0);
			mValueNum += 1;
		}

	}

	if (mValueNum != 0)
	{
		//��ƽ���;�ֵ
		sumValue = sumValue / mValueNum;

		double meanValue = CalMeanValueFromLongSeq(pBuffer, mNum, -9999, mScale);

		tValue = sqrt(sumValue - pow(meanValue, 2.0));
	}
	else
		tValue = -9999.0;

	return tValue;
}

bool CClsGeneralOperator::WriteSuferDatFile(CString mFileName, double *pBuffer, double mStartLog, double mStartLat, double LogSize, double LatSize, long mRows, long mCols)
{
	//д���ļ�
	USES_CONVERSION;
	const char* lpsz = W2A((LPCTSTR)mFileName.GetBuffer(mFileName.GetLength()));

	FILE *outFile;
	errno_t err;

	if ((err = fopen_s(&outFile, lpsz, "wb")) != 0)
	{
		//AfxMessageBox(_T("���ļ�ʧ��!"));
		QMessageBox::information(NULL, QStringLiteral("����"), QStringLiteral("���ļ�ʧ��!"));
		return false;
	}

	////д���ļ���ʾ��sample Sequence data sets
	fprintf(outFile, "SSDS\n");

	fprintf(outFile, "���� %d\n", mRows);
	fprintf(outFile, "���� %d\n", mCols);
	fprintf(outFile, "ֵ \n");

	for (long i = 0;i<mRows;i++)
	{
		double yLat = mStartLat + i*LatSize;

		for (long j = 0;j<mCols;j++)
		{
			double xLog = mStartLog + j*LogSize;

			fprintf(outFile, "%lf %lf %lf \n", yLat, xLog, pBuffer[i*mCols + j]);

		}
	}

	//	(mOutPath,pBuffer,1,mStartLat,1,tRe,mCutRows,mFileNum))

	//�ر��ļ�
	fclose(outFile);

	return true;

}

int CClsGeneralOperator::StringToInt(CString tempStr)
{
	USES_CONVERSION;
	char* tempChar;
	tempChar = W2A((LPCTSTR)tempStr.GetBuffer(tempStr.GetLength()));

	int tempNum;
	tempNum = atoi(tempChar);
	return tempNum;
}

//����γ�Ȳ����ȵ��ز���
void CClsGeneralOperator::SpatialResampleBasedOnUnevenLat(long *pOriBuffer, long *pTarBuffer, double *pOriLatBuffer, 
	long mOriRows, long mOriCols, long mTarRows, long mTarCols, 
	double startlog, double endlog, double startlat, double endlat)
{
	//double startlog;
	//double endlog;
	//double startlat;
	//double endlat;
	double *pOriLineLatBuffer = new double[mOriRows + 1];//�����洢դ���Եγ��ֵ
	//ȷ��γ��û�еߵ�
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
	////��ʼ����
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
				//����դ�����ľ�γ��
				double lon = j*mTarResolution + 0.5*mTarResolution + startlog;
				double lat = endlat - (i*mTarResolution + 0.5*mTarResolution);
				//���㾭γ����ԭ�ļ��е��С��к�,ע��γ�ȼ��������
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
				//����դ�����ľ�γ��
				double lon = j*mTarResolution + 0.5*mTarResolution + startlog;
				double lat = endlat - (i*mTarResolution + 0.5*mTarResolution);
				//���㾭γ����ԭ�ļ��е��С��к�,ע��γ�ȼ��������
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


bool CClsGeneralOperator::isNumber(CString str)
{
	USES_CONVERSION;
	if (strspn(W2A(str), "-0.123456789") != strlen(W2A(str)))//��ȫΪ����
		return false;
	else
		return true;
}

