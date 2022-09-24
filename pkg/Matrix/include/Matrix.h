/*
Name:Matrix
Author: wcb892534877@icloud.com
Description:矩阵计算
Date: 2018-08-19 15:00
Copyright:伍程斌 中国科学院空天信息研究院
*/
#pragma once
//#include "stdafx.h"
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
using namespace std;
class _declspec(dllexport) CMatrix
{
private:
	long Row, Col;
	double **dArr;

public:
	CMatrix(long Row,long Col);		
	CMatrix();											//构造空矩阵
	CMatrix(long Row, long Col, double *dArr);			//构造矩阵
	CMatrix(long Row, long Col, double **dArr);			//构造矩阵
	CMatrix(string dArr);								//通过string构造[1 2 3;4 5 6;7 8 9]
	~CMatrix();

public:
	inline long GetRow() { return Row; }
	inline long GetCol() { return Col; }
	inline double **GetArray() { return dArr; };

private:
	long IndexOfMax(long Row);

public:
	static CMatrix Inverse(CMatrix *);					//矩阵求逆
	static CMatrix Transposition(CMatrix*);				//矩阵转置
	static CMatrix GaussElimination(CMatrix*);			//高斯消去
	static CMatrix CreateIdentityMatrix(long Row);		//创建单位矩阵
	static double Det(CMatrix *);						//行列式值
	static void Print(CMatrix *);						//打印矩阵信息

	CMatrix operator *(CMatrix);					    //矩阵乘法

	CMatrix operator +(CMatrix);					    //矩阵加法
	CMatrix operator -(CMatrix);					    //矩阵减法

	CMatrix operator =(CMatrix);						//矩阵赋值
	CMatrix* operator =(CMatrix*);						//矩阵赋值
	CMatrix ConstantMultiplication(double);				//矩阵乘法(常数)
public:
	void CreateEmptyMatrix(long Row, long Col);			//创建空(全零)矩阵
	void ClearMatrix();									//矩阵置空
	void SwapRow(long r1,long r2);						//交换r1,r2行
public:
	CMatrix(CMatrix &obj);								//拷贝构造
	CMatrix(CMatrix *obj);								//拷贝构造

public:
	vector<string> split(const string &s, const string &seperator);
	

};


