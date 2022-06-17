#pragma once
#include <QStringList>
#define PI 3.14159265358979323846           //pi�Ĵ�С
#define EQUALR 6378.2                         //����뾶km
#define NoData -9999.0                      //��ֵ

//SST�������Сֵ
#define SSTMaxValue 32.0
#define SSTMinValue -2.0

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
	double StartLog; // -> -180 ?
	double EndLat;
	double EndLog; // -> 180 ?
	double Offset;
	string Date;
	string DataSetName;
	string ProductType;
	string Dimension;
	string DataType;
	string Projection;
};

const Meta Def{ 1.0, 0.001, -9999, -9999, 120, 360, 120 * 360, 60, -180, -60, 180, 0, "", "DataSet", "Product", "2ά" , "", "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",180.0],UNIT[\"Degree\",0.0174532925199433]]" } ;
//"GEOGCS[\"WGS 84\", DATUM[\"WGS_1984\", SPHEROID[\"WGS 84\", 6378137, 298.257223563, AUTHORITY[\"EPSG\", \"7030\"]], AUTHORITY[\"EPSG\", \"6326\"]], PRIMEM[\"Greenwich\", 180], UNIT[\"degree\", 0.0174532925199433], AUTHORITY[\"EPSG\", \"4326\"]]" };
//const Meta Test{ 1.0, 0.001, -9999, -9999, 3, 3, 3 * 3, 0, 0, 3, 3, 0, "", "DataSet", "Product", "2ά" , "", "" };
//const Meta Def = Test;

enum processType
{
	HDF42ASC, 
	HDF42PRODUCT,
	HDF52PRODUCT, 
	HDF42NETCDF, 
	HDF42GEOTIFF, 
	PositionDataConvert,
	HDF42DATONONE,
	GEOTIFF2HDF4PRODUCT,
	YEAR_ACCUMULATION, 
	MONTH_ACCUMULATION, 
	DAY_ACCUMULATION, 
	SEASON_ACCUMULATION,
	NETCDF2HDF4,
	OldHDF2NewHDF,
	ImageDateStandard_UPC,
	MaskErase,
	SpaceTrans,
	SEASONCELL_STRUCTURE,
	YEARCELL_STRUCTURE,
	MONTHCELL_STRUCTURE,
	NCtoCSV_Argo,
	CSVtoHDF_Argo,
	NCtoInfoCSV_Argo,
    InfoCSVtoHDF_Argo,
    NCtoCycleCSV_Argo,
    CycleCSVtoHDF_Argo
};

enum MarineParameter
{
	SST, 
	SSP, 
	SSS, 
	SLA, 
	SSW, 
	CHL, 
	NPP, 
	LOCAL, 
	NO_Parameter,
	QuikSCAT,
	MultiSatelliteFusion,
	CCMP,SeaWindFusionV07
};

enum DatasetType
{
	GLOBE_2D_S_N_NORMAL_MIDDLE,	  //ȫ��2ά�ϱ�̫ƽ���м�;
	GLOBE_2D_S_N_NORMAL_SPLIT,	  //ȫ��2ά�ϱ�̫ƽ��ֿ�;
	GLOBE_2D_S_N_EXCHANGE_MIDDLE, //ȫ��2ά�ϱ��Ե�̫ƽ���м�;
	GLOBE_2D_S_N_EXCHANGE_SPLIT,  //ȫ��2ά�ϱ��Ե�̫ƽ��ֿ�;
	GLOBE_3D_S_N_NORMAL_MIDDLE,	  //ȫ��3ά�ϱ�̫ƽ���м�
	GLOBE_3D_S_N_NORMAL_SPLIT,	  //ȫ��3ά�ϱ�̫ƽ��ֿ�
	GLOBE_3D_S_N_EXCHANGE_MIDDLE, //ȫ��3ά�ϱ��Ե�̫ƽ���м�
	GLOBE_3D_S_N_EXCHANGE_SPLIT,  //ȫ��3ά�ϱ��Ե�̫ƽ��ֿ�
	PART_2D,					  //����2ά
	PART_3D,					  //����3ά
	NO_Type,					     //ȱʡ
	GLOBE_2D_LON_LAT_NORMAL_SPLIT //ȫ��γ�Ե���̫ƽ��ֿ�
};

enum DataTypeSize     //�������͵Ĵ�С
{
	INTTYPE, 
	LONGTYPE, 
	FLOATTYPE, 
	DOUBLETYPE
};

enum AnalysisType    //դ�����
{
	Log, 
	Exponential, 
	trigonometric,
	SpaceNeighbourhood,
	TimeNeighbourhood,
	SpaceTimeNeighbourhood
};

enum SpaceTimeAnalysisType
{
	SpaceCountTimeSeries, 
	TimeMeanSpaceSpread, 
	LongitudeMeanLatitudeTimeSeries, 
	LatitudeMeanLongitudeTimeSeries
};

enum DataOperateType
{
	TimeSeriesDiscretization_one, 
	TimeSeriesDiscretization_two,
	SpaceDiscretization
};

enum AnomalyAnalysisType
{
	SpaceAnomaly, 
	TimeAnomaly, 
	TimeSeriesAnomaly, 
	StandardAnomaly,
	MonthlyAnomaly
};

enum SpaceTimetransformType
{
	SpaceTimeInsert, 
	SpaceInsert, 
	TimeSeriesInsert, 
	SpaceTimeResample,
	SpaceResample, 
	TimeResample,
	FillTimeResample,
	FillSpaceResample,
	FillSpaceTimeResample,
	SpatialResampleBasedOnUnevenLat
};

enum RelationshipAnalysisType
{
	SpaceSelfRelationshipAnalysis,
	SpaceTimeSelfRelationshipAnalysis,
	MuchFeatureRelationshipAnalysis,
	SpaceTimeTrendAnalysis
};


