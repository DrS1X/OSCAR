#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <array>

using namespace std;

class myCircle
{
public:
	double x;//Բ��x����
	double y;//Բ��y����
	double r;//Բ�뾶

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

	double length;//���γ���
	double width;//���ο��

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
		{//������
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
	int id;//Ψһ��ʶ
	int type;//�������
	/*
	5:���ϽǺ����½�ֵ������
	6:���ϽǺ����½�ֵ������	
	*/
	int row;//����
	int col;//ά��
	string dir1;//����1
	string dir2;//����2
	string outDir;//�߽��뷽��
	bool isUsed;//�Ƿ�ʹ��
	int power;//ǿ��
	int eventID;//����id
};

//�߽ṹ
class Line
{
public: 
	int id;//Ψһ��ʶ
	 vector<Node> nodes;//���е�
	 int minRow;//��С����
	 int minCol;//��С����
	 int maxRow;//�������
	 int maxCol;//�������
	 int type;//�����ͣ�0Ϊ�⻷��1Ϊ�ڻ�
	 int power;//ǿ��
	 int eventID;//�¼�id
	 double length;//����
};

class POL
{
public:
	int id;//Ψһ��ʶ
	int eventID;//�¼�id
	vector<Line> lines;//������
	int minRow;//��С����
	int minCol;//��С����
	int maxRow;//�������
	int maxCol;//�������
	double area;//ʵ�����
	double avgValue;//ƽ����ƽֵ
	double volume;//��ƽֵ���
	double maxValue;//����ƽֵ
	double minValue;//��С��ƽֵ
	int power;//ǿ��
	bool isMulti;//�Ƿ��Ƕ���
	double length;//�ܳ�
	double coreRow;//�����к�
	double coreCol;//�����к�
	myRectangle minRec;//��С����������
	myCircle minOutCir;//��С������Բ
	myCircle maxInCir;//�������ڽ�Բ
};

void Vectorization(vector<string>& oriFileNames, vector<string>& spFileNames, string outFolderPath);