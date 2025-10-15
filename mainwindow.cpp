#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <QMetaType>
#include "plotprocess.h"
#include "dataservice.h"
#include "commonfun.h"
#include <QElapsedTimer>
#include "msgbox.h"
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<QMap<int, QVector<QVector<QCPGraphData>>>>("QMap<int, QVector<QVector<QCPGraphData>>>&");
    connect(this,&MainWindow::modelDataRequest,dataService::getInstance(),&dataService::handleModelDataRequest);
    connect(plotProcess::getInstance(),&plotProcess::plotDataReady,this,&MainWindow::handlePlotDataReady);
    connect(ui->QcpText,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
    if(nullptr == CprjConfig)
    {
        CprjConfig = new projectConfigure;
    }
    if(nullptr == CwindowDisp)
    {
        CwindowDisp = new windowDisplay;
    }

    //初始化代码
    ui->QcpText->setInteractions(QCP::iRangeZoom);

    CwindowDisp->windPlotType[0] = 2;
    QcpText_2 = nullptr;
    QcpText_3 = nullptr;
    QcpText_4 = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;

    if(nullptr != CprjConfig)
    {
        delete  CprjConfig;
    }
    if(nullptr != CwindowDisp)
    {
        delete  CwindowDisp;
    }
}

void MainWindow::hideForm()
{
    ui->stackedWidget->hide();
}

void MainWindow::plotDataByAxis(QVector<QVector<QCPGraphData> > QcpData2D, qint64 windowStart, qint64 windowEnd,MyCustomPlot *& plotBoard,int axis)
{
    int RowSize = QcpData2D.size();
    //查找36个通道的最大最小值
    //并进行归一化到-50-50转换
    double ymin = 0,ymax = 0;
    for(int i=0;i<RowSize;i++)
    {
        int ColumnSize = QcpData2D[i].size();
        for(int j=0;j<ColumnSize;j++)
        {
            double& dy = QcpData2D[i][j].value;
            //归一化
            //等距通道显示样式 每个通道value值映射到2，例如通道一为-50到50，通道二为-49到51，。。。。，通道36为-15到85。
            if(1 == axis || 2 == axis)
            {
                dy = dy/plotProcess::getInstance()->hallUpperLimitXY*50;
            }
            else if(3 == axis)
            {
                dy = dy/plotProcess::getInstance()->hallUpperLimitZ*50;
            }
            if(windowStart >= 0)
            {
                QcpData2D[i][j].key += windowStart;
            }
            if(dy<ymin)
            {
                ymin = dy;
            }
            if(dy>ymax)
            {
                ymax = dy;
            }
            dy = dy + i;   //给每个探头增加偏移量，从而36个探头一张图显示
        }
    }

    plotBoard->xAxis->setLabel("距离");
    plotBoard->yAxis->setLabel("磁场强度");
    plotBoard->xAxis->setRange(windowStart,windowEnd);
    double yRangeMax = ymax + 35 + 1; //上下留1的余量
    double yRangeMin = ymin - 1;
    plotBoard->yAxis->setRange(yRangeMin,yRangeMax);
    //使用data指针
    QPen pen;
    pen.setColor(QColor(0,204,0));
    pen.setWidth(CwindowDisp->penWidth);
    for(int i=0;i<RowSize;i++)
    {
        if(plotBoard->graphCount() < 36)
        {
            plotBoard->addGraph();
        }
        plotBoard->graph(i)->setPen(pen);
        QSharedPointer<QCPGraphDataContainer>dataContainer = plotBoard->graph(i)->data();
        dataContainer->clear();
        dataContainer->set(QcpData2D[i],true);
    }
    plotBoard->replot();
}

void MainWindow::setMutiWindow(int Num)
{
    switch (Num) {
        case 1:   //1窗口显示
        {
            ui->gridLayout->addWidget(ui->infowidget,0,0,1,1);
            if(nullptr != QcpText_2)
            {
                delete QcpText_2;
                QcpText_2 = nullptr;
            }
            if(nullptr != QcpText_3)
            {
                delete QcpText_3;
                QcpText_3 = nullptr;
            }
            if(nullptr != QcpText_4)
            {
                delete QcpText_4;
                QcpText_4 = nullptr;
            }
            break;
        }
        case 2:   //2个窗口
        {
            ui->gridLayout->addWidget(ui->infowidget,0,0,1,1);
            if(nullptr != QcpText_3)
            {
                delete QcpText_3;
                QcpText_3 = nullptr;
            }
            if(nullptr != QcpText_4)
            {
                delete QcpText_4;
                QcpText_4 = nullptr;
            }
            if(nullptr == QcpText_2)
            {
                QcpText_2 = new MyCustomPlot(ui->plotPage);
                QcpText_2->setObjectName(QString::fromUtf8("QcpText_2"));
                QcpText_2->setEnabled(true);
                ui->gridLayout->addWidget(QcpText_2,2,0,1,1);
            }
            else {
                ui->gridLayout->addWidget(QcpText_2,2,0,1,1);
            }
            break;
        }
        case 3:
        {
            ui->gridLayout->addWidget(ui->infowidget,0,0,1,1);
            if(nullptr != QcpText_4)
            {
                delete QcpText_4;
                QcpText_4 = nullptr;
            }
            if(nullptr == QcpText_2)
            {
                QcpText_2 = new MyCustomPlot(ui->plotPage);
                QcpText_2->setObjectName(QString::fromUtf8("QcpText_2"));
                QcpText_2->setEnabled(true);
                ui->gridLayout->addWidget(QcpText_2,2,0,1,1);
            }
            else {
                ui->gridLayout->addWidget(QcpText_2,2,0,1,1);
            }
            if(nullptr == QcpText_3)
            {
                QcpText_3 = new MyCustomPlot(ui->plotPage);
                QcpText_3->setObjectName(QString::fromUtf8("QcpText_3"));
                QcpText_3->setEnabled(true);
                ui->gridLayout->addWidget(QcpText_3,3,0,1,1);
            }
            else {
                ui->gridLayout->addWidget(QcpText_3,3,0,1,1);
            }
            break;
        }
        case 4:
        {
            ui->gridLayout->addWidget(ui->infowidget,0,0,1,2);
            if(nullptr == QcpText_2)
            {
                QcpText_2 = new MyCustomPlot(ui->plotPage);
                QcpText_2->setObjectName(QString::fromUtf8("QcpText_2"));
                QcpText_2->setEnabled(true);
                ui->gridLayout->addWidget(QcpText_2,1,1,1,1);
            }
            else {
                ui->gridLayout->addWidget(QcpText_2,1,1,1,1);
            }
            if(nullptr == QcpText_3)
            {
                QcpText_3 = new MyCustomPlot(ui->plotPage);
                QcpText_3->setObjectName(QString::fromUtf8("QcpText_3"));
                QcpText_3->setEnabled(true);
                ui->gridLayout->addWidget(QcpText_3,2,0,1,1);
            }
            else {
                ui->gridLayout->addWidget(QcpText_3,2,0,1,1);
            }
            if(nullptr == QcpText_4)
            {
                QcpText_4 = new MyCustomPlot(ui->plotPage);
                QcpText_4->setObjectName(QString::fromUtf8("QcpText_4"));
                QcpText_4->setEnabled(true);
                ui->gridLayout->addWidget(QcpText_4,2,1,1,1);
            }
            else {
                ui->gridLayout->addWidget(QcpText_4,2,1,1,1);
            }
            break;
        }
    }
}

int MainWindow::handlePlotDataReady(QMap<int, QVector<QVector<QCPGraphData> > > &qmCPData)
{
    dataService::getInstance()->m_dataRwLock.lockForRead();
    QElapsedTimer qElapTimer;
    qElapTimer.start();
    if(qmCPData.empty())  //辅助空参数情况坐标轴翻页效果
    {
        QVector<QVector<QCPGraphData>> oneVector2D;
        for (int i=1;i<=4;i++) {
            qmCPData.insert(i,oneVector2D);
        }
    }
    //绘制磁力曲线
    if(1 == CwindowDisp->windNum)
    {
        switch (CwindowDisp->windPlotType[0])   //一个窗口
        {
            case 1:{    //X轴
                if(qmCPData.find(1) != qmCPData.end())
                {
                    QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[1];
                    plotDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,ui->QcpText,1);
                }
                break;
            }
            case 2:{    //Y轴
                if(qmCPData.find(2) != qmCPData.end())
                {
                    QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[2];
                    plotDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,ui->QcpText,2);
                }
                break;
            }
            case 3:{    //Z轴
                if(qmCPData.find(3) != qmCPData.end())
                {
                    QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[3];
                    plotDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,ui->QcpText,3);
                }
                break;
            }
            case 4:{    //VORTEX
                if(qmCPData.find(4) != qmCPData.end())
                {
                    QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[4];
                    plotDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,ui->QcpText,4);
                }
                break;
            }
        }
    }
    else if(2 == CwindowDisp->windNum)  //两个窗口
    {

    }
    else if(3 == CwindowDisp->windNum)  //三个窗口
    {

    }
    else {  //四个窗口

    }
    qDebug()<<"绘制图像所花费时间:"<<qElapTimer.elapsed()<<"ms";
    dataService::getInstance()->m_dataRwLock.unlock();
    return 0;
}

void MainWindow::handleSig_wheelEvent()
{
   qDebug()<<"x轴最小值"<<ceil(ui->QcpText->xAxis->range().lower);
   qDebug()<<"x轴最大值"<<floor(ui->QcpText->xAxis->range().upper);
   qint64 xLower = static_cast<qint64>(ceil(ui->QcpText->xAxis->range().lower));
   qint64 xUpper = static_cast<qint64>(floor(ui->QcpText->xAxis->range().upper));

   if(xLower <= 0)
   {
       CwindowDisp->startPos = 0;
   }
   else {
       CwindowDisp->startPos = xLower;
   }

   if(xLower < 0 && xUpper > 0)
   {
       CwindowDisp->offset = xUpper;
   }
   else if(xLower >0 && xUpper>0)
   {
       CwindowDisp->offset = xUpper - xLower;
   }
   else {
       CwindowDisp->offset = 0;
   }

   //超出Int范围不处理
   qint64 iSum = CwindowDisp->startPos + CwindowDisp->offset;
   if(iSum<INT_MAX && iSum>0)
   {
       QString qsfilePath = CprjConfig->dataDirPath+CprjConfig->curFileName;
       emit modelDataRequest(qsfilePath,CwindowDisp->startPos,CwindowDisp->offset);
   }
}

void MainWindow::on_plotWindow_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget->show();

    //emit
    if(CwindowDisp->bFirstPlot)
    {
        //获取qsfilePath下的文件列表 以及路径合法性判断
        CprjConfig->dataDirPath = "D:\\Soft\\管道漏磁内检测软件\\data\\测试数据\\测试数据12.10\\104\\";

        QDir dir(CprjConfig->dataDirPath);
        if(!dir.exists())
        {
            msgBox::show("告警","文件夹不存在",2);
            return;
        }

        //------未来迁移到打开项目按钮逻辑里-------------
        QMap<QString,int>().swap(CprjConfig->fileNameBytesMap);

        //获取文件夹下文件名
        int bytesNum = 0;
        QString qsfilePath="";
        QStringList filters = {"*.bin"};
        QFileInfoList files = dir.entryInfoList(filters,QDir::Files,QDir::Name);
        for (const QFileInfo &file : files) {
            QString fileName = file.fileName();
            CprjConfig->fileNameVec.append(fileName);
            QString subString = fileName.mid(9,3);
            int result = QString::compare("000",subString);
            if(0 == result)
            {
                CprjConfig->curFileName = file.fileName();
                qsfilePath = CprjConfig->dataDirPath+CprjConfig->curFileName;
            }
            //查询每个文件的帧数
            bytesNum = static_cast<int>(file.size());
            int frameNum = (bytesNum-20)/268;
            CprjConfig->fileNameBytesMap.insert(fileName,frameNum);
        }
        //------------------------------------------

        emit modelDataRequest(qsfilePath,CwindowDisp->startPos,CwindowDisp->offset);
        CwindowDisp->bFirstPlot = false;
    }
}

void MainWindow::on_nextPageBtn_clicked()
{
    CwindowDisp->startPos += CwindowDisp->pageOffset;
    QString qsfilePath = CprjConfig->dataDirPath+CprjConfig->curFileName;

    if(CwindowDisp->startPos < 0 && CwindowDisp->startPos+CwindowDisp->offset > 0)
    {
        emit modelDataRequest(qsfilePath,0,CwindowDisp->startPos+CwindowDisp->offset);
    }
    else if(CwindowDisp->startPos < 0 && CwindowDisp->startPos+CwindowDisp->offset < 0)
    {
        emit modelDataRequest(qsfilePath,0,0);
    }
    else {
        emit modelDataRequest(qsfilePath,CwindowDisp->startPos,CwindowDisp->offset);
    }
}

void MainWindow::on_previousPageBtn_clicked()
{
    CwindowDisp->startPos -= CwindowDisp->pageOffset;
    QString qsfilePath = CprjConfig->dataDirPath+CprjConfig->curFileName;

    if(CwindowDisp->startPos < 0 && CwindowDisp->startPos+CwindowDisp->offset > 0)
    {
        emit modelDataRequest(qsfilePath,0,CwindowDisp->startPos+CwindowDisp->offset);
    }
    else if(CwindowDisp->startPos < 0 && CwindowDisp->startPos+CwindowDisp->offset < 0)
    {
        emit modelDataRequest(qsfilePath,0,0);
    }
    else {
        emit modelDataRequest(qsfilePath,CwindowDisp->startPos,CwindowDisp->offset);
    }
}

void MainWindow::on_projectManage_triggered()
{
    //test
    setMutiWindow(2);
}

void MainWindow::on_userManage_triggered()
{
    //test
    setMutiWindow(1);
}

void MainWindow::on_login_triggered()
{
    //test
    setMutiWindow(3);
}

void MainWindow::on_logout_triggered()
{
    //test
    setMutiWindow(4);
}

void MainWindow::on_windowNumSet_triggered()
{

}
