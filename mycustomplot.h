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
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;

    //滚轮缩放逻辑参数
    QTimer* m_timer; //去抖动

    //按键拖拽逻辑参数
    QTimer* m_dragTimer;

signals:
    void sig_wheelEvent(); //鼠标滚轮和拖拽改变视角可通用此信号

public slots:
    void handleTimeout();
    void handleTimeoutBydrag();
};

#endif // MYCUSTOMPLOT_H
