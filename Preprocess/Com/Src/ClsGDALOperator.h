/*
利用GDAL库函数，处理其它遥感影像


*/
#pragma once

#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_srs_api.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "CONST.h"

using namespace std;

class CClsGDALOperator
{
public:
	CClsGDALOperator();
	CClsGDALOperator(Meta meta);
	static bool readGeoTiff(const char * in_fileName, double * pTiffData);
	static bool readGeoTiff(const string file, int* pBuffer);
	bool writeGeoTiff(string fileName, Meta meta, double *buf);
	bool writeGeoTiff(string fileName, Meta meta, int *buf);
	bool ReadFileByGDAL(const char* fileName); //读取 product HDF4文件
	bool WriteGeoTiffFileByGDAL(const char* fileName, double *pMemData);//将读入HDF4文件写为GeoTiff
	bool Convert_GeoTiff2HDF(const char * in_fileName, const char * out_fileName, double startLat = 0, double endLat = 0, double startLog = 0, double endLog = 0);

	const char* GetProjection(const char* fileName);//获取GeoTiff投影信息
	//argo
	//bool WriteVectorLineByGDAL(double X, double Y);
	//bool WriteVectorLineSHPByGDAL(long FID, QLineF Line);
	
public:

	long rowOff; //row offset
	long colOff;

	long iRow;
	long iCol;

	GDALDriver *geoTiffDriver;
	GDALDataset *pDataSet;
	GDALDataset *geoTiffDataset;

	double startLog = 0.f;
	double startLat = 0.f;
	double endLog = 0.f;
	double endLat = 0.f;
	double mScale = 0.001; //转product HDF时 使用0.01即可
	double mOffsets = 0.f;

	double maxValue = 0.f;
	double minValue = 0.f;
	double meanValue = 0.f;
	double stdValue = 0.f;

	int Cols = 0;
	int Rows = 0;

	double  fillValue = 0.f;
	double dsResolution = 1.f;

	string dataType = "";
	string productType = "";
	string dimension = "";
	string imgDate = "";
	string dsName = "";
	string projection = "";


	//long *pMemData;//用于存放数据
	double *pMemData;

	long *pGeoData;

	
};

