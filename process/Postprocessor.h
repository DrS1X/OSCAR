#pragma once

#include "util/util.h"
#include "_const.h"
#include <cmath>
#include <memory>
#include "DataModel.h"
#include "TifOpt.h"

class Postprocessor {
public:
	static void Resort(vector<string> RepeatFileList, vector<string> OtherFileList, string mSaveFilePath);
};