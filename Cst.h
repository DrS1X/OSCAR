#pragma once
#pragma warning (disable:4819)

#include <string>

using std::string;

const int Neighbor8[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};

const int N_DIM = 2;

const string H5_GROUP_NAME = "Grid";
const string H5_DATASET_NAME = "precipitation";
const string H5_SUFFIX = ".HDF5";
const string TIF_SUFFIX = ".tiff";
const string SHP_SUFFIX = ".shp";


const int NodeType_LeftTop = 1;
const int NodeType_RightTop = 2;
const int NodeType_LeftBot = 3;
const int NodeType_RightBot = 4;
const int NodeType_LeftTopAndRightBot = 5;
const int NodeType_RightTopAndLeftBot = 6;
const int NodeType_LeftRight = 7;
const int NodeType_TopBot = 8;

