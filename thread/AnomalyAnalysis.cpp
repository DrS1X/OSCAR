#include <_const.h>
#include <opt.h>
#include "AnomalyAnalysis.h"
#include "ThreadPool.h"
#include "AnomalyAnalysis.h"

using namespace std;

bool AnomalyAnalysis::StandardAnomaly(vector<string> allFiles, string outputPath, TimeScale timeScale) {

	vector<string> fileGroup[BUFFER_SCALE];
	/*
	for (int i = 0; i < BUFFER_SCALE; ++i) {
		fileGroup[i] 
	}
	*/
	for (int i = 0; i < allFiles.size(); ++i) {
		int serialNum = opt::getDayOfYear(allFiles[i]);
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

	//��ʼ��
	double* pMeanBuffer = new double[meta.Size];
	double* pStdBuffer = new double[meta.Size];
	
	hdfOpt::meanAndStandardDeviation(Files, pMeanBuffer, pStdBuffer);
		

	////����1-12�������㴦�ı�׼������
	//for (long j = 0; j<mRows*mCols; j++)
	//{
	//	if (pValueNum[j] != 0 && pMeanBuffer[j] != 0.0&& pStdBuffer[j] != 0.0)
	//	{
	//		double tV1 = (double)pStdBuffer[j] / pValueNum[j];
	//		double tV2 = pow(pMeanBuffer[j], 2.0);
	//		if (tV1 >= tV2)
	//			pStdBuffer[j] = sqrt(tV1 - tV2);
	//		else
	//			pStdBuffer[j] = 0.0;
	//	}
	//	else
	//	{
	//		pStdBuffer[j] = 0.0;
	//	}
	//}

	//�����쳣ֵ
	for (long i = 0; i < Files.size(); i++)
	{
		string tStr = Files[i].c_str();
		
		//���建����,�洢�����ֵ
		long *pBuffer = (long*)malloc(meta.Size * sizeof(long));

		//��ȡ����
		if (!ReadHDF.GetDsByDsnameFROMProduct(pBuffer, tStr, Def.DataSetName.c_str(), 0, meta.Rows, 0, meta.Cols))
		{
			//�ͷ��ڴ�
			free(pBuffer);	
			cout << "��ȡ���ݳ���" <<endl;
			return false;
		}
		
		//����
		for (long j = 0; j<meta.Size; j++)
		{
			//��ֵ��������һ��ʱ��ֵ-9999
			//if (pValueNum[iMonth][j] * 1.0 / FileNum < 0.5)
			//{
			//	pBuffer[j] = mMissingValue;
			//	continue;
			//}

			if (pBuffer[j] != (long)meta.MissingValue
				&& pStdBuffer[j] != 0.0)
				pBuffer[j] = (long)(((pBuffer[j] * meta.Scale - pMeanBuffer[j]) / pStdBuffer[j]) / meta.Scale +0.5); //��0.5��Ϊ��ǿתΪlong��ʱ����������

			if (pBuffer[j] != (long)meta.MissingValue && pStdBuffer[j] == 0.0)
				pBuffer[j] = 0;

		}
		opt pHDF4;// = new opt();
		double *Max_Min = new double[2];
		Max_Min = pHDF4.GetMin_Max(Max_Min, pBuffer, meta.Rows, meta.Cols);
		double mMaxValue = Max_Min[1];
		double mMinValue = Max_Min[0];
		double mMeanValue = pHDF4.GetMeanValue(pBuffer, meta.Rows, meta.Cols);
		double mStdValue = pHDF4.GetStdValue(pBuffer, meta.Rows, meta.Cols);
		delete Max_Min;

		hdfOpt pHDF;// = new hdfOpt();
		

		//д���ļ�		
		string date = opt::getDate(Files[i]);
		string folder = mOutPath + "/" + date.substr(0, 4) + "/";
		
		if (_access(folder.c_str(), 0) == -1)	//����ļ��в�����
			_mkdir(folder.c_str());

		string mOutFileName =  folder + "StandAnomaly" + date + ".hdf";
		//pReadHDF->WriteHDFFile(mOutFileName, mReDsName, mDsDate, mResolution, pBuffer, mStartLat, mEndLat, mStartLog, mEndLog, mRows, mCols, 0.001);
		if (!ReadHDF.WriteCustomHDF2DFile(mOutFileName.c_str(), date.c_str(), Def.ProductType.c_str(), "0",
			Def.DataSetName.c_str(), pBuffer, meta.Scale, meta.Offset, meta.StartLog, meta.EndLog, meta.StartLat, meta.EndLat,
			meta.Rows, meta.Cols, mMaxValue, mMinValue, mMeanValue, mStdValue, meta.MissingValue, meta.Resolution, "2ά"))
		{			
			free(pBuffer);
			//�ͷ��ڴ�	
			delete[] pMeanBuffer;
			delete[] pStdBuffer;
			cout << "д���ļ�����!" << endl;
			return false;
		}
		//�ͷ��ڴ�
		free(pBuffer);		
	}
	
	//�ͷ��ڴ�	
	delete[] pMeanBuffer;
	delete[] pStdBuffer;
	return true;
}

bool AnomalyAnalysis::SpatiotemporalAnomaly(vector<string> Files, string outputPath, float STDtime, bool generateHDF) {
	string positiveOutputPath = outputPath +"\\pos";
	string negativeOutputPath = outputPath + +"\\neg";
	
	Meta meta = Def;

	gdalOpt go(meta);
	hdfOpt ho(meta);

	unique_ptr<double[]> mean(new double[meta.Size]);
	unique_ptr<double[]> std(new double[meta.Size]);
	unique_ptr<double[]> posThreshold(new double[meta.Size]);
	unique_ptr<double[]> negThreshold(new double[meta.Size]);

	hdfOpt::meanAndStandardDeviation(Files, mean.get(), std.get());

	for (long i = 0; i < meta.Size; ++i) {
		posThreshold[i] = (mean[i] + std[i] * STDtime);
		negThreshold[i] = (mean[i] - std[i] * STDtime);
	}

	const char* folder = outputPath.c_str();

	if (_access(folder, 0) == -1)	//����ļ��в�����
		_mkdir(folder);

	for (long i = 0; i < Files.size(); i++)
	{
		string fileName = Files[i].c_str();
		/*
		double *pos = new double[meta.Size];
		double *neg = new double[meta.Size];
		long *pBuffer = new long[meta.Size];
		*/
		unique_ptr<double[]> pos(new double[meta.Size]);
		unique_ptr<double[]> neg(new double[meta.Size]);
		unique_ptr<long[]> pBuffer(new long[meta.Size]);

		//���建����,�洢�����ֵ
		//��ȡ����
		if (!ho.GetDsByDsnameFROMProduct(pBuffer.get(), fileName, Def.DataSetName.c_str(), 0, meta.Rows, 0, meta.Cols))
		{
			/*
			delete[] pBuffer;
			delete[] pos;
			delete[] neg;
			*/
			cout << "��ȡ���ݳ���" << endl;
			return false;
		}

		for (long j = 0; j < meta.Size; ++j) {
			double v = pBuffer[j] * meta.Scale;
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

		//�ռ�ƽ��
		SpatialSmooth(pos.get(), Def.Rows, Def.Cols, Def.FillValue);
		SpatialSmooth(neg.get(), Def.Rows, Def.Cols, Def.FillValue);

		string date = Files[i].substr(Files[i].size() - 12, 8);
		meta.Date = date;
		go.writeGeoTiff(positiveOutputPath + "\\Positive" + date + ".tif", meta, pos.get());
		go.writeGeoTiff(negativeOutputPath + "\\Negative" + date + ".tif", meta, neg.get());

		if(generateHDF)
		{
			unique_ptr<long[]> p(new long[meta.Size]);
			unique_ptr<long[]> n(new long[meta.Size]);
			for (long j = 0; j < meta.Size; ++j) {
				p[j] = pos[j];
				n[j] = neg[j];
			}

			ho.writeHDF(outputPath + "\\hdf\\pos\\Positive" + date + ".hdf", meta, p.get());
			ho.writeHDF(outputPath + "\\hdf\\neg\\Negative" + date + ".hdf", meta, n.get());
		}

		/*
		delete[] pBuffer;
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
			//ɾ��������
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
			//�������,��ֵ�˲���ȡ���� ?��ֵ��
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