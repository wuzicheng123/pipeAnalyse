#include "mycustomplot.h"

MyCustomPlot::MyCustomPlot(QWidget *parent):
    QCustomPlot(parent)
{
    m_timer = nullptr;
}

void MyCustomPlot::mouseReleaseEvent(QMouseEvent *event)
{
    QCustomPlot::mouseReleaseEvent(event);
    qDebug()<<"11111";
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

void MyCustomPlot::handleTimeout()
{
    emit sig_wheelEvent();
    qDebug()<<"22222";
}
