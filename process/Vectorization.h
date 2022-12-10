#pragma once
#include <string>
#include <vector>
#include <array>
#include <float.h>
#include <cmath>
#include <memory>
#include <array>
#include <ogrsf_frmts.h>
#include <ogr_spatialref.h>
#include "util/util.h"
#include "TifOpt.h"
#include "_const.h"
#include "hdfOpt.h"

using namespace std;

void Vectorization(vector<string>& oriFileNames, vector<string>& spFileNames, string outFolderPath);