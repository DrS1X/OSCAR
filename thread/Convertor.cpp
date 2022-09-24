#include "opt.h"
#include "Convertor.h"
#include <direct.h>
#include "_const.h"

void Convertor::GeoTiff2HDF(vector<string> strFileList, string strSavePath, double startLat, double endLat, double startLog, double endLog)
{
	gdalOpt *gdalOP = new gdalOpt();

	for (int i = 0; i < strFileList.size(); i++)
	{
		int endPos = strFileList.at(i).find(".tif");
		int startPos = strFileList.at(i).find_last_of("/");
		startPos = startPos != -1 ? startPos : strFileList.at(i).find_last_of("\\");
		
		string outFile = strSavePath + "\\" + strFileList.at(i).substr(startPos + 1, endPos - startPos) + "hdf";
		if (_access(strSavePath.c_str(), 0) == -1)	//����ļ��в�����
			_mkdir(strSavePath.c_str());
		if (_access(outFile.c_str(), 0) != -1)
			continue;
		if (!gdalOP->Convert_GeoTiff2HDF(strFileList.at(i).c_str(), outFile.c_str(), startLat, endLat, startLog, endLog))
			cout << "Fail!" << endl;
	}
}

bool Convertor::Resample(vector<string> strFileList, string strSavePath, double targetResolution) {
	Meta meta = hdfOpt::getHDFMeta(strFileList[0]);
	double ratio = targetResolution / meta.Resolution;

	Meta newMeta = meta;
	newMeta.Resolution = targetResolution;
	newMeta.Rows = meta.Rows / ratio;
	newMeta.Cols = meta.Cols / ratio;

	hdfOpt HDFIO;
	for (string f : strFileList) {
		long* src = new long[meta.Size];
		long* tar = new long[meta.Size];
		if (!HDFIO.GetDsByDsnameFROMProduct(src, f.c_str(), Def.DataSetName.c_str(), 0,meta.Rows, 0, meta.Cols))
		{
			delete[] src;
			delete[] tar;
			return false;
		}

		opt::DataSpatialConvertByMean(src, tar, meta.Rows, meta.Cols, newMeta.Rows, newMeta.Cols, ratio);

		//newMeta.Date = f.substr(f.find_last_of("\\") + 26, 8).c_str();
		//string fileName = opt::generateFileName(f, strSavePath, "resample","hdf", newMeta.Date);
		string fileName = opt::generateFileName(f, strSavePath, "resample", "hdf");
		if (!HDFIO.writeHDF(fileName, newMeta, tar))
		{
			delete[] src;
			delete[] tar;
			return false;
		}

		delete[] src;
		delete[] tar;
	}

	return true;
}

void Convertor::SpaceTransform(vector<string> strFileList, string strSavePath)
{
	hdfOpt *pReadHDF = new hdfOpt();
	string tempStr = strFileList[0];
	string DataType = pReadHDF->GetFileProductType((char*)tempStr.c_str());//

	string mDsName = Def.DataSetName.c_str();
	for (int i = 0; i < strFileList.size(); i++)
	{
		string tName =strFileList[i].c_str();
		double mResolution = pReadHDF->GetDatasetsSpatialResolution_New(tName, mDsName);
		string mDsDate = pReadHDF->GetFileDateTime(tName);
		double mScale = pReadHDF->GetDataSetsScale(tName, mDsName);
		double mMissingValue = pReadHDF->GetDataSetsMissingValue(tName, mDsName);
		long Rows = pReadHDF->GetDatasetsRows(tName, mDsName);
		long Cols = pReadHDF->GetDatasetsCols(tName, mDsName);
		double mStartLog = pReadHDF->GetDatasetsStartLog(tName, mDsName);
		double mStartLat = pReadHDF->GetDatasetsStartLat(tName, mDsName);
		double mEndLog = pReadHDF->GetDatasetsEndLog(tName, mDsName);
		double mEndLat = pReadHDF->GetDatasetsEndLat(tName, mDsName);

		long *pBuffer = NULL;
		pBuffer = (long*)malloc(Rows*Cols * sizeof(long));
		if (!pReadHDF->GetDsByDsnameFROMProduct(pBuffer, tName, mDsName, 0, Rows, 0, Cols))
		{
			free(pBuffer);
			return;
		}
		
		{
			long **temp = new long*[Rows];
			for (long i = 0; i < Rows; i++)
			{
				temp[i] = new long[Cols];
			}
			///
			for (long i = 0; i < Rows; i++)
			{
				for (long j = 0; j < Cols; j++)
				{
					if (j < Cols / 2)
					{
						temp[i][j + Cols / 2] = pBuffer[i*Cols + j];
					}
					else
					{
						temp[i][j - Cols / 2] = pBuffer[i*Cols + j];
					}
				}
			}
			//pBuffer = NULL;
			for (long i = 0; i < Rows; i++)
			{
				for (long j = 0; j < Cols; j++)
				{
					pBuffer[i*Cols + j] = temp[i][j];
				}
			}
			//�ͷ��ڴ�
			for (long i = 0; i < Rows; i++)
			{
				delete[] temp[i];
			}
			delete[]temp;
		}

		opt *pHDF4 = new opt();
		double *Max_Min = new double[2];
		Max_Min = pHDF4->GetMin_Max(Max_Min, pBuffer, Rows, Cols);
		double mMaxValue = Max_Min[1];
		double mMinValue = Max_Min[0];
		double mMeanValue = pHDF4->GetMeanValue(pBuffer, Rows, Cols);
		double mStdValue = pHDF4->GetStdValue(pBuffer, Rows, Cols);

		hdfOpt *pHDF = new hdfOpt();
		double mFillValue = pHDF->GetDataSetsMissingValue(tName, mDsName);
		string mProductType = pHDF->GetFileProductType(tName);
		double mOffset = pHDF->GetDataSetsOffsets(tName, mDsName);

		//д���ļ�		
		string mReDsName = mDsName;
		string mOutFileName = (strSavePath + strFileList[i].substr(strFileList[i].find_last_of("\\"))).c_str();
		if (!pReadHDF->WriteCustomHDF2DFile(mOutFileName, (string)mDsDate, mProductType, "0",
			mReDsName, pBuffer, mScale, mOffset, 0, 360, mStartLat, mEndLat,
			Rows, Cols, mMaxValue, mMinValue, mMeanValue, mStdValue, mFillValue, mResolution, "2ά"))
		{
			free(pBuffer);
			cout << "写入文件出错!!" << endl;
			return;
		}
		free(pBuffer);
		
	}
}