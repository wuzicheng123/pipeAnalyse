#include "mycustomplot.h"
#include "plotprocess.h"

MyCustomPlot::MyCustomPlot(QWidget *parent):
    QCustomPlot(parent)
{
    m_timer = nullptr;
    m_dragTimer = nullptr;
    m_dragging = false;
    m_initial = false;
    m_leftPress = false;
    QVector<TracerInfo>().swap(m_tracers);
    m_currentIndex = 0;
}

MyCustomPlot::~MyCustomPlot()
{
    if(nullptr != m_timer)
    {
        delete m_timer;
        m_timer = nullptr;
    }
    if(nullptr != m_dragTimer)
    {
        delete m_dragTimer;
        m_dragTimer = nullptr;
    }
    m_dragging = false;
    m_initial = false;
    m_leftPress = false;
}

void MyCustomPlot::mouseReleaseEvent(QMouseEvent *event)
{
    QCustomPlot::mouseReleaseEvent(event);
    if(Qt::RightButton == event->button())
    {
        m_dragging = false;
        if(nullptr != m_dragTimer)
        {
            if(m_dragTimer->isActive())
            {
                m_dragTimer->stop();
                handleTimeoutBydrag();
            }
        }
    }
    else if(Qt::LeftButton == event->button())
    {
        if(m_leftPress)
        {
            int iSize = m_tracers.size();
            for(int i=0;i<iSize;i++)
            {
                if(true == m_tracers[i].isActive)
                {
                    m_tracers[i].isActive = false;
                    m_tracers[i].tracer->setVisible(m_tracers[i].isActive);
                    m_tracers[i].label->setVisible(m_tracers[i].isActive);
                }
            }
            m_leftPress = false;
        }
        this->replot();
    }
}

void MyCustomPlot::wheelEvent(QWheelEvent *event)
{
    QCustomPlot::wheelEvent(event);
    if(nullptr == m_timer)
    {
        m_timer = new QTimer;
        connect(m_timer,&QTimer::timeout,this,&MyCustomPlot::handleTimeout);
        m_timer->setSingleShot(true);
        m_timer->start(500);
    }
    else {
        if(m_timer->isActive())
        {}
        else {
            m_timer->start(500);
        }
    }
}

void MyCustomPlot::mouseMoveEvent(QMouseEvent *event)
{
    QCustomPlot::mouseMoveEvent(event);
    if(m_dragging)
    {
        if(nullptr == m_dragTimer)
        {
            m_dragTimer = new QTimer;
            connect(m_dragTimer,&QTimer::timeout,this,&MyCustomPlot::handleTimeoutBydrag);
            m_dragTimer->setSingleShot(true);
            m_dragTimer->start(500);
        }
        else {
            if(m_dragTimer->isActive())
            {}
            else {
                m_dragTimer->start(500);
            }
        }
    }

    if(m_leftPress)
    {
        if(m_tracers.size() > m_currentIndex)
        {
            TracerInfo& oneTracer = m_tracers[m_currentIndex];
            double x_data = this->xAxis->pixelToCoord(event->pos().x());
            oneTracer.tracer->setGraphKey(x_data);
            double tracerY = oneTracer.tracer->position->value();

            //标牌显示内容
            QString info = "";
            int indexInbox = m_currentIndex%36;
            int boxNum = m_currentIndex/36+1;
            int probeNum = (indexInbox)/6+1;
            int sensorNum = indexInbox%6+1;
            //利用图表标题记录是那一类型数据
            double magnetic = 0.0;
            magnetic = tracerY - (boxNum-1)*36 - (probeNum-1)*6 - (sensorNum-1);
            if("霍尔X轴" == m_titleText)
            {
                magnetic = magnetic*plotProcess::getInstance()->hallUpperLimitXY/50;
            }
            else if("霍尔Y轴" == m_titleText)
            {
                magnetic = magnetic*plotProcess::getInstance()->hallUpperLimitXY/50;
            }
            else if("霍尔Z轴" == m_titleText)
            {
                magnetic = magnetic*plotProcess::getInstance()->hallUpperLimitZ/50;
            }
            else if("涡流" == m_titleText)
            {}
            //刷新标牌显示
            info = QString("传感器盒:%1 探头:%2 传感器:%3 \n%4 位置:%5 磁场强度:%6").arg(boxNum).arg(probeNum)
                    .arg(sensorNum).arg(m_titleText).arg(x_data).arg(magnetic);
            oneTracer.label->setText(info);

            this->replot();
        }
    }
}

void MyCustomPlot::mousePressEvent(QMouseEvent *event)
{
    QCustomPlot::mousePressEvent(event);
    if(Qt::RightButton == event->button())
    {
        m_dragging = true;
        m_prePoint.setX(QCursor::pos().rx());
        m_prePoint.setY(QCursor::pos().ry());
    }
    else if(Qt::LeftButton == event->button())
    {
        //初始化创建追踪器或追踪器数量少于图像数量
        int graphSize = this->graphCount();
        if(!m_initial || graphSize > m_tracers.size())
        {
            m_initial = true;
            for(int i=0;i<graphSize;i++)
            {
                if(i<m_tracers.size())
                {
                    TracerInfo& oneTracer = m_tracers[i];
                    QCPGraph* graph = this->graph(i);
                    if(graph && !graph->data()->isEmpty())
                    {
                        oneTracer.tracer->setGraph(graph);
                        oneTracer.label->position->setParentAnchor(oneTracer.tracer->position);
                    }
                }
                else {
                    TracerInfo oneTracer;
                    QCPGraph* graph = this->graph(i);
                    if(graph && !graph->data()->isEmpty())
                    {
                        oneTracer.isActive = false;
                        oneTracer.tracer = new QCPItemTracer(this);
                        oneTracer.tracer->setGraph(graph);
                        oneTracer.tracer->setInterpolating(false);
                        oneTracer.tracer->setStyle(QCPItemTracer::tsCrosshair);
                        oneTracer.tracer->setSize(6);
                        oneTracer.tracer->setVisible(oneTracer.isActive);

                        oneTracer.label = new QCPItemText(this);
                        oneTracer.label->setPositionAlignment(Qt::AlignLeft|Qt::AlignBottom);
                        oneTracer.label->position->setParentAnchor(oneTracer.tracer->position);
                        oneTracer.label->position->setCoords(10,-5);
                        oneTracer.label->setText(QString("Point %1").arg(i));
                        oneTracer.label->setTextAlignment(Qt::AlignLeft);
                        oneTracer.label->setFont(QFont(font().family(), 9));
                        oneTracer.label->setPen(QPen(Qt::black));
                        oneTracer.label->setBrush(QBrush(QColor(255,255,255,200)));
                        oneTracer.label->setPadding(QMargins(3, 1, 3, 1));
                        oneTracer.label->setVisible(oneTracer.isActive);
                    }
                    m_tracers.append(oneTracer);
                }
            }
        }
        //已初始化后更新追踪器（翻页或拖拽时）
        else if(m_initial && graphSize <= m_tracers.size())
        {
            for(int i=0;i<graphSize;i++)
            {
                TracerInfo& oneTracer = m_tracers[i];
                QCPGraph* graph = this->graph(i);
                if(graph && !graph->data()->isEmpty())
                {
                    oneTracer.tracer->setGraph(graph);
                    oneTracer.label->position->setParentAnchor(oneTracer.tracer->position);
                }
            }
        }

        if(false == m_leftPress)
        {
            m_leftPress = true;
            //找寻与鼠标位置最近的数据点
            //鼠标位置
            QPoint pos = event->pos();
            double x = xAxis->pixelToCoord(pos.x());
            double y = yAxis->pixelToCoord(pos.y());
            int selectIndex = 0;
            double minDistance = std::numeric_limits<double>::max();

            for(int i=0;i<graphSize;i++)
            {
                TracerInfo& oneTracer = m_tracers[i];
                oneTracer.tracer->setGraphKey(x);
                double tracerY = oneTracer.tracer->position->value();
                double distance = qAbs(tracerY - y);
                if(distance < minDistance)
                {
                    minDistance = distance;
                    selectIndex = i;
                }
            }

            //利用图表标题记录是那一类型数据
            QCPTextElement* titleElement = dynamic_cast<QCPTextElement*>(this->plotLayout()->element(0,0));
            m_titleText = "";
            if(titleElement)
            {
                m_titleText = titleElement->text();
            }

            //激活标牌显示
            TracerInfo& oneTracer = m_tracers[selectIndex];
            m_currentIndex = selectIndex;
            oneTracer.isActive = true;
            oneTracer.tracer->setVisible(oneTracer.isActive);
            oneTracer.label->setVisible(oneTracer.isActive);

            double tracerY = oneTracer.tracer->position->value();
            //标牌显示内容
            QString info = "";
            int indexInbox = m_currentIndex%36;
            int boxNum = m_currentIndex/36+1;
            int probeNum = (indexInbox)/6+1;
            int sensorNum = indexInbox%6+1;
            //利用图表标题记录是那一类型数据
            double magnetic = 0.0;
            magnetic = tracerY - (boxNum-1)*36 - (probeNum-1)*6 - (sensorNum-1);
            if("霍尔X轴" == m_titleText)
            {
                magnetic = magnetic*plotProcess::getInstance()->hallUpperLimitXY/50;
            }
            else if("霍尔Y轴" == m_titleText)
            {
                magnetic = magnetic*plotProcess::getInstance()->hallUpperLimitXY/50;
            }
            else if("霍尔Z轴" == m_titleText)
            {
                magnetic = magnetic*plotProcess::getInstance()->hallUpperLimitZ/50;
            }
            else if("涡流" == m_titleText)
            {}
            //刷新标牌显示
            info = QString("传感器盒:%1 探头:%2 传感器:%3 \n%4 位置:%5 磁场强度:%6").arg(boxNum).arg(probeNum)
                    .arg(sensorNum).arg(m_titleText).arg(x).arg(magnetic);
            oneTracer.label->setText(info);

            this->replot();
        }
    }
}

void MyCustomPlot::handleTimeout()
{
    qDebug()<<"x轴最小值"<<ceil(this->xAxis->range().lower);
    qDebug()<<"x轴最大值"<<floor(this->xAxis->range().upper);
    qint64 xLower = static_cast<qint64>(ceil(this->xAxis->range().lower));
    qint64 xUpper = static_cast<qint64>(floor(this->xAxis->range().upper));
    qint64 yLower = static_cast<qint64>(ceil(this->yAxis->range().lower));
    qint64 yUpper = static_cast<qint64>(floor(this->yAxis->range().upper));

    emit sig_wheelEvent(xLower,xUpper,yLower,yUpper);
}

void MyCustomPlot::handleTimeoutBydrag()
{
    int curRx = QCursor::pos().rx();
    int curRy = QCursor::pos().ry();

    //偏移像素
    int offsethor = curRx - m_prePoint.x(); //水平偏移（小于0右移）
    int offsetver = curRy - m_prePoint.y(); //垂直偏移 （大于0上移）
    //窗口像素
    int iwidth = this->size().width();
    int iheight = this->size().height();
    //窗口原坐标
    qint64 xLower = static_cast<qint64>(ceil(this->xAxis->range().lower));
    qint64 xUpper = static_cast<qint64>(floor(this->xAxis->range().upper));
    qint64 yLower = static_cast<qint64>(ceil(this->yAxis->range().lower));
    qint64 yUpper = static_cast<qint64>(floor(this->yAxis->range().upper));
    //坐标偏移量
    qint64 xOffsetAxis = (xUpper-xLower)*offsethor/iwidth;
    qint64 yOffsetAxis = (yUpper-yLower)*offsetver/iheight;
    //小于1时依然可以拖动
    if(0 == yOffsetAxis)
    {
        if(offsetver<0)
            yOffsetAxis = -1;
        else
            yOffsetAxis = 1;
    }
    if(0 == xOffsetAxis)
    {
        if(offsethor<0)
            xOffsetAxis = -1;
        else
            xOffsetAxis = 1;
    }

    //偏移后窗口坐标
    xLower -= xOffsetAxis;
    xUpper -= xOffsetAxis;
    yLower += yOffsetAxis;
    yUpper += yOffsetAxis;

    m_prePoint.setX(curRx);
    m_prePoint.setY(curRy);

    emit sig_wheelEvent(xLower,xUpper,yLower,yUpper);
}
