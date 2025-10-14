
#include "commonfun.h"
#include <QIODevice>
#include <QDataStream>
#include <cmath>
#include <QDebug>
#include "qcustomplot.h"

// half精度（16位）转float函数
float halfToFloat(quint16 half)
{
	quint32 sign = (half >> 15) & 0x00000001;  //1位
	quint32 exp = (half >> 10) & 0x0000001f;   //5位
	quint32 mant = half & 0x000003ff;          //10位

	if (exp == 0) {
		if (mant == 0) {
			return sign ? -0.0f : 0.0f;
		} else {
			return (sign ? -1 : 1) * std::ldexp(mant, -24);
		}
	} else if (exp == 31) {
		return mant == 0 ? (sign ? -INFINITY : INFINITY) : NAN;
	}
	return (sign ? -1 : 1) * std::ldexp(mant + 1024, exp - 25);
}

int readDataFromBin(QString qsfilePath)
{
	QFile file(qsfilePath);
	if (file.open(QIODevice::ReadOnly)) {
		// 打开失败
		// 变量声明
		const qint64 bufferSize = 268 * 1024 * 4; // 804KB缓冲区
		QByteArray buffer;
		qint64 totalBytes = 0;

		//0-19字节
		buffer = file.read(20);
		QString timeStr = buffer;
		qDebug() << "Time String:" << timeStr; 

		// ...可按需解析前20字节...

		QElapsedTimer qtimer;
		int cnt = 0;
        QVector<int> x;
        QVector<short> y;
        QVector<QCPGraphData>mData;
        QCPGraphData newPoint;
        static int aisX = 0;
        //内部结构
		//每268个字节，示例：将buffer中的数据转为float数组，减少拷贝
		while (!file.atEnd()) {
			qtimer.start();
			buffer = file.read(bufferSize);
			totalBytes += buffer.size();
			for (int i = 0; i < bufferSize-4; ++i) {
				// quint16 halfVal = (dataPtr[2*i] << 8) | dataPtr[2*i+1]; // 大端
				const unsigned char lowAddr =  (unsigned char)buffer[i];
				const unsigned char highAddr =  (unsigned char)buffer[++i];
				short halfVal = lowAddr & 0xFF;
				halfVal |= ((highAddr << 8) & 0xFF00); // 小端
                // TODO: 使用halfVal进行分析或存储
                x.append(aisX);
                aisX += 3;//增加采样间距
                y.append(halfVal);
				// qDebug() << "原先数据" << QString::number(halfVal,16) << "转换后short" << halfVal;
			}
			// 注意：如有字节对齐或大小端问题需处理
			qDebug() << "第"<< ++cnt <<"次循环花费时间"<<qtimer.elapsed()<<"ms";
		}

        //绘图测试
        //emit 数据到桌面
		// 转换测试
		// 位操作时 使用一个unsigned int变量来作为位容器。
		// unsigned char* bytes;
        // short aa = -6112;

		// //byte[] bytes = new byte[4];
		// memset(bytes, 0, 2);
		// bytes[0] = (unsigned char)(0xff & aa);
		// bytes[1] = (unsigned char)((0xff00 & aa) >> 8);

		// QByteArray qdisplay((const char*)bytes);
        // qDebug()<<QString::number(aa,16);

		// // // 位操作时 使用一个unsigned int变量来作为位容器。
        //  short addr = bytes[0] & 0xFF;
        //  addr |= ((bytes[1] << 8) & 0xFF00);
        //  qDebug()<<addr;

		file.close();

		qDebug() << "Total bytes read:" << totalBytes;

		return static_cast<int>(totalBytes); // 返回读取字节数或自定义结果
		// return 0;

	}
	else
	{
		qDebug()<<"打开文件失败";
		return -1;
	}
	
}

unsigned char xortest(unsigned char *data, unsigned char size)
{
	unsigned char dxor = 0;
	unsigned char i=0;
	for(i = 0; i < size; ++ i)
		dxor ^= data [i];
	return dxor;
}

quint16 getShortfromCharLittle(const unsigned char ucLow, const unsigned char ucHigh)
{
    quint16 sint=0;
    sint = ucLow & 0xFF;
    sint |= ((ucHigh << 8) & 0xFF00); // 小端
    return sint;
}
