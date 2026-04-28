#include "defectdetector.h"
#include <QDebug>
#include "commonfun.h"
#include "plotprocess.h"

defectdetector::defectdetector(QObject *parent) : QObject(parent)
{
    m_numChannels = 0;
}

QVector<DefectCandidate> defectdetector::processChannelBlock(int channel, const QVector<dataPoint> &blockData, double a_mm, qint64 blockStartIdx, double noiseThresh, double minPeakToPeak, int minDistPoints)
{
    Q_UNUSED(noiseThresh)
    QVector<DefectCandidate> candidates;
    int n = blockData.size();
    if (n < 3) return candidates;

    // 提取y值
    QVector<double> y(n);
    for (int i = 0; i < n; ++i) y[i] = blockData[i].y;

    // 使用新函数获取极值点
    auto extremaPoints = findExtremaWithPlateaus(y);
    // 分离极大值和极小值索引
    QVector<int> maxIndices, minIndices;
    for (const auto& ep : extremaPoints) {
        if (ep.isMax) maxIndices.append(ep.index);
        else minIndices.append(ep.index);
    }

    // 合并并排序
    QVector<QPair<int, bool>> extrema; // index, isMax
    for (int idx : maxIndices) extrema.append({idx, true});
    for (int idx : minIndices) extrema.append({idx, false});
    std::sort(extrema.begin(), extrema.end(),
              [](const QPair<int,bool>& a, const QPair<int,bool>& b) {
                  return a.first < b.first;
              });

    // 配对
    for (int i = 0; i < extrema.size()-1; ++i) {
        int idx1 = extrema[i].first;
        int idx2 = extrema[i+1].first;
        bool type1 = extrema[i].second;
        bool type2 = extrema[i+1].second;
        if (type1 != type2) {
            double pp = qAbs(y[idx1] - y[idx2]);
            if (pp < minPeakToPeak) continue;

            int distPoints = qAbs(idx2 - idx1);
            if (distPoints <= minDistPoints) continue; // 距离太近忽略

            // 转换为全局索引
            qint64 startIdx = blockStartIdx + qMin(idx1, idx2);
            qint64 endIdx = blockStartIdx + qMax(idx1, idx2);
            double start_mm = startIdx * a_mm;
            double end_mm = endIdx * a_mm;

            DefectCandidate cand;
            cand.channel = channel;
            cand.start_mm = start_mm;
            cand.end_mm = end_mm;
            cand.peakToPeak = pp;
            candidates.append(cand);
        }
    }
    return candidates;
}

QVector<ExtremaPoint> defectdetector::findExtremaWithPlateaus(const QVector<double> &y)
{
    QVector<ExtremaPoint> extrema;
    int n = y.size();
    if (n < 3) return extrema;

    int i = 0;
    while (i < n) {
        int start = i;
        double val = y[i];
        // 找到连续相等值的平台
        while (i+1 < n && qFuzzyCompare(y[i+1], val)) {
            ++i;
        }
        int end = i; // 平台结束索引（包含）
        int mid = (start + end) / 2; // 平台中心点

        // 获取左边邻居值（如果存在）
        double leftVal = (start > 0) ? y[start-1] : val; // 边界情况：若平台在开头，则没有左邻居，不能作为极值
        // 获取右边邻居值（如果存在）
        double rightVal = (end+1 < n) ? y[end+1] : val;

        // 判断是否为极值平台
        bool isPeak = false;
        bool isValley = false;
        if (start > 0 && end+1 < n) {
            if (val > leftVal && val > rightVal) {
                isPeak = true;
            } else if (val < leftVal && val < rightVal) {
                isValley = true;
            }
        }
        // 对于边界平台（开头或结尾），通常不作为极值，除非特殊情况（但信号端点一般不作为极值）
        // 如果平台长度==1，且满足普通极值条件，也记录
        if (start == end) {
            // 单点，原条件也可，但用新逻辑更统一
            if (start > 0 && end+1 < n) {
                if (y[start] > y[start-1] && y[start] > y[start+1]) isPeak = true;
                else if (y[start] < y[start-1] && y[start] < y[start+1]) isValley = true;
            }
        }

        if (isPeak) {
            extrema.append({mid, true});
        } else if (isValley) {
            extrema.append({mid, false});
        }

        // 移动到下一个不同值的点
        ++i;
    }
    return extrema;
}

QVector<DefectCandidate> defectdetector::processFullBlock(const QVector<QVector<dataPoint> > &blockData, double a_mm, qint64 blockStartIdx, double noiseThresh, double minPeakToPeak, int minDistPoints)
{
    QVector<DefectCandidate> allCandidates;
    m_numChannels = blockData.size(); // 模拟值为256(不定，根据实际盒子数确定，此处定值是算法初步模拟)
    for (int ch = 0; ch < m_numChannels; ++ch) {
        QVector<DefectCandidate> chCands = processChannelBlock(
            ch, blockData[ch], a_mm, blockStartIdx,
            noiseThresh, minPeakToPeak, minDistPoints);
        allCandidates.append(chCands);
    }
    return allCandidates;
}

bool defectdetector::isSameEvent(const DefectCandidate &a, const DefectCandidate &b)
{
    // 轴向重叠或间隙判断
    double a_start = a.start_mm, a_end = a.end_mm;
    double b_start = b.start_mm, b_end = b.end_mm;
    bool axialClose = (a_start <= b_end + AXIAL_GAP_MM) && (b_start <= a_end + AXIAL_GAP_MM);
    if (!axialClose) return false;
    // 通道连续（允许间隔1）
    int ch_gap = qAbs(a.channel - b.channel);
    return ch_gap <= 1;
}

void defectdetector::addToEvent(DefectEvent &event, const DefectCandidate &cand)
{
    // 避免重复添加相同的候选（极值对）
    for (const auto& existing : event.candidates) {
        if (existing == cand) return;
    }
    event.candidates.append(cand);
    event.axialStart_mm = qMin(event.axialStart_mm, cand.start_mm);
    event.axialEnd_mm   = qMax(event.axialEnd_mm, cand.end_mm);
    event.minChannel = qMin(event.minChannel, cand.channel);
    event.maxChannel = qMax(event.maxChannel, cand.channel);
    if (!event.channels.contains(cand.channel))
        event.channels.append(cand.channel);
}

QVector<DefectEvent> defectdetector::clusterCandidates(QVector<DefectCandidate> &candidates)
{
    // 1. 去重（完全相同的候选只保留一个）
    std::sort(candidates.begin(), candidates.end(),
              [](const DefectCandidate& a, const DefectCandidate& b) {
                  if (a.channel != b.channel) return a.channel < b.channel;
                  if (!qFuzzyCompare(a.start_mm,b.start_mm)) return a.start_mm < b.start_mm;
                  return a.end_mm < b.end_mm;
              });
    auto last = std::unique(candidates.begin(), candidates.end(),
                            [](const DefectCandidate& a, const DefectCandidate& b) {
                                return a.channel == b.channel &&
                                       qFuzzyCompare(a.start_mm, b.start_mm) &&
                                       qFuzzyCompare(a.end_mm, b.end_mm);
                            });
    candidates.erase(last, candidates.end());

    // 2. 按起始距离排序
    std::sort(candidates.begin(), candidates.end(),
              [](const DefectCandidate& a, const DefectCandidate& b) {
                  return a.start_mm < b.start_mm;
              });

    // 3. 聚类
    QVector<DefectEvent> events;
    for (const DefectCandidate& cand : candidates) {
        bool added = false;
        for (DefectEvent& ev : events) {
            // 检查是否与事件中任一候选属于同一缺陷
            bool canJoin = false;
            for (const DefectCandidate& ec : ev.candidates) {
                if (isSameEvent(ec, cand)) {
                    canJoin = true;
                    break;
                }
            }
            if (canJoin) {
                addToEvent(ev, cand);
                added = true;
                break;
            }
        }
        if (!added) {
            DefectEvent newEv;
            addToEvent(newEv, cand);
            events.append(newEv);
        }
    }

    // 4. 计算每个事件的轴向长度
    for (DefectEvent& ev : events) {
        ev.axialLength_mm = ev.axialEnd_mm - ev.axialStart_mm;
        // 可对 channels 排序去重（已在addToEvent中处理）
    }
    return events;
}

DefectType defectdetector::classifyEvent(const DefectEvent &event, double pitch_mm)
{
    int N = event.maxChannel - event.minChannel + 1;
    double L = event.axialLength_mm;

    // 1. 环焊缝：覆盖几乎所有通道，轴向很窄
    if (N >= m_numChannels-10 && L < 50.0) {
        return GirthWeld;
    }

    // 计算各通道的起始/结束位置及区间长度
    double startMin = 1e9, startMax = -1e9;
    double endMin = 1e9, endMax = -1e9;
    double totalLength = 0.0;
    QVector<double> lengths;

    for (const DefectCandidate& cand : event.candidates) {
        startMin = qMin(startMin, cand.start_mm);
        startMax = qMax(startMax, cand.start_mm);
        endMin   = qMin(endMin, cand.end_mm);
        endMax   = qMax(endMax, cand.end_mm);
        double len = cand.end_mm - cand.start_mm;
        totalLength += len;
        lengths.append(len);
    }

    double startSpan = startMax - startMin;
    double endSpan   = endMax - endMin;

    // 2. 补板：起始和结束非常一致，且通道数较多
    if (startSpan < 10.0 && endSpan < 10.0 && N > 20) {
        return Patch;
    }

    // 计算填充率
    double fillRatio = totalLength / (L * N);

//    // 计算区间长度变异系数（可选）
//    double meanLen = totalLength / lengths.size();
//    double var = 0.0;
//    for (double len : lengths) {
//        var += (len - meanLen) * (len - meanLen);
//    }
//    double cv = qSqrt(var / lengths.size()) / meanLen; // 变异系数

    // 3. 接管：需要同时满足长宽比接近1、通道数不多、对齐性好、填充率适中
    double circWidth = N * pitch_mm;
    double ratio = L / circWidth; //纵向半径除以横向半径
    if (ratio > 0.5 && ratio < 2.0) {
        // 接管特征：对齐性好（起始/结束跨度小），填充率接近圆形理论值
        if (abs(startSpan-endSpan)<10.0 && fillRatio>0.6 && fillRatio<0.9) {
            return Nozzle;
        }
    }

    // 4. 其它不规则形状归为普通金属损失
    return OrdinaryLoss;
}

void defectdetector::detectDefectsFromBlocks(double a_mm, double innerDiameter, projectConfigure *CprjConfig)
{
    //多读少取，例如读8000帧数据，取位于前4000帧位置的数据；再读2000-10000位置，取4000-8000位置；再读6000-14000，取8000-12000；以此类推
    //根据中心点位置判断取值
    //当最后范围不足八千时一次性操作，取上一次所选截至位置到最后
    //每读8000帧数据，将所取4000帧内的缺陷存库
    //左闭右开

    QString qsfilePath="";
    qint64 startPos=0,offset=8000;
    QVector<DefectCandidate>blockCands;
    QVector<DefectEvent> resultBlockEvents;
    double pitch_mm = 0.0;
    while(true)
    {
        readBlockData(qsfilePath,startPos,offset,CprjConfig);
        if(m_vecPoint2D.isEmpty())
            break;  //无更多数据,缺陷分析结束
        //纵向通道总数
        int channelsNumber = m_vecPoint2D.size();
        //通道间距（毫米）
        pitch_mm = innerDiameter/channelsNumber;
        //处理当前模块
        blockCands = processFullBlock(m_vecPoint2D,a_mm,startPos);
        //聚类块内候选
        QVector<DefectEvent> blockEvents = clusterCandidates(blockCands);
        //按中心点位置筛选输出部分块内结果
        double xCenter4000 = 4000*a_mm;
        double xCenterLow = (startPos+2000)*a_mm;
        double xCenterHigh = (startPos+offset-2000)*a_mm;
        for (const DefectEvent& event:blockEvents) {
            //X轴中心：轴向起始与结束的中点
            double xCenter = (event.axialStart_mm+event.axialEnd_mm)/2.0;
            if(!event.channels.isEmpty())
            {
                if(0 == startPos)
                {
                    if(xCenter>=0 && xCenter<xCenter4000)
                        resultBlockEvents.append(event);
                }
                else {
                    if(xCenter>=xCenterLow && xCenter<xCenterHigh)
                        resultBlockEvents.append(event);
                }
            }
        }
        //存数据库
        //打印验证
        //若resultBlockEvents>1000存库，清除resultBlockEvents；下次循环如是操作
        if(resultBlockEvents.size()>1000)
        {
            for(DefectEvent& ev : resultBlockEvents)
            {
                DefectType type = classifyEvent(ev, pitch_mm);
                QString typeStr;
                switch (type) {
                case OrdinaryLoss: typeStr = "普通金属损失"; break;
                case Nozzle:       typeStr = "接管"; break;
                case Patch:        typeStr = "补板"; break;
                case GirthWeld:    typeStr = "环焊缝"; break;
                }
                qDebug() << "缺陷事件: 通道" << ev.minChannel << "-" << ev.maxChannel
                         << ", 轴向" << ev.axialStart_mm << "~" << ev.axialEnd_mm << "mm"
                         << ", 类型:" << typeStr;
            }
            QVector<DefectEvent>().swap(resultBlockEvents);
        }

        //修改参数以便下次循环
        if(0 == startPos)
        {
            startPos = 2000;
        }
        else {
            startPos += 4000;
        }
    }
    //退出函数前，若resultBlockEvents>0,调用存库函数
    if(resultBlockEvents.size()>0)
    {
        for(DefectEvent& ev : resultBlockEvents)
        {
            DefectType type = classifyEvent(ev, pitch_mm);
            QString typeStr;
            switch (type) {
            case OrdinaryLoss: typeStr = "普通金属损失"; break;
            case Nozzle:       typeStr = "接管"; break;
            case Patch:        typeStr = "补板"; break;
            case GirthWeld:    typeStr = "环焊缝"; break;
            }
            qDebug() << "缺陷事件: 通道" << ev.minChannel << "-" << ev.maxChannel
                     << ", 轴向" << ev.axialStart_mm << "~" << ev.axialEnd_mm << "mm"
                     << ", 类型:" << typeStr;
        }
        QVector<DefectEvent>().swap(resultBlockEvents);
    }
    //缺陷分析结束通知窗口主线程
    emit detectDefectComplete();
}

int defectdetector::readDataFromFile(QString qsfilePath, QVector<QVector<dataPoint> > &vecPoint2D, qint64 startPos, qint64 offset,qint64& differ,int axis)
{
    QFile file(qsfilePath);
    if(file.open(QIODevice::ReadOnly)){
        qint64 bufferSize = 0;
        //offset
        if(offset >= 1 && offset < 1024*4)
        {
            bufferSize = 268*offset;
        }
        else if(offset >= 1024*4)
        {
            bufferSize = 268*1024*4;
        }
        else {
            bufferSize = 0;
        }

        const qint64 startBytes = 268*startPos+20;
        differ = 0;
        QByteArray buffer;
        qint32 currentBytes = 0;//Qfile.read读出字节数
        file.seek(startBytes);
        buffer = file.read(bufferSize);
        currentBytes = buffer.size();
        differ=offset-currentBytes/268;
        if(0 == currentBytes)
        {
            qDebug()<<"本文件读完，转至下一文件";
            return -2; //本文件读完，前往同级目录下一个文件读取
        }
        //内部结构
        //每268个字节
        if(1 == axis)
        {
            for(int i=0;i<currentBytes;i=i+268){
                //264个字节中探头一到探头六按序排列
                for(int j=0;j<6;j++)
                {
                    for(int m=0;m<6;m++)
                    {
                        dataPoint onePoint;
                        //hollX轴分析
                        int k=i+j*44;
                        int o=k++;
                        k=k+6*m;o=o+6*m;
                        onePoint.x = i/268;
                        onePoint.rawY=getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                        onePoint.y=static_cast<double>(plotProcess::getInstance()->convert2float(onePoint.rawY,1,5,12,1,0));
                        if(vecPoint2D.size()==36)
                        {
                            vecPoint2D[6*j+m].append(onePoint);
                        }
                        else
                        {
                            QVector<dataPoint>oneVec;
                            oneVec.append(onePoint);
                            vecPoint2D.append(oneVec);
                        }
                    }
                }
            }
        }
        file.close();
        qDebug() << "缺陷分析时Total bytes read:" << currentBytes;
        return 0;
    }
    else {
        qDebug()<<"打开文件失败";
        return -1;
    }
}

int defectdetector::readBlockData(QString &qsfilePath, qint64 startPos, qint64 offset, projectConfigure *CprjConfig)
{
    QVector<QVector<dataPoint>>().swap(m_vecPoint2D);
    qint64 initialStartPos = startPos;
    qint64 initialOffset = offset;
    //按盒子遍历（）
    for(int k=0;k<CprjConfig->boxDirPath.size();k++)
    {
        QString curFileName = "";
        qint64 differ = 0;
        QVector<QVector<dataPoint>>oneBoxPoint2D;
        QVector<QString>&fileNameVec = CprjConfig->fileNameVecByBox[k];
        QMap<QString,int>&fileNameBytesMap = CprjConfig->fileNameBytesMapByBox[k];
        int fileNumber = fileNameVec.size();

        //确定开始帧所在文件和内部位置
        startPos = initialStartPos;
        int i=0;
        //startPos==0
        if(0 == startPos)
        {
            curFileName = fileNameVec[0];
            qsfilePath = CprjConfig->boxDirPath[k]+curFileName;
        }
        //startPos!=0
        while(startPos>0)
        {
            if(i<fileNumber)
            {
                QString thisFileName = fileNameVec[i];
                int thisFileFrame = fileNameBytesMap[thisFileName];
                startPos = startPos - thisFileFrame;
                i++;
            }
            else{
                break;
            }
        }
        if(i>0 && i<=fileNumber)
        {
            if(startPos>0)
            {
                //目录下文件已读完(外层循环最后一轮遍历)
                m_vecPoint2D.append(oneBoxPoint2D);
                if(CprjConfig->boxDirPath.size()-1 == k)
                {
                    //emit发出m_vecPoint2D，完成读取
                    return 0;
                }
                else
                {
                    continue;
                }
            }
            if(0==startPos)
            {
                //从当前文件头开始读取
                curFileName = fileNameVec[i];
                qsfilePath = CprjConfig->boxDirPath[k] + curFileName;
                if(i == fileNumber)
                {
                    m_vecPoint2D.append(oneBoxPoint2D);
                    if(CprjConfig->boxDirPath.size()-1 == k)
                    {
                        //emit发出m_vecPoint2D，完成读取
                        return 0;
                    }
                    else {
                        continue;
                    }
                }
            }
            //startPos < 0
            else
            {
                //从前一个文件开始读取
                curFileName = fileNameVec[--i];
                qsfilePath = CprjConfig->boxDirPath[k] + curFileName;
                int thisFileFrame = fileNameBytesMap[curFileName];
                startPos = thisFileFrame + startPos;
            }
        }

        offset = initialOffset;
        do{
            int iRet = readDataFromFile(qsfilePath,oneBoxPoint2D,startPos,offset,differ,1);
            startPos = startPos+offset-differ;
            offset = differ;

            if(0 == iRet) //正常，继续循环
            {

            }
            else if(-1 == iRet) //打开文件失败
            {
                break;
            }
            else //返回值-2，读取下一个文件
            {
                //向后查找下一个文件
                int fileIndex = fileNameVec.indexOf(curFileName);
                int filesNum = fileNameVec.size();
                if(fileIndex >= 0 && fileIndex < filesNum-1)
                {
                    fileIndex++;
                    QString nextFilename = fileNameVec[fileIndex];
                    curFileName = nextFilename;
                    qsfilePath = CprjConfig->boxDirPath[k] + curFileName;
                    startPos = 0;
                }
                else {
                    qDebug()<<"缺陷分析线程---当前文件夹下的数据已全部读出";
                    break;
                }
            }
        }while(differ != 0);
        m_vecPoint2D.append(oneBoxPoint2D);
    }
    //emit发出m_vecPoint2D，完成读取
    return 0;
}

void defectdetector::handleStartDetectDefects(double a_mm, double innerDiameter, projectConfigure CprjConfig)
{
    qDebug()<<"defectdetector实际工作线程为:"<<QThread::currentThreadId();
    detectDefectsFromBlocks(a_mm,innerDiameter,&CprjConfig);
}
