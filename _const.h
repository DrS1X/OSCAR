#pragma once

#include "string"
#include <chrono>

using namespace std;


struct Meta {
	double Resolution;
	double Scale;
	double MissingValue;
	double FillValue;
	long Rows;
	long Cols;
	long Size;
	double StartLat;
	double StartLon; // -> -180 ?
	double EndLat;
	double EndLon; // -> 180 ?
	double Offset;
	string Date;
	string DataSetName;
	string ProductType;
	string Dimension;
	string DataType;
	string Projection;
};

static const Meta Def{ 1.0, 0.001, -9999, -9999, 120, 360, 120 * 360, 60, -180, -60, 180, 0, "", "DataSet", "Product", "2?" , "", "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",180.0],UNIT[\"Degree\",0.0174532925199433]]" } ;
//"GEOGCS[\"WGS 84\", DATUM[\"WGS_1984\", SPHEROID[\"WGS 84\", 6378137, 298.257223563, AUTHORITY[\"EPSG\", \"7030\"]], AUTHORITY[\"EPSG\", \"6326\"]], PRIMEM[\"Greenwich\", 180], UNIT[\"degree\", 0.0174532925199433], AUTHORITY[\"EPSG\", \"4326\"]]" };
//const Meta Test{ 1.0, 0.001, -9999, -9999, 3, 3, 3 * 3, 0, 0, 3, 3, 0, "", "DataSet", "Product", "2?" , "", "" };
//const Meta Def = Test;

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

const int POLYGON_FIELD_EMPTY = -1;

