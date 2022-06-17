#pragma once
#include <vector>
#include <string>
using namespace std;
class Convertor {

public:
	static void GeoTiff2HDF(vector<string> strFileList, string strSavePath, double startLat = 0, double endLat = 0, double startLog = 0, double endLog = 0);
	static bool Resample(vector<string> strFileList, string strSavePath, double targetResolution);
	static void SpaceTransform(vector<string> strFileList, string strSavePath);
};
