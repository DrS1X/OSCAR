#include "AnomalyAnalysis.h"

using namespace std;

bool AnomalyAnalysis::StandardAnomaly(vector<string> allFiles, string outputPath, TimeScale timeScale) {

	vector<string> fileGroup[BUFFER_SCALE]; // array of vector
	/*
	for (int i = 0; i < BUFFER_SCALE; ++i) {
		fileGroup[i] 
	}
	*/
	for (int i = 0; i < allFiles.size(); ++i) {
		int serialNum = util::getDayOfYear(allFiles[i]);
		if (serialNum == -1) continue;
		fileGroup[serialNum].push_back(allFiles[i]);
		//cout << i << endl;
	}
	Meta meta = hdfOpt::getHDFMeta(fileGroup[0][0]);
	
	ThreadPool pool(5);
	std::vector< std::future<bool> > results;
	for (int i = 0; i < timeScale; i++)
	{	
		vector<string> tmp = fileGroup[i];
		results.emplace_back(
			pool.enqueue([tmp,meta, outputPath] {
				return AnomalyAnalysis::StandardAnomaly_OnePeriod(tmp, meta, outputPath);
			})
		);		
	}

	for (auto && result : results)
		if (!result.get()) {
			cout << "Fail" << endl;
			return false;
		}
	return true;
}

bool AnomalyAnalysis::StandardAnomaly_OnePeriod(vector<string> Files, Meta meta, string mOutPath)
{	
	if (Files.size() == 0) return true;

	hdfOpt ReadHDF;

	double* pMeanBuffer = new double[meta.nPixel];
	double* pStdBuffer = new double[meta.nPixel];
	
	hdfOpt::meanAndStandardDeviation(Files, pMeanBuffer, pStdBuffer);
		

	for (long i = 0; i < Files.size(); i++)
	{
		string tStr = Files[i].c_str();
		
		long *pBuffer = (long*)malloc(meta.nPixel * sizeof(long));

		//ȡ
		if (!ReadHDF.GetDsByDsnameFROMProduct(pBuffer, tStr, META_DEF.DataSetName.c_str(), 0, meta.nRow, 0, meta.nCol))
		{
			free(pBuffer);
			cout << "ȡݳ" <<endl;
			return false;
		}
		
		//
		for (long j = 0; j<meta.nPixel; j++)
		{
			//ֵһʱֵ-9999
			//if (pValueNum[iMonth][j] * 1.0 / FileNum < 0.5)
			//{
			//	val[j] = mMissingValue;
			//	continue;
			//}

			if (pBuffer[j] != (long)meta.MissingValue
				&& pStdBuffer[j] != 0.0)
				pBuffer[j] = (long)(((pBuffer[j] * meta.scale - pMeanBuffer[j]) / pStdBuffer[j]) / meta.scale + 0.5); //0.5ΪǿתΪlongʱ

			if (pBuffer[j] != (long)meta.MissingValue && pStdBuffer[j] == 0.0)
				pBuffer[j] = 0;

		}
		util pHDF4;// = new util();
		double *Max_Min = new double[2];
		Max_Min = pHDF4.GetMin_Max(Max_Min, pBuffer, meta.nRow, meta.nCol);
		double mMaxValue = Max_Min[1];
		double mMinValue = Max_Min[0];
		double mMeanValue = pHDF4.GetMeanValue(pBuffer, meta.nRow, meta.nCol);
		double mStdValue = pHDF4.GetStdValue(pBuffer, meta.nRow, meta.nCol);
		delete Max_Min;

		hdfOpt pHDF;// = new hdfOpt();
		

		string date = util::getDate(Files[i]);
		string folder = mOutPath + "/" + date.substr(0, 4) + "/";
		
		if (_access(folder.c_str(), 0) == -1)	//ļв
			_mkdir(folder.c_str());

		string mOutFileName =  folder + "StandAnomaly" + date + ".hdfOpt";
		//pReadHDF->WriteHDFFile(mOutFileName, mReDsName, mDsDate, mResolution, val, mStartLat, mEndLat, mStartLog, mEndLog, mRows, mCols, 0.001);
		if (!ReadHDF.WriteCustomHDF2DFile(mOutFileName.c_str(), date.c_str(), META_DEF.ProductType.c_str(), "0",
                                          META_DEF.DataSetName.c_str(), pBuffer, meta.scale, meta.Offset, meta.startLon, meta.endLon, meta.startLat, meta.endLat,
                                          meta.nRow, meta.nCol, mMaxValue, mMinValue, mMeanValue, mStdValue, meta.MissingValue, meta.resolution, "2ά"))
		{			
			free(pBuffer);
			delete[] pMeanBuffer;
			delete[] pStdBuffer;
			return false;
		}
		free(pBuffer);
	}
	
	delete[] pMeanBuffer;
	delete[] pStdBuffer;
	return true;
}

bool AnomalyAnalysis::Filter(vector<string> Files, string outputPath, float STDtime, bool generateHDF) {
	string positiveOutputPath = outputPath +"\\pos";
	string negativeOutputPath = outputPath + +"\\neg";
	
	Meta meta = META_DEF;

	TifOpt go(meta);
	hdfOpt ho(meta);

	unique_ptr<double[]> mean(new double[meta.nPixel]);
	unique_ptr<double[]> std(new double[meta.nPixel]);
	unique_ptr<double[]> posThreshold(new double[meta.nPixel]);
	unique_ptr<double[]> negThreshold(new double[meta.nPixel]);

	hdfOpt::meanAndStandardDeviation(Files, mean.get(), std.get());

	for (long i = 0; i < meta.nPixel; ++i) {
		posThreshold[i] = (mean[i] + std[i] * STDtime);
		negThreshold[i] = (mean[i] - std[i] * STDtime);
	}

	const char* folder = outputPath.c_str();

	if (_access(folder, 0) == -1)
		_mkdir(folder);

	for (long i = 0; i < Files.size(); i++)
	{
		string fileName = Files[i].c_str();
		/*
		double *pos = new double[meta.Size];
		double *neg = new double[meta.Size];
		long *val = new long[meta.Size];
		*/
		unique_ptr<double[]> pos(new double[meta.nPixel]);
		unique_ptr<double[]> neg(new double[meta.nPixel]);
		unique_ptr<long[]> pBuffer(new long[meta.nPixel]);

		if (!ho.GetDsByDsnameFROMProduct(pBuffer.get(), fileName, META_DEF.DataSetName.c_str(), 0, meta.nRow, 0, meta.nCol))
		{
			/*
			delete[] val;
			delete[] pos;
			delete[] neg;
			*/
			cout << "ȡݳ" << endl;
			return false;
		}

		for (long j = 0; j < meta.nPixel; ++j) {
			double v = pBuffer[j] * meta.scale;
			//tmp[j] = v;
			if (v >= posThreshold[j])
				pos[j] = pBuffer[j];
			else
				pos[j] = 0;

			if (v <= negThreshold[j])
				neg[j] = pBuffer[j];
			else
				neg[j] = 0;

		}

		//ռƽ
		SpatialSmooth(pos.get(), META_DEF.nRow, META_DEF.nCol, META_DEF.fillValue);
		SpatialSmooth(neg.get(), META_DEF.nRow, META_DEF.nCol, META_DEF.fillValue);

		string date = Files[i].substr(Files[i].size() - 12, 8);
		meta.date = date;
		go.writeGeoTiff(positiveOutputPath + "\\Positive" + date + ".tif", meta, pos.get());
		go.writeGeoTiff(negativeOutputPath + "\\Negative" + date + ".tif", meta, neg.get());

		if(generateHDF)
		{
			unique_ptr<long[]> p(new long[meta.nPixel]);
			unique_ptr<long[]> n(new long[meta.nPixel]);
			for (long j = 0; j < meta.nPixel; ++j) {
				p[j] = pos[j];
				n[j] = neg[j];
			}

			ho.writeHDF(outputPath + "\\hdfOpt\\pos\\Positive" + date + ".hdfOpt", meta, p.get());
			ho.writeHDF(outputPath + "\\hdfOpt\\neg\\Negative" + date + ".hdfOpt", meta, n.get());
		}

		/*
		delete[] val;
		delete[] pos;
		delete[] neg;
		*/
	}
	/*
	delete[] mean;
	delete[] std;
	delete[] posThreshold;
	delete[] negThreshold;
	*/
	return true;
}

void AnomalyAnalysis::SpatialSmooth(double* pResultBuffer, int mRows, int mCols, double mFillValue)
{
	for (int m = 1; m < mRows - 1; m++)
	{
		for (int n = 1; n < mCols - 1; n++)
		{
			//ɾ
			int count_delete = 0;
			for (int mm = m - 1; mm < m + 2; mm++)
			{
				if (pResultBuffer[m * mCols + n] == 0 || pResultBuffer[m * mCols + n] == mFillValue)
					break;
				for (int nn = n - 1; nn < n + 2; nn++)
				{
					if (mm == m && nn == n)
					{
						continue;
					}
					else
					{
						if ((pResultBuffer[mm * mCols + nn] == 0 || pResultBuffer[mm * mCols + nn] == mFillValue))
						{
							count_delete++;
						}
					}
				}
			}
			if (count_delete >= 6)
			{
				pResultBuffer[m * mCols + n] = 0;
			}

			int count_fill = 0;
			double fill_value = 0;
			if (pResultBuffer[m * mCols + n] == 0 || pResultBuffer[m * mCols + n] == mFillValue)
			{
				for (int mm = m - 1; mm < m + 2; mm++)
				{
					for (int nn = n - 1; nn < n + 2; nn++)
					{
						if (mm == m && nn == n)
						{
							continue;
						}
						else
						{
							if (pResultBuffer[mm * mCols + nn] != 0 && pResultBuffer[mm * mCols + nn] != mFillValue)
							{
								count_fill++;
								fill_value += pResultBuffer[mm * mCols + nn];
							}
						}
					}
				}
				if (count_fill == 8)
				{
					pResultBuffer[m * mCols + n] = fill_value * 1.0 / count_fill;
				}
			}
		}
	}
}