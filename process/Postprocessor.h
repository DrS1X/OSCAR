#pragma once

#include "opt.h"
#include "_const.h"
#include <cmath>
#include <memory>
class Postprocessor {
public:
	static void Resort(vector<string> RepeatFileList, vector<string> OtherFileList, string mSaveFilePath);
};