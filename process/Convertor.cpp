#include "Convertor.h"

void Convertor::GeoTiff2HDF(vector<string> strFileList, string strSavePath, double startLat, double endLat, double startLog, double endLog)
{
	TifOpt *gdalOP = new TifOpt();

	for (int i = 0; i < strFileList.size(); i++)
	{
		int endPos = strFileList.at(i).find(".tif");
		int startPos = strFileList.at(i).find_last_of("/");
		startPos = startPos != -1 ? startPos : strFileList.at(i).find_last_of("\\");
		
		string outFile = strSavePath + "\\" + strFileList.at(i).substr(startPos + 1, endPos - startPos) + "hdfOpt";
		if (_access(strSavePath.c_str(), 0) == -1)	//����ļ��в�����
			_mkdir(strSavePath.c_str());
		if (_access(outFile.c_str(), 0) != -1)
			continue;
		if (!gdalOP->Convert_GeoTiff2HDF(strFileList.at(i).c_str(), outFile.c_str(), startLat, endLat, startLog, endLog))
			cout << "Fail!" << endl;
	}
}

bool Convertor::ResampleBatch(vector<string> strFileList, string strSavePath, double targetResolution) {
	Meta meta = hdfOpt::getHDFMeta(strFileList[0]);
	double ratio = targetResolution / meta.resolution;

	Meta newMeta = meta;
	newMeta.resolution = targetResolution;
	newMeta.nRow = meta.nRow / ratio;
	newMeta.nCol = meta.nCol / ratio;

	hdfOpt HDFIO;
	for (string f : strFileList) {
		float* src = new float[meta.nPixel];
        float* tar = new float[meta.nPixel];
		if (!HDFIO.GetDsByDsnameFROMProduct(src, f.c_str(), META_DEF.DataSetName.c_str(), 0, meta.nRow, 0, meta.nCol))
		{
			delete[] src;
			delete[] tar;
			return false;
		}

        Resample(src, tar, meta.nRow, meta.nCol, newMeta.nRow, newMeta.nCol, ratio);

		//newMeta.Date = f.substr(f.find_last_of("\\") + 26, 8).c_str();
		//string fileName = util::generateFileName(f, strSavePath, "resample","hdfOpt", newMeta.Date);
		string fileName = util::generateFileName(f, strSavePath, "resample", "hdfOpt");
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

void Convertor::Resample(float *src, float *tar, int srcRows, int srcCols, float ratio)
{
    int tarRows = srcRows / ratio;
    int tarCols = srcCols / ratio;

    int offset1 =  (int)(-ratio / 2 - 0.5);
    int offset2 =  (int)(ratio / 2 + 0.5);

    if (ratio > 1)    //聚合
    {
        for (int i = 0;i<tarRows;i++)
        {
            for (int j = 0;j<tarCols;j++)
            {
                //聚合计算
                float meanValue;
                double sumValue = 0;
                long num = 0;
                long sumNum = 0;
                for (int k = offset1; k <= offset2; k++)
                {
                    for (int l = offset1; l <= offset2; l++)
                    {
                        sumNum += 1;
                        int sr = (int)(i * ratio + k);
                        int sc = (int)(j * ratio + l);
                        if (sr >= 0 && sr < srcRows && sc >= 0 && sc < srcCols && !isFillValue(src[sr * srcCols + sc]))
                        {
                            sumValue += src[sr * srcCols + sc];
                            num += 1;
                        }
                    }
                }

                if (num != 0 && num >= sumNum / 3)
                    meanValue = sumValue / num;
                else
                    meanValue = FILL_VAL;

                tar[i*tarCols + j] = meanValue;

            }
        }
    }else           //插值
    {
        for (int i = 0;i<tarRows;i++)
        {
            int k = (int)(i / ratio + 0.5);

            for (int j = 0;j<tarCols;j++)
            {
                int l = (int)(j / ratio + 0.5);

                if (k<srcRows && l<srcCols)
                    tar[i*tarCols + j] = src[k*srcCols + l];
                else
                    tar[i*tarCols + j] = FILL_VAL;
            }
        }
    }
}

void Convertor::SpaceTransform(vector<string> strFileList, string strSavePath)
{
	hdfOpt *pReadHDF = new hdfOpt();
	string tempStr = strFileList[0];
	string DataType = pReadHDF->GetFileProductType((char*)tempStr.c_str());//

	string mDsName = META_DEF.DataSetName.c_str();
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
			//val = NULL;
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

		util *pHDF4 = new util();
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