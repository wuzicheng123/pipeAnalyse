#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <QMetaType>
#include "plotprocess.h"
#include "dataservice.h"
#include "commonfun.h"
#include <QElapsedTimer>
#include "msgbox.h"
#include "newuserdlg.h"
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
    if(nullptr == CcurrentUserMod)
    {
        CcurrentUserMod = new userDataModel;
    }

    //初始化代码
    ui->QcpText_1->setInteractions(QCP::iRangeZoom);
    connect(ui->QcpText_1,&MyCustomPlot::sig_wheelEvent,this,&MainWindow::handleSig_wheelEvent);
    //初始部分禁用菜单栏
    ui->projectManage->setEnabled(false);
    ui->plotWindow->setEnabled(false);
    ui->userManage->setEnabled(false);
    ui->logout->setEnabled(false);
    ui->windowNumSet->setEnabled(false);

    CwindowDisp->windNum = 1;
    CwindowDisp->windSensorType[0] = 1;
    CwindowDisp->windPlotType[0] = 1;
    QcpText_2 = nullptr;
    QcpText_3 = nullptr;
    QcpText_4 = nullptr;
    //窗体数量设置界面
    windowNumSetDlg = nullptr;
    m_loginDlg = nullptr;
    //用户列表界面--------------------------------------------------
    userTableModel = new QStandardItemModel(this);
    userTableModel->setColumnCount(2);
    userTableModel->setHorizontalHeaderLabels({"用户名","用户类别"});
    ui->userTableView->setModel(userTableModel);
    ui->userTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->userTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->userTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置列宽
    ui->userTableView->horizontalHeader()->setStretchLastSection(true);
    ui->userTableView->setColumnWidth(0, 200);
    // 设置表格样式
    ui->userTableView->setAlternatingRowColors(true);
    ui->userTableView->verticalHeader()->setVisible(false);
    ui->userTableView->setGridStyle(Qt::NoPen);
    //项目列表界面--------------------------------------------------
    prjTableModel = new QStandardItemModel(this);
    prjTableModel->setColumnCount(6);
    prjTableModel->setHorizontalHeaderLabels({"项目名","项目描述","壁厚","采样间距","创建时间","创建人"});
    ui->projectTableView->setModel(prjTableModel);
    ui->projectTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->projectTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->projectTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置列宽
    ui->projectTableView->horizontalHeader()->setStretchLastSection(true);
    ui->projectTableView->setColumnWidth(0, ui->projectTableView->size().width()/6);
    // 设置表格样式
    ui->projectTableView->setAlternatingRowColors(true);
    ui->projectTableView->verticalHeader()->setVisible(false);
    ui->projectTableView->setMouseTracking(true);
    connect(ui->projectTableView,&QTableView::entered,this,&MainWindow::showTooltip);
    ui->closePrj->setEnabled(false);

    m_dbWorker = nullptr;
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
    if(nullptr != CcurrentUserMod)
    {
        delete  CcurrentUserMod;
        CcurrentUserMod = nullptr;
    }
    if(nullptr != windowNumSetDlg)
    {
        disconnect(windowNumSetDlg,&windowNumSetDialog::windowNumSetData,this,&MainWindow::handleWindowNumSetData);
        disconnect(this,&MainWindow::initalWinNum,windowNumSetDlg,&windowNumSetDialog::handleInitalWinNum);
        delete windowNumSetDlg;
        windowNumSetDlg = nullptr;
    }
    if(nullptr != m_loginDlg)
    {
        //设置disconnect
        delete m_loginDlg;
        m_loginDlg = nullptr;
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

    plotBoard->xAxis->setLabel("帧数");
    plotBoard->yAxis->setLabel("通道");
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

void MainWindow::initialDatabase()
{
    if(nullptr == m_dbWorker)
    {
        QThread* thread = new QThread;
        m_dbWorker = new databaseWorker;
        m_dbWorker->moveToThread(thread);

        //连接信号槽
        connect(m_loginDlg,&loginDlg::loginRequest,m_dbWorker,&databaseWorker::handleLoginRequest);
        connect(m_dbWorker,&databaseWorker::loginResult,this,&MainWindow::handleLoginResult);
        connect(this,&MainWindow::queryAllUsers,m_dbWorker,&databaseWorker::handleQueryAllUsers);
        qRegisterMetaType<QVector<userDataModel>>("QVector<userDataModel>&");
        connect(m_dbWorker,&databaseWorker::qryAllUsersResult,this,&MainWindow::handleQryAllUsersResult);
        connect(m_dbWorker,&databaseWorker::showAddNewUser,this,&MainWindow::handleShowAddNewUser);
        connect(m_dbWorker,&databaseWorker::showEditUser,this,&MainWindow::handleShowEditUser);
        connect(this,&MainWindow::deleteUserRequest,m_dbWorker,&databaseWorker::handleDeleteUserRequest);
        connect(m_dbWorker,&databaseWorker::showDeleteUser,this,&MainWindow::handleShowDeleteUser);
        connect(this,&MainWindow::queryAllProjects,m_dbWorker,&databaseWorker::handleQueryAllProjects);
        qRegisterMetaType<QVector<projectDataModel>>("QVector<projectDataModel>&");
        connect(m_dbWorker,&databaseWorker::qryAllPrjsResult,this,&MainWindow::handleQryAllPrjsResult);
        connect(this,&MainWindow::queryProjectById,m_dbWorker,&databaseWorker::handleQueryProjectById);
        qRegisterMetaType<projectDataModel>("projectDataModel&");
        connect(m_dbWorker,&databaseWorker::qryProjectByIdResult,this,&MainWindow::handleQryProjectByIdResult);
        thread->start();
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

void MainWindow::handleLoginResult(int id,QString name,QString password,QString permission)
{
    if("" == permission)
    {
        //弹窗告警
        msgBox::show("登录异常","用户名或密码错误",2);
    }
    else
    {
        //使能菜单栏并记录当前用户信息
        ui->projectManage->setEnabled(true);
        ui->plotWindow->setEnabled(true);
        ui->userManage->setEnabled(true);
        ui->logout->setEnabled(true);
        ui->login->setEnabled(false);
        ui->windowNumSet->setEnabled(true);
        CcurrentUserMod->id = id;
        CcurrentUserMod->name = name;
        CcurrentUserMod->password = password;
        CcurrentUserMod->permission = permission;
        if("管理员" == permission)
        {
            ui->newUser->setEnabled(true);
            ui->editUser->setEnabled(true);
            ui->deleteUser->setEnabled(true);
        }
        //操作员
        else
        {
            ui->newUser->setEnabled(false);
            ui->editUser->setEnabled(false);
            ui->deleteUser->setEnabled(false);
        }
    }
}

void MainWindow::handleQryAllUsersResult(QVector<userDataModel> &vecUsers)
{
    userTableModel->removeRows(0,userTableModel->rowCount());
    int iSize = vecUsers.size();
    for(int i=0;i<iSize;i++)
    {
        QList<QStandardItem*>rowItems;
        QStandardItem* userNameItem = new QStandardItem(vecUsers[i].name);
        QStandardItem* userTypeItem = new QStandardItem(vecUsers[i].permission);
        rowItems<<userNameItem;
        rowItems<<userTypeItem;
        userTableModel->appendRow(rowItems);
    }
}

void MainWindow::handleShowAddNewUser(QString name, QString password, QString permission)
{
    Q_UNUSED(password);
    QStandardItem *usernameItem = new QStandardItem(name);
    QStandardItem *userTypeItem = new QStandardItem(permission);

    // 添加到表格
    QList<QStandardItem*> rowItems;
    rowItems << usernameItem;
    rowItems << userTypeItem;
    userTableModel->appendRow(rowItems);
}

void MainWindow::handleShowEditUser(int row, QString name, QString password, QString permission)
{
    Q_UNUSED(password);
    Q_UNUSED(permission);
    userTableModel->item(row,0)->setText(name);
    userTableModel->item(row,1)->setText(permission);
}

void MainWindow::handleShowDeleteUser(int row)
{
    userTableModel->removeRow(row);
}

void MainWindow::handleQryAllPrjsResult(QVector<projectDataModel> &vecPrjs)
{
    prjTableModel->removeRows(0,prjTableModel->rowCount());
    int iSize = vecPrjs.size();
    for(int i=0;i<iSize;i++)
    {
        QList<QStandardItem*>rowItems;
        QStandardItem* prjNameItem = new QStandardItem(vecPrjs[i].name);
        prjNameItem->setData(vecPrjs[i].id,Qt::UserRole+1);
        QStandardItem* discriptItem = new QStandardItem(vecPrjs[i].discript);
        QStandardItem* wallthicknessItem = new QStandardItem(vecPrjs[i].wallthicknesstype);
        QStandardItem* sampleintervalItem = new QStandardItem(QString::number(vecPrjs[i].sampleinterval));
        QStandardItem* createtimeItem = new QStandardItem(vecPrjs[i].createtime);
        QStandardItem* creatorNameItem = new QStandardItem(vecPrjs[i].creatorName);
        rowItems<<prjNameItem;
        rowItems<<discriptItem;
        rowItems<<wallthicknessItem;
        rowItems<<sampleintervalItem;
        rowItems<<createtimeItem;
        rowItems<<creatorNameItem;
        prjTableModel->appendRow(rowItems);
    }
}

void MainWindow::handleQryProjectByIdResult(projectDataModel &onePrj)
{
    //获取qsfilePath下的文件列表 以及路径合法性判断
    //数据有效性校验
    CprjConfig->dataDirPath = onePrj.datapath;
    QDir dir(CprjConfig->dataDirPath);
    if(!dir.exists())
    {
        msgBox::show("告警","打开项目错误，文件夹不存在",2);
        CprjConfig->dataDirPath = "";
        return;
    }

    QMap<QString,int>().swap(CprjConfig->fileNameBytesMap);

    //获取文件夹下文件名
    int bytesNum = 0;
    QString qsfilePath="";
    QStringList filters = {"*.bin"};
    QFileInfoList files = dir.entryInfoList(filters,QDir::Files,QDir::Name);
    if(files.isEmpty())
    {
        msgBox::show("告警","打开项目错误，无符合命名要求的数据文件",2);
        CprjConfig->dataDirPath = "";
        return;
    }
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
    CprjConfig->dInterval = onePrj.sampleinterval;
    ui->openPrj->setEnabled(false);
    ui->closePrj->setEnabled(true);
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
    if(ui->openPrj->isEnabled())
    {
        msgBox::show("警告","未选择项目打开",2);
        return;
    }
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget->show();

    //emit
    if(CwindowDisp->bFirstPlot)
    {
        QString qsfilePath=CprjConfig->dataDirPath+CprjConfig->curFileName;
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
    //emit查询项目列表请求
    emit queryAllProjects();
    ui->stackedWidget->setCurrentIndex(2);
    ui->stackedWidget->show();
}

void MainWindow::on_userManage_triggered()
{
    //查询请求，并显示列表
    emit queryAllUsers();
    ui->stackedWidget->setCurrentIndex(1);
    ui->stackedWidget->show();
}

void MainWindow::on_login_triggered()
{
    //打开页面
    if(nullptr == m_loginDlg)
    {
        m_loginDlg = new loginDlg(this);
        initialDatabase();
        m_loginDlg->exec();
    }
    else {
        m_loginDlg->exec();
    }
}

void MainWindow::on_logout_triggered()
{
    ui->projectManage->setEnabled(false);
    ui->plotWindow->setEnabled(false);
    ui->userManage->setEnabled(false);
    ui->logout->setEnabled(false);
    ui->login->setEnabled(true);
    ui->windowNumSet->setEnabled(false);
    CcurrentUserMod->id = -1;
    CcurrentUserMod->name = "";
    CcurrentUserMod->password = "";
    CcurrentUserMod->permission = "";
    //画板恢复空白
    setMutiWindow(1);
    ui->QcpText_1->clearPlottables();    // 清除所有图形
    ui->QcpText_1->clearItems();         // 清除所有图项
    ui->QcpText_1->xAxis->setLabel("");  // 清除X轴标签
    ui->QcpText_1->yAxis->setLabel("");  // 清除Y轴标签
    ui->QcpText_1->replot();             // 重绘
    setCPtittle(ui->QcpText_1,"");
    CwindowDisp->bFirstPlot = true;
    CwindowDisp->startPos = 0;
    CwindowDisp->offset = 2000;
    CwindowDisp->yLower = 0;
    CwindowDisp->yUpper = 0;
    CwindowDisp->pageOffset = 1500;
    for(int i=0;i<4;i++)
    {
        CwindowDisp->windSensorType[i]=0;
        CwindowDisp->windPlotType[i]=0;
    }
    CwindowDisp->windNum = 1;
    CwindowDisp->windSensorType[0] = 1;
    CwindowDisp->windPlotType[0] = 1;
    CprjConfig->dInterval = 0;
    CprjConfig->dataDirPath = "";
    CprjConfig->curFileName = "";
    QVector<QString>().swap(CprjConfig->fileNameVec);
    QMap<QString,int>().swap(CprjConfig->fileNameBytesMap);
    ui->openPrj->setEnabled(true);
    ui->closePrj->setEnabled(false);
    emit initalWinNum();
    ui->stackedWidget->hide();
}

void MainWindow::on_windowNumSet_triggered()
{
    if(nullptr == windowNumSetDlg)
    {
        windowNumSetDlg = new windowNumSetDialog(this);
        connect(windowNumSetDlg,&windowNumSetDialog::windowNumSetData,this,&MainWindow::handleWindowNumSetData);
        connect(this,&MainWindow::initalWinNum,windowNumSetDlg,&windowNumSetDialog::handleInitalWinNum);
        windowNumSetDlg->exec();
    }
    else {
        windowNumSetDlg->exec();
    }
}

void MainWindow::on_newUser_clicked()
{
    newuserdlg* newUserDlg = new newuserdlg(this);
    newUserDlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(newUserDlg,&newuserdlg::newUserRequest,m_dbWorker,&databaseWorker::handleNewUserRequest);
    newUserDlg->trans2newDlg();
    newUserDlg->exec();
}

void MainWindow::on_editUser_clicked()
{
    QModelIndexList selectedIndexes = ui->userTableView->selectionModel()->selectedRows();
    if(selectedIndexes.isEmpty())
    {
        return;
    }
    int row = selectedIndexes.first().row();
    QString currentName = userTableModel->item(row,0)->text();
    if("admin" == currentName)
    {
        msgBox::show("警告","禁止修改admin账户",2);
        return;
    }
    QString currentPermission = userTableModel->item(row,1)->text();
    //修改可与新建共用同一界面
    newuserdlg* editDlg = new newuserdlg(this);
    editDlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(editDlg,&newuserdlg::editUserRequest,m_dbWorker,&databaseWorker::handleEditUserRequest);
    editDlg->trans2editDlg(row,currentName,currentPermission);
    editDlg->exec();
}

void MainWindow::on_deleteUser_clicked()
{
    QModelIndexList selectedIndexes = ui->userTableView->selectionModel()->selectedRows();
    if(selectedIndexes.isEmpty())
    {
        return;
    }
    int row = selectedIndexes.first().row();
    QString username = userTableModel->item(row,0)->text();
    if("admin" == username)
    {
        msgBox::show("警告","禁止删除admin账户",2);
        return;
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,"确认删除",QString("确定要删除用户'%1'吗").arg(username),
                                  QMessageBox::Yes|QMessageBox::No);
    if(QMessageBox::Yes == reply)
    {
        emit deleteUserRequest(row,username);
    }
}

void MainWindow::showTooltip(const QModelIndex &index)
{
    QToolTip::showText(QCursor::pos(),index.data().toString());
}

void MainWindow::on_openPrj_clicked()
{
    QModelIndexList selectedIndexes = ui->projectTableView->selectionModel()->selectedRows();
    if(selectedIndexes.isEmpty())
    {
        return;
    }
    int row = selectedIndexes.first().row();
    QStandardItem* prjnameItem = prjTableModel->item(row,0);
    int prjId = prjnameItem->data(Qt::UserRole+1).toInt();
    emit queryProjectById(prjId);
}

void MainWindow::on_closePrj_clicked()
{
    CwindowDisp->startPos = 0;
    CwindowDisp->offset = 2000;
    CwindowDisp->yLower = 0;
    CwindowDisp->yUpper = 0;
    CwindowDisp->pageOffset = 1500;
    CprjConfig->dInterval = 0;
    CprjConfig->dataDirPath = "";
    CprjConfig->curFileName = "";
    QVector<QString>().swap(CprjConfig->fileNameVec);
    QMap<QString,int>().swap(CprjConfig->fileNameBytesMap);
    CwindowDisp->bFirstPlot = true;
    ui->openPrj->setEnabled(true);
    ui->closePrj->setEnabled(false);
}
