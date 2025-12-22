#include "dataservice.h"
#include <QDebug>
#include <QElapsedTimer>
#include "commonfun.h"

dataService* dataService::m_instance = nullptr;
QMutex dataService::m_mutex;

dataService::dataService(QObject *parent) : QObject(parent)
{
    m_exMainW = nullptr;
}

dataService *dataService::getInstance()
{
    if(!m_instance)
    {
        QMutexLocker locker(&m_mutex);
        if(!m_instance){
            m_instance = new dataService();
        }
    }
    return m_instance;
}

int dataService::readDataFromBinAll(QString qsfilePath, QMap<int, QVector<dataModel> > &qmdataModel)
{
    QFile file(qsfilePath);
    if (file.open(QIODevice::ReadOnly)) {
        // 打开成功
        // 变量声明
        const qint64 bufferSize = 268 * 1024 * 4; // 804KB缓冲区
        QByteArray buffer;
        qint64 totalBytes = 0;  //除去头二十字节的字节数
        qint32 currentBytes = 0; //当前缓冲区字节数

        //0-19字节
        buffer = file.read(20);
        QString timeStr = buffer;
        qDebug() << "Time String:" << timeStr;

        // ...可按需解析前20字节...

        QElapsedTimer qtimer;
        int cnt = 0;

        //内部结构
        //每268个字节
        while (!file.atEnd()) {
            qtimer.start();
            buffer = file.read(bufferSize);
            currentBytes = buffer.size();
            totalBytes += currentBytes;
            for (int i = 0; i < currentBytes; i=i+268) {
                //264个字节中探头一到探头六按序排列
                for(int j = 0; j < 6; j++)
                {
                    // quint16 halfVal = (dataPtr[2*i] << 8) | dataPtr[2*i+1]; // 大端
                    dataModel oneData;
                    int k=i+j*44;
                    int o=k++;
                    oneData._Hall1XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall1YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall1ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall2XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall2YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall2ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall3XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall3YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall3ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall4XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall4YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall4ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall5XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall5YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall5ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall6XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall6YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Hall6ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Vortex1Channel = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Vortex3Channel = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Vortex4Channel = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    k=k+2;o=o+2;
                    oneData._Vortex6Channel = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                    oneData._Vortex2Channel = (oneData._Vortex1Channel+oneData._Vortex3Channel)/2;
                    oneData._Vortex5Channel = (oneData._Vortex4Channel+oneData._Vortex6Channel)/2;

                    // TODO: 存储到Qmap
                    int key = j+1;
                    QMap<int,QVector<dataModel>>::iterator iter = qmdataModel.find(key);
                    if(iter != qmdataModel.end())
                    {
                        iter.value().append(oneData);
                    }
                    else
                    {
                        QVector<dataModel> oneVec;
                        oneVec.append(oneData);
                        qmdataModel.insert(key,oneVec);
                    }
                    // qDebug() << "原先数据" << QString::number(halfVal,16) << "转换后short" << halfVal;
                }
            }
            // 注意：如有字节对齐或大小端问题需处理
            qDebug() << "第"<< ++cnt <<"次循环花费时间"<<qtimer.elapsed()<<"ms";
        }

        //绘图测试
        //emit 数据到桌面

        file.close();

        qDebug() << "Total bytes read:" << totalBytes;

//        return static_cast<int>(totalBytes); // 返回读取字节数或自定义结果
         return 0;

    }
    else
    {
        qDebug()<<"打开文件失败";
        return -1;
    }
}

int dataService::readDataFromBinByOffset(QString qsfilePath, QMap<int, QVector<dataModel> > &qmdataModel, qint64 startPos, qint64 offset, qint64 &differ)
{
    QFile file(qsfilePath);
    if (file.open(QIODevice::ReadOnly)) {
        // 打开成功
        // 变量声明
        //根据总帧数确定缓冲区
        qint64 bufferSize = 0; // 需要查询：offset对应的字节数
        if(1 == offset)
        {
            bufferSize = 268;
        }
        else if(offset > 1 && offset < 1024*4)
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
        const qint64 startBytes = 268 * startPos + 20;
        differ = 0;

        QByteArray buffer;
        qint32 currentBytes = 0; //Qfile.read读出字节数

        QElapsedTimer qtimer;

        qtimer.start();
        file.seek(startBytes);
        buffer = file.read(bufferSize);
        currentBytes = buffer.size();

        differ = offset - currentBytes/268;


        if(0 == currentBytes)
        {
            qDebug()<<"本文件读完，转至下一文件";
            return -2; //本文件读完，前往同级目录下一个文件读取
        }

        //内部结构
        //每268个字节
        for (int i = 0; i < currentBytes; i=i+268) {
            //264个字节中探头一到探头六按序排列
            for(int j = 0; j < 6; j++)
            {
                // quint16 halfVal = (dataPtr[2*i] << 8) | dataPtr[2*i+1]; // 大端
                dataModel oneData;
                int k=i+j*44;
                int o=k++;
                oneData._Hall1XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall1YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall1ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall2XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall2YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall2ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall3XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall3YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall3ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall4XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall4YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall4ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall5XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall5YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall5ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall6XAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall6YAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Hall6ZAxis = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Vortex1Channel = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Vortex3Channel = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Vortex4Channel = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                k=k+2;o=o+2;
                oneData._Vortex6Channel = getShortfromCharLittle(static_cast<unsigned char>(buffer[o]),static_cast<unsigned char>(buffer[k]));
                oneData._Vortex2Channel = (oneData._Vortex1Channel+oneData._Vortex3Channel)/2;
                oneData._Vortex5Channel = (oneData._Vortex4Channel+oneData._Vortex6Channel)/2;

                // TODO: 存储到Qmap
                int key = j+1;
                QMap<int,QVector<dataModel>>::iterator iter = qmdataModel.find(key);
                if(iter != qmdataModel.end())
                {
                    iter.value().append(oneData);
                }
                else
                {
                    QVector<dataModel> oneVec;
                    oneVec.append(oneData);
                    qmdataModel.insert(key,oneVec);
                }
            }
        }
        // 注意：如有字节对齐或大小端问题需处理
        qDebug() << "从bin中读取"<<offset<<"帧,花费时间"<<qtimer.elapsed()<<"ms";

        file.close();

        qDebug() << "Total bytes read:" << currentBytes;

//        return static_cast<int>(totalBytes); // 返回读取字节数或自定义结果
         return 0;

    }
    else
    {
        qDebug()<<"打开文件失败";
        return -1;
    }
}

void dataService::setMainWindow(MainWindow *exMainW)
{
    m_exMainW = exMainW;
}

void dataService::handleModelDataRequest(QString& qsfilePath, qint64 startPos, qint64 offset)
{
    m_dataRwLock.lockForWrite();
    //使用swap代替clear清除，可避免内存溢出问题
    QVector<QMap<int,QVector<dataModel>>>().swap(m_dataModelVec);
    //查询前一帧数据，用于预处理操作
    QVector<QMap<int,QVector<dataModel>>>oneMapVec;
    qint64 initialStartPos = startPos;
    qint64 initialOffset = offset;
    //按盒子遍历
    for(int k=0;k<m_exMainW->CprjConfig->boxDirPath.size();k++)
    {
        qint64 differ2Pre = 0;
        qint64 differ = 0;
        startPos = initialStartPos;
        //前一帧所在文件和所在位置
        QMap<int,QVector<dataModel>>oneMap;
        QMap<int,QVector<dataModel>>dataModelMap;
        QVector<QString>&fileNameVec = m_exMainW->CprjConfig->fileNameVecByBox[k];
        QMap<QString,int>&fileNameBytesMap = m_exMainW->CprjConfig->fileNameBytesMapByBox[k];
        int fileNumber = fileNameVec.size();
        int i = 0;

        while (startPos > 0) {
            if(i < fileNumber)
            {
                QString thisFileName = fileNameVec[i];
                int thisFileFrame = fileNameBytesMap[thisFileName];
                startPos = startPos - thisFileFrame;
                i++;
            }
            else {
                break;
            }
        }
        if(i>0 && i<=fileNumber && startPos<=0)
        {
            m_exMainW->CprjConfig->curFileNamevec[k] = fileNameVec[--i];
            qsfilePath = m_exMainW->CprjConfig->boxDirPath[k] + m_exMainW->CprjConfig->curFileNamevec[k];
            int thisFileFrame = fileNameBytesMap[m_exMainW->CprjConfig->curFileNamevec[k]];
            startPos = thisFileFrame + startPos - 1; //前一帧位置
            readDataFromBinByOffset(qsfilePath,oneMap,startPos,1,differ2Pre);
        }
        oneMapVec.append(oneMap);

        //开始帧所在文件和内部位置
        startPos = initialStartPos;
        i = 0;
        //startPos == 0
        if(0 == startPos)
        {
            m_exMainW->CprjConfig->curFileNamevec[k] = fileNameVec[0];
            qsfilePath = m_exMainW->CprjConfig->boxDirPath[k] + m_exMainW->CprjConfig->curFileNamevec[k];
        }
        //startPos != 0
        while (startPos > 0) {
            if(i < fileNumber)
            {
                QString thisFileName = fileNameVec[i];
                int thisFileFrame = fileNameBytesMap[thisFileName];
                startPos = startPos - thisFileFrame;
                i++;
            }
            else {
                break;
            }
        }
        if(i>0 && i<=fileNumber)
        {
            if(startPos > 0)
            {
                //目录下文件已读完(外层循环最后一轮遍历)
                m_dataModelVec.append(dataModelMap);
                if(m_exMainW->CprjConfig->boxDirPath.size()-1 == k)
                {
                    m_dataRwLock.unlock();
                    //emit 发送读取到的数据m_dataModelVec
                    emit dataModel2PlotProcessBybox(m_dataModelVec,oneMapVec,initialStartPos);
                    return;
                }
                else
                {
                    continue;
                }
            }
            if(0 == startPos)
            {
                //从当前文件头开始读取
                m_exMainW->CprjConfig->curFileNamevec[k] = fileNameVec[i];
                qsfilePath = m_exMainW->CprjConfig->boxDirPath[k] + m_exMainW->CprjConfig->curFileNamevec[k];
                if(i == fileNumber)
                {
                    m_dataModelVec.append(dataModelMap);
                    if(m_exMainW->CprjConfig->boxDirPath.size()-1 == k)
                    {
                        m_dataRwLock.unlock();
                        //emit 发送读取到的数据m_dataModelVec
                        emit dataModel2PlotProcessBybox(m_dataModelVec,oneMapVec,initialStartPos);
                        return;
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
                m_exMainW->CprjConfig->curFileNamevec[k] = fileNameVec[--i];
                qsfilePath = m_exMainW->CprjConfig->boxDirPath[k] + m_exMainW->CprjConfig->curFileNamevec[k];
                int thisFileFrame = fileNameBytesMap[m_exMainW->CprjConfig->curFileNamevec[k]];
                startPos = thisFileFrame + startPos;
            }
        }

        offset = initialOffset;
        do{
            int iRet = readDataFromBinByOffset(qsfilePath,dataModelMap,startPos,offset,differ);
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
                if(nullptr != m_exMainW)
                {
                    //向后查找下一个文件
                    int fileIndex = fileNameVec.indexOf(m_exMainW->CprjConfig->curFileNamevec[k]);
                    int filesNum = fileNameVec.size();
                    if(fileIndex >= 0 && fileIndex < filesNum-1)
                    {
                        fileIndex++;
                        QString nextFilename = fileNameVec[fileIndex];
                        m_exMainW->CprjConfig->curFileNamevec[k] = nextFilename;
                        qsfilePath = m_exMainW->CprjConfig->boxDirPath[k] + m_exMainW->CprjConfig->curFileNamevec[k];
                        startPos = 0;
                    }
                    else {
                        qDebug()<<"当前文件夹下的数据已全部读出";
                        break;
                    }
                }
                else {
                    qDebug()<<"dataService中获取主窗体指针失败";
                    break;
                }
            }
        }while(differ != 0);
        m_dataModelVec.append(dataModelMap);
    }
    m_dataRwLock.unlock();
    //emit 发送读取到的数据m_dataModelVec
    emit dataModel2PlotProcessBybox(m_dataModelVec,oneMapVec,initialStartPos);
}
