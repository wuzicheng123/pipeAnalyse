#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "define.h"
#include "qcustomplot.h"
#include "mycustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void hideForm();
    //前台绘制波形曲线图
    //QcpData2D-36通道电磁数据
    //windowStart-图表开始点,windowEnd-图表结束点
    //plotBoard - MyCustomPlot画板
    //axis-对应坐标轴 1-X，2-Y，3-Z，4-Vortex
    void plotDataByAxis(QVector<QVector<QCPGraphData>>QcpData2D, qint64 windowStart, qint64 windowEnd, MyCustomPlot *&plotBoard, int axis);
    //多窗体设置函数  //Num窗体数量
    void setMutiWindow(int Num);

    //窗口控件
    //多窗体窗口控件(窗体1在ui中)  //QcpText//窗体1
    MyCustomPlot *QcpText_2; //窗体2
    MyCustomPlot *QcpText_3; //窗体3
    MyCustomPlot *QcpText_4; //窗体4

signals:
    //startPos需大于等于0（根据主窗体CwindowDisp类中的窗体实际坐标轴判定，从文件中开始读取的位置，因此必须大于0）
    //添加y轴的范围
    void modelDataRequest(QString& qsfilePath,qint64 startPos,qint64 offset);

private slots:
    int handlePlotDataReady(QMap<int, QVector<QVector<QCPGraphData> > > &qmCPData);

    void on_plotWindow_triggered();

    void on_nextPageBtn_clicked();

    void on_previousPageBtn_clicked();

    void on_projectManage_triggered();

    void on_userManage_triggered();

    void on_login_triggered();

    void on_logout_triggered();

    void on_windowNumSet_triggered();

    void handlePlottableClick(QCPAbstractPlottable* plottable,int dataIndex,QMouseEvent* event);

public slots:
    void handleSig_wheelEvent(qint64 xLower,qint64 xUpper,qint64 yLower,qint64 yUpper);

private:
    Ui::MainWindow *ui;

public:
    projectConfigure *CprjConfig = nullptr;
    windowDisplay *CwindowDisp = nullptr;
};

Q_DECLARE_METATYPE(QCPGraphData)

#endif // MAINWINDOW_H
