#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "define.h"
#include "qcustomplot.h"
#include "mycustomplot.h"
#include "windownumsetdialog.h"
#include "logindlg.h"
#include "databaseworker.h"
#include <QStandardItemModel>

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
    //前台绘制波形曲线图（曲线图绘制子函数）
    //QcpData2D-36通道电磁数据
    //windowStart-图表开始点,windowEnd-图表结束点
    //plotBoard - MyCustomPlot画板
    //axis-对应坐标轴 1-X，2-Y，3-Z，4-Vortex
    void plotLineChartDataByAxis(QVector<QVector<QCPGraphData>>QcpData2D, qint64 windowStart, qint64 windowEnd, MyCustomPlot *&plotBoard, int axis, int boxNum, int boxSize);
    //handlePlotDataReady中的子函数（曲线图绘制函数）
    //sensorType绘图传感器类型，qmCPData绘制数据，plotBoard画板
    //boxNum为阿拉伯数字减一，循环变量
    void plotLineChartbySensorType(int sensorType, QMap<int,QVector<QVector<QCPGraphData>>>&qmCPData, MyCustomPlot*& plotBoard, int boxNum, int boxSize);
    //doubleArray的外层数组0-代表X，1-Y。。。3代表Vortex（存储灰度图像处理数据），别的参数同上
    //灰度图像处理函数
    //doubleArray2D外层数组大小为4，0-X，1-Y以此类推
    void plotGrayChartbySensorType(int sensorType, QMap<int,QVector<QVector<QCPGraphData>>>&qmCPData, MyCustomPlot*& plotBoard, int boxNum, int boxSize, QVector<QVector<double>>&doubleArray2D);
    //灰度图绘制子函数
    //QcpData2D-36通道电磁数据
    //windowStart-图表开始点,windowEnd-图表结束点
    //plotBoard - MyCustomPlot画板
    //axis-对应坐标轴 1-X，2-Y，3-Z，4-Vortex
    //根据axis值传入对应的doubleArray
    void plotGrayChartDataByAxis(QVector<QVector<QCPGraphData>>QcpData2D, qint64 windowStart, qint64 windowEnd, MyCustomPlot *&plotBoard, int axis,
                                 int boxNum, int boxSize, QVector<double> &doubleArray);
    //多窗体设置函数  //Num窗体数量
    void setMutiWindow(int Num);
    //设置图表标题
    void setCPtittle(MyCustomPlot*& plotboard, QString strTitle);
    //初始化数据库及数据库后台线程
    void initialDatabase();
    //清图函数，传入画板参数
    void clearPlotboard(MyCustomPlot*& plotBoard);
    //清图函数子函数，对于item项：只清除QCPItemPixmap，保留QCPItemTracer和QCPItemText，从而避免QCPItemText被删除后依然调用
    void removePixmapItem(MyCustomPlot*& plotBoard);
    //openCV
    //openCV功能测试函数
    void testOpenCV();

    //窗口控件
    //多窗体窗口控件(窗体1在ui中)  //QcpText//窗体1
    MyCustomPlot *QcpText_2; //窗体2
    MyCustomPlot *QcpText_3; //窗体3
    MyCustomPlot *QcpText_4; //窗体4
    windowNumSetDialog *windowNumSetDlg; //多窗体设置页面
    loginDlg *m_loginDlg;//登录界面
    QStandardItemModel *userTableModel;
    QStandardItemModel *prjTableModel;
    //优化滚轮和拖动手势显示效果，在绘图中不触发(只针对滚轮和拖动，和上一页下一页、未来跳转功能无关)
    bool m_plotting;//false未进行中，true绘制中

signals:
    //startPos需大于等于0（根据主窗体CwindowDisp类中的窗体实际坐标轴判定，从文件中开始读取的位置，因此必须大于0）
    //添加y轴的范围
    //无需传入qsfilePath，在槽函数中会拼接生成,且当前所读文件会记录在curFileNamevec中
    void modelDataRequest(QString& qsfilePath,qint64 startPos,qint64 offset);
    void plotCacheDataRequestBybox();
    void initalWinNum();
    void queryAllUsers();
    void deleteUserRequest(int row,QString name);
    void queryAllProjects();
    //type==1打开工程按钮中逻辑;2工程详细按钮逻辑
    void queryProjectById(int id,int type);
    void deleteProjectRequest(int row,int projectId);

private slots:
    int handlePlotDataReadyBybox(QVector<QMap<int,QVector<QVector<QCPGraphData>>>> &qmCPDatavec);
    void handleWindowNumSetData(int windNum,int* windPlotType,int* windSensorType);
    void handleLoginResult(int id, QString name, QString password, QString permission);
    void handleQryAllUsersResult(QVector<userDataModel>&vecUsers);
    void handleShowAddNewUser(QString name,QString password,QString permission);
    void handleShowEditUser(int row,QString name,QString password,QString permission);
    void handleShowDeleteUser(int row);
    void handleQryAllPrjsResult(QVector<projectDataModel>&vecPrjs);
    void handleQryProjectByIdResult(projectDataModel& onePrj);
    void handleShowAddNewProject(projectDataModel& onePrj);
    void handleShowEditProject(int row,projectDataModel& onePrj);
    void handleShowDeleteProject(int row);
    void handleShowDetailProject(projectDataModel& onePrj);
    //原有触发逻辑都不变，只有在读的时候多个盒子一起读，在转换的时候多个盒子一起转换
    void on_plotWindow_triggered();

    void on_nextPageBtn_clicked();

    void on_previousPageBtn_clicked();

    void on_projectManage_triggered();

    void on_userManage_triggered();

    void on_login_triggered();

    void on_logout_triggered();

    void on_windowNumSet_triggered();

    void on_newUser_clicked();

    void on_editUser_clicked();

    void on_deleteUser_clicked();
    //tooltip项目列表提示
    void showTooltip(const QModelIndex& index);

    void on_openPrj_clicked();

    void on_closePrj_clicked();

    void on_newPrj_clicked();

    void on_editPrj_clicked();

    void on_deletePrj_clicked();

    void on_detailPrj_clicked();

public slots:
    void handleSig_wheelEvent(qint64 xLower,qint64 xUpper,qint64 yLower,qint64 yUpper);

private:
    Ui::MainWindow *ui;
    databaseWorker *m_dbWorker;

public:
    projectConfigure *CprjConfig = nullptr;
    windowDisplay *CwindowDisp = nullptr;
    userDataModel *CcurrentUserMod = nullptr;
};

Q_DECLARE_METATYPE(QCPGraphData)

#endif // MAINWINDOW_H
