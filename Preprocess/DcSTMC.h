#pragma once
#include "ClsHDF4Operator.h"

class RoSTCM
{
public:
	int rsID = -1; //栅格像元ID
	int rsclusterId = -1; //所属聚类ID
	bool isKeyrs; //是否为核心对象
	bool Visited; //是否已访问 
	double Attribute = 0;//专题属性，即栅格值
	int t;//时间属性
	int x;
	int y;//空间属性
	int a;//转换属性（高值区为1，低值区为-1，其余为0）
	vector<int> neighborgrids; //时空领域数据点id列表

							   //RoSTCM() {}

	bool IsKey()
	{
		return this->isKeyrs;
	}

	//设置核心对象标志
	void SetKey(bool isKeyrs)
	{
		this->isKeyrs = isKeyrs;
	}

	//获取DpId方法
	int GetrsID()
	{
		return this->rsID;
	}

	//设置DpId方法
	void SetrsId(int rsID)
	{
		this->rsID = rsID;
	}

	//GetIsVisited方法
	bool isVisited()
	{
		return this->Visited;
	}

	//SetIsVisited方法
	void SetVisited(bool Visited)
	{
		this->Visited = Visited;
	}

	//GetClusterId方法
	long GetrsClusterId()
	{
		return this->rsclusterId;
	}

	//GetClusterId方法
	void SetrsClusterId(int rsclusterId)
	{
		this->rsclusterId = rsclusterId;
	}

	//GetArrivalPoints方法
	vector<int> Getneighborgrids()
	{
		return neighborgrids;
	}

	//~RoSTCM()
	//{

	//}
};

struct Param
{
	int startID;
	int endID;
	int mTempFileNum;
	int mRows;
	int mCols;
	//vector<RoSTCM> Rasterpixels;
	int drID;
	int rsclusterId;
	int minnum;

	int timesID;
	double mFillValue;
	double mScale;
	double ProgressingPer;
	int Index;
};

class DcSTMC {
private:
	int T = 2;
	int Neighborhood = 8;//空间邻域
	int mPerNum = 200;//每次聚类文件数
	int rsclusterId = 2;//簇编号
						//double NNeighbor = 0.3;//属性阈值

	int MinNum = 100; // 400;//猜想：改为（8*((mRows*mCols)/20000)*mFileNum）更好！
	int CP = 15;//核心点邻域个数阈值

	int mRows = Def.Rows;
	int mCols = Def.Cols;
	double mScale = Def.Scale;
	double mFillValue = Def.FillValue;
	Meta meta = Def;
	CClsHDF4Operator ho;

	void ExpandCluster1(vector<RoSTCM>& Rasterpixels, int drID, int rsclusterId, int row, int col, double avg, int num);
	void ClusterFilter(vector<RoSTCM>& Rasterpixels, int startID, int endID, int mTempFileNum, int size);
	void Lable(vector<RoSTCM>& Rasterpixels, int mTempFileNum);
	void OutputRasterpixels(vector<RoSTCM>& Rasterpixels, vector<string>& fileList, int mStartFileIndex, int mTempFileNum, string outputPath);
public:

	void Run(vector<string>& FileList, string outputPath, int in_mPerNum, int in_T);
};