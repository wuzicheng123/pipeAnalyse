#include "mycustomplot.h"

MyCustomPlot::MyCustomPlot(QWidget *parent):
    QCustomPlot(parent)
{
    m_timer = nullptr;
}

void MyCustomPlot::mouseReleaseEvent(QMouseEvent *event)
{
    QCustomPlot::mouseReleaseEvent(event);
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
        qDebug()<<event->button();

        qDebug()<<event->type();

        qDebug()<<event->pos().x();
        qDebug()<<event->pos().y();
}

void MyCustomPlot::mousePressEvent(QMouseEvent *event)
{

}

void MyCustomPlot::handleTimeout()
{
    emit sig_wheelEvent();
}

void MyCustomPlot::handleTimeoutBydrag()
{

}
