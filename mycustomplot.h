#ifndef MYCUSTOMPLOT_H
#define MYCUSTOMPLOT_H
#include "qcustomplot.h"
#include <QTimer>

struct TracerInfo
{
    QCPItemTracer* tracer;
    QCPItemText* label;
    bool isActive;
    TracerInfo()
    {
        tracer = nullptr;
        label = nullptr;
        isActive = false;
    }
};

class MyCustomPlot : public QCustomPlot
{
    Q_OBJECT
public:
    MyCustomPlot(QWidget *parent = nullptr);
    virtual ~MyCustomPlot() override;

private:
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;

    //避免label设置父锚点报错的子函数
    void safeSetParentAnchor(QCPItemPosition* childPos,QCPItemAnchor* parentAnchor);

    //标牌显示图层（在默认图层之上）
    QCPLayer* m_textLayer;

    //滚轮缩放逻辑参数
    QTimer* m_timer; //去抖动

    //鼠标右键拖拽逻辑参数
    QTimer* m_dragTimer;
    bool m_dragging;
    QPoint m_prePoint; //0.5s前上一个点

    //鼠标左键显示图像数据
    bool m_leftPress;
    QVector<TracerInfo>m_tracers;
    int m_currentIndex;//m_tracers数组当前选中下标
    QString m_titleText;//当前图表标题

signals:
    void sig_wheelEvent(qint64 xLower,qint64 xUpper,qint64 yLower,qint64 yUpper); //鼠标滚轮和拖拽改变视角可通用此信号

public slots:
    void handleTimeout();
    void handleTimeoutBydrag();
};

#endif // MYCUSTOMPLOT_H
