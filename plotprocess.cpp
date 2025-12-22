#include "plotprocess.h"
#include "dataservice.h"
#include <QElapsedTimer>

plotProcess* plotProcess::m_instance = nullptr;
QMutex plotProcess::m_mutex;

plotProcess *plotProcess::getInstance()
{
    if(nullptr == m_instance)
    {
        QMutexLocker locker(&m_mutex);
        if(nullptr == m_instance)
        {
            m_instance = new plotProcess();
        }
    }
    return m_instance;
}

//vec为增加盒子逻辑，按盒子顺序存入vec中
//interval,采样间隙mm
//qmDataModel的key 123456 对应探头123456
//qmCPData的key 1234对应HallX-Y-Z-Vortex四种视图
//qmCPData的value是二维数组，1-6行对应探头一的6个传感器，7-12行对应探头二的6个传感器，以此类推
int plotProcess::dataModel2PlotDataBybox(QVector<QMap<int, QVector<dataModel> > > &qmDataModelvec, QVector<QMap<int, QVector<QVector<QCPGraphData> > > > &qmCPDatavec)
{
    for(int i=0;i<qmDataModelvec.size();i++)
    {
        QMap<int,QVector<dataModel>>&qmDataModel = qmDataModelvec[i];
        QMap<int,QVector<QVector<QCPGraphData>>> qmCPData;

        QMap<int,QVector<dataModel>>::iterator itMap = qmDataModel.begin();
        for(;itMap != qmDataModel.end();itMap++)
        {
            switch (itMap.key()) {
            case 1:{   //探头一
                QVector<dataModel>& oneModelVec = itMap.value();
                int rowOffset = (itMap.key()-1)*6;
                dataModel2PlotDataByProbe(oneModelVec,qmCPData,rowOffset);
                break;
            }
            case 2:{   //探头二
                QVector<dataModel>& oneModelVec = itMap.value();
                int rowOffset = (itMap.key()-1)*6;
                dataModel2PlotDataByProbe(oneModelVec,qmCPData,rowOffset);
                break;
            }
            case 3:{   //探头三
                QVector<dataModel>& oneModelVec = itMap.value();
                int rowOffset = (itMap.key()-1)*6;
                dataModel2PlotDataByProbe(oneModelVec,qmCPData,rowOffset);
                break;
            }
            case 4:{   //探头四
                QVector<dataModel>& oneModelVec = itMap.value();
                int rowOffset = (itMap.key()-1)*6;
                dataModel2PlotDataByProbe(oneModelVec,qmCPData,rowOffset);
                break;
            }
            case 5:{   //探头五
                QVector<dataModel>& oneModelVec = itMap.value();
                int rowOffset = (itMap.key()-1)*6;
                dataModel2PlotDataByProbe(oneModelVec,qmCPData,rowOffset);
                break;
            }
            case 6:{   //探头六
                QVector<dataModel>& oneModelVec = itMap.value();
                int rowOffset = (itMap.key()-1)*6;
                dataModel2PlotDataByProbe(oneModelVec,qmCPData,rowOffset);
                break;
            }
            default:
            {}
            }
        }
        qmCPDatavec.append(qmCPData);
    }
    return 0;
}

//从大变小，然后恢复，区间内的值：  16320  -16000      2*16383.5-16000=16767
//从小变大，然后恢复，区间内的值：  -16000 16320       16320-2*16383.5=-16477
void plotProcess::dataPreProcessingBybox(QVector<QMap<int, QVector<QVector<QCPGraphData> > > > &qmCPDatavec, QVector<QMap<int, QVector<QVector<QCPGraphData> > > > &qmPreCPDatavec, qint64 startPos)
{
    double thresholdforXY = 6000;
    double thresholdforZ = 3000;
    int boxSize = qmCPDatavec.size();
    for(int kk=0;kk<boxSize;kk++)
    {
        QMap<int, QVector<QVector<QCPGraphData>>>&qmCPData = qmCPDatavec[kk];
        //文件中的第一帧数据时(0 == startPos)，m_preCPData设置为第一帧数据
        //(startPos <= 0)时，用qmPreCPData给m_preCPData赋初值，qmPreCPData为全文前一帧数据
        if(startPos <= 0)
        {
            //恢复m_preProcessFlagBybox初始状态
            initalPlotProcess();
            //给m_preCPDataBybox赋初值
            QMap<int,QVector<QCPGraphData>>().swap(m_preCPData);
            for (int i=1 ; i<=3 ; i++)
            {
                if(qmCPData.end() != qmCPData.find(i))
                {
                    QVector<QVector<QCPGraphData>>& valueVec2D = qmCPData[i];
                    int iSize = valueVec2D.size();
                    for(int j=0;j<iSize;j++)
                    {
                        QVector<QCPGraphData>& valueVec1D = valueVec2D[j];
                        if(valueVec1D.size() > 0)
                        {
                            if(m_preCPData.end() != m_preCPData.find(i))
                            {
                                QVector<QCPGraphData>& preCPDataVec = m_preCPData[i];
                                preCPDataVec.append(valueVec1D[0]);
                            }
                            else {
                                QVector<QCPGraphData> preCPDataVec;
                                preCPDataVec.append(valueVec1D[0]);
                                m_preCPData.insert(i,preCPDataVec);
                            }
                        }
                    }
                }
            }
            //从k=1开始比较
            for (int i=1 ; i<=3 ; i++)
            {
                if(qmCPData.end() != qmCPData.find(i))
                {
                    QVector<QVector<QCPGraphData>>& valueVec2D = qmCPData[i];
                    int iSize = valueVec2D.size();
                    for(int j=0;j<iSize;j++)
                    {
                        QVector<QCPGraphData>& valueVec1D = valueVec2D[j];
                        int vec1DSize = valueVec1D.size();
                        QCPGraphData& preData = m_preCPData[i][j];
                        quint8& transFlag = m_preProcessFlag[i][j];
                        for(int k=1;k<vec1DSize;k++)
                        {
                            QCPGraphData& currentData = valueVec1D[k];
                            //一个通道一个通道比较，36个通道，而后每通道逐一记录前一帧数据
                            if(3 != i)  //X和Y轴
                            {
                                dataPreProcessOnePoint(preData,currentData,transFlag,thresholdforXY,hallUpperLimitXY,hallLowerLimitXY);
                            }
                            // 3==i
                            else {     //Z轴
                                dataPreProcessOnePoint(preData,currentData,transFlag,thresholdforZ,hallUpperLimitZ,hallLowerLimitZ);
                            }
                        }
                    }
                }
            }
        }
        // startPos>0
        else
        {
            //恢复m_preProcessFlag初始状态
            initalPlotProcess();
            //用qmPreCPData给m_preCPData赋初值
            QMap<int,QVector<QCPGraphData>>().swap(m_preCPData);
            QMap<int, QVector<QVector<QCPGraphData>>>&qmPreCPData = qmPreCPDatavec[kk];
            for (int i=1 ; i<=3 ; i++)
            {
                if(qmPreCPData.end() != qmPreCPData.find(i))
                {
                    QVector<QVector<QCPGraphData>>& valueVec2D = qmPreCPData[i];
                    int iSize = valueVec2D.size();
                    for(int j=0;j<iSize;j++)
                    {
                        QVector<QCPGraphData>& valueVec1D = valueVec2D[j];
                        if(valueVec1D.size() > 0)
                        {
                            if(m_preCPData.end() != m_preCPData.find(i))
                            {
                                QVector<QCPGraphData>& preCPDataVec = m_preCPData[i];
                                preCPDataVec.append(valueVec1D[0]);
                            }
                            else {
                                QVector<QCPGraphData> preCPDataVec;
                                preCPDataVec.append(valueVec1D[0]);
                                m_preCPData.insert(i,preCPDataVec);
                            }
                        }
                    }
                }
            }
            //从k=0开始比较
            for (int i=1 ; i<=3 ; i++)
            {
                if(qmCPData.end() != qmCPData.find(i))
                {
                    QVector<QVector<QCPGraphData>>& valueVec2D = qmCPData[i];
                    int iSize = valueVec2D.size();
                    for(int j=0;j<iSize;j++)
                    {
                        QVector<QCPGraphData>& valueVec1D = valueVec2D[j];
                        int vec1DSize = valueVec1D.size();
                        QCPGraphData& preData = m_preCPData[i][j];
                        quint8& transFlag = m_preProcessFlag[i][j];
                        for(int k=0;k<vec1DSize;k++)
                        {
                            QCPGraphData& currentData = valueVec1D[k];
                            //一个通道一个通道比较，36个通道，而后每通道逐一记录前一帧数据
                            if(3 != i)  //X和Y轴
                            {
                                dataPreProcessOnePoint(preData,currentData,transFlag,thresholdforXY,hallUpperLimitXY,hallLowerLimitXY);
                            }
                            // 3==i
                            else {     //Z轴
                                dataPreProcessOnePoint(preData,currentData,transFlag,thresholdforZ,hallUpperLimitZ,hallLowerLimitZ);
                            }
                        }
                    }
                }
            }
        }
    }
}

void plotProcess::dataPreProcessOnePoint(QCPGraphData &preData, QCPGraphData &currentData, quint8 &transFlag, double &threshold, double &hallUpperLimit, double &hallLowerLimit)
{
    //正常状态
    if(0 == transFlag)
    {
        //判断是否从大到小
        if(preData.value > hallUpperLimit-threshold && currentData.value < hallLowerLimit+threshold)
        {
            //当前值需改变
            double temp = currentData.value;
            currentData.value = 2*hallUpperLimit+currentData.value;
            preData.value = temp;
            transFlag = 1;
        }
        //判断是否从小变大
        else if(preData.value < hallLowerLimit+threshold && currentData.value > hallUpperLimit-threshold)
        {
            //当前值需改变
            double temp = currentData.value;
            currentData.value = 2*hallLowerLimit+currentData.value;
            preData.value = temp;
            transFlag = 2;
        }
        //余下情况不变化,只需更新preData
        else {
            preData.value = currentData.value;
        }
    }
    //山谷型异常状态
    else if (1 == transFlag) {
        //从小到大（恢复正常）
        if(preData.value < hallLowerLimit+threshold && currentData.value > hallUpperLimit-threshold)
        {
            preData.value = currentData.value;
            transFlag = 0;
        }
        //区间内，持续偏移恢复处理
        else
        {
            double temp = currentData.value;
            currentData.value = 2*hallUpperLimit+currentData.value;
            preData.value = temp;
        }
    }
    //山峰型异常状态 2 == transFlag
    else
    {
        //从大到小（恢复正常）
        if(preData.value > hallUpperLimit-threshold && currentData.value < hallLowerLimit+threshold)
        {
            preData.value = currentData.value;
            transFlag = 0;
        }
        //区间内，持续偏移恢复处理
        else
        {
            double temp = currentData.value;
            currentData.value = 2*hallLowerLimit+currentData.value;
            preData.value = temp;
        }
    }
}

//dataModel2PlotData中每个探头的解析子函数   每个探头的数据oneModelVec
//根据rowOffest指定QVector<QVector<QCPGraphData> >数组行号 取值（0，6，12，18，24，30）
int plotProcess::dataModel2PlotDataByProbe(QVector<dataModel>& oneModelVec, QMap<int, QVector<QVector<QCPGraphData> > > &qmCPData,int rowOffest)
{
    for(int i=0;i<oneModelVec.size();i++)
    {
        dataModel& oneDataModel = oneModelVec[i];
        int type=1;//HallX
        QMap<int, QVector<QVector<QCPGraphData> >>::iterator itMap_CP = qmCPData.find(type);
        if(qmCPData.end() != itMap_CP)
        {
            QVector<QVector<QCPGraphData>>&oneVector = itMap_CP.value();
            QCPGraphData onecpData;
            onecpData.key = i;            
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall1XAxis,1,5,12,1,0));
            oneVector[0+rowOffest].append(onecpData);  //hall1X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall2XAxis,1,5,12,1,0));
            oneVector[1+rowOffest].append(onecpData);  //hall2X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall3XAxis,1,5,12,1,0));
            oneVector[2+rowOffest].append(onecpData);  //hall3X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall4XAxis,1,5,12,1,0));
            oneVector[3+rowOffest].append(onecpData);  //hall4X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall5XAxis,1,5,12,1,0));
            oneVector[4+rowOffest].append(onecpData);  //hall5X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall6XAxis,1,5,12,1,0));
            oneVector[5+rowOffest].append(onecpData);  //hall6X
        }
        else
        {
            QVector<QVector<QCPGraphData>> oneVector(36);
            QCPGraphData onecpData;
            onecpData.key = i;
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall1XAxis,1,5,12,1,0));
            oneVector[0+rowOffest].append(onecpData);  //hall1X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall2XAxis,1,5,12,1,0));
            oneVector[1+rowOffest].append(onecpData);  //hall2X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall3XAxis,1,5,12,1,0));
            oneVector[2+rowOffest].append(onecpData);  //hall3X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall4XAxis,1,5,12,1,0));
            oneVector[3+rowOffest].append(onecpData);  //hall4X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall5XAxis,1,5,12,1,0));
            oneVector[4+rowOffest].append(onecpData);  //hall5X
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall6XAxis,1,5,12,1,0));
            oneVector[5+rowOffest].append(onecpData);  //hall6X
            qmCPData.insert(type,oneVector);
        }

        type=2;//HallY
        itMap_CP=qmCPData.find(type);
        if(qmCPData.end() != itMap_CP)
        {
            QVector<QVector<QCPGraphData>>&oneVector = itMap_CP.value();
            QCPGraphData onecpData;
            onecpData.key = i;
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall1YAxis,2,5,12,1,0));
            oneVector[0+rowOffest].append(onecpData);  //hall1Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall2YAxis,2,5,12,1,0));
            oneVector[1+rowOffest].append(onecpData);  //hall2Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall3YAxis,2,5,12,1,0));
            oneVector[2+rowOffest].append(onecpData);  //hall3Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall4YAxis,2,5,12,1,0));
            oneVector[3+rowOffest].append(onecpData);  //hall4Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall5YAxis,2,5,12,1,0));
            oneVector[4+rowOffest].append(onecpData);  //hall5Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall6YAxis,2,5,12,1,0));
            oneVector[5+rowOffest].append(onecpData);  //hall6Y
        }
        else
        {
            QVector<QVector<QCPGraphData>> oneVector(36);
            QCPGraphData onecpData;
            onecpData.key = i;
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall1YAxis,2,5,12,1,0));
            oneVector[0+rowOffest].append(onecpData);  //hall1Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall2YAxis,2,5,12,1,0));
            oneVector[1+rowOffest].append(onecpData);  //hall2Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall3YAxis,2,5,12,1,0));
            oneVector[2+rowOffest].append(onecpData);  //hall3Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall4YAxis,2,5,12,1,0));
            oneVector[3+rowOffest].append(onecpData);  //hall4Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall5YAxis,2,5,12,1,0));
            oneVector[4+rowOffest].append(onecpData);  //hall5Y
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall6YAxis,2,5,12,1,0));
            oneVector[5+rowOffest].append(onecpData);  //hall6Y
            qmCPData.insert(type,oneVector);
        }

        type=3;//HallZ
        itMap_CP=qmCPData.find(type);
        if(qmCPData.end() != itMap_CP)
        {
            QVector<QVector<QCPGraphData>>&oneVector = itMap_CP.value();
            QCPGraphData onecpData;
            onecpData.key = i;
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall1ZAxis,3,5,12,0,0));
            oneVector[0+rowOffest].append(onecpData);  //hall1Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall2ZAxis,3,5,12,0,0));
            oneVector[1+rowOffest].append(onecpData);  //hall2Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall3ZAxis,3,5,12,0,0));
            oneVector[2+rowOffest].append(onecpData);  //hall3Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall4ZAxis,3,5,12,0,0));
            oneVector[3+rowOffest].append(onecpData);  //hall4Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall5ZAxis,3,5,12,0,0));
            oneVector[4+rowOffest].append(onecpData);  //hall5Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall6ZAxis,3,5,12,0,0));
            oneVector[5+rowOffest].append(onecpData);  //hall6Z
        }
        else
        {
            QVector<QVector<QCPGraphData>> oneVector(36);
            QCPGraphData onecpData;
            onecpData.key = i;
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall1ZAxis,3,5,12,0,0));
            oneVector[0+rowOffest].append(onecpData);  //hall1Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall2ZAxis,3,5,12,0,0));
            oneVector[1+rowOffest].append(onecpData);  //hall2Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall3ZAxis,3,5,12,0,0));
            oneVector[2+rowOffest].append(onecpData);  //hall3Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall4ZAxis,3,5,12,0,0));
            oneVector[3+rowOffest].append(onecpData);  //hall4Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall5ZAxis,3,5,12,0,0));
            oneVector[4+rowOffest].append(onecpData);  //hall5Z
            onecpData.value = static_cast<double>(convert2float(oneDataModel._Hall6ZAxis,3,5,12,0,0));
            oneVector[5+rowOffest].append(onecpData);  //hall6Z
            qmCPData.insert(type,oneVector);
        }

        type=4;//Vortex
        itMap_CP=qmCPData.find(type);
        if(qmCPData.end() != itMap_CP)
        {
            QVector<QVector<QCPGraphData>>&oneVector = itMap_CP.value();
            QCPGraphData onecpData;
            onecpData.key = i;
            onecpData.value = oneDataModel._Vortex1Channel;
            oneVector[0+rowOffest].append(onecpData);  //Vortex1
            onecpData.value = oneDataModel._Vortex2Channel;
            oneVector[1+rowOffest].append(onecpData);  //Vortex2
            onecpData.value = oneDataModel._Vortex3Channel;
            oneVector[2+rowOffest].append(onecpData);  //Vortex3
            onecpData.value = oneDataModel._Vortex4Channel;
            oneVector[3+rowOffest].append(onecpData);  //Vortex4
            onecpData.value = oneDataModel._Vortex5Channel;
            oneVector[4+rowOffest].append(onecpData);  //Vortex5
            onecpData.value = oneDataModel._Vortex6Channel;
            oneVector[5+rowOffest].append(onecpData);  //Vortex6
        }
        else
        {
            QVector<QVector<QCPGraphData>> oneVector(36);
            QCPGraphData onecpData;
            onecpData.key = i;
            onecpData.value = oneDataModel._Vortex1Channel;
            oneVector[0+rowOffest].append(onecpData);  //Vortex1
            onecpData.value = oneDataModel._Vortex2Channel;
            oneVector[1+rowOffest].append(onecpData);  //Vortex2
            onecpData.value = oneDataModel._Vortex3Channel;
            oneVector[2+rowOffest].append(onecpData);  //Vortex3
            onecpData.value = oneDataModel._Vortex4Channel;
            oneVector[3+rowOffest].append(onecpData);  //Vortex4
            onecpData.value = oneDataModel._Vortex5Channel;
            oneVector[4+rowOffest].append(onecpData);  //Vortex5
            onecpData.value = oneDataModel._Vortex6Channel;
            oneVector[5+rowOffest].append(onecpData);  //Vortex6
            qmCPData.insert(type,oneVector);
        }
    }
    return 0;
}

float plotProcess::convert2float(quint16 &raw, quint8 Axis, quint8 gain_sel, quint8 hallconf, quint8 res, quint8 tcmp_en)
{
    float xy_sens;
    float z_sens;
    float data=0.f;

    switch(hallconf){
      default:
      case 0:
        xy_sens = base_xy_sens_hc0;
        z_sens = base_z_sens_hc0;
        break;
      case 0xc:
        xy_sens = base_xy_sens_hc0xc;
        z_sens = base_z_sens_hc0xc;
        break;
    }

    float gain_factor = gain_multipliers[gain_sel & 0x7];

    if(Axis != 3){  //xy
        if (tcmp_en){
            data = ( (raw - 32768.f) * xy_sens * gain_factor * (1 << res) );
        }
        else {
            switch(res){
            case 0:
            case 1:
              data = int16_t(raw) * xy_sens * gain_factor * (1 << res);
              break;
            case 2:
              data = ( (raw - 32768.f) * xy_sens * gain_factor * (1 << res) );
              break;
            case 3:
              data = ( (raw - 16384.f) * xy_sens * gain_factor * (1 << res) );
              break;
            }
        }
    }
    else           //z
    {
        if (tcmp_en){
            data = ( (raw - 32768.f) * z_sens * gain_factor * (1 << res) );
        }
        else {
            switch(res){
            case 0:
            case 1:
              data = int16_t(raw) * z_sens * gain_factor * (1 << res);
              break;
            case 2:
              data = ( (raw - 32768.f) * z_sens * gain_factor * (1 << res) );
              break;
            case 3:
              data = ( (raw - 16384.f) * z_sens * gain_factor * (1 << res) );
              break;
            }
        }
    }

    return data;
}

void plotProcess::initalPlotProcess()
{
    QMap<int,QVector<quint8>>().swap(m_preProcessFlag);
    QVector<quint8>oneVector(36,0);
    for(int i=1;i<=3;i++)
    {
        m_preProcessFlag.insert(i,oneVector);
    }
}

void plotProcess::handledataModel2PlotProcessBybox(QVector<QMap<int, QVector<dataModel> > > &qmDataModelVec, QVector<QMap<int, QVector<dataModel> > > &onePreDatavec, qint64 startPos)
{
    //添加读写锁
    dataService::getInstance()->m_dataRwLock.lockForRead();
    QVector<QMap<int,QVector<QVector<QCPGraphData>>>>().swap(m_CPDataVec);
    dataModel2PlotDataBybox(qmDataModelVec,m_CPDataVec);
    //预处理求前一帧数据
    QVector<QMap<int,QVector<QVector<QCPGraphData>>>>onePreCPDataVec;
    if(startPos > 0)
    {
        dataModel2PlotDataBybox(onePreDatavec,onePreCPDataVec);
    }
    //数据预处理（过滤异常数据）
    dataPreProcessingBybox(m_CPDataVec,onePreCPDataVec,startPos);
    dataService::getInstance()->m_dataRwLock.unlock();
    //emit数据发送到mainwindow
    emit plotDataReadyBybox(m_CPDataVec);
}

void plotProcess::handleplotCacheDataRequestBybox()
{
    if(!m_CPDataVec.empty())
    {
        emit plotDataReadyBybox(m_CPDataVec);
    }
}

plotProcess::plotProcess(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QVector<QMap<int,QVector<dataModel>>>>("QVector<QMap<int,QVector<dataModel>>>&");
    connect(dataService::getInstance(),&dataService::dataModel2PlotProcessBybox,this,&plotProcess::handledataModel2PlotProcessBybox);

    // gain steps derived from datasheet section 15.1.4 tables
    gain_multipliers[0] = 5.f;
    gain_multipliers[1] = 4.f;
    gain_multipliers[2] = 3.f;
    gain_multipliers[3] = 2.5f;
    gain_multipliers[4] = 2.f;
    gain_multipliers[5] = 1.66666667f;
    gain_multipliers[6] = 1.33333333f;
    gain_multipliers[7] = 1.f;

    // from datasheet
    // for hallconf = 0
    base_xy_sens_hc0 = 0.196f;
    base_z_sens_hc0 = 0.316f;
    // for hallconf = 0xc
    base_xy_sens_hc0xc = 0.150f;
    base_z_sens_hc0xc = 0.242f;

    //hall取值范围
    hallUpperLimitXY = 16383.5;
    hallLowerLimitXY = -16383.5;
    hallUpperLimitZ = 8191.75;
    hallLowerLimitZ = -8191.75;

    initalPlotProcess();
}
