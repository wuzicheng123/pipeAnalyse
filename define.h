#ifndef DEFINE_H
#define DEFINE_H

#include <QString>
#include <QElapsedTimer>
#include <QThread>
#include <QVector>
#include <QMap>
#include <QFile>
#include <cmath>

class dataModel
{
public:
    quint16 _Hall1XAxis;
    quint16 _Hall1YAxis;
    quint16 _Hall1ZAxis;

    quint16 _Hall2XAxis;
    quint16 _Hall2YAxis;
    quint16 _Hall2ZAxis;

    quint16 _Hall3XAxis;
    quint16 _Hall3YAxis;
    quint16 _Hall3ZAxis;

    quint16 _Hall4XAxis;
    quint16 _Hall4YAxis;
    quint16 _Hall4ZAxis;

    quint16 _Hall5XAxis;
    quint16 _Hall5YAxis;
    quint16 _Hall5ZAxis;

    quint16 _Hall6XAxis;
    quint16 _Hall6YAxis;
    quint16 _Hall6ZAxis;

    //预留
    quint16 _Vortex1Channel;
    quint16 _Vortex2Channel;
    quint16 _Vortex3Channel;
    quint16 _Vortex4Channel;
    quint16 _Vortex5Channel;
    quint16 _Vortex6Channel;

    dataModel()
    {
        _Hall1XAxis=0;
        _Hall1YAxis=0;
        _Hall1ZAxis=0;

        _Hall2XAxis=0;
        _Hall2YAxis=0;
        _Hall2ZAxis=0;

        _Hall3XAxis=0;
        _Hall3YAxis=0;
        _Hall3ZAxis=0;

        _Hall4XAxis=0;
        _Hall4YAxis=0;
        _Hall4ZAxis=0;

        _Hall5XAxis=0;
        _Hall5YAxis=0;
        _Hall5ZAxis=0;

        _Hall6XAxis=0;
        _Hall6YAxis=0;
        _Hall6ZAxis=0;

        _Vortex1Channel=0;
        _Vortex2Channel=0;
        _Vortex3Channel=0;
        _Vortex4Channel=0;
        _Vortex5Channel=0;
        _Vortex6Channel=0;
    }
};

//false是不做变换使用原始数据，true是区间内数据
class dataModelPreProcessingFlag
{
public:
    bool _Hall1XAxisFlag;
    bool _Hall1YAxisFlag;
    bool _Hall1ZAxisFlag;

    bool _Hall2XAxisFlag;
    bool _Hall2YAxisFlag;
    bool _Hall2ZAxisFlag;

    bool _Hall3XAxisFlag;
    bool _Hall3YAxisFlag;
    bool _Hall3ZAxisFlag;

    bool _Hall4XAxisFlag;
    bool _Hall4YAxisFlag;
    bool _Hall4ZAxisFlag;

    bool _Hall5XAxisFlag;
    bool _Hall5YAxisFlag;
    bool _Hall5ZAxisFlag;

    bool _Hall6XAxisFlag;
    bool _Hall6YAxisFlag;
    bool _Hall6ZAxisFlag;


    dataModelPreProcessingFlag()
    {
        _Hall1XAxisFlag=false;
        _Hall1YAxisFlag=false;
        _Hall1ZAxisFlag=false;

        _Hall2XAxisFlag=false;
        _Hall2YAxisFlag=false;
        _Hall2ZAxisFlag=false;

        _Hall3XAxisFlag=false;
        _Hall3YAxisFlag=false;
        _Hall3ZAxisFlag=false;

        _Hall4XAxisFlag=false;
        _Hall4YAxisFlag=false;
        _Hall4ZAxisFlag=false;

        _Hall5XAxisFlag=false;
        _Hall5YAxisFlag=false;
        _Hall5ZAxisFlag=false;

        _Hall6XAxisFlag=false;
        _Hall6YAxisFlag=false;
        _Hall6ZAxisFlag=false;
    }
};

//工程配置类
class projectConfigure
{
public:
    double dInterval;//采样间隔
    QString dataDirPath;//当前文件夹路径
    QVector<QString>fileNameVec;//文件夹下的文件名
    QString curFileName;//当前读取数据的文件名
    QMap<QString,int>fileNameBytesMap;//文件名--数据帧数
    projectConfigure()
    {
        dInterval = 0;
        dataDirPath = "";
        curFileName = "";
    }
};

//窗体显示类
class windowDisplay
{
public:
    bool bFirstPlot;//第一次绘制
    qint64 startPos;//窗体显示波形数据起始点（可以为负数）
    qint64 offset;//窗体显示波形所需数据长度
    qint64 yLower;//y轴窗体显示区间下界
    qint64 yUpper;//y轴窗体显示区间上界
    qint64 pageOffset;//点击翻页按钮偏移量
    int windNum;//同屏窗体显示数量 1，2，3，4
    //windPlotType[0]对应图一，[1]对应图二。。。
    //windPlotType[0]:1-X,2-Y,3-Z,4-Vortex
    int windPlotType[4];//绘制内容
    //缩放比例scale（放大缩小影响scale）
    //offset，pageOffset和scale大小相关
    int penWidth;//绘图线宽
    windowDisplay()
    {
        bFirstPlot = true;
        startPos = 0;
        offset = 2000;
        yLower = 0;
        yUpper = 0;
        pageOffset = 1500;
        windNum = 1;
        for(int i=0;i<4;i++)
        {
            windPlotType[i]=0;
        }
        penWidth = 1;
    }
};

Q_DECLARE_METATYPE(dataModel)

#endif // DEFINE_H
