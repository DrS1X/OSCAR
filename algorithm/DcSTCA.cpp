
#include "Algo.h"

using namespace std;


void DcSTCA::ExpandCluster(vector<Pixel> &Rasterpixels, int drID, int cid, int row, int col) {
    Pixel &scr = Rasterpixels[drID];
    if (!scr.IsKey() || scr.a == 1) return;
    scr.a = 1; // a=1, flag
    clusters[cid]->expand(scr.Attribute);
    vector<int> arrvals = scr.Getneighborgrids();
    double diff = 0;
    for (int i = 0; i < arrvals.size(); i++) {
        Pixel &des = Rasterpixels[arrvals[i]];
        diff = ::fabs(des.Attribute - clusters[cid]->avg);
        if (!des.isVisited() && diff <= valueThreshold) {
            des.SetrsClusterId(cid);
            des.SetVisited(true);
            clusters[cid]->expand(des.Attribute);

            if (des.IsKey() && des.a != 1) {
                ExpandCluster(Rasterpixels, arrvals[i], cid, row, col);
            }
        }
    }
}

void DcSTCA::ClusterFilter(vector<Pixel> &Rasterpixels, int startID, int endID, int mTempFileNum, int size) {
    for (int l = startID; l <= endID; l++) {
        int idnum = 0;
        for (int m = 0; m < mTempFileNum * size; m++) {
            if (Rasterpixels[m].cid == l) {
                idnum = idnum + 1;
            }
        }
        if (idnum <= MinNum) {
            for (int m = 0; m < mTempFileNum * size; m++) {
                if (Rasterpixels[m].cid == l) {
                    Rasterpixels[m].SetrsClusterId(0);
                }
            }
            //rsclusterIdnum = rsclusterIdnum - 1;
        }
    }
}

void DcSTCA::Lable(vector<Pixel> &Rasterpixels, int mTempFileNum) {
    unique_ptr<double[]> TT(new double[2 * T]);//ʱ���ھӣ���ʱ���������Դ������
    unique_ptr<int[]> TID(new int[2 * T]);//ʱ���ھӣ���ʱ������դ��ID�������
    unique_ptr<double[]> NN(new double[(2 * T + 1) * (Neighborhood + 1) - 1]);//ʱ���ھ����Դ������
    unique_ptr<int[]> ID(new int[(2 * T + 1) * (Neighborhood + 1) - 1]);//ʱ���ھ�ID�������

    for (int j = 0; j < mTempFileNum * mRows * mCols; j++) {
        if (Rasterpixels[j].Attribute == mFillValue * mScale) {
            Rasterpixels[j].SetVisited(true);
            Rasterpixels[j].SetKey(false);
            Rasterpixels[j].SetrsClusterId((int) mFillValue);
        } else if (Rasterpixels[j].Attribute == 0) {
            Rasterpixels[j].SetVisited(true);
            Rasterpixels[j].SetKey(false);
            Rasterpixels[j].SetrsClusterId(0);
        } else if (Rasterpixels[j].x == 0 || Rasterpixels[j].x == mRows - 1 || Rasterpixels[j].y == 0 ||
                   Rasterpixels[j].y == mCols - 1)
            Rasterpixels[j].SetKey(false);

            //时空立方体
        else if (Rasterpixels[j].t >= T && Rasterpixels[j].t <= mTempFileNum - T - 1) {
            for (int n = 0; n < T; n++) {
                TID[n * 2] = Rasterpixels[j - (n + 1) * mRows * mCols].pid;
                TT[n * 2] = Rasterpixels[j - (n + 1) * mRows * mCols].Attribute;
                TID[n * 2 + 1] = Rasterpixels[j + (n + 1) * mRows * mCols].pid;
                TT[n * 2 + 1] = Rasterpixels[j + (n + 1) * mRows * mCols].Attribute;
            }

            for (int tt = 0; tt < T; tt++) {
                ID[tt] = TID[tt];
                NN[tt] = TT[tt];
            }

            {
                ID[T] = Rasterpixels[j - 1].pid;
                ID[T + 1] = Rasterpixels[j + 1].pid;
                ID[T + 2] = Rasterpixels[j - mCols].pid;
                ID[T + 3] = Rasterpixels[j - mCols - 1].pid;
                ID[T + 4] = Rasterpixels[j - mCols + 1].pid;
                ID[T + 5] = Rasterpixels[j + mCols].pid;
                ID[T + 6] = Rasterpixels[j + mCols - 1].pid;
                ID[T + 7] = Rasterpixels[j + mCols + 1].pid;
                NN[T] = Rasterpixels[j - 1].Attribute;
                NN[T + 1] = Rasterpixels[j + 1].Attribute;
                NN[T + 2] = Rasterpixels[j - mCols].Attribute;
                NN[T + 3] = Rasterpixels[j - mCols - 1].Attribute;
                NN[T + 4] = Rasterpixels[j - mCols + 1].Attribute;
                NN[T + 5] = Rasterpixels[j + mCols].Attribute;
                NN[T + 6] = Rasterpixels[j + mCols - 1].Attribute;
                NN[T + 7] = Rasterpixels[j + mCols + 1].Attribute;
            }

            for (int tt = 0; tt < T * 2; tt++) {
                ID[T + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - 1].pid;
                ID[T + 1 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + 1].pid;
                ID[T + 2 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols].pid;
                ID[T + 3 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols - 1].pid;
                ID[T + 4 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols + 1].pid;
                ID[T + 5 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols].pid;
                ID[T + 6 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols - 1].pid;
                ID[T + 7 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols + 1].pid;

                NN[T + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - 1].Attribute;
                NN[T + 1 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + 1].Attribute;
                NN[T + 2 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols].Attribute;
                NN[T + 3 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols - 1].Attribute;
                NN[T + 4 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] - mCols + 1].Attribute;
                NN[T + 5 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols].Attribute;
                NN[T + 6 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols - 1].Attribute;
                NN[T + 7 + Neighborhood * (tt + 1)] = Rasterpixels[TID[tt] + mCols + 1].Attribute;
            }
            for (int tt = 0; tt < T; tt++) {
                ID[Neighborhood * (T * 2 + 1) + T + tt] = TID[T + tt];
                NN[Neighborhood * (T * 2 + 1) + T + tt] = TT[T + tt];
            }


            //检验时间相邻的栅格是否有值
            if (TT[0] == mFillValue * mScale || TT[1] == mFillValue * mScale || TT[0] == 0 || TT[1] == 0) {

            } else {
                for (int k = 0; k < (2 * T + 1) * (Neighborhood + 1) - 1; k++) {

                    if (NN[k] == mFillValue * mScale || NN[k] == 0 ||
                        fabs(NN[k] - Rasterpixels[j].Attribute) > valueThreshold)
                        continue;

                    Rasterpixels[j].neighborGrids.push_back(ID[k]);
                }
            }

            //若连续2*T+1个有效值，则标记为聚类核心
            Pixel srcDr = Rasterpixels[j];
            vector<int> nb = srcDr.Getneighborgrids();

            int c = 0;
            int d = 0;
            for (int r = 0; r < nb.size(); r++) {
                Pixel desDr = Rasterpixels[nb[r]]; //获取领域内所有数据点
                if (desDr.pid == Rasterpixels[j].pid - T * mRows * mCols) {
                    c = 1;
                }
                if (desDr.pid == Rasterpixels[j].pid + T * mRows * mCols) {
                    d = 1;
                }
            }

            if (Rasterpixels[j].Getneighborgrids().size() >= coreThreshold && c == 1 && d == 1)
                Rasterpixels[j].SetKey(true);
            else
                Rasterpixels[j].SetKey(false);
        }
    }
}


int Split(const string& str, const char split, vector<string>& res)
{
    if (str == "")		return -1;
    string strs = str + split;
    size_t prev = 0;
    size_t pos = strs.find_first_of(split);

    while (pos != strs.npos)
    {
        string temp = strs.substr(prev, pos - prev);
        res.push_back(temp);
        prev = pos + 1;
        pos = strs.find(split, pos + 1);
    }
    return res.size();
}

void Fill(vector<string>& mSameList, int* pBuffer, int mRows, int mCols)
{
    for (int i = 0; i < mSameList.size(); i++)
    {
        vector<string> strID;
        int Num = Split(mSameList[i], '=', strID);
        vector<int> ID(Num);

        int MinID = 1000000;
        for (int j = 0; j < Num; j++)
        {
            ID[j] = atoi(strID[j].c_str());
            if (ID[j] < MinID)
                MinID = ID[j];
        }

        for (int j = 0; j < mRows * mCols; j++)
        {
            if (pBuffer[j] == -1)
            {
                pBuffer[j] = 0;
                continue;
            }
            if (pBuffer[j] <= MinID)
                continue;
            for (int k = 0; k < Num; k++)
            {
                if (pBuffer[j] == ID[k])
                {
                    pBuffer[j] = MinID;
                    break;
                }
            }
        }
    }
}

void GetSameEvent(string& str, const vector<string>& mIDList, vector<bool>& Event, int Index)
{
    vector<string> temp1(2);
    vector<string> temp2(2);
    temp1[0] = mIDList[Index].substr(0, mIDList[Index].find('='));
    temp1[1] = mIDList[Index].substr(mIDList[Index].find('=') + 1);
    if (!Event[Index])
        str = str + mIDList[Index];
    Event[Index] = true;
    for (int i = 0; i < mIDList.size(); i++)
    {
        if (Event[i] || i == Index)
            continue;

        temp2[0] = mIDList[i].substr(0, mIDList[i].find('='));
        temp2[1] = mIDList[i].substr(mIDList[i].find('=') + 1);
        int a = 0;
        if (temp1[0] == temp2[0] || temp1[1] == temp2[0])
        {
            str = str + "=" + temp2[1];
            a = 1;
            Event[i] = true;
        }
        if (temp1[0] == temp2[1] || temp1[1] == temp2[1])
        {
            str = str + "=" + temp2[0];
            a = 1;
            Event[i] = true;
        }
        if (a == 1)
            GetSameEvent(str, mIDList, Event, i);
    }
}

vector<string> Resort(vector<string> RepeatFileList, vector<string> OtherFileList, string mSaveFilePath)
{
    vector<string> outputFileList;
    mSaveFilePath += "\\sorted";
    CheckFolderExist(mSaveFilePath);

    int mOtherFileNum = OtherFileList.size();
    int mCols = Meta::DEF.nCol;
    int mRows = Meta::DEF.nRow;
    double mScale = Meta::DEF.scale;
    double mResolution = Meta::DEF.resolution;
    double mFillValue = Meta::DEF.fillValue;

    unique_ptr<int[]> pBuffer1(new int[mRows * mCols]);
    unique_ptr<int[]> pBuffer2(new int[mRows * mCols]);
    vector<string> mIDList;//�ظ��ļ���ͬλ���ϵ�ͳ��
    vector<string> mIDList2;//�ռ������ڴص�ͳ�ƽ���������ظ��ļ��ϲ��󡢺ϲ�ǰ�Լ������ļ���ͳ��
    int Neibor = 3;//�����С��5x5
    vector<int> NeiborID;
    vector<vector<int>> Image(mRows);
    for (int i = 0; i < mRows; i++){
        vector<int> tmp(mCols);
        Image[i] = tmp;
    }

    int mRepeatFileNum = RepeatFileList.size();

    if (mRepeatFileNum != 0)
    {
        int CountNum = 0;//λ����ͬ�¼���Ų�ͬ��դ�������������դ����

        //ͳ��2���ظ��ļ��ϵ��ص�cluster
        for (int i = 0; i < mRepeatFileNum / 2; i++)
        {
            Tif::readInt(RepeatFileList[2 * i], pBuffer1.get());
            Tif::readInt(RepeatFileList[2 * i + 1], pBuffer2.get());
            for (int j = 0; j < mRows * mCols; j++)
            {
                if (pBuffer1[j] != pBuffer2[j] && pBuffer1[j] > 1 && pBuffer2[j] > 1)
                {
                    //shit code, why not use SET
                    string str = to_string(pBuffer1[j]) + "=" + to_string(pBuffer2[j]);
                    if (mIDList.size() == 0)
                    {
                        mIDList.push_back(str);
                        CountNum++;
                        continue;
                    }
                    bool isIn = false;
                    for (int k = 0; k < mIDList.size(); k++)
                    {
                        if (str == mIDList[k])
                        {
                            isIn = true;
                            break;
                        }
                    }
                    if (!isIn)
                    {
                        CountNum++;
                        mIDList.push_back(str);
                    }
                }
            }
        }

        //ͳ��2���ظ��ļ��пռ����ڵ�cluster
        for (int i = 0; i < mRepeatFileNum / 2; i++)
        {
            Tif::readInt(RepeatFileList[2 * i], pBuffer1.get());
            Tif::readInt(RepeatFileList[2 * i + 1], pBuffer2.get());
            for (int j = 0; j < mRows * mCols; j++)
            {
                if (pBuffer1[j] < 2 && pBuffer2[j] >= 2)
                    pBuffer1[j] = pBuffer2[j];

                int row = j / mCols;
                int col = j % mCols;
                Image[row][col] = pBuffer1[j];
            }

            for (int m = 0; m < mRows; m++)
            {
                if (m < Neibor / 2 || mRows - m <= Neibor / 2)
                    continue;
                for (int n = 0; n < mCols; n++)
                {
                    if (n < Neibor / 2 || mCols - n <= Neibor / 2 || Image[m][n] < 2)
                        continue;

                    //������ͳ��
                    for (int x = m - Neibor / 2; x <= m + Neibor / 2; x++)
                    {
                        for (int y = n - Neibor / 2; y <= n + Neibor / 2; y++)
                        {
                            if (Image[x][y] < 2)
                                continue;
                            if (NeiborID.size() == 0)
                            {
                                NeiborID.push_back(Image[x][y]);
                                continue;
                            }
                            bool isIn = false;
                            for (int k = 0; k < NeiborID.size(); k++)
                            {
                                if (Image[x][y] == NeiborID[k])
                                {
                                    isIn = true;
                                    break;
                                }
                            }
                            if (!isIn)
                                NeiborID.push_back(Image[x][y]);
                        }
                    }
                    if (NeiborID.size() <= 1)
                    {
                        NeiborID.clear(); // careful
                        continue;
                    }
                    for (int k = 0; k < NeiborID.size() - 1; k++)
                    {
                        string str1 = to_string(NeiborID[k]) + "=" + to_string(NeiborID[k + 1]);
                        string str2 = to_string(NeiborID[k + 1]) + "=" + to_string(NeiborID[k]);
                        if (mIDList2.size() == 0)
                        {
                            mIDList2.push_back(str1);
                            continue;
                        }
                        bool isIn = false;
                        for (int j = 0; j < mIDList2.size(); j++)
                        {
                            if (str1 == mIDList2[j] || str2 == mIDList2[j])
                            {
                                isIn = true;
                                break;
                            }
                        }
                        if (!isIn)
                            mIDList2.push_back(str1);
                    }
                    NeiborID.clear(); // careful
                }
            }
        }

        //ͳ��1���ظ��ļ�������cluster
        for (int i = 0; i < mRepeatFileNum; i++)
        {
            Tif::readInt(RepeatFileList[i], pBuffer1.get());
            for (int j = 0; j < mRows * mCols; j++)
            {
                int row = j / mCols;
                int col = j % mCols;
                Image[row][col] = pBuffer1[j];
            }

            for (int m = 0; m < mRows; m++)
            {
                if (m < Neibor / 2 || mRows - m <= Neibor / 2)
                    continue;
                for (int n = 0; n < mCols; n++)
                {
                    if (n < Neibor / 2 || mCols - n <= Neibor / 2 || Image[m][n] < 2)
                        continue;

                    //������ͳ��
                    for (int x = m - Neibor / 2; x <= m + Neibor / 2; x++)
                    {
                        for (int y = n - Neibor / 2; y <= n + Neibor / 2; y++)
                        {
                            if (Image[x][y] < 2)
                                continue;
                            if (NeiborID.size() == 0)
                            {
                                NeiborID.push_back(Image[x][y]);
                                continue;
                            }
                            bool isIn = false;
                            for (int k = 0; k < NeiborID.size(); k++)
                            {
                                if (Image[x][y] == NeiborID[k])
                                {
                                    isIn = true;
                                    break;
                                }
                            }
                            if (!isIn)
                                NeiborID.push_back(Image[x][y]);
                        }
                    }
                    if (NeiborID.size() <= 1)
                    {
                        NeiborID.clear();
                        continue;
                    }
                    for (int k = 0; k < NeiborID.size() - 1; k++)
                    {
                        string str1 = to_string(NeiborID[k]) + "=" + to_string(NeiborID[k + 1]);
                        string str2 = to_string(NeiborID[k + 1]) + "=" + to_string(NeiborID[k]);
                        if (mIDList2.size() == 0)
                        {
                            mIDList2.push_back(str1);
                            continue;
                        }
                        bool isIn = false;
                        for (int j = 0; j < mIDList2.size(); j++)
                        {
                            if (str1 == mIDList2[j] || str2 == mIDList2[j])
                            {
                                isIn = true;
                                break;
                            }
                        }
                        if (!isIn)
                            mIDList2.push_back(str1);
                    }
                    NeiborID.clear();
                }
            }
        }
    }

    //�����ļ���ͳ�����ڵ�cluster
    for (int i = 0; i < mOtherFileNum; i++)
    {
        Tif::readInt(OtherFileList[i], pBuffer2.get());
        for (int j = 0; j < mRows * mCols; j++)
        {
            int row = j / mCols;
            int col = j % mCols;
            Image[row][col] = pBuffer2[j];
        }
        for (int m = 0; m < mRows; m++)
        {
            if (m < Neibor / 2 || mRows - m <= Neibor / 2)
                continue;
            for (int n = 0; n < mCols; n++)
            {
                if (n < Neibor / 2 || mCols - n <= Neibor / 2 || Image[m][n] < 2)
                    continue;

                //������ͳ��
                for (int x = m - Neibor / 2; x <= m + Neibor / 2; x++)
                {
                    for (int y = n - Neibor / 2; y <= n + Neibor / 2; y++)
                    {
                        if (Image[x][y] < 2)
                            continue;
                        if (NeiborID.size() == 0)
                        {
                            NeiborID.push_back(Image[x][y]);
                            continue;
                        }
                        bool isIn = false;
                        for (int k = 0; k < NeiborID.size(); k++)
                        {
                            if (Image[x][y] == NeiborID[k])
                            {
                                isIn = true;
                                break;
                            }
                        }
                        if (!isIn)
                            NeiborID.push_back(Image[x][y]);
                    }
                }
                if (NeiborID.size() <= 1)
                {
                    NeiborID.clear();
                    continue;
                }
                for (int k = 0; k < NeiborID.size() - 1; k++)
                {
                    string str1 = to_string(NeiborID[k]) + "=" + to_string(NeiborID[k + 1]);
                    string str2 = to_string(NeiborID[k + 1]) + "=" + to_string(NeiborID[k]);
                    if (mIDList2.size() == 0)
                    {
                        mIDList2.push_back(str1);
                        continue;
                    }
                    bool isIn = false;
                    for (int j = 0; j < mIDList2.size(); j++)
                    {
                        if (str1 == mIDList2[j] || str2 == mIDList2[j])
                        {
                            isIn = true;
                            break;
                        }
                    }
                    if (!isIn)
                        mIDList2.push_back(str1);
                }
                NeiborID.clear();
            }
        }
    }

    //�ϲ�mIDList��mIDList2
    if (mIDList2.size() != 0)
    {
        for (int i = 0; i < mIDList2.size(); i++)
        {
            string str = mIDList2[i];
            mIDList.push_back(str);
        }
        mIDList2.clear();
    }

    //�ص���cluster pair����Ϊcluster set
    vector<bool> Event(mIDList.size(), false);
    vector<string> mSameList;
    string mSameEvent;
    for (int i = 0; i < mIDList.size(); i++)
    {
        if (Event[i])
            continue;
        mSameEvent = "";
        GetSameEvent(mSameEvent, mIDList, Event, i);
        mSameList.push_back(mSameEvent);
    }

    Meta meta = Meta::DEF;
    for (int i = 0; i < mOtherFileNum; i++)
    {
        Tif::readInt(OtherFileList[i], pBuffer1.get());
        Fill(mSameList, pBuffer1.get(), mRows, mCols);
        string mOutFileName = GenerateFileName(OtherFileList[i], mSaveFilePath, TIF_SUFFIX);
        outputFileList.push_back(mOutFileName);
        meta.date = GetDate(OtherFileList[i]);
        Tif::writeInt(mOutFileName, meta, pBuffer1.get());
    }

    for (int i = 0; i < mRepeatFileNum / 2; i++)
    {
        Tif::readInt(RepeatFileList[2 * i], pBuffer1.get());
        Tif::readInt(RepeatFileList[2 * i + 1], pBuffer2.get());
        for (int j = 0; j < mRows * mCols; j++)
        {
            if (pBuffer1[j] < 2 && pBuffer2[j] > 2)
                pBuffer1[j] = pBuffer2[j];
        }
        Fill(mSameList, pBuffer1.get(), mRows, mCols);

        string mOutFileName = GenerateFileName(RepeatFileList[2 * i], mSaveFilePath , TIF_SUFFIX);
        outputFileList.push_back(mOutFileName);
        meta.date = GetDate(RepeatFileList[2 * i]);
        Tif::writeInt(mOutFileName, meta, pBuffer1.get());
    }
    return outputFileList;
}


bool ClusterRstPolygonize(vector<string> fileList, string outPath, string inPath, Cluster* BG){
    outPath += "\\vec";
    CheckFolderExist(outPath);

    vector<string> srcFileList;
    GetFileList(inPath, srcFileList);

    assert(fileList.size() == srcFileList.size());

    std::sort(fileList.begin(), fileList.end());
    for (int k = 0; k < fileList.size(); ++k){
        Tif *srcTif = new Tif(Meta::DEF, srcFileList[k]);
        srcTif->read();

        string f = fileList[k];
        Tif *tif = new Tif(Meta::DEF, f);
        tif->read();

        assert(srcTif->meta.date == tif->meta.date);

        for(int i = 0; i < Meta::DEF.nRow; ++i){
            for(int j = 0; j < Meta::DEF.nCol; ++j){
                float v = tif->get(i,j);
                if(IsEqual(v, 0) || IsEqual(v, -1)){
                    BG->pix += 1;
                    BG->sum += v;
                    BG->dev += v * v;
                }
            }
        }

        int idx = f.find_last_of('\\');
        int idx2 = f.find_last_of('.');
        string fileName = f.substr(idx + 1, idx2 - idx - 1);
        string shpFileName = outPath + "\\" + fileName + SHP_SUFFIX;
        GDALDataset *poShp = Shp::driver->Create(shpFileName.c_str(), 0, 0, 0, GDT_Unknown, NULL);
        OGRLayer *poLayer = poShp->CreateLayer(shpFileName.c_str(), Meta::DEF.spatialReference, wkbPolygon, NULL);
        Shp::createFields(poLayer);

        tif->polygonize(poLayer, 0);

        OGRFeature * feature;
        while(feature = poLayer->GetNextFeature()) {
            int pixCnt = feature->GetGeometryRef()->toPolygon()->get_Area();
            feature->SetField(11, pixCnt);
        }
    }

    return true;
}

vector<float> DcSTCA::Run(string inPath, string outPath, int _T, int cTh, float vTh) {
    batchSize = 21;

    meta = Meta::DEF;
    mRows = Meta::DEF.nRow;
    mCols = Meta::DEF.nCol;
    mFillValue = Meta::DEF.fillValue;
    coreThreshold = cTh;
    valueThreshold = vTh;
    T = _T;

    // mask background
    static double datasetMean;
    float stdDevTime = 1.0f;
    string maskPath(outPath);
    maskPath += "\\mask_" + to_string(stdDevTime);
    if(!CheckFolderExist(maskPath,false))
        Background(inPath,&datasetMean,&stdDevTime,maskPath);


    outPath += "\\DcSTCA_" + to_string(_T) + "_" + to_string(cTh) + "_" + to_string(vTh);
    CheckFolderExist(outPath);

    vector<string> fileList;
    GetFileList(maskPath, fileList);

    set<string> repeatFileSet;
    set<string> otherFileSet;

    string repeatFilePath = outPath + "\\RepeatFile";
    string otherFilePath = outPath + "\\OtherFile";
    CheckFolderExist(repeatFilePath);
    CheckFolderExist(otherFilePath);
    repeatFilePath += "\\";
    otherFilePath += "\\";

    int mFileNum = fileList.size();
    vector<Pixel> Rasterpixels;
    unique_ptr<int[]> outt(new int[mRows * mCols]);

    for (int i = 0; i < mFileNum / (batchSize - T * 2) + 1; i++) {
        if (batchSize == mFileNum && i > 0)//һ���Դ���
            continue;

        //input
        int mStartFileIndex = i * (batchSize - T * 2);
        if (mStartFileIndex < 0)
            mStartFileIndex = 0;
        int mEndFileIndex = mStartFileIndex + batchSize;
        if (mEndFileIndex >= mFileNum)
            mEndFileIndex = mFileNum;
        int mTempFileNum = mEndFileIndex - mStartFileIndex;
        if (mTempFileNum <= 0) {
            mEndFileIndex = mFileNum;
            mStartFileIndex = mEndFileIndex - 2 * T;
            mTempFileNum = 2 * T;
        }
        for (int j = mStartFileIndex; j < mEndFileIndex; j++) {
            int *pBuffer = new int[mRows * mCols];
            Tif::readInt(fileList[j], pBuffer, mScale);
            for (int k = 0; k < mRows * mCols; k++) {
                Pixel rsets;
                rsets.Attribute = pBuffer[k] * mScale;
                rsets.x = k / mCols; //row
                rsets.y = k % mCols; //col
                rsets.t = j - mStartFileIndex;
                rsets.a = 0;
                rsets.SetrsId(k + (j - mStartFileIndex) * mRows * mCols);
                rsets.SetVisited(false);
                rsets.SetKey(false);
                rsets.SetrsClusterId(-1);
                Rasterpixels.push_back(rsets);
            }
            delete[] pBuffer;
        }
        //input

        //LabelData (attribute, row, col, t, visited (false), key(false),clusterID(0));
        this->Lable(Rasterpixels, mTempFileNum);
        //LabelData


        //Clustering
        int mstartClusterID = clusterId;
        for (int j = 0; j < mTempFileNum * mRows * mCols; j++) {
            if (!Rasterpixels[j].isVisited() && Rasterpixels[j].IsKey()) {
                clusters[clusterId] = new Cluster(clusterId);

                Rasterpixels[j].SetrsClusterId(clusterId);
                Rasterpixels[j].SetVisited(true);

                this->ExpandCluster(Rasterpixels, j, clusterId, mRows, mCols);

                clusterId++;
            }
        }
        //Clustering


        //Filter small clusters
        /*if (MinNum > 0) {
            while (mstartClusterID <= clusterId) {
                int mendClusterID = mstartClusterID + 50;
                if (mendClusterID > clusterId)
                    mendClusterID = clusterId;
                this->ClusterFilter(Rasterpixels, mstartClusterID, mendClusterID, mTempFileNum, mRows * mCols);
                mstartClusterID = mendClusterID + 1;
            }
        }*/
        //Filter small clusters


        //output
        for (int ii = 0; ii < mTempFileNum; ii++) {
            for (int j = ii * mRows * mCols; j < (ii + 1) * mRows * mCols; j++) {
                outt[j - ii * mRows * mCols] = Rasterpixels[j].cid;
            }

            bool isRepeat = false;
            string mOutFileName = GenerateFileName(fileList[mStartFileIndex + ii], otherFilePath,
                                                   "_Tcluster" + TIF_SUFFIX);
            if (mFileNum != batchSize) {
                if (mTempFileNum - ii <= T * 2 && i != mFileNum / (batchSize - T * 2)) {
                    mOutFileName = GenerateFileName(fileList[mStartFileIndex + ii], repeatFilePath,
                                                    "_Tcluster" + TIF_SUFFIX);
                    isRepeat = true;
                }

                if (i != 0 && ii < T * 2) {
                    mOutFileName = GenerateFileName(fileList[mStartFileIndex + ii], repeatFilePath,
                                                    "_Tcluster_2" + TIF_SUFFIX);
                    isRepeat = true;
                }
            }

            if (isRepeat)
                repeatFileSet.insert(mOutFileName);
            else
                otherFileSet.insert(mOutFileName);

            meta.date = GetDate(fileList[mStartFileIndex + ii]);

            Tif::writeInt(mOutFileName, meta, outt.get());
        }
        Rasterpixels.clear();
    }

    // output
    vector<string> repeatFileList, otherFileList;
    for (std::set<string>::iterator it = repeatFileSet.begin(); it != repeatFileSet.end(); ++it)
        repeatFileList.push_back(*it);
    for (std::set<string>::iterator it = otherFileSet.begin(); it != otherFileSet.end(); ++it)
        otherFileList.push_back(*it);

    // resort
    vector<string> sorted = Resort(repeatFileList, otherFileList, outPath);

    // Polygonize
    Cluster *BG = new Cluster(0);
    ClusterRstPolygonize(sorted, outPath, inPath, BG);

    // statistic
    vector<float> res  = InnerEval(BG, datasetMean, clusters);

    return res;
}



