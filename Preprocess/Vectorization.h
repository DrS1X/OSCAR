#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <array>

using namespace std;

class myCircle
{
public:
	double x;//圆心x坐标
	double y;//圆心y坐标
	double r;//圆半径

	myCircle(double _x, double _y, double _r)
	{
		x = _x;
		y = _y;
		r = _r;
	}

	myCircle()
	{
		x = 0.0;
		y = 0.0;
		r = 0.0;
	}
};

class myRectangle
{
public:
	double minX;
	double maxX;
	double minY;
	double maxY;
	double area;

	array<double, 2> p1;
	array<double, 2> p2;
	array<double, 2> p3;
	array<double, 2> p4;

	double length;//矩形长度
	double width;//矩形宽度

	myRectangle(double m_minX, double m_maxX, double m_minY, double m_maxY, double m_area)
	{
		minX = m_minX;
		maxX = m_maxX;
		minY = m_minY;
		maxY = m_maxY;
		area = m_area;

		length = m_maxX - m_minX;
		width = m_maxY - m_minY;
		if (width > length)
		{//长宽交换
			double temp = length;
			length = width;
			width = temp;
		}

		p1[0] = minX; p1[1] = minY;
		p2[0] = maxX; p2[1] = minY;
		p3[0] = minX; p3[1] = maxY;
		p4[0] = maxX; p4[1] = maxY;
	}

	myRectangle()
	{
		minX = DBL_MIN;
		maxX = DBL_MAX;
		minY = DBL_MIN;
		maxY = DBL_MAX;
		area = DBL_MAX;

		length = DBL_MAX;
		width = DBL_MIN;

		p1[0] = minX; p1[1] = minY;
		p2[0] = maxX; p2[1] = minY;
		p3[0] = minX; p3[1] = maxY;
		p4[0] = maxX; p4[1] = maxY;
	}

	myRectangle(array<double, 2> m_p1, array<double, 2> m_p2, array<double, 2> m_p3, array<double, 2> m_p4, double m_length, double m_width, double m_area)
	{
		area = m_area;

		p1[0] = m_p1[0]; p1[1] = m_p1[1];
		p2[0] = m_p2[0]; p2[1] = m_p2[1];
		p3[0] = m_p3[0]; p3[1] = m_p3[1];
		p4[0] = m_p4[0]; p4[1] = m_p4[1];

		length = m_length;
		width = m_width;

		array<double, 4> m_xa { m_p1[0], m_p2[0], m_p3[0], m_p4[0] };
		array<double, 4> m_ya { m_p1[1], m_p2[1], m_p3[1], m_p4[1] };

		maxX = *max_element(m_xa.begin(), m_xa.end());
		minX = *min_element(m_xa.begin(), m_xa.end());
		minY = *min_element(m_ya.begin(), m_ya.end());
		maxY = *max_element(m_ya.begin(), m_ya.end());
	}
};

class Node
{
public:
	int id;//唯一标识
	int type;//结点类型
	/*
	5:左上角和右下角值大于零
	6:右上角和左下角值大于零	
	*/
	int row;//经度
	int col;//维度
	string dir1;//方向1
	string dir2;//方向2
	string outDir;//线进入方向
	bool isUsed;//是否被使用
	int power;//强度
	int eventID;//暴雨id
};

//线结构
class Line
{
public: 
	int id;//唯一标识
	 vector<Node> nodes;//所有点
	 int minRow;//最小行数
	 int minCol;//最小列数
	 int maxRow;//最大行数
	 int maxCol;//最大列数
	 int type;//线类型，0为外环，1为内环
	 int power;//强度
	 int eventID;//事件id
	 double length;//长度
};

class POL
{
public:
	int id;//唯一标识
	int eventID;//事件id
	vector<Line> lines;//所有线
	int minRow;//最小行数
	int minCol;//最小列数
	int maxRow;//最大行数
	int maxCol;//最大列数
	double area;//实际面积
	double avgValue;//平均距平值
	double volume;//距平值体积
	double maxValue;//最大距平值
	double minValue;//最小距平值
	int power;//强度
	bool isMulti;//是否是多面
	double length;//周长
	double coreRow;//重心行号
	double coreCol;//重心列号
	myRectangle minRec;//最小面积外包矩形
	myCircle minOutCir;//最小面积外接圆
	myCircle maxInCir;//最大面积内接圆
};

void Vectorization(vector<string>& oriFileNames, vector<string>& spFileNames, string outFolderPath);