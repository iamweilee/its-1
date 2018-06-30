#ifndef FUNC_H
#define FUNC_H

#include "datalib/Array.h"
#include "datalib/SimpleMath.h"



double DATALIB_API Average(NumericSeries prices, int length);
double DATALIB_API AverageFC(NumericSeries avgs, NumericSeries prices, int length);
//@param x_averages ��ָ��ƽ��������
//@param price ���۸�����
//@param length ������
double DATALIB_API XAverage(NumericSeries xAvgs, NumericSeries prices, int length);

double DATALIB_API Summation(NumericSeries prices, int length);
double DATALIB_API SummationFC(NumericSeries sums, NumericSeries prices, int length);

double DATALIB_API Highest(NumericSeries prices, int length, int offset = 0);
double DATALIB_API Lowest(NumericSeries prices, int length, int offset = 0);

// �����´����ϴ�
bool DATALIB_API DownCross(NumericSeries fastMovAvg, NumericSeries slowMovAvg);
bool DATALIB_API UpCross(NumericSeries fastMovAvg, NumericSeries slowMovAvg);

// ���ǿ��
double DATALIB_API RelativeStrength(NumericSeries prices, int length);
double DATALIB_API RelativeStrengthFC(NumericSeries rsis, NumericSeries prices, int length);

// ��׼��
double DATALIB_API StandardDev(NumericSeries avgs, NumericSeries prices, int length);

#endif