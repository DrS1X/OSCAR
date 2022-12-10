#include "util/util.h"
#include "TifOpt.h"
#include "hdfOpt.h"
#include <float.h>
#include <ogrsf_frmts.h>
#include <ogr_spatialref.h>
#include <ogr_geometry.h>
#include <cpl_conv.h>

using namespace std;

TifOpt::TifOpt()
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	geoTiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
}
TifOpt::TifOpt(Meta meta)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	geoTiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	startLog = meta.startLon;
	startLat = meta.startLat;
	endLog = meta.endLon;
	endLat = meta.endLat;
	mScale = meta.scale; //转product HDF时 使用0.01即可
	mOffsets = meta.Offset;

	iRow = meta.nRow;
	iCol = meta.nCol;

	fillValue = meta.fillValue;
	dsResolution = meta.resolution;

	projection = meta.projection;
}

bool TifOpt::write(std::string fileName, Meta meta, void* data){
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
        geoTiffDataset->SetMetadataItem("scale", tempChar);

        sprintf_s(tempChar, "%f", mOffsets);
        geoTiffDataset->SetMetadataItem("Offsets", tempChar);

        sprintf_s(tempChar, "%f", startLog);
        geoTiffDataset->SetMetadataItem("startLon", tempChar);

        sprintf_s(tempChar, "%f", endLog);
        geoTiffDataset->SetMetadataItem("endLon", tempChar);

        sprintf_s(tempChar, "%f", startLat);
        geoTiffDataset->SetMetadataItem("startLat", tempChar);

        sprintf_s(tempChar, "%f", endLat);
        geoTiffDataset->SetMetadataItem("endLat", tempChar);

        sprintf_s(tempChar, "%d", Rows);
        geoTiffDataset->SetMetadataItem("rows", tempChar);

        sprintf_s(tempChar, "%d", Cols);
        geoTiffDataset->SetMetadataItem("cols", tempChar);

        sprintf_s(tempChar, "%f", fillValue);
        geoTiffDataset->SetMetadataItem("FillValue", tempChar);
        geoTiffDataset->GetRasterBand(1)->SetNoDataValue(fillValue);

        //计算最大最小值 必须先计算
        double *min_max = new double[2];
        min_max = util::GetMin_Max(min_max, buf, iRow, iCol, fillValue);
        minValue = min_max[0]; maxValue = min_max[1];
        stdValue = util::GetStdValue(buf, iRow, iCol, fillValue);
        meanValue = util::GetMeanValue(buf, iRow, iCol, fillValue);

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

bool TifOpt::readGeoTiff(const char* in_fileName, double* pTiffData) {
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

bool TifOpt::readGeoTiff(const string file, int* pBuffer) {
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

bool TifOpt::writeGeoTiff(string fileName, Meta meta, int *buf) {

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
		geoTiffDataset->SetMetadataItem("scale", tempChar);

		sprintf_s(tempChar, "%f", mOffsets);
		geoTiffDataset->SetMetadataItem("Offsets", tempChar);

		sprintf_s(tempChar, "%f", startLog);
		geoTiffDataset->SetMetadataItem("startLon", tempChar);

		sprintf_s(tempChar, "%f", endLog);
		geoTiffDataset->SetMetadataItem("endLon", tempChar);

		sprintf_s(tempChar, "%f", startLat);
		geoTiffDataset->SetMetadataItem("startLat", tempChar);

		sprintf_s(tempChar, "%f", endLat);
		geoTiffDataset->SetMetadataItem("endLat", tempChar);

		sprintf_s(tempChar, "%d", Rows);
		geoTiffDataset->SetMetadataItem("rows", tempChar);

		sprintf_s(tempChar, "%d", Cols);
		geoTiffDataset->SetMetadataItem("cols", tempChar);

		sprintf_s(tempChar, "%f", fillValue);
		geoTiffDataset->SetMetadataItem("FillValue", tempChar);
		geoTiffDataset->GetRasterBand(1)->SetNoDataValue(fillValue);

		//计算最大最小值 必须先计算
		double sum = 0;
		//获取最大值、最小值
		double maxValue = DBL_MIN, minValue = DBL_MAX;
		int count = 0;
		for (int j = 0; j < meta.nPixel; j++)
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
		for (int j = 0; j < meta.nPixel; j++)
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

bool TifOpt::writeGeoTiff(string fileName, Meta meta, double *buf) {
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
		geoTiffDataset->SetMetadataItem("scale", tempChar);

		sprintf_s(tempChar, "%f", mOffsets);
		geoTiffDataset->SetMetadataItem("Offsets", tempChar);

		sprintf_s(tempChar, "%f", startLog);
		geoTiffDataset->SetMetadataItem("startLon", tempChar);

		sprintf_s(tempChar, "%f", endLog);
		geoTiffDataset->SetMetadataItem("endLon", tempChar);

		sprintf_s(tempChar, "%f", startLat);
		geoTiffDataset->SetMetadataItem("startLat", tempChar);

		sprintf_s(tempChar, "%f", endLat);
		geoTiffDataset->SetMetadataItem("endLat", tempChar);

		sprintf_s(tempChar, "%d", Rows);
		geoTiffDataset->SetMetadataItem("rows", tempChar);

		sprintf_s(tempChar, "%d", Cols);
		geoTiffDataset->SetMetadataItem("cols", tempChar);

		sprintf_s(tempChar, "%f", fillValue);
		geoTiffDataset->SetMetadataItem("FillValue", tempChar);
		geoTiffDataset->GetRasterBand(1)->SetNoDataValue(fillValue);

		//计算最大最小值 必须先计算
		double *min_max = new double[2];
		min_max = util::GetMin_Max(min_max, buf, iRow, iCol, fillValue);
		minValue = min_max[0]; maxValue = min_max[1];
		stdValue = util::GetStdValue(buf, iRow, iCol, fillValue);
		meanValue = util::GetMeanValue(buf, iRow, iCol, fillValue);

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

bool TifOpt::Convert_GeoTiff2HDF(const char* in_fileName, const char* out_fileName, double startLat, double endLat, double startLog, double endLog)
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
				util::split(mdArr->at(i), s2, &tempStr);

				//如果为product产品，覆盖
				//C++不支持switch(string) case,用if...else if...else代替
				if (tempStr[0] == "ImageDate")imgDate = tempStr[1];				
				else if (tempStr[0] == "DatasetName")dsName = tempStr[1];
				else if (tempStr[0] == "ProductType")productType = tempStr[1];
				else if (tempStr[0] == "DataType")dataType = tempStr[1];
				else if (tempStr[0] == "ProductType")productType = tempStr[1];
				else if (tempStr[0] == "Dimension")dimension = tempStr[1];
				else if (tempStr[0] == "scale")
				{
					mScale /= atof(tempStr[1].c_str());
				}
				else if (tempStr[0] == "Offsets")mOffsets = atof(tempStr[1].c_str());
				else if (tempStr[0] == "startLon")startLog = atof(tempStr[1].c_str());
				else if (tempStr[0] == "startLat")startLat = atof(tempStr[1].c_str());
				else if (tempStr[0] == "endLon")endLog = atof(tempStr[1].c_str());
				else if (tempStr[0] == "endLat")endLat = atof(tempStr[1].c_str());
				else if (tempStr[0] == "rows")Rows = atoi(tempStr[1].c_str());
				else if (tempStr[0] == "cols")Cols = atoi(tempStr[1].c_str());
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
		min_max = util::GetMin_Max(min_max, pGeoData, iRow, iCol, fillValue);
		minValue = min_max[0];
		maxValue = min_max[1];
		stdValue = util::GetStdValue(pGeoData, iRow, iCol, fillValue);
		meanValue = util::GetMeanValue(pGeoData, iRow, iCol, fillValue);
		CPLFree(pTiffData);
	}	
	hdfOpt *pHDF = new hdfOpt();
	if (pHDF->WriteCustomHDF2DFile(out_fileName, imgDate.c_str(), productType.c_str(), dataType.c_str(), dsName.c_str(), pGeoData, mScale, mOffsets, startLog, endLog, startLat, endLat, iRow, iCol, maxValue, minValue, meanValue, stdValue, fillValue, dsResolution))
	{
		CPLFree(pGeoData);
		return true;
	}
	CPLFree(pGeoData);
	delete pHDF;

	return false;
}

const char * TifOpt::GetProjection(const char * mFileName)
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

// TODO UTM是等角投影，根据此投影坐标系计算的面积是否准确；投影和面积计算是否存在性能消耗；面积字段用处多吗
double getArea(OGRGeometry* poGeom) {
    if (poGeom != NULL && wkbFlatten(poGeom->getGeometryType()) == wkbPolygon)
    {
        OGRPolygon* polygon = (OGRPolygon*) poGeom;
        OGRSpatialReference* spatialReference =  poGeom->getSpatialReference();

        spatialReference->SetProjCS( "UTM 17 (wgs84) in northern hemisphere" );
        spatialReference->SetWellKnownGeogCS( "WGS84" );
        spatialReference->SetUTM( 17, TRUE );

        polygon->transformTo(spatialReference);
        double area = polygon->get_Area();
        return area;
    }
    return 0.0;
}

void TifOpt::save(string outputPath, string startTime, AnomalyType anomalyType, vector<Poly>& polygons){
    string anomalyTypeStr = anomalyType == AnomalyType::Positive ? "Positive" : "Negative";
    save(outputPath, startTime, anomalyTypeStr, META_DEF.startLon, META_DEF.startLat, META_DEF.resolution, polygons);
}

void TifOpt::save(string outPath, string startTime, string abnormalType, const double startLog, const double startLat, const double resolution, vector<Poly>& polygons) {
    GDALAllRegister();
    //保存shp
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
    // 为了使属性表字段支持中文，请添加下面这句
    CPLSetConfigOption("SHAPE_ENCODING", "");

    const char *pszDriverName = "ESRI Shapefile";
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
    if (poDriver == NULL)
    {
        printf("%s driver not available.\n", pszDriverName);
        return;
    }

    GDALDataset *poDS;

    poDS = poDriver->Create(outPath.c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poDS == NULL)
    {
        printf("Creation of output file-operator failed.\n");
        return;
    }

    OGRLayer *poLayer;

    OGRSpatialReference * ref;
    ref = new OGRSpatialReference(META_DEF.projection.c_str());

    string fileName = abnormalType + startTime;

    poLayer = poDS->CreateLayer(fileName.c_str(), ref, wkbPolygon, NULL);
    if (poLayer == NULL)
    {
        printf("Layer creation failed.\n");
        return;
    }

    //Fields
    {
        OGRFieldDefn oFieldName("Name", OFTString);
        oFieldName.SetWidth(20);
        poLayer->CreateField(&oFieldName, 1);

        // 先创建一个叫FieldID的整型属性
        OGRFieldDefn oFieldStormID("PRID", OFTInteger);
        poLayer->CreateField(&oFieldStormID, 1);

        // 先创建一个叫FieldID的整型属性
        OGRFieldDefn oFieldStateID("STID", OFTString);
        oFieldStateID.SetWidth(20);
        poLayer->CreateField(&oFieldStateID, 1);

        // 再创建一个叫FeatureName的字符型属性，字符长度为50
        OGRFieldDefn oFieldSeqID("SQID", OFTString);
        oFieldSeqID.SetWidth(20);
        poLayer->CreateField(&oFieldSeqID, 1);

        // 再创建一个叫FeatureName的字符型属性，字符长度为50
        OGRFieldDefn oFieldTime("Time", OFTString);
        oFieldTime.SetWidth(20);
        poLayer->CreateField(&oFieldTime, 1);

        OGRFieldDefn oFieldMinLog("MinLon", OFTReal);
        oFieldMinLog.SetWidth(20);
        oFieldMinLog.SetPrecision(8);
        poLayer->CreateField(&oFieldMinLog, 1);
        OGRFieldDefn oFieldMinLat("MinLat", OFTReal);
        oFieldMinLat.SetWidth(20);
        oFieldMinLat.SetPrecision(8);
        poLayer->CreateField(&oFieldMinLat, 1);
        OGRFieldDefn oFieldMaxLog("MaxLon", OFTReal);
        oFieldMaxLog.SetWidth(20);
        oFieldMaxLog.SetPrecision(8);
        poLayer->CreateField(&oFieldMaxLog, 1);
        OGRFieldDefn oFieldMaxLat("MaxLat", OFTReal);
        oFieldMaxLat.SetWidth(20);
        oFieldMaxLat.SetPrecision(8);
        poLayer->CreateField(&oFieldMaxLat, 1);

        //创建area字段
        OGRFieldDefn oFieldArea("Area", OFTReal);
        oFieldArea.SetWidth(20);
        oFieldArea.SetPrecision(8);
        poLayer->CreateField(&oFieldArea, 1);

        //创建平均降雨量字段
        OGRFieldDefn oFieldAvgRainFall("AvgValue", OFTReal);
        oFieldAvgRainFall.SetWidth(20);
        oFieldAvgRainFall.SetPrecision(8);
        poLayer->CreateField(&oFieldAvgRainFall, 1);

        //创建最大降雨量字段
        OGRFieldDefn oFieldMaxRainFall("MaxValue", OFTReal);
        oFieldMaxRainFall.SetWidth(20);
        oFieldMaxRainFall.SetPrecision(8);
        poLayer->CreateField(&oFieldMaxRainFall, 1);

        //创建最大降雨量字段
        OGRFieldDefn oFieldMinRainFall("MinValue", OFTReal);
        oFieldMinRainFall.SetWidth(20);
        oFieldMinRainFall.SetPrecision(8);
        poLayer->CreateField(&oFieldMinRainFall, 1);

        //创建质心字段
        OGRFieldDefn oFieldLogCore("CentLon", OFTReal);
        oFieldLogCore.SetWidth(20);
        oFieldLogCore.SetPrecision(8);
        poLayer->CreateField(&oFieldLogCore, 1);

        //创建质心字段
        OGRFieldDefn oFieldLatCore("CentLat", OFTReal);
        oFieldLatCore.SetWidth(20);
        oFieldLatCore.SetPrecision(8);
        poLayer->CreateField(&oFieldLatCore, 1);

        //创建Power字段
        OGRFieldDefn oFieldPower("Power", OFTInteger);
        poLayer->CreateField(&oFieldPower, 1);

        //创建Abnormal异常类型
        OGRFieldDefn oFieldAbnormal("Abnormal", OFTString);
        oFieldAbnormal.SetWidth(20);
        poLayer->CreateField(&oFieldAbnormal, 1);
    }

    for (auto polygon : polygons) {
        OGRFeature *poFeature;
        poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());


        string polygonStr = "POLYGON (";
        for (auto line: polygon.lines) {
            polygonStr += "(";
            for (auto node: line.nodes) {
                int _row = node.row;//点行号
                int _col = node.col;//点列号

                double log = startLog + (_col + 1) * resolution;//经度
                double lat = startLat - (_row + 1) * resolution;//纬度

                polygonStr += to_string(log) + " " + to_string(lat);
                polygonStr += ",";
            }
            //移除最后一个逗号
            if (polygonStr[polygonStr.size() - 1] == ',') {
                polygonStr = polygonStr.substr(0, polygonStr.size() - 1);
            }
            polygonStr += "),";
        }
        //移除最后一个逗号
        if (polygonStr[polygonStr.size() - 1] == ',') {
            polygonStr = polygonStr.substr(0, polygonStr.size() - 1);
        }
        polygonStr += ")";

        char *pszWkt = (char *) polygonStr.c_str();

        OGRGeometry *poGeom;
        OGRGeometryFactory::createFromWkt(&pszWkt, ref, &poGeom);

        {
            // calculate field of polygon
            poFeature->SetField(0, "pr");
            poFeature->SetField(1, polygon.clusterId);
            poFeature->SetField(4, startTime.c_str());
            double minLog = startLog + (polygon.minCol + 1) * resolution;//最小经度
            double minLat = startLat - (polygon.maxRow + 1) * resolution;//最小纬度
            double maxLog = startLog + (polygon.maxCol + 1) * resolution;//最大经度
            double maxLat = startLat - (polygon.minRow + 1) * resolution;//最大纬度

            double centroidLog = startLog + (polygon.centroidCol + 0.5) * resolution;//质心经度
            double centroidLat = startLat - (polygon.centroidRow + 0.5) * resolution;//质心纬度

            poFeature->SetField(5, minLog);
            poFeature->SetField(6, minLat);
            poFeature->SetField(7, maxLog);
            poFeature->SetField(8, maxLat);

            double area = polygon.area;
            /*if(area == POLYGON_FIELD_EMPTY){
                area = getArea(poGeom);
            }*/
            poFeature->SetField(9, area);

            poFeature->SetField(10, polygon.avgValue);
            //poFeature->SetField(11, polygon.volume);
            poFeature->SetField(11, polygon.maxValue);
            poFeature->SetField(12, polygon.minValue);

            //poFeature->SetField(15, polygon.length);
            poFeature->SetField(13, centroidLog);
            poFeature->SetField(14, centroidLat);
            poFeature->SetField(15, polygon.power);
            poFeature->SetField(16, abnormalType.c_str());
        }

        poFeature->SetGeometryDirectly(poGeom);

        OGRErr err = poLayer->CreateFeature(poFeature);
        if (err != OGRERR_NONE)
        {
            cout << "error: [save]Failed to create feature in shapefile. err:" << to_string(err) << endl;
            return;
        }

        OGRFeature::DestroyFeature(poFeature);
    }
    GDALClose(poDS);
}
