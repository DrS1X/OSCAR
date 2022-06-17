#include "ClsGeneralOperator.h"
#include "ClsGDALOperator.h"
#include "ClsHDF4Operator.h"
#include <algorithm>

//#include "CONST.h"

CClsGDALOperator::CClsGDALOperator()
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	geoTiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
}
CClsGDALOperator::CClsGDALOperator(Meta meta)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	geoTiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	startLog = meta.StartLog;
	startLat = meta.StartLat;
	endLog = meta.EndLog;
	endLat = meta.EndLat;
	mScale = meta.Scale; //转product HDF时 使用0.01即可
	mOffsets = meta.Offset;

	iRow = meta.Rows;
	iCol = meta.Cols;

	fillValue = meta.MissingValue;
	dsResolution = meta.Resolution;

	projection = meta.Projection;
}

bool CClsGDALOperator::readGeoTiff(const char* in_fileName, double* pTiffData) {
	//GDALDriver *geoTiffDriver;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDataset *pDataSet = (GDALDataset*)GDALOpen(in_fileName, GA_ReadOnly);
	if (pDataSet != NULL)
	{
		long iRow = pDataSet->GetRasterYSize();
		long iCol = pDataSet->GetRasterXSize();
		GDALRasterBand *demband = pDataSet->GetRasterBand(1);
		demband->RasterIO(GF_Read, 0, 0, iCol, iRow, pTiffData, iCol, iRow, GDT_Float64, 0, 0);
		demband->SetNoDataValue(-9999);
		GDALClose(GDALDatasetH(pDataSet));
		return true;
	}
	return false;
}

bool CClsGDALOperator::readGeoTiff(const string file, int* pBuffer) {
	const char* in_fileName = file.c_str();
	//GDALDriver *geoTiffDriver;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDataset *pDataSet = (GDALDataset*)GDALOpen(in_fileName, GA_ReadOnly);
	if (pDataSet != NULL)
	{
		long iRow = pDataSet->GetRasterYSize();
		long iCol = pDataSet->GetRasterXSize();
		GDALRasterBand *demband = pDataSet->GetRasterBand(1);
		demband->RasterIO(GF_Read, 0, 0, iCol, iRow, pBuffer, iCol, iRow, GDT_Int32, 0, 0);
		demband->SetNoDataValue(-9999);
		GDALClose(GDALDatasetH(pDataSet));
		return true;
	}
	return false;
}

bool CClsGDALOperator::writeGeoTiff(string fileName, Meta meta, int *buf) {

	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	if (geoTiffDriver)
	{
		geoTiffDataset = geoTiffDriver->Create(fileName.c_str(), iCol, iRow, 1, GDT_Int32, NULL);

		double *argout = new double[6];
		argout[0] = startLog; // 左上角横坐标
		argout[1] = dsResolution;
		argout[2] = 0;
		argout[3] = startLat; //左上角纵坐标
		argout[4] = 0;
		argout[5] = dsResolution*(-1);
		geoTiffDataset->SetGeoTransform(argout);

		geoTiffDataset->SetMetadataItem("ImageDate", imgDate.c_str());
		geoTiffDataset->SetMetadataItem("DataType", dataType.c_str());
		geoTiffDataset->SetMetadataItem("ProductType", productType.c_str());
		geoTiffDataset->SetMetadataItem("Dimension", dimension.c_str());
		geoTiffDataset->SetMetadataItem("DatasetName", dsName.c_str());

		char tempChar[100];

		sprintf_s(tempChar, "%f", 1.0);//在读取之后已经乘以比例系数，输出tiff时为原始值
		geoTiffDataset->SetMetadataItem("Scale", tempChar);

		sprintf_s(tempChar, "%f", mOffsets);
		geoTiffDataset->SetMetadataItem("Offsets", tempChar);

		sprintf_s(tempChar, "%f", startLog);
		geoTiffDataset->SetMetadataItem("StartLog", tempChar);

		sprintf_s(tempChar, "%f", endLog);
		geoTiffDataset->SetMetadataItem("EndLog", tempChar);

		sprintf_s(tempChar, "%f", startLat);
		geoTiffDataset->SetMetadataItem("StartLat", tempChar);

		sprintf_s(tempChar, "%f", endLat);
		geoTiffDataset->SetMetadataItem("EndLat", tempChar);

		sprintf_s(tempChar, "%d", Rows);
		geoTiffDataset->SetMetadataItem("Rows", tempChar);

		sprintf_s(tempChar, "%d", Cols);
		geoTiffDataset->SetMetadataItem("Cols", tempChar);

		sprintf_s(tempChar, "%f", fillValue);
		geoTiffDataset->SetMetadataItem("FillValue", tempChar);
		geoTiffDataset->GetRasterBand(1)->SetNoDataValue(fillValue);

		//计算最大最小值 必须先计算
		double sum = 0;
		//获取最大值、最小值
		double maxValue = DBL_MIN, minValue = DBL_MAX;
		int count = 0;
		for (int j = 0; j < meta.Size; j++)
		{
			if (buf[j] > maxValue)
				maxValue = buf[j];
			if (buf[j] < minValue)
				minValue = buf[j];
			sum += buf[j];
			count++;
		}
		//获取平均值
		double meanValue = sum / count;
		sum = 0;
		for (int j = 0; j < meta.Size; j++)
		{
			sum += ((double)buf[j] - meanValue) * ((double)buf[j] - meanValue);
		}
		//获取方差
		double stdValue = sqrt(sum / count);

		sprintf_s(tempChar, "%f", maxValue);
		geoTiffDataset->SetMetadataItem("MaxValue", tempChar);

		sprintf_s(tempChar, "%f", minValue);
		geoTiffDataset->SetMetadataItem("MinValue", tempChar);

		sprintf_s(tempChar, "%f", meanValue);
		geoTiffDataset->SetMetadataItem("MeanValue", tempChar);

		sprintf_s(tempChar, "%f", stdValue);
		geoTiffDataset->SetMetadataItem("StdValue", tempChar);

		sprintf_s(tempChar, "%f", dsResolution);
		geoTiffDataset->SetMetadataItem("DSResolution", tempChar);

		const char * Projection_info = projection.c_str();
		if (Projection_info != "\0")
		{
			geoTiffDataset->SetProjection(Projection_info);
		}
		geoTiffDataset->RasterIO(GF_Write, 0, 0, iCol, iRow, buf, iCol, iRow, GDT_Int32, 1, NULL, 0, 0, 0);
		GDALClose(GDALDatasetH(geoTiffDataset));
		return true;
	}
	return false;
}

bool CClsGDALOperator::writeGeoTiff(string fileName, Meta meta, double *buf) {
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	if (geoTiffDriver)
	{
		geoTiffDataset = geoTiffDriver->Create(fileName.c_str(), iCol, iRow, 1, GDT_Float64, NULL);

		double *argout = new double[6];
		argout[0] = startLog;
		argout[1] = dsResolution;
		argout[2] = 0;
		argout[3] = startLat;
		argout[4] = 0;
		argout[5] = dsResolution*(-1);
		geoTiffDataset->SetGeoTransform(argout);

		geoTiffDataset->SetMetadataItem("ImageDate", imgDate.c_str());
		geoTiffDataset->SetMetadataItem("DataType", dataType.c_str());
		geoTiffDataset->SetMetadataItem("ProductType", productType.c_str());
		geoTiffDataset->SetMetadataItem("Dimension", dimension.c_str());
		geoTiffDataset->SetMetadataItem("DatasetName", dsName.c_str());

		char tempChar[100];

		sprintf_s(tempChar, "%f", 1.0);//在读取之后已经乘以比例系数，输出tiff时为原始值
		geoTiffDataset->SetMetadataItem("Scale", tempChar);

		sprintf_s(tempChar, "%f", mOffsets);
		geoTiffDataset->SetMetadataItem("Offsets", tempChar);

		sprintf_s(tempChar, "%f", startLog);
		geoTiffDataset->SetMetadataItem("StartLog", tempChar);

		sprintf_s(tempChar, "%f", endLog);
		geoTiffDataset->SetMetadataItem("EndLog", tempChar);

		sprintf_s(tempChar, "%f", startLat);
		geoTiffDataset->SetMetadataItem("StartLat", tempChar);

		sprintf_s(tempChar, "%f", endLat);
		geoTiffDataset->SetMetadataItem("EndLat", tempChar);

		sprintf_s(tempChar, "%d", Rows);
		geoTiffDataset->SetMetadataItem("Rows", tempChar);

		sprintf_s(tempChar, "%d", Cols);
		geoTiffDataset->SetMetadataItem("Cols", tempChar);

		sprintf_s(tempChar, "%f", fillValue);
		geoTiffDataset->SetMetadataItem("FillValue", tempChar);
		geoTiffDataset->GetRasterBand(1)->SetNoDataValue(fillValue);

		//计算最大最小值 必须先计算
		double *min_max = new double[2];
		min_max = CClsGeneralOperator::GetMin_Max(min_max, buf, iRow, iCol, fillValue);
		minValue = min_max[0]; maxValue = min_max[1];
		stdValue = CClsGeneralOperator::GetStdValue(buf, iRow, iCol, fillValue);
		meanValue = CClsGeneralOperator::GetMeanValue(buf, iRow, iCol, fillValue);

		sprintf_s(tempChar, "%f", maxValue);
		geoTiffDataset->SetMetadataItem("MaxValue", tempChar);

		sprintf_s(tempChar, "%f", minValue);
		geoTiffDataset->SetMetadataItem("MinValue", tempChar);

		sprintf_s(tempChar, "%f", meanValue);
		geoTiffDataset->SetMetadataItem("MeanValue", tempChar);

		sprintf_s(tempChar, "%f", stdValue);
		geoTiffDataset->SetMetadataItem("StdValue", tempChar);

		sprintf_s(tempChar, "%f", dsResolution);
		geoTiffDataset->SetMetadataItem("DSResolution", tempChar);

		const char * Projection_info = projection.c_str();
		if (Projection_info != "\0")
		{
			geoTiffDataset->SetProjection(Projection_info);
		}
		geoTiffDataset->RasterIO(GF_Write, 0, 0, iCol, iRow, buf, iCol, iRow, GDT_Float64, 1, NULL, 0, 0, 0);
		GDALClose(GDALDatasetH(geoTiffDataset));
		//CPLFree(buf);
		return true;
	}
	return false;
}

bool CClsGDALOperator::ReadFileByGDAL(const char * mFileName)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	pDataSet = (GDALDataset*)GDALOpen(mFileName, GA_ReadOnly);
	if (pDataSet != NULL)
	{
		iRow = pDataSet->GetRasterYSize();
		iCol = pDataSet->GetRasterXSize();

		char ** metadatas = pDataSet->GetMetadata("");

		if (metadatas != NULL)
		{
			vector<string> *mdArr = new vector<string>();
			for (int i = 0; metadatas[i] != NULL; i++)
			{
				mdArr->push_back(string(metadatas[i]));
			}
			vector<string>tempStr(2);
			string s1 = "";
			string s2 = "=";
			CClsHDF4Operator *pHdf = new CClsHDF4Operator();
			string testString = string(mFileName);
			QString file = QString(QString::fromLocal8Bit(testString.c_str()));
			dsName = CClsGeneralOperator::CStrToQStr(pHdf->GetDatasetsNameByIndex(CClsGeneralOperator::QStrToCStr(file),0)).toStdString();
			transform(dsName.begin(), dsName.end(), dsName.begin(), ::toupper);

			for (int i = 0; i < mdArr->size(); i++)
			{
				tempStr.clear();
				CClsGeneralOperator::split(mdArr->at(i), s2, &tempStr);
				//C++不支持switch(string) case,用if...else if...else代替
				if (tempStr[0] == "ImageDate")imgDate = tempStr[1];
				else if (tempStr[0] == "DataType")dataType = tempStr[1];
				else if (tempStr[0] == "ProductType")productType = tempStr[1];
				else if (tempStr[0] == "Dimension")dimension = tempStr[1];
				else if (tempStr[0] == "Scale")mScale = atof(tempStr[1].c_str());
				else if (tempStr[0] == "Offsets")mOffsets = atof(tempStr[1].c_str());
				else if (tempStr[0] == "StartLog")startLog = atof(tempStr[1].c_str());
				else if (tempStr[0] == "StartLat")startLat = atof(tempStr[1].c_str());
				else if (tempStr[0] == "EndLog")endLog = atof(tempStr[1].c_str());
				else if (tempStr[0] == "EndLat")endLat = atof(tempStr[1].c_str());
				else if (tempStr[0] == "Rows")Rows = atoi(tempStr[1].c_str());
				else if (tempStr[0] == "Cols")Cols = atoi(tempStr[1].c_str());
				else if (tempStr[0] == "FillValue")fillValue = atof(tempStr[1].c_str());
				/*else if (tempStr[0] == "MaxValue")maxValue = atof(tempStr[1].c_str());
				else if (tempStr[0] == "MinValue")minValue = atof(tempStr[1].c_str());
				else if (tempStr[0] == "MeanValue")meanValue = atof(tempStr[1].c_str());
				else if (tempStr[0] == "StdValue")stdValue = atof(tempStr[1].c_str());*/
				else if (tempStr[0] == "DSResolution")dsResolution = atof(tempStr[1].c_str());
				else;

			}
		}
					
		//pMemData = (long*)CPLMalloc(sizeof(long)*iCol*iRow);
		pMemData = (double*)CPLMalloc(sizeof(double)*iCol*iRow);

		GDALRasterBand *demband = pDataSet->GetRasterBand(1);
		demband->SetOffset(mOffsets);
		demband->SetScale(mScale);
		demband->SetNoDataValue(fillValue);
		demband->RasterIO(GF_Read, 0, 0, iCol, iRow, pMemData, iCol, iRow, GDT_Float64, 0, 0);

		for (int i = 0; i < iRow*iCol; i++)
		{
			if (pMemData[i] != fillValue)
			{
				pMemData[i] = pMemData[i] * mScale;
			}
		}

		GDALClose(GDALDatasetH(pDataSet));
		return true;
	}
	return false;
}

bool CClsGDALOperator::Convert_GeoTiff2HDF(const char* in_fileName, const char* out_fileName, double startLat, double endLat, double startLog, double endLog)
{
	pDataSet = (GDALDataset*)GDALOpen(in_fileName, GA_ReadOnly);
	if (pDataSet != NULL)
	{
		double *argout = new double[6];
		pDataSet->GetGeoTransform(argout);
		//In a north up image, padfTransform[1] is the pixel width, and padfTransform[5] is the pixel height. 
		//The upper left corner of the upper left pixel is at position (padfTransform[0],padfTransform[3]).
		dsResolution = argout[1];

		if (startLog == endLog) {
			colOff = 0;
			iCol = pDataSet->GetRasterXSize();
			startLog = argout[0];
			endLog = argout[0] + argout[1] * iCol;
		}else {
			colOff = startLog / argout[5] + 1;
			iCol = endLog / argout[5] - colOff + 1;
		}
		if (startLat == endLat) {
			rowOff = 0;
			iRow = pDataSet->GetRasterYSize();
			endLat = argout[3];
			startLat = argout[3] + argout[5] * iRow;
		} else {
			rowOff = (90 - startLat) / argout[1] + 1;
			iRow = (90 - endLat) / argout[1] - rowOff + 1;
		}
		
		//string date = string(in_fileName);
		//date = date.substr(0, date.find_last_of("."));
		//imgDate = date.substr(date.find_last_of("/")+1);
		imgDate = string(in_fileName).substr(string(in_fileName).find_last_of("\\") + 26, 8);

		char ** metadatas = pDataSet->GetMetadata("");

		if (metadatas != NULL)
		{
			vector<string> *mdArr = new vector<string>();
			for (int i = 0; metadatas[i] != NULL; i++)
			{
				mdArr->push_back(string(metadatas[i]));
			}
			vector<string>tempStr(2);
			string s1 = "";
			string s2 = "=";
			for (int i = 0; i < mdArr->size(); i++)
			{
				tempStr.clear();
				CClsGeneralOperator::split(mdArr->at(i), s2, &tempStr);

				//如果为product产品，覆盖
				//C++不支持switch(string) case,用if...else if...else代替
				if (tempStr[0] == "ImageDate")imgDate = tempStr[1];				
				else if (tempStr[0] == "DatasetName")dsName = tempStr[1];
				else if (tempStr[0] == "ProductType")productType = tempStr[1];
				else if (tempStr[0] == "DataType")dataType = tempStr[1];
				else if (tempStr[0] == "ProductType")productType = tempStr[1];
				else if (tempStr[0] == "Dimension")dimension = tempStr[1];
				else if (tempStr[0] == "Scale")
				{
					mScale /= atof(tempStr[1].c_str());
				}
				else if (tempStr[0] == "Offsets")mOffsets = atof(tempStr[1].c_str());
				else if (tempStr[0] == "StartLog")startLog = atof(tempStr[1].c_str());
				else if (tempStr[0] == "StartLat")startLat = atof(tempStr[1].c_str());
				else if (tempStr[0] == "EndLog")endLog = atof(tempStr[1].c_str());
				else if (tempStr[0] == "EndLat")endLat = atof(tempStr[1].c_str());
				else if (tempStr[0] == "Rows")Rows = atoi(tempStr[1].c_str());
				else if (tempStr[0] == "Cols")Cols = atoi(tempStr[1].c_str());
				else if (tempStr[0] == "FillValue")fillValue = atof(tempStr[1].c_str());
				else if (tempStr[0] == "MaxValue")maxValue = atof(tempStr[1].c_str());
				else if (tempStr[0] == "MinValue")minValue = atof(tempStr[1].c_str());
				else if (tempStr[0] == "MeanValue")meanValue = atof(tempStr[1].c_str());
				else if (tempStr[0] == "StdValue")stdValue = atof(tempStr[1].c_str());
				else if (tempStr[0] == "DSResolution")dsResolution = atof(tempStr[1].c_str());
				else;

			}
		}

		double *pTiffData;
		pTiffData=(double*)CPLMalloc(sizeof(double)*iCol*iRow);

		pGeoData = (long*)CPLMalloc(sizeof(long)*iCol*iRow);


		GDALRasterBand *demband = pDataSet->GetRasterBand(1);
		fillValue=demband->GetNoDataValue();

		CPLErr iRes = demband->RasterIO(GF_Read, colOff, rowOff, iCol, iRow, pTiffData, iCol, iRow, GDT_Float64, 0, 0);
		GDALClose(GDALDatasetH(pDataSet));
		
		for (long i=0; i < iCol*iRow; i++)
		{
			if (isnan(pTiffData[i])) 
				pTiffData[i] = fillValue;
			if (pTiffData[i] != fillValue)
				pGeoData[i] = pTiffData[i] / mScale;
			else
				pGeoData[i] = pTiffData[i];

		}

		//为了去除0值 
		//for (long i = 0; i < iCol*iRow; i++)
		//{
		//	if (pGeoData[i] == 0)
		//	{
		//		pGeoData[i] = fillValue;
		//	}
		//}
		double *min_max = new double[2];
		min_max = CClsGeneralOperator::GetMin_Max(min_max, pGeoData, iRow, iCol, fillValue);
		minValue = min_max[0];
		maxValue = min_max[1];
		stdValue = CClsGeneralOperator::GetStdValue(pGeoData, iRow, iCol, fillValue);
		meanValue = CClsGeneralOperator::GetMeanValue(pGeoData, iRow, iCol, fillValue);
		CPLFree(pTiffData);
	}	
	CClsHDF4Operator *pHDF = new CClsHDF4Operator();
	if (pHDF->WriteCustomHDF2DFile(out_fileName, imgDate.c_str(), productType.c_str(), dataType.c_str(), dsName.c_str(), pGeoData, mScale, mOffsets, startLog, endLog, startLat, endLat, iRow, iCol, maxValue, minValue, meanValue, stdValue, fillValue, dsResolution))
	{
		CPLFree(pGeoData);
		return true;
	}
	CPLFree(pGeoData);
	delete pHDF;

	return false;
}

const char * CClsGDALOperator::GetProjection(const char * mFileName)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	pDataSet = (GDALDataset*)GDALOpen(mFileName, GA_ReadOnly);
	if (pDataSet != NULL)
	{
		return(pDataSet->GetProjectionRef());
	}
	return "";
}

