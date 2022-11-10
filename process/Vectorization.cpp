#include <cmath>
#include <memory>
#include "Vectorization.h"
#include <array>
#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"
#include "ogr_geometry.h"
#include "opt.h"
#include "_const.h"

using namespace std;

Node GetNextNode(Node tailNode, vector<Node>& nodes, int& minRow, int& minCol, int& maxRow, int& maxCol)
{
	if (tailNode.outDir == "r")
	{//向右搜索
		for (int i = (tailNode.id + 1); i < nodes.size(); i++)
		{//向后搜索
			if (tailNode.row == nodes[i].row)
			{//行号相同
				if (nodes[i].isVisited == false)
				{//该节点没有被使用
					Node node = nodes[i];
					if (node.type <= 4)
					{//前四种结点
						node.isVisited = true;
						if (node.dir1 == "l") node.outDir = node.dir2;
						else node.outDir = node.dir1;
					}
					else if (node.type == 5) node.outDir = "b";//第五种结点
					else if (node.type == 6) node.outDir = "t";//第六种结点
					nodes[i] = node;//替换
					//判断最大最小行列数
					if (node.row < minRow) minRow = node.row;//最小行数
					else if (node.row > maxRow) maxRow = node.row;//最大行数
					if (node.col < minCol) minCol = node.col;//最小列数
					else if (node.col > maxCol) maxCol = node.col;//最大列数
					//vector<Node> nextNodes = GetNextNodes(node, ref nodes, ref minRow, ref minCol, ref maxRow, ref maxCol);
					//nextNodes.Insert(0, node);//将该结点添加到头上
					//return nextNodes;
				}
				return nodes[i];
			}
		}
	}
	else if (tailNode.outDir == "b")
	{//向下搜索
		for (int i = (tailNode.id + 1); i < nodes.size(); i++)
		{//向后搜索
			if (tailNode.col == nodes[i].col)
			{//列号相同
				if (nodes[i].isVisited == false)
				{//该节点没有被使用
					Node node = nodes[i];
					//node.isVisited = true;
					//if (node.dir1 == "t")
					//{
					//    node.outDir = node.dir2;
					//}
					//else
					//{
					//    node.outDir = node.dir1;
					//}
					if (node.type <= 4)
					{//前四种结点
						node.isVisited = true;
						if (node.dir1 == "t") node.outDir = node.dir2;
						else node.outDir = node.dir1;
					}
					else if (node.type == 5) node.outDir = "r";//第五种结点
					else if (node.type == 6) node.outDir = "l";//第六种结点
					nodes[i] = node;//替换
					if (node.row < minRow) minRow = node.row;//最小行数
					else if (node.row > maxRow) maxRow = node.row;//最大行数
					if (node.col < minCol) minCol = node.col;//最小列数
					else if (node.col > maxCol) maxCol = node.col;//最大列数
					//vector<Node> nextNodes = GetNextNodes(node, ref nodes, ref minRow, ref minCol, ref maxRow, ref maxCol);
					//nextNodes.Insert(0, node);//将该结点添加到头上
					//return nextNodes;
				}
				return nodes[i];
			}
		}
	}
	else if (tailNode.outDir == "l")
	{//向左搜索
		for (int i = (tailNode.id - 1); i >= 0; i--)
		{//向前搜索
			if (tailNode.row == nodes[i].row)
			{//行号相同
				if (nodes[i].isVisited == false)
				{//该节点没有被使用
					Node node = nodes[i];
					//node.isVisited = true;
					//if (node.dir1 == "r")
					//{
					//    node.outDir = node.dir2;
					//}
					//else
					//{
					//    node.outDir = node.dir1;
					//}
					if (node.type <= 4)
					{//前四种结点
						node.isVisited = true;
						if (node.dir1 == "r") node.outDir = node.dir2;
						else node.outDir = node.dir1;
					}
					else if (node.type == 5) node.outDir = "t";//第五种结点
					else if (node.type == 6) node.outDir = "b";//第六种结点
					nodes[i] = node;//替换
					if (node.row < minRow) minRow = node.row;//最小行数
					else if (node.row > maxRow) maxRow = node.row;//最大行数
					if (node.col < minCol) minCol = node.col;//最小列数
					else if (node.col > maxCol) maxCol = node.col;//最大列数
				}
				return nodes[i];
			}
		}
	}
	else if (tailNode.outDir == "t")
	{//向左搜索
		for (int i = (tailNode.id - 1); i >= 0; i--)
		{//向前搜索
			if (tailNode.col == nodes[i].col)
			{//列号相同
				if (nodes[i].isVisited == false)
				{//该节点没有被使用
					Node node = nodes[i];
					//node.isVisited = true;
					//if (node.dir1 == "b")
					//{
					//    node.outDir = node.dir2;
					//}
					//else
					//{
					//    node.outDir = node.dir1;
					//}
					if (node.type <= 4)
					{//前四种结点
						node.isVisited = true;
						if (node.dir1 == "b") node.outDir = node.dir2;
						else node.outDir = node.dir1;
					}
					else if (node.type == 5) node.outDir = "l";//第五种结点
					else if (node.type == 6) node.outDir = "r";//第六种结点
					nodes[i] = node;//替换
					if (node.row < minRow) minRow = node.row;//最小行数
					else if (node.row > maxRow) maxRow = node.row;//最大行数
					if (node.col < minCol) minCol = node.col;//最小列数
					else if (node.col > maxCol) maxCol = node.col;//最大列数
				}
				return nodes[i];
			}
		}
	}
	Node empty;
	return empty;//理论上不会执行
}

double GetStandDev(double v1, double v2, double v3)
{
	double avg = (v1 + v2 + v3) / 3.0;//平均值
	double variance = (pow(v1 - avg, 2) + pow(v2 - avg, 2) + pow(v3 - avg, 2)) / 3;//方差
	double dev = sqrt(variance);//标准差
	return dev;
}

array<double, 2> GetMovePoint(double x1, double y1, double x, double y, double step)
{
	double length = sqrt(pow((x1 - x), 2) + pow((y1 - y), 2));//线段长度
	double xm = x + ((x - x1) / length) * step;//计算移动后的x坐标
	double ym = y + ((y - y1) / length) * step;//计算移动后的y坐标
	array<double,2> mp { xm, ym };
	return mp;
}

double GetPointDistance(double x1, double y1, double x2, double y2)
{
	double dis = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
	return dis;
}

double GetPointLineDistance(double x, double y, double px, double py, double nx, double ny, array<double, 2> &  disPoint)
{
	double distance = 0.0;
	if (px == nx)
	{//竖线
		if (y > max(py, ny) || y < min(py, ny))
		{//垂足不在线段上
		 //distance = DBL_MAX;
			double d1 = GetPointDistance(x, y, px, py);//计算两点间的位置
			double d2 = GetPointDistance(x, y, nx, ny);//计算两点间的位置
			if (d1 < d2)
			{
				disPoint[0] = px;
				disPoint[1] = py;
				distance = d1;
			}
			else
			{
				disPoint[0] = nx;
				disPoint[1] = ny;
				distance = d2;
			}
		}
		else
		{
			disPoint[0] = px;
			disPoint[1] = y;
			distance = abs(x - px);
		}
	}
	else if (py == ny)
	{//横线
		if (x > max(px, nx) || x < min(px, nx))
		{
			//distance = DBL_MAX;
			double d1 = GetPointDistance(x, y, px, py);//计算两点间的位置
			double d2 = GetPointDistance(x, y, nx, ny);//计算两点间的位置
			if (d1 < d2)
			{
				disPoint[0] = px;
				disPoint[1] = py;
				distance = d1;
			}
			else
			{
				disPoint[0] = nx;
				disPoint[1] = ny;
				distance = d2;
			}
		}
		else
		{
			disPoint[0] = x;
			disPoint[1] = py;
			distance = abs(y - py);
		}
	}
	else
	{
		cout << "没有考虑该情况哦！" << endl;
	}
	return distance;
}

double GetPointLineDistance(double x, double y, double px, double py, double nx, double ny)
{
	double distance = 0.0;
	if (px == nx)
	{//竖线
		if (y > max(py, ny) || y < min(py, ny))
		{//垂足不在线段上
		 //distance = DBL_MAX;
			double d1 = GetPointDistance(x, y, px, py);//计算两点间的位置
			double d2 = GetPointDistance(x, y, nx, ny);//计算两点间的位置
			if (d1 < d2)
			{
				distance = d1;
			}
			else
			{
				distance = d2;
			}
		}
		else
		{
			distance = abs(x - px);
		}
	}
	else if (py == ny)
	{//横线
		if (x > max(px, nx) || x < min(px, nx))
		{
			//distance = DBL_MAX;
			double d1 = GetPointDistance(x, y, px, py);//计算两点间的位置
			double d2 = GetPointDistance(x, y, nx, ny);//计算两点间的位置
			if (d1 < d2)
			{
				distance = d1;
			}
			else
			{
				distance = d2;
			}
		}
		else
		{
			distance = abs(y - py);
		}
	}
	else
	{
		cout << "没有考虑该情况哦！" << endl;
	}
	return distance;
}

double GetRadiusPoint(Poly polygon, double x, double y)
{
	Line line = polygon.lines[0];
	vector<Node> nodeList = line.nodes;
	//vector<vector<double> > points(nodeList.size(), vector<double>(2));
	vector<array<double, 2> > points(nodeList.size());
	for (int i = 0; i < nodeList.size(); i++)
	{//取出xy
		points[i][0] = (double)nodeList[i].row;//x
		points[i][1] = (double)nodeList[i].col;//y
	}

	double distances = DBL_MAX;//点与线段的距离
	for (int i = 0; i < nodeList.size() - 1; i++)
	{//计算距离
		double _distances = GetPointLineDistance(x, y, points[i][0], points[i][1], points[i + 1][0], points[i + 1][1]);//计算距离
		if (_distances < distances)
			distances = _distances;//记录更小的
	}

	return distances;
}

bool IsOnLine(double x, double y, int x1, int y1, int x2, int y2)
{
	if (x == x1 && y == y1 || x == x2 && y == y2) return true;//端点

	if (x >= min(x1, x2) && x <= max(x1, x2) && y >= min(y1, y2) && y <= max(y1, y2))
	{
		if (x == x1 && x1 == x2 || y == y1 && y1 == y2) return true;
		else return false;
	}
	else
	{
		return false;
	}
}

bool IsInPolygonNoBorder(double row, double col,const vector<Node>& nodes)
{
	bool inside = false;
	Node n1, n2;
	for (int i = 0; i < nodes.size() - 1; i++)
	{
		n1 = nodes[i];
		n2 = nodes[i + 1];
		bool isOnLine = IsOnLine(row, col, n1.row, n1.col, n2.row, n2.col);
		if (isOnLine) return false;//边界点不认为在内
		if (n1.col > col)
		{//右侧
			if (n1.row >= row && n2.row < row)
			{
				inside = !inside;
			}
			else if (n1.row < row && n2.row >= row)
			{
				inside = !inside;
			}
		}
	}
	return inside;
}

myCircle GetMaxInCir(Poly& polygon, double step, double x, double y)
{
    double xOir = x;
    double yOir = y;
    if (xOir == 561 && yOir == 639.5)
    {
        int aa = 0;
    }
    double stepOir = step;//记录原始值
    step = step * 0.5;
    Line line = polygon.lines[0];
    vector<Node> nodeList = line.nodes;
    vector<array<double, 2>> points( nodeList.size());
    for (int i = 0; i < nodeList.size(); i++)
    {//取出xy
        points[i][0] = nodeList[i].row;//x
        points[i][1] = nodeList[i].col;//y
    }

    double lastStandDev = DBL_MAX;//记录上一个标准差
    double standDev = DBL_MAX;//当前标准差
    double r = 0.0;//圆的半径
    while (true)
    {//迭代求最大面积内切圆
        vector<double> distances(nodeList.size() - 1);//点与线段的距离
        vector<array<double, 2>> disPoints(nodeList.size() - 1);//计算线段距离的另一个端点

        vector<int> minDisPosList;//记录距离最小点位置
        for (int i = 0; i < distances.size(); i++)
        {//计算距离
            array<double,2> disPoint;//计算距离的另一个点
            distances[i] = floor(GetPointLineDistance(x, y, points[i][0], points[i][1], points[i + 1][0], points[i + 1][1], disPoint)
                                 * pow(10,4) + 0.5) / pow(10,4);
            //计算距离 四舍五入取4位小数
            disPoints[i][0] = disPoint[0];//记录计算距离端点位置
            disPoints[i][1] = disPoint[1];
            bool isInsert = false;//记录是否插入
            for (int j = 0; j < minDisPosList.size(); j++)
            {
                if (distances[i] < distances[minDisPosList[j]])
                {//更小的距离
                    minDisPosList.insert(minDisPosList.begin() + j, i);//插入该位置
                    if (minDisPosList.size() > 3)
                        minDisPosList.pop_back();//移除后面的
                    isInsert = true;
                    break;
                }
            }
            if (!isInsert && minDisPosList.size() < 3)
            {//没有插入
                minDisPosList.push_back(i);//在最后记录下该位置
            }
            //if (minDisPosList.size() > 3) minDisPosList.RemoveRange(3, minDisPosList.size()() - 3);//移除后面的
        }
        r = *min_element(distances.begin(), distances.end());//选取最小距离最为圆的半径
        //if(r>1000)
        //{
        //    r = 0;
        //}
        standDev = GetStandDev(distances[minDisPosList[0]], distances[minDisPosList[1]], distances[minDisPosList[2]]);//计算标准

        if (standDev < (stepOir * 0.01))
        {//标准差很小
            break;//结束迭代运算
        }
        else
        {
            int minDisPos = minDisPosList[0];//最短距离位置
            array<double, 2> pointMove = GetMovePoint(disPoints[minDisPos][0], disPoints[minDisPos][1], x, y, step);//获取移动后的点
            x = pointMove[0];
            y = pointMove[1];
            array<double, 2> pointMove2 = GetMovePoint(disPoints[minDisPos][0], disPoints[minDisPos][1], 561, 639.5, step);//获取移动后的点
        }

        if (standDev >= lastStandDev)
        {//标准差变大了

            step = step * 0.5;//调整距离变为减半
        }
        lastStandDev = standDev;//记录标准差

        if (step < (stepOir * (pow(0.5, 100))))
        {//调整很小
            break;//结束迭代运算
        }
    }
    myCircle cir(x, y, r);
    return cir;
}

myCircle GetMaxInCir(Poly& polygon, double step)
{
	double minX = polygon.minRow;
	double minY = polygon.minCol;
	double maxX = polygon.maxRow;
	double maxY = polygon.maxCol;

	//double[,] ccs = new double[Convert.ToInt32(((maxX - minX) / step - 1) * ((maxY - minY) / step - 1)), 2];//初始远心点
	vector<myCircle> cirs;//最大半径的圆
	myCircle tmp;
	cirs.push_back(tmp);
	//double radiusMax = 0.0;
	//double xm = 0.0;
	//double ym = 0.0;
	for (double _x = minX + step; _x < maxX; _x += step)
	{
		for (double _y = minY + step; _y < maxY; _y += step)
		{
			if (IsInPolygonNoBorder(_x, _y, polygon.lines[0].nodes))
			{//点位于多边形内
				double radius = GetRadiusPoint(polygon, _x, _y);//获取该点为圆心的内切圆半径

				if (radius > cirs[0].r)
				{
					cirs.clear();
					myCircle tmp(_x, _y, radius);
					cirs.push_back(tmp);
				}
				else if (radius == cirs[0].r)
				{
					myCircle tmp(_x, _y, radius);
					cirs.push_back(tmp);
				}
			}
		}
	}
	myCircle maxInCir;
	for(auto cir : cirs)
	{
		myCircle _maxInCir = GetMaxInCir(polygon, step, cir.x, cir.y);//计算调整后的圆
		if (_maxInCir.r > maxInCir.r) maxInCir = _maxInCir;//寻找最大半径圆
	}
	return maxInCir;
}

bool IsInPolygonNew(const Node checkNode,const vector<Node>& nodes)
{
	bool inside = false;
	Node n1, n2;
	for (int i = 0; i < nodes.size() - 1; i++)
	{
		n1 = nodes[i];
		n2 = nodes[i + 1];
		if (n1.col > checkNode.col)
		{//右侧
			if (n1.row >= checkNode.row && n2.row < checkNode.row)
			{
				inside = !inside;
			}
			else if (n1.row < checkNode.row && n2.row >= checkNode.row)
			{
				inside = !inside;
			}
		}
	}
	return inside;
}

bool IsInPolygonNew(float row, float col, const vector<Node>& nodes)
{
	bool inside = false;
	Node n1, n2;
	for (int i = 0; i < nodes.size() - 1; i++)
	{
		n1 = nodes[i];
		n2 = nodes[i + 1];
		if (n1.col > col)
		{//右侧
			if (n1.row >= row && n2.row < row)
			{
				inside = !inside;
			}
			else if (n1.row < row && n2.row >= row)
			{
				inside = !inside;
			}
		}
	}
	return inside;
}

array<double, 2> GetPointRotate(double x, double y, int angle)
{
	double angleR = angle * M_PI / 180;//计算弧度制角度
	double xr = x * cos(angleR) + y * sin(angleR);
	double yr = -x * sin(angleR) + y * cos(angleR);
	array<double, 2>  pr = { xr, yr };
	return pr;
}

myCircle getCircleRotate(myCircle minCircle, int angle)
{
	array<double, 2> pointR = GetPointRotate(minCircle.x, minCircle.y, angle);
	myCircle minCirR(pointR[0], pointR[1], minCircle.r);
	return minCirR;
}

myCircle getVerCir(const vector<array<double, 2>> & pointsR)
{
	array<double, 2> maxYPoint = { 0, DBL_MIN };
	array<double, 2> minYPoint = { 0, DBL_MAX };
	for (int i = 0; i < pointsR.size(); i++)
	{
		array<double, 2> point = { pointsR[i][0], pointsR[i][1] };//取出该点
		if (point[1] > maxYPoint[1]) maxYPoint = point;//记录y值更大的点
		if (point[1] < minYPoint[1]) minYPoint = point;//记录y值更小的点
	}
	double x = (maxYPoint[0] + minYPoint[0]) / 2;//圆心横坐标
	double y = (maxYPoint[1] + minYPoint[1]) / 2;//圆心纵坐标
	double r = (maxYPoint[1] - minYPoint[1]) / 2;//圆心半径
	myCircle cir(x, y, r);
	return cir;
}

double GetRasterArea(double rasterStartLat, double rasterEndLat, double rasterLog)
{
	double earthRadius = 6371.393;//单位千米
	double cutCount = 360.0 / rasterLog;//一个圆环被切割的份数

	//将地球视为球体的经纬1°网格计算公式为2πr²(sin(α+1)-sin(α))/360，半径为r,在纬度为α，https://www.zybang.com/question/deba669df1201d9c8d5c95e003716524.html
	double rasterArea = 2 * M_PI * earthRadius * earthRadius * (sin(M_PI * rasterEndLat / 180) - sin(M_PI * rasterStartLat / 180)) / cutCount;//积分计算面积

	return rasterArea;
}

vector<array<double, 2> > getPointsRotate(const vector<array<double, 2>>& points, int angle)
{
	vector<array<double, 2>> pointsR(points.size());//用来存储旋转后的点
	for (int i = 0; i < points.size(); i++)
	{
		array<double, 2>  point = { points[i][0], points[i][1] };//取出一个点
		array<double, 2>  pointR = GetPointRotate(point[0], point[1], angle);//获取改点绕原点旋转坐标
		pointsR[i][0] = pointR[0];
		pointsR[i][1] = pointR[1];
	}
	return pointsR;
}

vector<array<double, 2>> getPoints(const vector<Node>& pointList)
{
	vector<array<double, 2>> points(pointList.size());

	for (int i = 0; i < pointList.size(); i++)
	{
		points[i][0] = pointList[i].row;
		points[i][1] = pointList[i].col;
	}

	return points;
}

myCircle GetMinOutCir(Poly polygon)
{
	myCircle minCircle;

	Line line1 = polygon.lines[0];
	vector<Node> pointList = line1.nodes;

	vector<array<double, 2>> points = getPoints(pointList);//row=x col=y

	int minAngle = 0;//符合条件时旋转角度

	for (int angle = 0; angle < 360; angle++)
	{//旋转每个角度
		vector<array<double, 2>> pointsR = getPointsRotate(points, angle);//获取旋转后的点
		myCircle cir = getVerCir(pointsR);//计算以垂直方向最大最小点确定的面积最小的点
		if (cir.r > minCircle.r)
		{//更大半径的外接圆
			minCircle = cir;
			minAngle = angle;
		}
	}

	myCircle minRecR = getCircleRotate(minCircle, -minAngle);//获取旋转后的矩形

	return minRecR;
}

myRectangle getRec(vector<array<double, 2>> pointsR)
{
	myRectangle rec;
	rec.minX = pointsR[0][0];
	rec.maxX = pointsR[0][0];
	rec.minY = pointsR[0][1];
	rec.maxY = pointsR[0][1];
	for (int i = 0; i < pointsR.size(); i++)
	{//每个点
		double _x = pointsR[i][0];
		double _y = pointsR[i][1];
		if (_x < rec.minX) rec.minX = _x; else if (_x > rec.maxX) rec.maxX = _x;
		if (_y < rec.minY) rec.minY = _y; else if (_y > rec.maxY) rec.maxY = _y;
	}
	rec.area = (rec.maxX - rec.minX) * (rec.maxY - rec.minY);
	return rec;
}

myRectangle getRecRotate(myRectangle minRec, int angle)
{
	array<double, 2> p1{ minRec.minX, minRec.minY };
	array<double, 2> p2{ minRec.maxX, minRec.minY };
	array<double, 2> p3{ minRec.maxX, minRec.maxY };
	array<double, 2> p4{ minRec.minX, minRec.maxY };
	array<double, 2> p1R = GetPointRotate(p1[0], p1[1], angle);
	array<double, 2> p2R = GetPointRotate(p2[0], p2[1], angle);
	array<double, 2> p3R = GetPointRotate(p3[0], p3[1], angle);
	array<double, 2> p4R = GetPointRotate(p4[0], p4[1], angle);

	double length = minRec.maxX - minRec.minX;
	double width = minRec.maxY - minRec.minY;
	if (width > length)
	{//交换长宽
		double temp = length;
		length = width;
		width = temp;
	}

	myRectangle RecR(p1R, p2R, p3R, p4R, length, width, minRec.area);
	return RecR;
}

myRectangle GetMinAreaRec(Poly polygon)
{
	myRectangle minRec(DBL_MAX, DBL_MIN, DBL_MAX, DBL_MAX, DBL_MAX);

	Line line1 = polygon.lines[0];
	vector<Node> pointList = line1.nodes;

	vector<array<double, 2>> points = getPoints(pointList);//row=x col=y

	int minAreaAngle = 0;//最小外包矩形旋转角度

	for (int angle = 0; angle < 360; angle++)
	{//旋转每个角度
		vector<array<double, 2>> pointsR = getPointsRotate(points, angle);//获取旋转后的点
		myRectangle rec = getRec(pointsR);//计算平行于坐标轴的最小外包矩形
		if (rec.area < minRec.area)
		{//更小的外包矩形
			minRec = rec;
			minAreaAngle = angle;
		}
	}
	minRec.area = (minRec.maxX - minRec.minX) * (minRec.maxY - minRec.minY);
	myRectangle minRecR = getRecRotate(minRec, -minAreaAngle);//获取旋转后的矩形

	return minRecR;
}

void Pixel2Node(const vector<vector<int>>& spImg, vector<Node>& nodes, int row, int col) {
	for (int i = 0; i < row - 1; i++)
	{//行循环
		for (int j = 0; j < col - 1; j++)
		{
			int ltv = spImg[i][j];//左上角栅格值
			int rtv = spImg[i][j + 1];//右上角栅格值
			int lbv = spImg[i + 1][j];//左下角栅格值
			int rbv = spImg[i + 1][j + 1];//右下角栅格值
			if (ltv != rtv && rtv == lbv && lbv == rbv)
			{//左上角不同，左和上方向
				string dir1 = "l";//左方向
				string dir2 = "t";//上方向
				Node node;
				node.id = nodes.size();
				node.type = 1;
				node.row = i;
				node.col = j;
				node.dir1 = dir1;
				node.dir2 = dir2;
				node.isVisited = false;
				if (ltv > 0)
				{
					node.power = ltv;
				}
				else
				{
					node.power = rtv;
				}
				nodes.emplace_back(node);
			}
			else if (rtv != ltv && ltv == lbv && lbv == rbv)
			{//右上角不同，上和右方向
				string dir1 = "t";
				string dir2 = "r";
				Node node;
				node.id = nodes.size();
				node.type = 2;
				node.row = i;
				node.col = j;
				node.dir1 = dir1;
				node.dir2 = dir2;
				node.isVisited = false;
				if (rtv > 0)
				{
					node.power = rtv;
				}
				else
				{
					node.power = ltv;
				}
				nodes.emplace_back(node);
			}
			else if (lbv != ltv && ltv == rtv && rtv == rbv)
			{//左下角不同，下和左方向
				string dir1 = "b";
				string dir2 = "l";
				Node node;
				node.id = nodes.size();
				node.type = 3;
				node.row = i;
				node.col = j;
				node.dir1 = dir1;
				node.dir2 = dir2;
				node.isVisited = false;
				if (lbv > 0)
				{
					node.power = lbv;
				}
				else
				{
					node.power = ltv;
				}
				nodes.emplace_back(node);
			}
			else if (rbv != ltv && ltv == rtv && rtv == lbv)
			{//右下角不同，右和下方向
				string dir1 = "r";
				string dir2 = "b";
				Node node;
				node.id = nodes.size();
				node.type = 4;
				node.row = i;
				node.col = j;
				node.dir1 = dir1;
				node.dir2 = dir2;
				node.isVisited = false;
				if (rbv > 0)
				{
					node.power = rbv;
				}
				else
				{
					node.power = ltv;
				}
				nodes.emplace_back(node);
			}
			else if (ltv == rbv && ltv != rtv && rtv == lbv)
			{//对角相等，相邻不等
				string dir1 = "n";
				string dir2 = "n";
				Node node;
				node.id = nodes.size();
				if (ltv > 0)
				{//左上角和右下角值大于零
					node.type = 5;
					node.power = ltv;
				}
				else
				{//右上角和左下角值大于零
					node.type = 6;
					node.power = rtv;
				}
				node.row = i;
				node.col = j;
				node.dir1 = dir1;
				node.dir2 = dir2;
				node.isVisited = false;
				nodes.emplace_back(node);
			}
		}
	}
}

void Point2Line(const vector<vector<int>>& spImg, vector<Node> & nodes, vector<Line>& lines, int row, double startLat, double resolution) {
	for (int i = 0; i < nodes.size(); i++)
	{//寻找每一条线
		if (nodes[i].isVisited == true || nodes[i].type > 4) continue;//被使用，跳出本次循环
		Node headNode = nodes[i];//头结点
		headNode.isVisited = true;//记录被使用
		headNode.outDir = headNode.dir1;//记录出去的方向
		nodes[i] = headNode;//进行保存
		Line line;//新建一条线
		int minRow = headNode.row;
		int minCol = headNode.col;
		int maxRow = headNode.row;
		int maxCol = headNode.col;
		//line.nodes.emplace_back(headNode);//
		vector<Node> lineNodes;//线中所有点
		lineNodes.emplace_back(headNode);//将第一个点添加进去
		Node tailNode = headNode;//尾结点
		double lineLength = 0.0;//线的长度
		do
		{
			int startNodeRow = tailNode.row;
			int startNodeCol = tailNode.col;
			tailNode = GetNextNode(tailNode, nodes, minRow, minCol, maxRow, maxCol);

			//计算周长
			double logInterval = abs(tailNode.col - startNodeCol) * resolution;//经度差值
			if (logInterval == 0.0)
			{//竖线
				double latInterval = abs(tailNode.row - startNodeRow) * resolution;//纬度差值
				double _length = 20037.0 * latInterval / 180.0;//经线长度为20017km（百度百科），每条经线长度都相同
				lineLength += _length;
			}
			else
			{//横线
				const double equatorLength = 40076.0;//赤道周长，单位km
				double lineLat = startLat + (row - tailNode.row) * resolution;//纬度
				double equatorLengthNow = equatorLength * cos(lineLat * M_PI / 180.0);//当前纬线周长
				double _length = equatorLengthNow * logInterval / 360.0;
				lineLength += _length;
			}

			lineNodes.emplace_back(tailNode);//将点添加进去
		} while (tailNode.id != headNode.id);

		//判断是内环还是外环
		Node firstNode = lineNodes[0];//第一个结点
		if (firstNode.type == 1)
		{
			if (spImg[firstNode.row][firstNode.col] > 0)//右下角栅格值
			{
				line.type = 0;//外环
			}
			else
			{
				line.type = 1;//内环
			}
		}
		else if (firstNode.type == 2)
		{
			if (spImg[firstNode.row][firstNode.col + 1] > 0)//右下角栅格值
			{
				line.type = 0;//外环
			}
			else
			{
				line.type = 1;//内环
			}
		}
		else if (firstNode.type == 3)
		{
			if (spImg[firstNode.row + 1][firstNode.col] > 0)//右下角栅格值
			{
				line.type = 0;//外环
			}
			else
			{
				line.type = 1;//内环
			}
		}
		else if (firstNode.type == 4)
		{
			if (spImg[firstNode.row + 1][firstNode.col + 1] > 0)//右下角栅格值
			{
				line.type = 0;//外环
			}
			else
			{
				line.type = 1;//内环
			}
		}

		line.id = lines.size();
		line.nodes = lineNodes;//进行保存
		line.minRow = minRow;
		line.minCol = minCol;
		line.maxRow = maxRow;
		line.maxCol = maxCol;
		line.power = headNode.power;//值
		line.length = lineLength;//线的长度
		lines.emplace_back(line);
	}
}

void Line2Polygon(const vector<vector<double>>& oriImg, const vector<vector<int>>& spImg, vector<Line>& lines,
                  vector<Poly>& polygons, int row, double startLat, double resolution) {
	for (int i = 0; i < lines.size(); i++)
	{//循环每一条线
		if (lines[i].type != 0) continue;//不是外环，退出本次循环

		Poly polygon;
		vector<Line> pLines;
		pLines.emplace_back(lines[i]);//添加外环
		polygon.id = polygons.size();
		polygon.minCol = lines[i].minCol;
		polygon.minRow = lines[i].minRow;
		polygon.maxCol = lines[i].maxCol;
		polygon.maxRow = lines[i].maxRow;
		polygon.power = lines[i].power;
		for (int j = i + 1; j < lines.size(); j++)
		{//寻找该外环包含的内环，内环肯定在后面
		 //if (lines[j].minRow >= lines[i].maxRow) break;//最小行数大于等于最大行数，不需要继续执行
			if (lines[j].type == 0) continue;//外环
			if (lines[j].minRow > lines[i].minRow && lines[j].minCol > lines[i].minCol && lines[j].maxRow < lines[i].maxRow && lines[j].maxCol < lines[i].maxCol)
			{//邻接矩形包含，进一步判断
			 //只要一个点在外环内，就是内环
				bool isIn = IsInPolygonNew(lines[j].nodes[0], lines[i].nodes);//判断是否在外环里面
				if (isIn)
				{//内环
					Line line = lines[j];//取出线
					reverse(line.nodes.begin(), line.nodes.end());//顺序反转
					lines[j] = line;//保存修改后的
					pLines.emplace_back(line);//添加内环
				}
			}
		}
		polygon.lines = pLines;
		double avgValue = 0.0;//平均
		double volume = 0.0;//累计
		double maxValue = DBL_MIN;//最大
		double minValue = DBL_MAX;//最小
		double area = 0.0;//面积
		double _rowCore = 0.0;//重心行号中间量
		double _colCore = 0.0;//重心列号中间量
		for (int _row = polygon.minRow + 1; _row <= polygon.maxRow; _row++)
		{//行循环，行列号对应节点左上角栅格
			double rasterStartLat = startLat + (row - _row - 1) * resolution;//栅格下边缘纬度
			double rasterEndLat = rasterStartLat + resolution;//栅格上边缘纬度
			double rasterArea = GetRasterArea(rasterStartLat, rasterEndLat, resolution);//计算一个网格面积
			for (int _col = polygon.minCol + 1; _col <= polygon.maxCol; _col++)
			{//列循环
			 //if (idImg[_row, _col] == polygon.stormID)
				if (IsInPolygonNew(_row - 0.5f, _col - 0.5f, pLines[0].nodes) /*spImg[_row, _col] > 0*/)
				{//当前对象
					float _rowF = _row - 0.5f;//栅格行列号转为矢量行列号
					float _colF = _col - 0.5f;
					if (IsInPolygonNew(_rowF, _colF, polygon.lines[0].nodes) && spImg[_row][_col] > 0)
					{//包含关系
						area += rasterArea;//增加面积
						double value = oriImg[_row][_col];

						polygon.clusterId = spImg[_row][_col];
						if (value > maxValue) maxValue = value;//最大值
						if (value < minValue) minValue = value;
						double _volume = rasterArea * value;//
						volume += _volume;
						//加权
						_rowCore += _volume * _row;
						_colCore += _volume * _col;
					}
				}
			}
		}
		//栅格矢量化有待完善,过滤掉小异常对象
		if (polygon.clusterId <= 0.1/* || area <= 20000*/)
			continue;
		//平均
		polygon.centroidRow = _rowCore / volume;
		polygon.centroidCol = _colCore / volume;

		avgValue = volume / area;//计算平均
		polygon.avgValue = avgValue;//保存平均
		polygon.maxValue = maxValue;//保存最大
		polygon.minValue = minValue;//保存最大
		polygon.area = area;

		//volume = volume * 1000;//立方米
		//polygon.volume = volume;
		polygon.length = polygon.lines[0].length;//周长
		polygon.minRec = GetMinAreaRec(polygon);
		polygon.minOutCir = GetMinOutCir(polygon);
		polygon.maxInCir = GetMaxInCir(polygon, 0.5);
		polygons.push_back(polygon);
	}
}

void RasterToVectorBasedonSpace(string oriPath, string spPath, string outPath, string AbnormalType)
{
	//打开hdf文件, 获取基本信息
	int col = Def.Cols;
	int row = Def.Rows;
	const double startLog = Def.StartLon;//起始经度
	const double startLat = Def.StartLat;//起始维度
	const double endLog = Def.EndLon;//结束经度
	const double endLat = Def.EndLat;//结束维度
	double mScale = 1.0;//比例
	string dataType = "";
	string imgDate = "";
	double fillValue = 0.0;
	const double resolution = Def.Resolution;
	string oriFileName = oriPath.substr(oriPath.find_last_of('\\') + 1);
	int TimeStringStartIndex = oriFileName.size() - 12;
	string startTime = oriFileName.substr(TimeStringStartIndex, 4)
		+ "-" + oriFileName.substr(TimeStringStartIndex + 4, 2)
		+ "-" + oriFileName.substr(TimeStringStartIndex + 6, 2)
		+ " 00:00:00";

	unique_ptr<double[]> oriData(new double[row * col]);//存储
	unique_ptr<int[]> spData(new int[row * col]);//存储

	gdalOpt::readGeoTiff(oriPath.c_str(), oriData.get());
	hdfOpt ho(Def);
	ho.readHDF(spPath, spData.get());

	vector<vector<double>> oriImg(row);//二维数组，边缘不处理
	vector<vector<int>> spImg(row);//二维数组，边缘不处理
	for (int i = 0; i < row; ++i) {
		vector<double> tmp1(col);
		vector<int> tmp2(col);
		oriImg[i] = tmp1;
		spImg[i] = tmp2;
	}

	//将一维数组转换为二维，方便处理
	for (int i = 0; i < row * col; i++)
	{
		int _rowNow = i / col;//行号
		int _colNow = i % col;//列号
		if (_rowNow == 0 || _colNow == 0 || _rowNow == row - 1 || _colNow == col - 1) continue;//边界点不添加

		//oriImg[_rowNow, _colNow] = oriData[i] * mScale;//乘以系数
		if (oriData[i] != fillValue)
		{
			oriImg[_rowNow][_colNow] = oriData[i] * mScale;//乘以系数
		}
		else
		{//将0和负值赋值为0
			oriImg[_rowNow][_colNow] = 0.0;
		}
		if (spData[i] > 0)
		{
			spImg[_rowNow][_colNow] = spData[i];
		}
		else
		{
			spImg[_rowNow][_colNow] = 0;
		}
	}

	//寻找结点
	vector<Node> nodes;//结点链表
	Pixel2Node(spImg, nodes, row, col);

	//所有线
	vector<Line> lines;
	//结点连成线
	Point2Line(spImg, nodes, lines, row, startLat, resolution);

	//线构成面
	vector<Poly> polygons;//所有面
	Line2Polygon(oriImg, spImg, lines, polygons, row, startLat, resolution);
	
	//保存shp	
	gdalOpt::save(outPath, startTime, AbnormalType, -180, startLat, resolution, polygons);
}

void Vectorization(vector<string>& oriFileNames, vector<string>& spFileNames, string outFolderPath) {
	for (int i = 0; i < spFileNames.size(); i++) {
		string oriPath = oriFileNames[i];//原始图像路径
		string spPath = spFileNames[i];//空间图像路径
		int beginIndex = spPath.find_last_of('\\') + 1;
		string fileName = spPath.substr(beginIndex, spPath.find_last_of('.') - beginIndex);//文件名没有后缀
		string outPath = outFolderPath + "\\" + fileName + ".shp";
		RasterToVectorBasedonSpace(oriPath, spPath, outPath, "positive");
	}
}