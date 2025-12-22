#ifndef DATASERVICE_H
#define DATASERVICE_H

#include <QObject>
#include <QMutex>
#include <QReadWriteLock>
#include "define.h"
#include "mainwindow.h"

class dataService : public QObject
{
    Q_OBJECT
public: 
    //key:1-6  对应探头一到六（多个盒子读取数据构成一个向量）
    QVector<QMap<int,QVector<dataModel>>>m_dataModelVec;
    static dataService* getInstance();
    //对m_dataModel中数据读写时加读写锁，写独占，读多个
    QReadWriteLock m_dataRwLock;

    //QMap中key对应六个探头（1-6），value是每个探头的数据结构
    //读取bin文件中全部数据
    int readDataFromBinAll(QString qsfilePath, QMap<int, QVector<dataModel> >& qmdataModel);
    //根据偏移量读取数据（鼠标操作，按键操作）  //differ = offset - 当前文件只剩下的值，因此还需去下一个文件读differ个帧（1帧=268字节）
    //offset 所求数据总帧数
    //differ=0时读取结束，否则循环读
    //return 0-成功 -1-打开文件失败  -2-下一个文件读
    int readDataFromBinByOffset(QString qsfilePath, QMap<int, QVector<dataModel> >& qmdataModel, qint64 startPos, qint64 offset, qint64 &differ);
    //获取主窗体
    void setMainWindow(MainWindow* exMainW);

signals:
    //更新为按盒子堆叠展示的模式解析，因此多嵌套一层QVector
    void dataModel2PlotProcessBybox(QVector<QMap<int,QVector<dataModel>>>&qmDataModelVec,QVector<QMap<int,QVector<dataModel>>>&onePreDatavec,qint64 startPos);

public slots:
    //startPos是窗体显示的开始位置（划分刻度为整体）
    //由于mainwindow中限制，startPos取值范围：>=0
    //boxDirPath每个采集盒的完整路径vec，curFileNamevec每个盒子下当前所读文件vec，二者结合为每个盒子下当前所读文件
    void handleModelDataRequest(QString& qsfilePath, qint64 startPos, qint64 offset);

private:
    explicit dataService(QObject *parent = nullptr);

    //禁止拷贝和赋值
    dataService(const dataService&) = delete;
    dataService& operator=(const dataService&)=delete;

    static dataService* m_instance;
    static QMutex m_mutex; //单例线程安全锁    
    MainWindow* m_exMainW;

};


#endif // DATASERVICE_H
