#include <cmath>
#include <memory>
#include "Algo.h"

using namespace std;

void DcSTMC::ExpandCluster1(vector<RoSTCM>& Rasterpixels, int drID, int rsclusterId, int row, int col, double avg, int num)
{
	//��������ֵ�����ݹ�
	RoSTCM scr = Rasterpixels[drID];
	if (!scr.IsKey()) return;
	vector<int> arrvals = scr.Getneighborgrids();//��ȡ�����
	double r = 0;//�¼���դ����������ֵ��ԭ�д����Ծ�ֵ�Ĳ�
	for (int i = 0; i < arrvals.size(); i++)
	{
		//�ж�ʱ������Ҫ���Ƿ�������
		RoSTCM & des = Rasterpixels[arrvals[i]];//��ȡ��i�������
		r = abs(des.Attribute - avg);
		if (!des.isVisited() && r <= V_THRESHOLD)
		{
			//��ʼ����
			des.SetrsClusterId(rsclusterId);
			des.SetVisited(true);
			num = num + 1;
			avg = (avg + des.Attribute) / num; //���¼���ؾ�ֵ ����
		}
	}
	for (int i = 0; i < arrvals.size(); i++)
	{
		RoSTCM & des = Rasterpixels[arrvals[i]];
		//double s =0;
		if (des.isVisited() && des.IsKey() && des.a != 1 )
			/*&& abs(
			(Rasterpixels[des.rsID + T * col * row].Attribute 
				+ Rasterpixels[des.rsID - T * col * row].Attribute) / 2 
				- (avg * num - des.Attribute) / (num - 1)
			) <= 2)*/
		{
			des.a = 1;//a=1����õ��Ѿ������࣬��ֹ������ѭ������ջ���
			ExpandCluster1(Rasterpixels, arrvals[i], rsclusterId, row, col, avg, num);//��չ�ô�
		}
	}
	//GC.Collect();
}

void DcSTMC::ClusterFilter(vector<RoSTCM>& Rasterpixels, int startID, int endID, int mTempFileNum, int size)
{
	for (int l = startID; l <= endID; l++)
	{
		int idnum = 0;
		for (int m = 0; m < mTempFileNum * size; m++)
		{
			if (Rasterpixels[m].rsclusterId == l)
			{
				idnum = idnum + 1;
			}
		}
		if (idnum <= MinNum)
		{
			for (int m = 0; m < mTempFileNum * size; m++)
			{
				if (Rasterpixels[m].rsclusterId == l)
				{
					Rasterpixels[m].SetrsClusterId(0);
				}
			}
			//rsclusterIdnum = rsclusterIdnum - 1;
		}
	}
}

void DcSTMC::Lable(vector<RoSTCM>& Rasterpixels, int mTempFileNum) {
	unique_ptr<double[]> TT(new double[2 * T]);//ʱ���ھӣ���ʱ���������Դ������
	unique_ptr<int[]> TID(new int[2 * T]);//ʱ���ھӣ���ʱ������դ��ID�������
	unique_ptr<double[]> NN(new double[(2 * T + 1) * (Neighborhood + 1) - 1]);//ʱ���ھ����Դ������
	unique_ptr<int[]> ID(new int[(2 * T + 1) * (Neighborhood + 1) - 1]);//ʱ���ھ�ID�������

	for (int j = 0; j < mTempFileNum * mRows * mCols; j++)
	{
		if (Rasterpixels[j].Attribute == mFillValue * mScale)
		{
			Rasterpixels[j].SetVisited(true);
			Rasterpixels[j].SetKey(false);
			Rasterpixels[j].SetrsClusterId((int)mFillValue);
		}
		else if (Rasterpixels[j].Attribute == 0)
		{
			Rasterpixels[j].SetVisited(true);
			Rasterpixels[j].SetKey(false);
			Rasterpixels[j].SetrsClusterId(0);
		}
		else if (Rasterpixels[j].x == 0 || Rasterpixels[j].x == mRows - 1 || Rasterpixels[j].y == 0 || Rasterpixels[j].y == mCols - 1)
			Rasterpixels[j].SetKey(false);

		//时空立方体
		else if (Rasterpixels[j].t >= T && Rasterpixels[j].t <= mTempFileNum - T - 1)
		{
			for (int n = 0; n < T; n++)
			{
                TID[n * 2] = Rasterpixels[j - (n + 1) * mRows * mCols].rsID;
                TT[n * 2] = Rasterpixels[j - (n + 1) * mRows * mCols].Attribute;
                TID[n * 2 + 1] = Rasterpixels[j + (n + 1) * mRows * mCols].rsID;
                TT[n * 2 + 1] = Rasterpixels[j + (n + 1) * mRows * mCols].Attribute;
			}

			for (int tt = 0; tt < T; tt++)
			{
				ID[tt] = TID[tt];
				NN[tt] = TT[tt];
			}

			{
				ID[T] = Rasterpixels[j - 1].rsID;
				ID[T + 1] = Rasterpixels[j + 1].rsID;
				ID[T + 2] = Rasterpixels[j - mCols].rsID;
				ID[T + 3] = Rasterpixels[j - mCols - 1].rsID;
				ID[T + 4] = Rasterpixels[j - mCols + 1].rsID;
				ID[T + 5] = Rasterpixels[j + mCols].rsID;
				ID[T + 6] = Rasterpixels[j + mCols - 1].rsID;
				ID[T + 7] = Rasterpixels[j + mCols + 1].rsID;
				NN[T] = Rasterpixels[j - 1].Attribute;
				NN[T + 1] = Rasterpixels[j + 1].Attribute;
				NN[T + 2] = Rasterpixels[j - mCols].Attribute;
				NN[T + 3] = Rasterpixels[j - mCols - 1].Attribute;
				NN[T + 4] = Rasterpixels[j - mCols + 1].Attribute;
				NN[T + 5] = Rasterpixels[j + mCols].Attribute;
				NN[T + 6] = Rasterpixels[j + mCols - 1].Attribute;
				NN[T + 7] = Rasterpixels[j + mCols + 1].Attribute;
			}

            for (int tt = 0; tt < T * 2; tt++)
            {
                ID[T + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - 1].rsID;
                ID[T + 1 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + 1].rsID;
                ID[T + 2 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols].rsID;
                ID[T + 3 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols - 1].rsID;
                ID[T + 4 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols + 1].rsID;
                ID[T + 5 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols].rsID;
                ID[T + 6 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols - 1].rsID;
                ID[T + 7 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols + 1].rsID;

                NN[T + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - 1].Attribute;
                NN[T + 1 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + 1].Attribute;
                NN[T + 2 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols].Attribute;
                NN[T + 3 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols - 1].Attribute;
                NN[T + 4 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols + 1].Attribute;
                NN[T + 5 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols].Attribute;
                NN[T + 6 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols - 1].Attribute;
                NN[T + 7 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols + 1].Attribute;
            }
            for (int tt = 0; tt < T; tt++)
            {
                ID[Neighborhood * (T * 2 + 1) + T + tt] = TID[T + tt];
                NN[Neighborhood * (T * 2 + 1) + T + tt] = TT[T + tt];
            }


			//检验时间相邻的栅格是否有值
            if (TT[0] == mFillValue * mScale || TT[1] == mFillValue * mScale || TT[0] == 0 || TT[1] == 0){

            }else {
                for (int k = 0; k < (2 * T + 1) * (Neighborhood + 1) - 1; k++) {

				    if (NN[k] == mFillValue * mScale || NN[k] == 0)
					    continue;

                    Rasterpixels[j].neighborgrids.push_back(ID[k]);
                }
            }

			//若连续2*T+1个有效值，则标记为聚类核心
			RoSTCM srcDr = Rasterpixels[j];
			vector<int> nb = srcDr.Getneighborgrids();

			int c = 0; int d = 0;
			for (int r = 0; r < nb.size(); r++)
			{
				RoSTCM desDr = Rasterpixels[nb[r]]; //获取领域内所有数据点
				if (desDr.rsID == Rasterpixels[j].rsID - T * mRows * mCols)
				{
					c = 1;
				}
				if (desDr.rsID == Rasterpixels[j].rsID + T * mRows * mCols)
				{
					d = 1;
				}
			}

			if (Rasterpixels[j].Getneighborgrids().size() >= CP && c == 1 && d == 1)
				Rasterpixels[j].SetKey(true);
			else
				Rasterpixels[j].SetKey(false);
		}
	}
}

pair<vector<string>, vector<string>> DcSTMC::Run(vector<string>& FileList, string outputPath, int in_mPerNum, int in_T) {
    set<string> repeatFileSet;
    set<string> otherFileSet;

	this->mPerNum = in_mPerNum;
	this->T = in_T;
	string repeatFilePath = outputPath + "\\RepeatFile";
	string otherFilePath = outputPath + "\\OtherFile";
    CheckFolderExist(repeatFilePath);
    CheckFolderExist(otherFilePath);
    repeatFilePath += "\\";
    otherFilePath += "\\";

	int mFileNum = FileList.size();
	vector<RoSTCM> Rasterpixels;
	unique_ptr<int[]> outt(new int[mRows * mCols]);

	for (int i = 0; i < mFileNum / (mPerNum - T * 2) + 1; i++)
	{
		if (mPerNum == mFileNum && i > 0)//һ���Դ���
			continue;

		//input
		int mStartFileIndex = i * (mPerNum - T * 2);//�������ξ�������ʱ�䴰���ظ�
		if (mStartFileIndex < 0)
			mStartFileIndex = 0;
		int mEndFileIndex = mStartFileIndex + mPerNum;
		if (mEndFileIndex >= mFileNum)
			mEndFileIndex = mFileNum;
		int mTempFileNum = mEndFileIndex - mStartFileIndex;
		if (mTempFileNum <= 0)
		{
			mEndFileIndex = mFileNum;
			mStartFileIndex = mEndFileIndex - 2 * T;
			mTempFileNum = 2 * T;
		}
		for (int j = mStartFileIndex; j < mEndFileIndex; j++)
		{
			int* pBuffer = new int[mRows * mCols];
            Tif::readInt(FileList[j],pBuffer);
			for (int k = 0; k < mRows * mCols; k++)
			{
				RoSTCM rsets;
				rsets.Attribute = pBuffer[k] * mScale;
				rsets.x = k / mCols; //row
				rsets.y = k % mCols; //col
				rsets.t = j - mStartFileIndex;
				rsets.a = 0;
				rsets.SetrsId(k + (j - mStartFileIndex) * mRows * mCols);
				rsets.SetVisited(false);
				rsets.SetKey(false);
				rsets.SetrsClusterId(-1);
				Rasterpixels.emplace_back(rsets);
			}
			delete[] pBuffer;
		}
		//input

		//LabelData (attribute, row, col, t, visited (false), key(false),clusterID(0));
		this->Lable(Rasterpixels, mTempFileNum);
		//LabelData

		OutputRasterpixels(Rasterpixels, FileList, mStartFileIndex, mTempFileNum, "E:\\pr\\pos2015_2016\\mon\\old\\tmp\\key");

		//Clustering
		int mstartClusterID = rsclusterId;
		for (int j = 0; j < mTempFileNum * mRows * mCols; j++)
		{				
			if (!Rasterpixels[j].isVisited() && Rasterpixels[j].IsKey())
			{
				//Task
				{						
					Rasterpixels[j].SetrsClusterId(rsclusterId);
					Rasterpixels[j].SetVisited(true);
					Rasterpixels[j].a = 1;
					Param p;
					//p.Rasterpixels = Rasterpixels; �����Ϊ���û�ָ��
					p.drID = j; p.rsclusterId = rsclusterId;
					p.mRows = mRows; p.mCols = mCols;
					//thread(RTreeParam p)
					this->ExpandCluster1(Rasterpixels, p.drID, p.rsclusterId, p.mRows, p.mCols, Rasterpixels[p.drID].Attribute, 1);
				}
				rsclusterId++;
			}				
		}
		//Clustering

        OutputRasterpixels(Rasterpixels, FileList, mStartFileIndex, mTempFileNum, "E:\\pr\\pos2015_2016\\mon\\old\\tmp\\clusterBeforeFilter");

		//Filter small clusters
		while (mstartClusterID <= rsclusterId)
		{				
			int mendClusterID = mstartClusterID + 50;
			if (mendClusterID > rsclusterId)
				mendClusterID = rsclusterId;				
			this->ClusterFilter(Rasterpixels, mstartClusterID, mendClusterID, mTempFileNum, mRows * mCols);
			mstartClusterID = mendClusterID + 1;
		}
		//Filter small clusters

		//output
		for (int ii = 0; ii < mTempFileNum; ii++)
		{				
			for (int j = ii * mRows * mCols; j < (ii + 1) * mRows * mCols; j++)
			{
				outt[j - ii * mRows * mCols] = Rasterpixels[j].rsclusterId;//�Ӵ��������������ļ�
			}

            bool isRepeat = false;
			string mOutFileName = util::generateFileName(FileList[mStartFileIndex + ii], otherFilePath, "_Tcluster" + TIF_SUFFIX);
			if (mFileNum != mPerNum)
			{
				if (mTempFileNum - ii <= T * 2 && i != mFileNum / (mPerNum - T * 2)) {
                    mOutFileName = util::generateFileName(FileList[mStartFileIndex + ii], repeatFilePath,"_Tcluster" + TIF_SUFFIX);
                    isRepeat = true;
                }

                if (i != 0 && ii < T * 2) {
                    mOutFileName = util::generateFileName(FileList[mStartFileIndex + ii], repeatFilePath,"_Tcluster_2" + TIF_SUFFIX);
                    isRepeat = true;
                }
            }

            if(isRepeat)
                repeatFileSet.insert(mOutFileName);
            else
                otherFileSet.insert(mOutFileName);

			meta.date = GetDate(FileList[mStartFileIndex + ii]);

			Tif::writeInt(mOutFileName, meta, outt.get());
		}
		Rasterpixels.clear();
	}

    vector<string> repeatFileList, otherFileList;
    for(std::set<string>::iterator it = repeatFileSet.begin(); it != repeatFileSet.end(); ++it)
        repeatFileList.push_back(*it);
    for(std::set<string>::iterator it = otherFileSet.begin(); it != otherFileSet.end(); ++it)
        otherFileList.push_back(*it);

    pair<vector<string>, vector<string>> pr{repeatFileList, otherFileList};
    return pr;
}

void DcSTMC::OutputRasterpixels(vector<RoSTCM>& Rasterpixels, vector<string>& fileList, int mStartFileIndex, int mTempFileNum, string outputPath) {
    CheckFolderExist(outputPath);

	unique_ptr<int[]> outt(new int[mRows * mCols]);
	for (int ii = 0; ii < mTempFileNum; ii++)
	{
		for (int j = ii * mRows * mCols; j < (ii + 1) * mRows * mCols; j++)
		{
			outt[j - ii * mRows * mCols] = (int) Rasterpixels[j].GetrsClusterId();
		}
		string mOutFileName = util::generateFileName(fileList[mStartFileIndex + ii], outputPath, "cluster", TIF_SUFFIX);
		meta.date = GetDate(fileList[mStartFileIndex + ii]);

        Tif::writeInt(mOutFileName,meta,  outt.get());
	}
}
