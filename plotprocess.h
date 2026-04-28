#ifndef PLOTPROCESS_H
#define PLOTPROCESS_H

#include <QObject>
#include <QMutex>
#include "define.h"
#include "qcustomplot.h"

class plotProcess : public QObject
{
    Q_OBJECT
    friend class MainWindow;
public:
    static plotProcess* getInstance();
    QVector<QMap<int,QVector<QVector<QCPGraphData>>>>m_CPDataVec;
    //vec为增加盒子逻辑，按盒子顺序存入vec中
    //qmDataModel的key 123456 对应探头123456
    //qmCPData的key 1234对应HallX-Y-Z-Vortex四种视图
    //qmCPData的value是二维数组，1-6行对应探头一的6个传感器，7-12行对应探头二的6个传感器，以此类推
    int dataModel2PlotDataBybox(QVector<QMap<int,QVector<dataModel>>>&qmDataModelvec, QVector<QMap<int,QVector<QVector<QCPGraphData>>>>&qmCPDatavec);
    //数据预处理（过滤异常数据）,处理硬件溢出数据,规则如下
    //以浮点数为计算单位
    //xy轴的正上限16383.5，下限-16383.5；
    //z轴的正上限8191.75，下限-8191.75
    //startPos大于0时，qmPreCPData为前一帧数据，否则为空
    //针对Hall数据的预处理
    void dataPreProcessingBybox(QVector<QMap<int,QVector<QVector<QCPGraphData>>>> &qmCPDatavec, QVector<QMap<int,QVector<QVector<QCPGraphData>>>>&qmPreCPDatavec, qint64 startPos);
    //dataPreProcessing中的子函数。功能：处理一条通道中的其中一点。包含参数：前一点、当前点、状态标志、判断阈值
    void dataPreProcessOnePoint(QCPGraphData& preData,QCPGraphData& currentData,quint8& transFlag,double& threshold,double& hallUpperLimit,double& hallLowerLimit);
    //dataModel2PlotData中每个探头的解析子函数   每个探头的数据oneModelVec
    //根据rowOffest指定QVector<QVector<QCPGraphData> >数组行号 取值（0，6，12，18，24，30）
    int dataModel2PlotDataByProbe(QVector<dataModel> &oneModelVec, QMap<int, QVector<QVector<QCPGraphData> > > &qmCPData, int rowOffest);
    //raw原始数据，Axis是轴向1-X、2-Y、3-Z，后面四个参数是寄存器配置的参数，res的值与Axis相关
    float convert2float(quint16& raw,quint8 Axis,quint8 gain_sel,quint8 hallconf,quint8 res,quint8 tcmp_en);
    //初始化
    void initalPlotProcess();

signals:
    //修改类型，update==0全部修改，update==1修改灰度和伪彩色
    int plotDataReadyBybox(QVector<QMap<int,QVector<QVector<QCPGraphData>>>> &qmCPDatavec,int updateType);

public slots:
    void handledataModel2PlotProcessBybox(QVector<QMap<int,QVector<dataModel>>>&qmDataModelVec,QVector<QMap<int,QVector<dataModel>>>&onePreDatavec,qint64 startPos);
    void handleplotCacheDataRequestBybox(int updateType);

private:
    explicit plotProcess(QObject *parent = nullptr);
    plotProcess(const plotProcess&) = delete;
    plotProcess& operator=(const plotProcess&) = delete;

    static plotProcess* m_instance;
    static QMutex m_mutex; //单例线程安全锁

    //和浮点转换相关的参数
    float gain_multipliers[8];
    float base_xy_sens_hc0;
    float base_z_sens_hc0;
    float base_xy_sens_hc0xc;
    float base_z_sens_hc0xc;

    //上一帧数据，key-1,2,3对应x，y，z轴，QVector是大小为36的数组，表示36个通道。
    QMap<int,QVector<QCPGraphData>> m_preCPData;
    //key-1,2,3对应x，y，z轴，QVector是大小为36的数组，表示36个通道。
    //0是代表使用原始数据，1是做从大变小预处理偏移计算，2是做从小变大预处理偏移计算
    QMap<int,QVector<quint8>>m_preProcessFlag;

public:
    double hallUpperLimitXY;
    double hallLowerLimitXY;
    double hallUpperLimitZ;
    double hallLowerLimitZ;
};

#endif // PLOTPROCESS_H
