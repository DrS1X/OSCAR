#include "opt.h"

class RoSTCM
{
public:
	int rsID = -1; //դ����ԪID
	int rsclusterId = -1; //��������ID
	bool isKeyrs; //�Ƿ�Ϊ���Ķ���
	bool Visited; //�Ƿ��ѷ��� 
	double Attribute = 0;//ר�����ԣ���դ��ֵ
	int t;//ʱ������
	int x;
	int y;//�ռ�����
	int a;//ת�����ԣ���ֵ��Ϊ1����ֵ��Ϊ-1������Ϊ0��
	vector<int> neighborgrids; //ʱ���������ݵ�id�б�

							   //RoSTCM() {}

	bool IsKey()
	{
		return this->isKeyrs;
	}

	//���ú��Ķ����־
	void SetKey(bool isKeyrs)
	{
		this->isKeyrs = isKeyrs;
	}

	//��ȡDpId����
	int GetrsID()
	{
		return this->rsID;
	}

	//����DpId����
	void SetrsId(int rsID)
	{
		this->rsID = rsID;
	}

	//GetIsVisited����
	bool isVisited()
	{
		return this->Visited;
	}

	//SetIsVisited����
	void SetVisited(bool Visited)
	{
		this->Visited = Visited;
	}

	//GetClusterId����
	long GetrsClusterId()
	{
		return this->rsclusterId;
	}

	//GetClusterId����
	void SetrsClusterId(int rsclusterId)
	{
		this->rsclusterId = rsclusterId;
	}

	//GetArrivalPoints����
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
	int Neighborhood = 8;//�ռ�����
	int mPerNum = 200;//ÿ�ξ����ļ���
	int rsclusterId = 2;//�ر��
						//double NNeighbor = 0.3;//������ֵ

	int MinNum = 100; // 400;//���룺��Ϊ��8*((mRows*mCols)/20000)*mFileNum�����ã�
	int CP = 15;//���ĵ����������ֵ

	int mRows = Def.Rows;
	int mCols = Def.Cols;
	double mScale = Def.Scale;
	double mFillValue = Def.FillValue;
	Meta meta = Def;
	hdfOpt ho;

	void ExpandCluster1(vector<RoSTCM>& Rasterpixels, int drID, int rsclusterId, int row, int col, double avg, int num);
	void ClusterFilter(vector<RoSTCM>& Rasterpixels, int startID, int endID, int mTempFileNum, int size);
	void Lable(vector<RoSTCM>& Rasterpixels, int mTempFileNum);
	void OutputRasterpixels(vector<RoSTCM>& Rasterpixels, vector<string>& fileList, int mStartFileIndex, int mTempFileNum, string outputPath);
public:

	void Run(vector<string>& FileList, string outputPath, int in_mPerNum, int in_T);
};