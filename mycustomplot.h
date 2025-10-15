#ifndef MYCUSTOMPLOT_H
#define MYCUSTOMPLOT_H
#include "qcustomplot.h"
#include <QTimer>

class MyCustomPlot : public QCustomPlot
{
    Q_OBJECT
public:
    MyCustomPlot(QWidget *parent = nullptr);

private:
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

    QTimer* m_timer;

signals:
    void sig_wheelEvent();

public slots:
    void handleTimeout();
};

#endif // MYCUSTOMPLOT_H
