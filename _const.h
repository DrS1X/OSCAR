#pragma once
#pragma warning (disable:4819)

#include <string>
#include <chrono>
#include <hdf5.h>
#include <H5Cpp.h>

using std::string;

const float FILL_VAL = -9999.9;

const int Neighbor8[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};

const int N_DIM = 2;

const int NodeType_LeftTop = 1;
const int NodeType_RightTop = 2;
const int NodeType_LeftBot = 3;
const int NodeType_RightBot = 4;
const int NodeType_LeftTopAndRightBot = 5;
const int NodeType_RightTopAndLeftBot = 6;
const int NodeType_LeftRight = 7;
const int NodeType_TopBot = 8;

extern bool DEBUG;

