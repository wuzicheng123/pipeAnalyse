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
    this->setWindowTitle("管道分析软件");

    qRegisterMetaType<QMap<int, QVector<QVector<QCPGraphData>>>>("QMap<int, QVector<QVector<QCPGraphData>>>&");
    connect(this,&MainWindow::modelDataRequest,dataService::getInstance(),&dataService::handleModelDataRequest);
    connect(this,&MainWindow::plotCacheDataRequest,plotProcess::getInstance(),&plotProcess::handleplotCacheDataRequest);
    connect(plotProcess::getInstance(),&plotProcess::plotDataReady,this,&MainWindow::handlePlotDataReady); 
    if(nullptr == CprjConfig)
    {
        CprjConfig = new projectConfigure;
    }
    if(nullptr == CwindowDisp)
    {
        CwindowDisp = new windowDisplay;
    }

    //初始化代码
    ui->QcpText_1->setInteractions(QCP::iRangeZoom);
    connect(ui->QcpText_1,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);

    CwindowDisp->windNum = 1;
    CwindowDisp->windSensorType[0] = 1;
    CwindowDisp->windPlotType[0] = 1;
    QcpText_2 = nullptr;
    QcpText_3 = nullptr;
    QcpText_4 = nullptr;
    windowNumSetDlg = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;

    if(nullptr != CprjConfig)
    {
        delete  CprjConfig;
        CprjConfig = nullptr;
    }
    if(nullptr != CwindowDisp)
    {
        delete  CwindowDisp;
        CwindowDisp = nullptr;
    }
    if(nullptr != windowNumSetDlg)
    {
        disconnect(windowNumSetDlg,&windowNumSetDialog::windowNumSetData,this,&MainWindow::handleWindowNumSetData);
        delete windowNumSetDlg;
        windowNumSetDlg = nullptr;
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
    if(0 == CwindowDisp->yLower && 0 == CwindowDisp->yUpper)
    {
        CwindowDisp->yLower = -4;
        CwindowDisp->yUpper = 38;
        plotBoard->yAxis->setRange(CwindowDisp->yLower,CwindowDisp->yUpper);
    }
    else {
        plotBoard->yAxis->setRange(CwindowDisp->yLower,CwindowDisp->yUpper);
    }
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

void MainWindow::plotbySensorType(int sensorType, QMap<int, QVector<QVector<QCPGraphData> > > &qmCPData, MyCustomPlot *&plotBoard)
{
    switch (sensorType)   //一个窗口
    {
        case 1:{    //X轴
            if(qmCPData.find(1) != qmCPData.end())
            {
                QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[1];
                plotDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,1);
            }
            break;
        }
        case 2:{    //Y轴
            if(qmCPData.find(2) != qmCPData.end())
            {
                QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[2];
                plotDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,2);
            }
            break;
        }
        case 3:{    //Z轴
            if(qmCPData.find(3) != qmCPData.end())
            {
                QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[3];
                plotDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,3);
            }
            break;
        }
        case 4:{    //VORTEX
            if(qmCPData.find(4) != qmCPData.end())
            {
                QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[4];
                plotDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,4);
            }
            break;
        }
    }
}

void MainWindow::setMutiWindow(int Num)
{
    switch (Num) {
        case 1:   //1窗口显示
        {
            ui->gridLayout->addWidget(ui->infowidget,0,0,1,1);
            if(nullptr != QcpText_2)
            {
                disconnect(QcpText_2,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
                delete QcpText_2;
                QcpText_2 = nullptr;
            }
            if(nullptr != QcpText_3)
            {
                disconnect(QcpText_3,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
                delete QcpText_3;
                QcpText_3 = nullptr;
            }
            if(nullptr != QcpText_4)
            {
                disconnect(QcpText_4,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
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
                disconnect(QcpText_3,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
                delete QcpText_3;
                QcpText_3 = nullptr;
            }
            if(nullptr != QcpText_4)
            {
                disconnect(QcpText_4,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
                delete QcpText_4;
                QcpText_4 = nullptr;
            }
            if(nullptr == QcpText_2)
            {
                QcpText_2 = new MyCustomPlot(ui->plotPage);
                QcpText_2->setObjectName(QString::fromUtf8("QcpText_2"));
                QcpText_2->setEnabled(true);
                ui->gridLayout->addWidget(QcpText_2,2,0,1,1);
                QcpText_2->setInteractions(QCP::iRangeZoom);
                connect(QcpText_2,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
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
                disconnect(QcpText_4,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
                delete QcpText_4;
                QcpText_4 = nullptr;
            }
            if(nullptr == QcpText_2)
            {
                QcpText_2 = new MyCustomPlot(ui->plotPage);
                QcpText_2->setObjectName(QString::fromUtf8("QcpText_2"));
                QcpText_2->setEnabled(true);
                ui->gridLayout->addWidget(QcpText_2,2,0,1,1);
                QcpText_2->setInteractions(QCP::iRangeZoom);
                connect(QcpText_2,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
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
                QcpText_3->setInteractions(QCP::iRangeZoom);
                connect(QcpText_3,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
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
                QcpText_2->setInteractions(QCP::iRangeZoom);
                connect(QcpText_2,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
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
                QcpText_3->setInteractions(QCP::iRangeZoom);
                connect(QcpText_3,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
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
                QcpText_4->setInteractions(QCP::iRangeZoom);
                connect(QcpText_4,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
            }
            else {
                ui->gridLayout->addWidget(QcpText_4,2,1,1,1);
            }
            break;
        }
    }
    //设置画板标题
    QVector<MyCustomPlot*>vecMyCP; //画板数组，作为函数入参
    vecMyCP.append(ui->QcpText_1);
    vecMyCP.append(QcpText_2);
    vecMyCP.append(QcpText_3);
    vecMyCP.append(QcpText_4);
    for(int i=0;i<Num;i++)
    {
        if(1 == CwindowDisp->windPlotType[i]) //曲线图
        {
            switch (CwindowDisp->windSensorType[i]) {
                case 1:
                {
                    setCPtittle(vecMyCP[i],"霍尔X轴");
                    break;
                }
                case 2:
                {
                    setCPtittle(vecMyCP[i],"霍尔Y轴");
                    break;
                }
                case 3:
                {
                    setCPtittle(vecMyCP[i],"霍尔Z轴");
                    break;
                }
                case 4:
                {
                    setCPtittle(vecMyCP[i],"涡流");
                    break;
                }
            }
        }
    }
}

void MainWindow::setCPtittle(MyCustomPlot *&plotboard, QString strTitle)
{
    QCPTextElement* title = dynamic_cast<QCPTextElement*>(plotboard->plotLayout()->element(0,0));
    if(title)
    {
        title->setText(strTitle);
    }
    else {
        title = new QCPTextElement(plotboard);
        title->setText(strTitle);
        title->setFont(QFont("Arial",18,QFont::Bold));
        title->setTextColor(Qt::black);
        plotboard->plotLayout()->insertRow(0);
        plotboard->plotLayout()->addElement(0,0,title);
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
    QVector<MyCustomPlot*>vecMyCP; //画板数组，作为函数入参
    vecMyCP.append(ui->QcpText_1);
    vecMyCP.append(QcpText_2);
    vecMyCP.append(QcpText_3);
    vecMyCP.append(QcpText_4);
    //根据窗口数量，循环刷新
    for(int i=0;i<CwindowDisp->windNum;i++)
    {
        if(1 == CwindowDisp->windPlotType[i]) //曲线图
        {
            plotbySensorType(CwindowDisp->windSensorType[i],qmCPData,vecMyCP[i]);
        }
        else if(2 == CwindowDisp->windPlotType[i]) //灰度图
        {

        }
        else if(3 == CwindowDisp->windPlotType[i]) //彩色图
        {

        }
    }
    qDebug()<<"绘制图像所花费时间:"<<qElapTimer.elapsed()<<"ms";
    dataService::getInstance()->m_dataRwLock.unlock();
    return 0;
}

void MainWindow::handleWindowNumSetData(int windNum, int *windPlotType, int *windSensorType)
{
    CwindowDisp->windNum = windNum;
    for(int i=0;i<4;i++)
    {
        CwindowDisp->windPlotType[i]=windPlotType[i];
        CwindowDisp->windSensorType[i]=windSensorType[i];
    }
    setMutiWindow(CwindowDisp->windNum);
    emit plotCacheDataRequest();
    if(nullptr != windPlotType)
    {
        delete[] windPlotType;
        windPlotType = nullptr;
    }
    if(nullptr != windSensorType)
    {
        delete[] windSensorType;
        windSensorType = nullptr;
    }
}

void MainWindow::handleSig_wheelEvent(qint64 xLower, qint64 xUpper, qint64 yLower, qint64 yUpper)
{
   CwindowDisp->startPos = xLower;
   CwindowDisp->offset = xUpper - CwindowDisp->startPos;
   CwindowDisp->yLower = yLower;
   CwindowDisp->yUpper = yUpper;

   qint64 startPosInFile = 0;
   qint64 offsetInfile = 0;

   if(xLower <= 0)
   {
       startPosInFile = 0;
   }
   else {
       startPosInFile = xLower;
   }

   if(xLower < 0 && xUpper > 0)
   {
       offsetInfile = xUpper;
   }
   else if(xLower >0 && xUpper>0)
   {
       offsetInfile = xUpper - xLower;
   }
   else {
       offsetInfile = 0;
   }

   //超出Int范围不处理
   qint64 iSum = startPosInFile + offsetInfile;
   if(iSum>0)
   {
       QString qsfilePath = CprjConfig->dataDirPath+CprjConfig->curFileName;
       emit modelDataRequest(qsfilePath,startPosInFile,offsetInfile);
       //未来多窗口显示触发改动在此处
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

        setMutiWindow(CwindowDisp->windNum);
        emit modelDataRequest(qsfilePath,CwindowDisp->startPos,CwindowDisp->offset);
        CwindowDisp->bFirstPlot = false;
    }
}

void MainWindow::on_nextPageBtn_clicked()
{
    CwindowDisp->startPos += CwindowDisp->offset;
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
    CwindowDisp->startPos -= CwindowDisp->offset;
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
    if(nullptr == windowNumSetDlg)
    {
        windowNumSetDlg = new windowNumSetDialog(this);
        connect(windowNumSetDlg,&windowNumSetDialog::windowNumSetData,this,&MainWindow::handleWindowNumSetData);
        windowNumSetDlg->exec();
    }
    else {
        windowNumSetDlg->exec();
    }
}
