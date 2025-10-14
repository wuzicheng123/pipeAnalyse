#ifndef MYCUSTOMPLOT_H
#define MYCUSTOMPLOT_H
#include "qcustomplot.h"


class MyCustomPlot : public QCustomPlot
{
public:
    MyCustomPlot(QWidget *parent = nullptr);

private:
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
};

#endif // MYCUSTOMPLOT_H
