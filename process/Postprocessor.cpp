#include "Postprocessor.h"

using namespace std;

void GetStatisticsValue(int* pBuffer, double* StatiValue, int mRows, int mCols)
{
	int sum = 0, count = 0;
	double min = 100000, max = -9999, std = 0, mean = 0;
	for (int i = 0; i < mRows * mCols; i++)
	{
		if (pBuffer[i] < 2)
			continue;
		if (pBuffer[i] < min)
			min = pBuffer[i];
		if (pBuffer[i] > max)
			max = pBuffer[i];

		sum += pBuffer[i];
		std += pBuffer[i] * pBuffer[i];
		count++;
	}
	if (count != 0)
	{
		mean = sum * 1.0 / count;
		std = std / count - mean * mean;
		if (std >= 0)
			std = sqrt(std);
		else
			std = 0;
		StatiValue[0] = max;
		StatiValue[1] = min;
		StatiValue[2] = mean;
		StatiValue[3] = std;
	}
	else
	{
		StatiValue[0] = 0;
		StatiValue[1] = 0;
		StatiValue[2] = 0;
		StatiValue[3] = 0;
	}
}

int Split(const string& str, const char split, vector<string>& res)
{
	if (str == "")		return -1;
	//���ַ���ĩβҲ����ָ����������ȡ���һ��
	string strs = str + split;
	size_t prev = 0;
	size_t pos = strs.find_first_of(split);

	// ���Ҳ����������ַ��������������� npos
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

void Postprocessor::Resort(vector<string> RepeatFileList, vector<string> OtherFileList, string mSaveFilePath)
{
	int mOtherFileNum = OtherFileList.size();
	int mCols = Def.Cols;
	int mRows = Def.Rows;
	string DataSetName = Def.DataSetName;
	double mStartLog = Def.StartLon;
	double mStartLat = Def.StartLat;
	double mEndLog = Def.EndLon;
	double mEndLat = Def.EndLat;
	double mScale = Def.Scale;
	string mDataType = Def.DataType;
	double mResolution = Def.Resolution;
	double mFillValue = Def.FillValue;

	//ͳ����ͬ�¼�
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
	hdfOpt HO;
	
	int mRepeatFileNum = RepeatFileList.size();
	
	if (mRepeatFileNum != 0)
	{		
		int CountNum = 0;//λ����ͬ�¼���Ų�ͬ��դ�������������դ����

		//ͳ��2���ظ��ļ��ϵ��ص�cluster
		for (int i = 0; i < mRepeatFileNum / 2; i++)
		{
			HO.readHDF(RepeatFileList[2 * i], pBuffer1.get());
			HO.readHDF(RepeatFileList[2 * i + 1], pBuffer2.get());
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
			HO.readHDF(RepeatFileList[2 * i], pBuffer1.get());
			HO.readHDF(RepeatFileList[2 * i + 1], pBuffer2.get());
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
			HO.readHDF(RepeatFileList[i], pBuffer1.get());
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
		HO.readHDF(OtherFileList[i], pBuffer2.get());
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

	//�������
	//�����ļ�
	Meta meta = Def;
	for (int i = 0; i < mOtherFileNum; i++)
	{
		HO.readHDF(OtherFileList[i], pBuffer1.get());
		Fill(mSameList, pBuffer1.get(), mRows, mCols);
		string mOutFileName = opt::generateFileName(OtherFileList[i], mSaveFilePath, ".hdfOpt");
		meta.Date = HO.GetFileDateTime(OtherFileList[i].c_str());
		HO.writeHDF(mOutFileName, meta, pBuffer1.get());
	}

	//�ϲ��ظ��ļ�
	for (int i = 0; i < mRepeatFileNum / 2; i++)
	{
		HO.readHDF(RepeatFileList[2 * i], pBuffer1.get());
		HO.readHDF(RepeatFileList[2 * i + 1], pBuffer2.get());
		for (int j = 0; j < mRows * mCols; j++)
		{
			if (pBuffer1[j] < 2 && pBuffer2[j] > 2)
				pBuffer1[j] = pBuffer2[j];
		}
		Fill(mSameList, pBuffer1.get(), mRows, mCols);

		string mOutFileName = opt::generateFileName(RepeatFileList[2 * i], mSaveFilePath , ".hdfOpt");
		meta.Date = HO.GetFileDateTime(RepeatFileList[2 * i].c_str());
		HO.writeHDF(mOutFileName, meta, pBuffer1.get());
	}
}