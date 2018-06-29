#ifndef FUNC_H
#define FUNC_H

#include "libdata/Array.h"
#include "libdata/SimpleMath.h"



double Average(NumericSeries prices, int length);
double AverageFC(NumericSeries avgs, NumericSeries prices, int length);
//@param x_averages ��ָ��ƽ��������
//@param price ���۸�����
//@param length ������
double XAverage(NumericSeries xAvgs, NumericSeries prices, int length);

double Summation(NumericSeries prices, int length);
double SummationFC(NumericSeries sums, NumericSeries prices, int length);

double Highest(NumericSeries prices, int length, int offset = 0);
double Lowest(NumericSeries prices, int length, int offset = 0);

// �����´����ϴ�
bool DownCross(NumericSeries fastMovAvg, NumericSeries slowMovAvg);
bool UpCross(NumericSeries fastMovAvg, NumericSeries slowMovAvg);

// ���ǿ��
double RelativeStrength(NumericSeries prices, int length);
double RelativeStrengthFC(NumericSeries rsis, NumericSeries prices, int length);

// ��׼��
double StandardDev(NumericSeries avgs, NumericSeries prices, int length);

#endif