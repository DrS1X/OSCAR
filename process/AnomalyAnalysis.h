#ifndef ANOMALYANALYSIS_H
#define ANOMALYANALYSIS_H

#include "_const.h"
#include "util.h"
#include "ThreadPool.h"
#include "tiffOpt.h"
#include "hdfOpt.h"

class AnomalyAnalysis
{
public:
	enum TimeScale { Month = 12, Day = 365, Test = 2 };

	static bool StandardAnomaly(vector<string> allFiles, string mOutPath, TimeScale timeScale);
	static bool StandardAnomaly_OnePeriod(vector<string> Files, Meta meta, string mOutPath);

	static bool SpatiotemporalAnomaly(vector<string> Files, string outputPath, float STDtime, bool generateHDF = true);
	static void SpatialSmooth(double* pResultBuffer, int mRows, int mCols, double mFillValue);
private:
	const static int BUFFER_SCALE = 365;
};
#endif