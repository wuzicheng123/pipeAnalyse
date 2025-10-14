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
