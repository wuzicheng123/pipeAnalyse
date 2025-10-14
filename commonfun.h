#ifndef COMMONFUN_H
#define COMMONFUN_H

#include "define.h"

//读文件
int readDataFromBin(QString qsfilePath);
// half精度（16位）转float函数
float halfToFloat(quint16 half);
//异或校验
unsigned char xortest(unsigned char *data, unsigned char size);
//小端转换
quint16 getShortfromCharLittle(const unsigned char ucLow, const unsigned char ucHigh);

#endif // COMMONFUN_H
