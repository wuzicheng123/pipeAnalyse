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
#include "projectdlg.h"
#include <QDir>
#include "opencv2/opencv.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("管道分析软件");

    qRegisterMetaType<projectConfigure>("projectConfigure");
    qRegisterMetaType<QVector<QMap<int,QVector<QVector<QCPGraphData>>>>>("QVector<QMap<int,QVector<QVector<QCPGraphData>>>>&");
    connect(this,&MainWindow::modelDataRequest,dataService::getInstance(),&dataService::handleModelDataRequest);
    connect(this,&MainWindow::plotCacheDataRequestBybox,plotProcess::getInstance(),&plotProcess::handleplotCacheDataRequestBybox);
    connect(plotProcess::getInstance(),&plotProcess::plotDataReadyBybox,this,&MainWindow::handlePlotDataReadyBybox);
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
    ui->grayscaleSetSlider->setVisible(false);
    m_grayScaleQsliderValue = 100;
    bupdateGrayScaleing = false;
    grayValueLower = 0;
    grayValueUpper = 255;

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
    prjTableModel->setColumnCount(8);
    prjTableModel->setHorizontalHeaderLabels({"项目名","项目描述","壁厚","采样间距","壁厚值","外管径","创建时间","创建人"});
    ui->projectTableView->setModel(prjTableModel);
    ui->projectTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->projectTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->projectTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置列宽
    ui->projectTableView->horizontalHeader()->setStretchLastSection(true);
    ui->projectTableView->setColumnWidth(0, 100);
    ui->projectTableView->setColumnWidth(6, 180);
    // 设置表格样式
    ui->projectTableView->setAlternatingRowColors(true);
    ui->projectTableView->verticalHeader()->setVisible(false);
    ui->projectTableView->setMouseTracking(true);
    connect(ui->projectTableView,&QTableView::entered,this,&MainWindow::showTooltip);
    ui->closePrj->setEnabled(false);

    m_dbWorker = nullptr;
    m_defectdetectorWorker = nullptr;
    m_plotting = false;
    m_detectDefectingFlag = false;
    m_detectDefectingPrjName = "";
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

void MainWindow::plotLineChartDataByAxis(QVector<QVector<QCPGraphData> > QcpData2D, qint64 windowStart, qint64 windowEnd,MyCustomPlot *& plotBoard,int axis,int boxNum,int boxSize)
{
    if(0 == boxNum)
    {
        //若存在灰度图像，清除灰度图像
        removePixmapItem(plotBoard);
    }
    int RowSize = QcpData2D.size();
//    //查找36个通道的最大最小值
//    //并进行归一化到-50-50转换
//    double ymin = 0,ymax = 0;
//    //乘以采样间隔
//    double xRealLower = windowStart*CprjConfig->dInterval;
//    double xRealHigher = windowEnd*CprjConfig->dInterval;
//    for(int i=0;i<RowSize;i++)
//    {
//        int ColumnSize = QcpData2D[i].size();
//        for(int j=0;j<ColumnSize;j++)
//        {
//            double& dy = QcpData2D[i][j].value;
//            QcpData2D[i][j].key = QcpData2D[i][j].key*CprjConfig->dInterval;
//            //归一化
//            //等距通道显示样式 每个通道value值映射到2，例如通道一为-50到50，通道二为-49到51，。。。。，通道36为-15到85。
//            if(1 == axis || 2 == axis)
//            {
//                dy = dy/plotProcess::getInstance()->hallUpperLimitXY*50;
//            }
//            else if(3 == axis)
//            {
//                dy = dy/plotProcess::getInstance()->hallUpperLimitZ*50;
//            }
//            if(windowStart >= 0)
//            {
//                QcpData2D[i][j].key += xRealLower;
//            }
//            if(dy<ymin)
//            {
//                ymin = dy;
//            }
//            if(dy>ymax)
//            {
//                ymax = dy;
//            }
//            //给每个探头增加偏移量，从而每个盒子的36个探头一张图显示
//            //45为36通道+间隔美观20行
//            //按box号调整
//            dy = dy + i + 36*boxNum;
//        }
//    }

    //优化版代码
    //乘以采样间隔
    double xRealLower = windowStart*CprjConfig->dInterval;
    double xRealHigher = windowEnd*CprjConfig->dInterval;
    double scaleFactor = 50.0;
    double upperLimit = (1 == axis || 2 == axis) ? plotProcess::getInstance()->hallUpperLimitXY : plotProcess::getInstance()->hallUpperLimitZ;
    for(int i=0;i<RowSize;i++)
    {
        int ColumnSize = QcpData2D[i].size();
        for(int j=0;j<ColumnSize;j++)
        {
            QCPGraphData& data = QcpData2D[i][j];
            data.key = data.key * CprjConfig->dInterval;
            //归一化并偏移
            if(1 == axis || 2 == axis || 3 == axis)
            {
                data.value = data.value / upperLimit * scaleFactor + i + 36 * boxNum;
            }
            else // axis 4
            {
                data.value = data.value + i + 36 * boxNum;
            }
            if(windowStart >= 0)
            {
                data.key += xRealLower;
            }
        }
    }

    plotBoard->xAxis->setLabel("距离");
    plotBoard->yAxis->setLabel("通道");
    plotBoard->xAxis->setRange(xRealLower,xRealHigher);
    double yRangeMax = 36*boxSize+20;
    double yRangeMin = -20 ;
    plotBoard->yAxis->setRange(yRangeMin,yRangeMax);
    if(0 == CwindowDisp->yLower && 0 == CwindowDisp->yUpper)
    {
        if(yRangeMax > 36+20)
        {
            CwindowDisp->yUpper = 36*boxSize+20;
            CwindowDisp->yLower = -20;
        }
        else {
            CwindowDisp->yUpper = 38;
            CwindowDisp->yLower = -4;
        }
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
        if(plotBoard->graphCount() < 36*(boxNum+1))
        {
            plotBoard->addGraph();
        }
        plotBoard->graph(36*boxNum+i)->setPen(pen);
        QSharedPointer<QCPGraphDataContainer>dataContainer = plotBoard->graph(36*boxNum+i)->data();
        dataContainer->clear();
        dataContainer->set(QcpData2D[i],true);
    }
    //优化显示效果，在所有数据填充到指针后一次刷新绘制，可显著提升速度
    if(boxNum == boxSize-1)
    {
        plotBoard->replot();
    }
}

void MainWindow::plotLineChartbySensorType(int sensorType, QMap<int, QVector<QVector<QCPGraphData> > > &qmCPData, MyCustomPlot *&plotBoard,int boxNum,int boxSize)
{
    switch (sensorType)   //一个窗口
    {
        case 1:{    //X轴
            if(qmCPData.find(1) != qmCPData.end())
            {
                QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[1];
                plotLineChartDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,1,boxNum,boxSize);
            }
            break;
        }
        case 2:{    //Y轴
            if(qmCPData.find(2) != qmCPData.end())
            {
                QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[2];
                plotLineChartDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,2,boxNum,boxSize);
            }
            break;
        }
        case 3:{    //Z轴
            if(qmCPData.find(3) != qmCPData.end())
            {
                QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[3];
                plotLineChartDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,3,boxNum,boxSize);
            }
            break;
        }
        case 4:{    //VORTEX
            if(qmCPData.find(4) != qmCPData.end())
            {
                QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[4];
                plotLineChartDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,4,boxNum,boxSize);
            }
            break;
        }
    }
}

void MainWindow::plotGrayOrColorChartbySensorType(int sensorType, QMap<int, QVector<QVector<QCPGraphData> > > &qmCPData, MyCustomPlot *&plotBoard, int boxNum, int boxSize, QVector<QVector<double> > &doubleArray2D, int colorType)
{
    switch (sensorType) {
    case 1:{    //X轴
        if(qmCPData.find(1) != qmCPData.end())
        {
            QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[1];
            plotGrayOrColorChartDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,1,boxNum,boxSize,doubleArray2D[0],colorType);
        }
        break;
    }
    case 2:{    //Y轴
        if(qmCPData.find(2) != qmCPData.end())
        {
            QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[2];
            plotGrayOrColorChartDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,2,boxNum,boxSize,doubleArray2D[1],colorType);
        }
        break;
    }
    case 3:{    //Z轴
        if(qmCPData.find(3) != qmCPData.end())
        {
            QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[3];
            plotGrayOrColorChartDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,3,boxNum,boxSize,doubleArray2D[2],colorType);
        }
        break;
    }
    case 4:{    //VORTEX
        if(qmCPData.find(4) != qmCPData.end())
        {
            QVector<QVector<QCPGraphData>> &QcpData2D = qmCPData[4];
            plotGrayOrColorChartDataByAxis(QcpData2D,CwindowDisp->startPos,CwindowDisp->startPos+CwindowDisp->offset,plotBoard,4,boxNum,boxSize,doubleArray2D[3],colorType);
        }
        break;
    }
    }
}

void MainWindow::plotGrayOrColorChartDataByAxis(QVector<QVector<QCPGraphData> > QcpData2D, qint64 windowStart, qint64 windowEnd, MyCustomPlot *&plotBoard, int axis, int boxNum, int boxSize, QVector<double>&doubleArray, int colorType)
{
    //将QcpData2D转换为一维数组,并按axis分类修改数值，并归一化映射到0-255范围
    //未避免使用prepend,带来的额外开销，提前预设数组空间，按坐标填入数据
    int RowSize = QcpData2D.size();
    int ColumnSizeOut = static_cast<int>(windowEnd - windowStart);
    //窗口实际显示范围（乘以采样间隔）
    double xRealLower = windowStart*CprjConfig->dInterval;
    double xRealHigher = windowEnd*CprjConfig->dInterval;
    int columnOffset = 0;
    if(windowStart<0 && windowEnd>0)
    {
        columnOffset = 0-static_cast<int>(windowStart);
    }
    for(int i=0;i<RowSize;i++)
    {
        int ColumnSize = QcpData2D[i].size();
        //当前界面的矩阵元素
        int elementSizeInBox = RowSize*ColumnSizeOut;
        int doubleArraySize = elementSizeInBox*boxSize;
        int hallXYdivisor = static_cast<int>(2*plotProcess::getInstance()->hallUpperLimitXY);
        int hallZdivisor = static_cast<int>(2*plotProcess::getInstance()->hallUpperLimitZ);
        if(doubleArray.isEmpty())
        {
            doubleArray.resize(doubleArraySize);
            doubleArray.fill(1.0);
            if(doubleArray.size() != doubleArraySize)
            {
                qDebug()<<"分配异常，实际大小为:"<<doubleArray.size();
            }
            else {
                qDebug()<<"分配容量正确";
            }
        }
        for(int j=0;j<ColumnSize;j++)
        {
            double& dy = QcpData2D[i][j].value;
            if(1 == axis || 2 == axis)
            {
                if(dy > plotProcess::getInstance()->hallUpperLimitXY)
                {
                    dy = plotProcess::getInstance()->hallUpperLimitXY;
                }
                else if(dy < plotProcess::getInstance()->hallLowerLimitXY)
                {
                    dy = plotProcess::getInstance()->hallLowerLimitXY;
                }
                //归一化，转换到0-1范围；
                //白色：1<==>255
                //黑色：0
                //取反，黑色显示1更明显
                dy = 1-(dy+plotProcess::getInstance()->hallUpperLimitXY)/hallXYdivisor;
            }
            else if(3 == axis)
            {
                if(dy > plotProcess::getInstance()->hallUpperLimitZ)
                {
                    dy = plotProcess::getInstance()->hallUpperLimitZ;
                }
                else if(dy < plotProcess::getInstance()->hallLowerLimitZ)
                {
                    dy = plotProcess::getInstance()->hallLowerLimitZ;
                }
                //归一化，转换到0-1范围；
                dy = 1-(dy+plotProcess::getInstance()->hallUpperLimitZ)/hallZdivisor;
            }
            else if(4 == axis)
            {
                if(dy > 100)
                {
                    dy = 100;
                }
                //归一化，转换到0-1范围；
                dy = 1-dy/100;
            }
            //将二维映射到一维数组
            int index = elementSizeInBox*(boxSize-1-boxNum)+ColumnSizeOut*(RowSize-1-i)+j+columnOffset;
            doubleArray[index] = dy;
        }
    }
    //设置qcustomplot坐标轴
    plotBoard->xAxis->setLabel("距离");
    plotBoard->yAxis->setLabel("通道");
    plotBoard->xAxis->setRange(xRealLower,xRealHigher);
    double yRangeMax = 36*boxSize+20;
    double yRangeMin = -20;
    plotBoard->yAxis->setRange(yRangeMin,yRangeMax);
    if(0 == CwindowDisp->yLower && 0 == CwindowDisp->yUpper)
    {
        if(yRangeMax > 36+20)
        {
            CwindowDisp->yUpper = 36*boxSize+20;
            CwindowDisp->yLower = -20;
        }
        else {
            CwindowDisp->yUpper = 38;
            CwindowDisp->yLower = -4;
        }
        plotBoard->yAxis->setRange(CwindowDisp->yLower,CwindowDisp->yUpper);
    }
    else {
        plotBoard->yAxis->setRange(CwindowDisp->yLower,CwindowDisp->yUpper);
    }
//    最后一个盒子时，转换为cv::Mat，并放入plotBoard中
    if(boxNum == boxSize-1)
    {
        //清除曲线图
        plotBoard->clearPlottables();
        //若存在灰度图像，清除灰度图像
        removePixmapItem(plotBoard);

        cv::Mat doubleMat(RowSize*boxSize,ColumnSizeOut,CV_64FC1,const_cast<double*>(doubleArray.data()));
        cv::Mat grayMat;
        doubleMat.convertTo(grayMat,CV_8UC1,255.0);
        if(grayMat.empty())
        {
            plotBoard->replot();
            return;
        }

        //基于背景识别的分块处理（处理不同区域）- 简化快速版本
        // 预处理：中值滤波去噪（只对非背景区域）
        cv::Mat preprocessed = grayMat.clone();  // 先复制原图
        // 定义背景颜色范围（假设灰色背景的灰度值在[120, 135]之间）
        // 根据实际情况调整这些阈值
        int gray_low = 120;
        int gray_high = 135;
        // 创建背景掩码
        cv::Mat background_mask;
        cv::inRange(grayMat, gray_low, gray_high, background_mask);
        // 对非背景区域应用中值滤波
        cv::Mat non_background;
        cv::Mat non_background_filtered;
        grayMat.copyTo(non_background, ~background_mask);  // 提取非背景区域
        cv::medianBlur(non_background, non_background_filtered, 3);
        non_background_filtered.copyTo(preprocessed, ~background_mask);
        //----------新逻辑，通过滑动条设置灰度图范围，提升对比度，替代后文算法----
        //创建查找表(0~255)
        uchar lut[256];
        for(int i=0;i<256;i++)
        {
            if(i <= grayValueLower)
                lut[i] = 0;
            else if(i >= grayValueUpper)
                lut[i] = 255;
            else {
                lut[i] = static_cast<uchar>((i-grayValueLower)*255/(grayValueUpper-grayValueLower));
            }
        }
        cv::Mat final_result;
        cv::LUT(grayMat,cv::Mat(1,256,CV_8UC1,lut),final_result);
        //---------------------------新逻辑----------------------------

//---------下述代码为对比度增强，现按需求修改为通过滑动条修改灰度取值范围，以此增加对比度，因此下述原逻辑代码注释保留---------------
//        cv::Mat final_result = grayMat.clone();  // 初始化为原图，保留背景不变
//        // 分块处理（只处理非背景区域）
//        int block_size = 64;
//        for (int y = 0; y < grayMat.rows; y += block_size) {
//            for (int x = 0; x < grayMat.cols; x += block_size) {
//                cv::Rect roi(x, y, cv::min(block_size, grayMat.cols - x),
//                         cv::min(block_size, grayMat.rows - y));
//                cv::Mat block = preprocessed(roi);
//                cv::Mat block_mask = background_mask(roi);
//                // 计算块中背景像素的比例
//                int total_pixels = block.rows * block.cols;
//                int background_pixels = cv::countNonZero(block_mask);
//                double background_ratio = static_cast<double>(background_pixels) / total_pixels;
//                // 如果块中主要是背景（比如超过95%是背景），跳过处理
//                if (background_ratio > 0.95) {
//                    continue;  // 保持原背景不变
//                }
//                // 简化：直接使用高质量插值，无需梯度计算
//                cv::Mat processed_block;
//                // 使用保护边缘的插值
//                cv::resize(block, processed_block, cv::Size(), 2, 2, cv::INTER_CUBIC);
//                // 缩小回原尺寸
//                cv::resize(processed_block, processed_block, block.size(), 0, 0, cv::INTER_CUBIC);
//                // 只将非背景区域的修改应用到结果中
//                processed_block.copyTo(final_result(roi), ~block_mask);
//            }
//        }
//        // 后处理：对比度增强（只对非背景区域）
//        cv::Mat non_bg_enhanced;
//        final_result.copyTo(non_bg_enhanced, ~background_mask);
//        cv::normalize(non_bg_enhanced, non_bg_enhanced, 0, 255, cv::NORM_MINMAX);
//        non_bg_enhanced.copyTo(final_result, ~background_mask);
//------------------------------------------------------分界线------------------------------------------------------------

        //彩色图像(添加额外处理)
        cv::Mat colorImg;
        QImage::Format imgFormat;
        if(1 == colorType)
        {
            //白色区域检测（考虑连续性和面积）
            //步骤1：检测高灰度值区域
            cv::Mat whiteMask;
            cv::threshold(final_result,whiteMask,254,255,cv::THRESH_BINARY);
            //步骤2：形态学操作，去除小的白色噪点
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(3,3));
            cv::morphologyEx(whiteMask,whiteMask,cv::MORPH_CLOSE,kernel);
            //步骤3：查找白色连通区域
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(whiteMask,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
            //步骤4：创建只包含大片白色区域的掩码
            int minWhiteArea = 100;
            cv::Mat largeWhiteMask = cv::Mat::zeros(final_result.size(),CV_8UC1);
            for(const auto& contour:contours)
            {
                double area = cv::contourArea(contour);
                if(area >= minWhiteArea)
                {
                    cv::drawContours(largeWhiteMask,std::vector<std::vector<cv::Point>>{contour},0,255,cv::FILLED);
                }
            }
            //步骤5:应用彩色映射
            cv::applyColorMap(final_result,colorImg,cv::COLORMAP_JET);
            //步骤6：保持大片白色区域为白色
            for (int i=0;i<colorImg.rows;i++) {
                for (int j=0;j<colorImg.cols;j++) {
                    if(largeWhiteMask.at<uchar>(i,j) > 0)
                    {
                        colorImg.at<cv::Vec3b>(i,j) = cv::Vec3b(255,255,255);
                    }
                }
            }
            cv::cvtColor(colorImg,final_result,cv::COLOR_BGR2RGB);
            imgFormat = QImage::Format_RGB888;
        }
        //灰度
        else
        {
            imgFormat = QImage::Format_Grayscale8;
        }

        //转为QImage显示
        QImage image(final_result.data,final_result.cols,final_result.rows,static_cast<int>(final_result.step),imgFormat);
        double yLowerAPI = plotBoard->yAxis->range().lower;
        double yUpperAPI = plotBoard->yAxis->range().upper;
        int imageHeight = RowSize*boxSize;
        double topleftScaleY = 0.0;
        double bottomrightScaleY = 1.0;
        //根据坐标轴纵向裁剪图片
        QImage displayImage;
        //yUpperAPI小于0，或yLowerAPI大于imageHeight-----显示白板
        if(yUpperAPI<=0 || yLowerAPI>=imageHeight)
        {
            plotBoard->replot();
            return;
        }
        //计算y轴显示比列坐标；x轴因做了偏移填充，因此无需计算topleftScaleX、bottomrightScaleX，默认为0和1
        //yUpperAPI大于0小于imageHeight，yLowerAPI下限小于0------显示图像上半部分
        else if(yUpperAPI>0 && yUpperAPI<=imageHeight && yLowerAPI<0)
        {
            displayImage = image.copy(0,static_cast<int>(imageHeight-yUpperAPI),ColumnSizeOut,static_cast<int>(yUpperAPI));
            topleftScaleY = 0.0;
            bottomrightScaleY = (yUpperAPI - 0)/(yUpperAPI-yLowerAPI);
        }
        //yLowerAPI与yUpperAPI在0到imageHeight之间------显示图像局部区域
        else if(yLowerAPI>=0 && yLowerAPI<imageHeight && yUpperAPI>0 && yUpperAPI<=imageHeight)
        {
            displayImage = image.copy(0,static_cast<int>(imageHeight-yUpperAPI),ColumnSizeOut,static_cast<int>(yUpperAPI-yLowerAPI));
            topleftScaleY = 0.0;
            bottomrightScaleY = 1.0;
        }
        //yLowerAPI大于0小于imageHeight，yUpperAPI大于imageHeight-----显示图像下半部分
        else if(yLowerAPI>=0 && yLowerAPI<imageHeight && yUpperAPI>imageHeight)
        {
            displayImage = image.copy(0,0,ColumnSizeOut,static_cast<int>(imageHeight-yLowerAPI));
            topleftScaleY = (yUpperAPI-imageHeight)/(yUpperAPI-yLowerAPI);
            bottomrightScaleY = 1.0;
        }
        //yLowerAPI小于0，yUpperAPI大于imageHeight-----显示图像全部及超出部分空白显示
        else if(yLowerAPI<0 && yUpperAPI>imageHeight)
        {
            displayImage = image;
            topleftScaleY = (yUpperAPI-imageHeight)/(yUpperAPI-yLowerAPI);
            bottomrightScaleY = (yUpperAPI - 0)/(yUpperAPI-yLowerAPI);
        }
        //创建QPixmap
        QPixmap pixmap = QPixmap::fromImage(displayImage);
        QCPItemPixmap *pixmapItem = new QCPItemPixmap(plotBoard);
        pixmapItem->setPixmap(pixmap);
        pixmapItem->setScaled(true,Qt::IgnoreAspectRatio);
        //设置显示区域位置大小，灰度图固定于坐标轴0-imageHeight之间显示
        pixmapItem->topLeft->setType(QCPItemPosition::ptAxisRectRatio);
        pixmapItem->bottomRight->setType(QCPItemPosition::ptAxisRectRatio);
        pixmapItem->topLeft->setCoords(0,topleftScaleY);
        pixmapItem->bottomRight->setCoords(1,bottomrightScaleY);
        plotBoard->replot();
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
        qRegisterMetaType<projectDataModel>("projectDataModel&");
        qRegisterMetaType<QVector<projectDataModel>>("QVector<projectDataModel>&");
        connect(m_dbWorker,&databaseWorker::qryAllPrjsResult,this,&MainWindow::handleQryAllPrjsResult);
        connect(this,&MainWindow::queryProjectById,m_dbWorker,&databaseWorker::handleQueryProjectById);
        connect(m_dbWorker,&databaseWorker::qryProjectByIdResult,this,&MainWindow::handleQryProjectByIdResult);
        connect(m_dbWorker,&databaseWorker::showAddNewProject,this,&MainWindow::handleShowAddNewProject);
        connect(m_dbWorker,&databaseWorker::showEditProject,this,&MainWindow::handleShowEditProject);
        connect(this,&MainWindow::deleteProjectRequest,m_dbWorker,&databaseWorker::handleDeleteProjectRequest);
        connect(m_dbWorker,&databaseWorker::showDeleteProject,this,&MainWindow::handleShowDeleteProject);
        connect(m_dbWorker,&databaseWorker::showDetailProject,this,&MainWindow::handleShowDetailProject);
        thread->start();
    }
}

void MainWindow::clearPlotboard(MyCustomPlot *&plotBoard)
{
    if(nullptr != plotBoard)
    {
        plotBoard->clearPlottables();    // 清除所有图形
        removePixmapItem(plotBoard);     // 清除所有图项
        plotBoard->xAxis->setLabel("");  // 清除X轴标签
        plotBoard->yAxis->setLabel("");  // 清除Y轴标签
        plotBoard->replot();             // 重绘
        setCPtittle(plotBoard,"");
    }
}

void MainWindow::removePixmapItem(MyCustomPlot *&plotBoard)
{
    //从后向前遍历避免删除时索引问题
    for (int i=plotBoard->itemCount()-1;i>=0;--i)
    {
        QCPAbstractItem* item = plotBoard->item(i);
        if(item)
        {
            QCPItemPixmap* pixmapItem = dynamic_cast<QCPItemPixmap*>(item);
            if(pixmapItem)
            {
                plotBoard->removeItem(pixmapItem);
            }
        }
    }
}

void MainWindow::testOpenCV()
{
    qDebug() << "=== OpenCV 基本环境测试 ===";

    // 1. 检查版本
    qDebug() << "OpenCV版本: " << CV_VERSION ;
    qDebug() << "主版本: " << CV_MAJOR_VERSION;
    qDebug() << "次版本: " << CV_MINOR_VERSION;

    // 2. 检查编译信息
    qDebug() << "\n编译信息:";
    qDebug() << QString::fromStdString(cv::getBuildInformation());

    // 3. 创建测试图像
    qDebug() << "\n创建测试图像...";
    cv::Mat testImage(300, 400, CV_8UC3, cv::Scalar(100, 150, 200));

    if (testImage.empty()) {
        std::cerr << "错误: 无法创建图像";
        return;
    }

    qDebug() << "图像创建成功!";
    qDebug() << "尺寸: " << testImage.cols << "x" << testImage.rows;
    qDebug() << "通道数: " << testImage.channels();
    qDebug() << "深度: " << testImage.depth();

    // 4. 保存测试图像
    cv::imwrite("test_output.png", testImage);
    qDebug() << "测试图像已保存为 test_output.png";

    // 5. 加载图像测试
    qDebug() << "\n加载图像测试...";
    cv::Mat loadedImage = cv::imread("test_output.png");

    if (loadedImage.empty()) {
        std::cerr << "错误: 无法加载图像";
        return;
    }

    qDebug() << "图像加载成功!";
    qDebug() << "加载的尺寸: " << loadedImage.cols << "x" << loadedImage.rows;

    // 6. 显示图像（如果有GUI支持）
    #ifdef HAVE_OPENCV_HIGHGUI
    qDebug() << "\n显示图像 (5秒后关闭)...";
    cv::imshow("OpenCV测试图像", testImage);
    cv::waitKey(5000);
    cv::destroyAllWindows();
    #else
    qDebug() << "\nGUI模块不可用，跳过显示";
    #endif

    qDebug() << "\n=== 测试通过！ ===";
}

int MainWindow::handlePlotDataReadyBybox(QVector<QMap<int, QVector<QVector<QCPGraphData> > > > &qmCPDatavec,int updateType)
{
    dataService::getInstance()->m_dataRwLock.lockForRead();
    QElapsedTimer qElapTimer;
    qElapTimer.start();
    if(!qmCPDatavec.isEmpty())
    {
        //辅助空参数情况坐标轴翻页效果
        int boxSize = qmCPDatavec.size();
        for(int k=0;k<boxSize;k++)
        {
            QMap<int, QVector<QVector<QCPGraphData>>>&qmCPData = qmCPDatavec[k];
            if(qmCPData.isEmpty())
            {
                QVector<QVector<QCPGraphData>> oneVector2D;
                for (int i=1;i<=4;i++) {
                    qmCPData.insert(i,oneVector2D);
                }
            }
        }
        //绘制磁力曲线
        QVector<MyCustomPlot*>vecMyCP; //画板数组，作为函数入参
        vecMyCP.append(ui->QcpText_1);
        vecMyCP.append(QcpText_2);
        vecMyCP.append(QcpText_3);
        vecMyCP.append(QcpText_4);
        //灰度图所用参数
        //0-代表X，1-Y。。。3代表Vortex（存储灰度图像处理数据）
        QVector<QVector<double>>doubleArray(4);
        for(int k=0;k<boxSize;k++)
        {
            QMap<int, QVector<QVector<QCPGraphData>>>&qmCPData = qmCPDatavec[k];
            //根据窗口数量，循环刷新
            for(int i=0;i<CwindowDisp->windNum;i++)
            {
                if(1 == CwindowDisp->windPlotType[i] && updateType==0) //曲线图
                {
                    plotLineChartbySensorType(CwindowDisp->windSensorType[i],qmCPData,vecMyCP[i],k,boxSize);
                }
                else if(2 == CwindowDisp->windPlotType[i]) //灰度图
                {
                    plotGrayOrColorChartbySensorType(CwindowDisp->windSensorType[i],qmCPData,vecMyCP[i],k,boxSize,doubleArray,0);
                }
                else if(3 == CwindowDisp->windPlotType[i]) //彩色图
                {
                    plotGrayOrColorChartbySensorType(CwindowDisp->windSensorType[i],qmCPData,vecMyCP[i],k,boxSize,doubleArray,1);
                }
            }
        }
    }
    qDebug()<<"绘制图像所花费时间:"<<qElapTimer.elapsed()<<"ms";
    dataService::getInstance()->m_dataRwLock.unlock();
    m_plotting = false;
    if(bupdateGrayScaleing)
        bupdateGrayScaleing = false;
    return 0;
}

void MainWindow::handleWindowNumSetData(int windNum, int *windPlotType, int *windSensorType)
{
    CwindowDisp->windNum = windNum;
    bool bgraySliderShow = false;
    for(int i=0;i<4;i++)
    {
        CwindowDisp->windPlotType[i]=windPlotType[i];
        if(i<windNum)
        {
            if(2==windPlotType[i] || 3==windPlotType[i])
            {
                bgraySliderShow = true;
            }
        }
        CwindowDisp->windSensorType[i]=windSensorType[i];
    }
    if(!bgraySliderShow)
        ui->grayscaleSetSlider->setVisible(false);
    else
        ui->grayscaleSetSlider->setVisible(true);
    setMutiWindow(CwindowDisp->windNum);
    emit plotCacheDataRequestBybox(0);
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
        QStandardItem* wallthicknessNumberItem = new QStandardItem(QString::number(vecPrjs[i].dwallthickness));
        QStandardItem* outerDiameterItem = new QStandardItem(QString::number(vecPrjs[i].outerDiameter));
        QStandardItem* createtimeItem = new QStandardItem(vecPrjs[i].createtime);
        QStandardItem* creatorNameItem = new QStandardItem(vecPrjs[i].creatorName);
        rowItems<<prjNameItem;
        rowItems<<discriptItem;
        rowItems<<wallthicknessItem;
        rowItems<<sampleintervalItem;
        rowItems<<wallthicknessNumberItem;
        rowItems<<outerDiameterItem;
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
    QVector<QMap<QString,int>>().swap(CprjConfig->fileNameBytesMapByBox);

    //获取文件夹下box子文件夹，及box文件夹内的文件名
    QFileInfoList folderInfos = dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot,
                                                  QDir::Name|QDir::IgnoreCase);
    for(const QFileInfo &info:folderInfos)
    {
        QString absolutePath = info.absoluteFilePath()+"/";
        CprjConfig->boxDirPath.append(absolutePath);
    }

    int boxNum = CprjConfig->boxDirPath.size();
    if(0 == boxNum)
    {
        msgBox::show("告警","文件格式错误，盒子文件夹不存在",2);
        QVector<QString>().swap(CprjConfig->boxDirPath);
        CprjConfig->dataDirPath = "";
        return;
    }
    for(int i=0;i<boxNum;i++)
    {
        int bytesNum = 0;
        QString boxPath = CprjConfig->boxDirPath[i];
        QDir boxDir(boxPath);
        QString qsfilePath="";
        QStringList filters = {"*.bin"};
        QFileInfoList files = boxDir.entryInfoList(filters,QDir::Files,QDir::Name);
        if(files.isEmpty())
        {
            msgBox::show("告警",QString("打开项目错误，盒子:%1路径下无数据文件").arg(boxPath),2);
            CprjConfig->dataDirPath = "";
            QVector<QString>().swap(CprjConfig->boxDirPath);
            return;
        }
        QVector<QString>filenameVec;
        QMap<QString,int>fileNameBytesMap;
        for (const QFileInfo &file : files) {
            QString fileName = file.fileName();
            filenameVec.append(fileName);
            QString subString = fileName.mid(9,3);
            int result = QString::compare("000",subString);
            if(0 == result)
            {
                CprjConfig->curFileNamevec.append(file.fileName());
            }
            //查询每个文件的帧数
            bytesNum = static_cast<int>(file.size());
            int frameNum = (bytesNum-20)/268;
            fileNameBytesMap.insert(fileName,frameNum);
        }
        CprjConfig->fileNameVecByBox.append(filenameVec);
        CprjConfig->fileNameBytesMapByBox.append(fileNameBytesMap);
    }
    CprjConfig->dInterval = onePrj.sampleinterval;
    CprjConfig->outerDiameter = onePrj.outerDiameter;
    CprjConfig->dwallthickness = onePrj.dwallthickness;
    ui->prjNamelabel->setText("当前项目:"+onePrj.name);
    ui->openPrj->setEnabled(false);
    ui->closePrj->setEnabled(true);
}

void MainWindow::handleShowAddNewProject(projectDataModel &onePrj)
{
    QList<QStandardItem*>rowItems;
    QStandardItem* prjNameItem = new QStandardItem(onePrj.name);
    prjNameItem->setData(onePrj.id,Qt::UserRole+1);
    QStandardItem* discriptItem = new QStandardItem(onePrj.discript);
    QStandardItem* wallthicknessItem = new QStandardItem(onePrj.wallthicknesstype);
    QStandardItem* sampleintervalItem = new QStandardItem(QString::number(onePrj.sampleinterval));
    QStandardItem* wallthicknessNumberItem = new QStandardItem(QString::number(onePrj.dwallthickness));
    QStandardItem* outerDiameterItem = new QStandardItem(QString::number(onePrj.outerDiameter));
    QStandardItem* createtimeItem = new QStandardItem(onePrj.createtime);
    QStandardItem* creatorNameItem = new QStandardItem(onePrj.creatorName);
    rowItems<<prjNameItem;
    rowItems<<discriptItem;
    rowItems<<wallthicknessItem;
    rowItems<<sampleintervalItem;
    rowItems<<wallthicknessNumberItem;
    rowItems<<outerDiameterItem;
    rowItems<<createtimeItem;
    rowItems<<creatorNameItem;
    prjTableModel->appendRow(rowItems);
}

void MainWindow::handleShowEditProject(int row, projectDataModel &onePrj)
{
    prjTableModel->item(row,0)->setText(onePrj.name);
    prjTableModel->item(row,1)->setText(onePrj.discript);
    prjTableModel->item(row,2)->setText(onePrj.wallthicknesstype);
    prjTableModel->item(row,3)->setText(QString::number(onePrj.sampleinterval));
    prjTableModel->item(row,4)->setText(QString::number(onePrj.dwallthickness));
    prjTableModel->item(row,5)->setText(QString::number(onePrj.outerDiameter));
}

void MainWindow::handleShowDeleteProject(int row)
{
    prjTableModel->removeRow(row);
}

void MainWindow::handleShowDetailProject(projectDataModel &onePrj)
{
    projectDlg* prjDetailDlg = new projectDlg(this);
    prjDetailDlg->setAttribute(Qt::WA_DeleteOnClose);
    prjDetailDlg->trans2detailDlg(onePrj);
    prjDetailDlg->exec();
}

void MainWindow::handleDetectDefectComplete()
{
    if(m_detectDefectingFlag)
        m_detectDefectingFlag=false;
}

void MainWindow::handleSig_wheelEvent(qint64 xLower, qint64 xUpper, qint64 yLower, qint64 yUpper)
{
    //优化滚轮和拖动手势显示效果，在绘图中不触发
    if(!m_plotting)
    {
        m_plotting = true;
    }
    else {
        return;
    }

    CwindowDisp->startPos = static_cast<qint64>(xLower/CprjConfig->dInterval);
    CwindowDisp->offset = static_cast<qint64>((xUpper-xLower)/CprjConfig->dInterval);
    CwindowDisp->yLower = yLower;
    CwindowDisp->yUpper = yUpper;

    qint64 startPosInFile = 0;
    qint64 offsetInfile = 0;

    if(xLower <= 0)
    {
        startPosInFile = 0;
    }
    else {
        startPosInFile = static_cast<qint64>(xLower/CprjConfig->dInterval);
    }

    if(xLower < 0 && xUpper > 0)
    {
        offsetInfile = static_cast<qint64>(xUpper/CprjConfig->dInterval);
    }
    else if(xLower >0 && xUpper>0)
    {
        offsetInfile = static_cast<qint64>((xUpper-xLower)/CprjConfig->dInterval);
    }
    else {
        offsetInfile = 0;
    }

    //超出Int范围不处理
    qint64 iSum = startPosInFile + offsetInfile;
    if(iSum>=0)
    {
        QString qsfilePath = "";
        emit modelDataRequest(qsfilePath,startPosInFile,offsetInfile);
    }
    else {
        m_plotting = false;
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
        //无需传入qsfilePath，在槽函数中会拼接生成
        QString qsfilePath = "";
        setMutiWindow(CwindowDisp->windNum);
        emit modelDataRequest(qsfilePath,CwindowDisp->startPos,CwindowDisp->offset);
        CwindowDisp->bFirstPlot = false;
    }
}

void MainWindow::on_nextPageBtn_clicked()
{
    CwindowDisp->startPos += CwindowDisp->offset;
    QString qsfilePath = "";

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
    QString qsfilePath = "";

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
    removePixmapItem(ui->QcpText_1);     //若存在灰度图像，清除灰度图像，保留追踪器相关item
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
    CprjConfig->outerDiameter = 0.0;
    CprjConfig->dwallthickness = 0.0;
    CprjConfig->dataDirPath = "";
    QVector<QString>().swap(CprjConfig->curFileNamevec);
    QVector<QString>().swap(CprjConfig->boxDirPath);
    QVector<QVector<QString>>().swap(CprjConfig->fileNameVecByBox);
    QVector<QMap<QString,int>>().swap(CprjConfig->fileNameBytesMapByBox);
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
//    QMessageBox::StandardButton reply;
//    reply = QMessageBox::question(this,"确认删除",QString("确定要删除用户'%1'吗").arg(username),
//                                  QMessageBox::Yes|QMessageBox::No);
    QMessageBox qmsgBox(this);
    qmsgBox.setWindowTitle("确认删除");
    qmsgBox.setText(QString("确定要删除用户'%1'吗").arg(username));
    qmsgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    qmsgBox.setButtonText(QMessageBox::Yes,"是");
    qmsgBox.setButtonText(QMessageBox::No,"否");
    int reply = qmsgBox.exec();
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
    emit queryProjectById(prjId,1);
}

void MainWindow::on_closePrj_clicked()
{
    //画板恢复空白
    clearPlotboard(ui->QcpText_1);
    clearPlotboard(QcpText_2);
    clearPlotboard(QcpText_3);
    clearPlotboard(QcpText_4);
    CwindowDisp->bFirstPlot = true;
    CwindowDisp->startPos = 0;
    CwindowDisp->offset = 2000;
    CwindowDisp->yLower = 0;
    CwindowDisp->yUpper = 0;
    CwindowDisp->pageOffset = 1500;
    CprjConfig->dInterval = 0;
    CprjConfig->outerDiameter = 0.0;
    CprjConfig->dwallthickness = 0.0;
    CprjConfig->dataDirPath = "";
    QVector<QString>().swap(CprjConfig->curFileNamevec);
    QVector<QString>().swap(CprjConfig->boxDirPath);
    QVector<QVector<QString>>().swap(CprjConfig->fileNameVecByBox);
    QVector<QMap<QString,int>>().swap(CprjConfig->fileNameBytesMapByBox);

    ui->openPrj->setEnabled(true);
    ui->closePrj->setEnabled(false);
}

void MainWindow::on_newPrj_clicked()
{
    projectDlg* newPrjDlg = new projectDlg(this);
    newPrjDlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(newPrjDlg,&projectDlg::newProjectRequest,m_dbWorker,&databaseWorker::handleNewProjectRequest);
    newPrjDlg->trans2newDlg();
    newPrjDlg->setCurrentUser(*CcurrentUserMod);
    newPrjDlg->exec();
}

void MainWindow::on_editPrj_clicked()
{
    QModelIndexList selectedIndexes = ui->projectTableView->selectionModel()->selectedRows();
    if(selectedIndexes.isEmpty())
    {
        return;
    }
    int row = selectedIndexes.first().row();
    int projectId = prjTableModel->item(row,0)->data(Qt::UserRole+1).toInt();
    QString prjName = prjTableModel->item(row,0)->text();
    QString prjDescribe = prjTableModel->item(row,1)->text();
    QString thicknessType = prjTableModel->item(row,2)->text();
    double dInterval = prjTableModel->item(row,3)->text().toDouble();
    double dwallthickness = prjTableModel->item(row,4)->text().toDouble();
    double douterDiameter = prjTableModel->item(row,5)->text().toDouble();
    projectDlg* prjEditDlg = new projectDlg(this);
    prjEditDlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(prjEditDlg,&projectDlg::editProjectRequest,m_dbWorker,&databaseWorker::handleEditProjectRequest);
    prjEditDlg->trans2editDlg(projectId,prjName,prjDescribe,thicknessType,dInterval,dwallthickness,douterDiameter,"",row);
    prjEditDlg->exec();
}

void MainWindow::on_deletePrj_clicked()
{
    QModelIndexList selectedIndexes = ui->projectTableView->selectionModel()->selectedRows();
    if(selectedIndexes.isEmpty())
    {
        return;
    }
    int row = selectedIndexes.first().row();
    int projectId = prjTableModel->item(row,0)->data(Qt::UserRole+1).toInt();
    QString projectName = prjTableModel->item(row,0)->text();
//    QMessageBox::StandardButton reply;
//    reply = QMessageBox::question(this,"确认删除",QString("确定要删除项目'%1'吗").arg(projectName),
//                                  QMessageBox::Yes|QMessageBox::No);
    //将对话框中的yes|no改为中文
    QMessageBox qmsgBox(this);
    qmsgBox.setWindowTitle("确认删除");
    qmsgBox.setText(QString("确定要删除项目'%1'吗").arg(projectName));
    qmsgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    qmsgBox.setButtonText(QMessageBox::Yes,"是");
    qmsgBox.setButtonText(QMessageBox::No,"否");
    int reply = qmsgBox.exec();
    if(QMessageBox::Yes == reply)
    {
        emit deleteProjectRequest(row,projectId);
    }
}

void MainWindow::on_detailPrj_clicked()
{
    QModelIndexList selectedIndexes = ui->projectTableView->selectionModel()->selectedRows();
    if(selectedIndexes.isEmpty())
    {
        return;
    }
    int row = selectedIndexes.first().row();
    int projectId = prjTableModel->item(row,0)->data(Qt::UserRole+1).toInt();
    emit queryProjectById(projectId,2);
}

void MainWindow::on_detectDefect_triggered()
{
    if(m_detectDefectingFlag)
    {
        qDebug()<<"当前进行缺陷分析的项目名："<<m_detectDefectingPrjName;
        return;
    }
    else {
        if(ui->openPrj->isEnabled())
        {
            msgBox::show("警告","未选择项目打开",2);
            return;
        }
        //缺陷分析线程创立及触发操作
        qDebug()<<"创建defectdetector所在线程为:"<<QThread::currentThreadId();
        if(nullptr == m_defectdetectorWorker)
        {
            QThread* thread = new QThread;
            m_defectdetectorWorker = new defectdetector;
            m_defectdetectorWorker->moveToThread(thread);

            //连接信号槽
            connect(this,&MainWindow::startDetectDefects,m_defectdetectorWorker,&defectdetector::handleStartDetectDefects);
            connect(m_defectdetectorWorker,&defectdetector::detectDefectComplete,this,&MainWindow::handleDetectDefectComplete);
            thread->start();
            //emit
            emit startDetectDefects(CprjConfig->dInterval,CprjConfig->outerDiameter-2*CprjConfig->dwallthickness,*CprjConfig);
        }
        else {
            //emit
            emit startDetectDefects(CprjConfig->dInterval,CprjConfig->outerDiameter-2*CprjConfig->dwallthickness,*CprjConfig);
        }


        m_detectDefectingFlag = true;
        int colonPos = ui->prjNamelabel->text().indexOf(":");
        if(-1 != colonPos)
        {
            m_detectDefectingPrjName=ui->prjNamelabel->text().mid(colonPos+1);
        }
    }
}

void MainWindow::on_grayscaleSetSlider_valueChanged(int value)
{
    int currentValue = qRound(value/static_cast<double>(5))*5;
    if(currentValue != m_grayScaleQsliderValue)
    {
        m_grayScaleQsliderValue = currentValue;
        ui->grayscaleSetSlider->blockSignals(true);
        ui->grayscaleSetSlider->setValue(m_grayScaleQsliderValue);
        ui->grayscaleSetSlider->blockSignals(false);
        qDebug()<<"滑动条近似数据："<<m_grayScaleQsliderValue;
        if(!bupdateGrayScaleing)
        {
            bupdateGrayScaleing = true;
            //emit
            grayValueLower = static_cast<int>(127-m_grayScaleQsliderValue/100.0*127);
            grayValueUpper = static_cast<int>(128+m_grayScaleQsliderValue/100.0*127);
            qDebug()<<"灰度值下界为:"<<grayValueLower<<"---"<<"灰度值上界为:"<<grayValueUpper;
            emit plotCacheDataRequestBybox(1);
        }
    }
}

void MainWindow::on_showDefect_triggered(bool checked)
{
    if(ui->openPrj->isEnabled())
    {
        msgBox::show("警告","未选择项目打开",2);
        ui->showDefect->setChecked(false);
        return;
    }
    if(checked && !m_detectDefectingFlag)
    {

    }
    else {
        qDebug()<<"关闭显示缺陷";
    }
}
