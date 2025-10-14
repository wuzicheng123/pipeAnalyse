#ifndef MYCUSTOMPLOT_H
#define MYCUSTOMPLOT_H
#include "qcustomplot.h"


class MyCustomPlot : public QCustomPlot
{
    Q_OBJECT
public:
    MyCustomPlot(QWidget *parent = nullptr);

private:
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

signals:
    void sig_wheelEvent();
};

#endif // MYCUSTOMPLOT_H
