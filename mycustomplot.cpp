#include "mycustomplot.h"

MyCustomPlot::MyCustomPlot(QWidget *parent):
    QCustomPlot(parent)
{

}

void MyCustomPlot::mouseReleaseEvent(QMouseEvent *event)
{
    QCustomPlot::mouseReleaseEvent(event);
    qDebug()<<"11111";
}

void MyCustomPlot::wheelEvent(QWheelEvent *event)
{
    QCustomPlot::wheelEvent(event);
    emit sig_wheelEvent();
}
