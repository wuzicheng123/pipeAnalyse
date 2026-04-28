#ifndef COMMONFUN_H
#define COMMONFUN_H

#include <QString>

//读文件
int readDataFromBin(QString qsfilePath);
// half精度（16位）转float函数
float halfToFloat(quint16 half);
//异或校验
unsigned char xortest(unsigned char *data, unsigned char size);
//小端转换
quint16 getShortfromCharLittle(const unsigned char ucLow, const unsigned char ucHigh);
//比较两个浮点是否相等
bool isEqual(double a,double b,double epsilon=1e-9);

#endif // COMMONFUN_H
