#pragma once
#include <vector>
#include <string>
#include "util/util.h"
#include "Convertor.h"
#include <direct.h>
#include "_const.h"
#include "hdfOpt.h"
#include "TifOpt.h"

using namespace std;
class Convertor {
public:
	static void GeoTiff2HDF(vector<string> strFileList, string strSavePath, double startLat = 0, double endLat = 0, double startLog = 0, double endLog = 0);
	static bool ResampleBatch(vector<string> strFileList, string strSavePath, double targetResolution);
    static void Resample(float *src, float *tar, int srcRows, int srcCols, float ratio);
	static void SpaceTransform(vector<string> strFileList, string strSavePath);
};
