#ifndef DEFECTDETECTOR_H
#define DEFECTDETECTOR_H

#include <QObject>
#include "define.h"

class defectdetector : public QObject
{
    Q_OBJECT
public:
    explicit defectdetector(QObject *parent = nullptr);

    // 处理单个通道的一块数据，返回该块内的缺陷候选（全局坐标）
    QVector<DefectCandidate> processChannelBlock(
        int channel,
        const QVector<dataPoint>& blockData,   // 该通道在该块内的数据点
        double a_mm,                       // 采样间隔（毫米）
        qint64 blockStartIdx,               // 该块起始全局采样点序号
        double noiseThresh = 200.0,
        double minPeakToPeak = 400.0,
        int minDistPoints = 4               // 对应 b/a > 3
    );
    //processChannelBlock中的辅助处理函数，综合考虑平峰平谷的极值点情况
    QVector<ExtremaPoint> findExtremaWithPlateaus(const QVector<double>& y);
    // 处理一个数据块（包含256个通道的数据），返回该块内所有缺陷候选
    QVector<DefectCandidate> processFullBlock(
        const QVector<QVector<dataPoint>>& blockData,  // 外层256(模拟值)通道，内层每个通道的点数据
        double a_mm,
        qint64 blockStartIdx,                       // 该块起始全局采样点序号
        double noiseThresh = 200.0,
        double minPeakToPeak = 400.0,
        int minDistPoints = 4
    );
    /**
     * 判断两个候选是否可能属于同一缺陷
     * 条件：轴向有重叠或间隙小于阈值，且通道号连续（允许间隔1）
     */
    bool isSameEvent(const DefectCandidate& a, const DefectCandidate& b);
    /**
     * 将一个候选添加到缺陷事件中，更新事件的范围
     */
    void addToEvent(DefectEvent& event, const DefectCandidate& cand);
    /**
     * 聚类所有候选，生成缺陷事件列表
     */
    QVector<DefectEvent> clusterCandidates(QVector<DefectCandidate>& candidates);
    /**
     * 根据缺陷事件的形状进行分类
     * @param event     缺陷事件
     * @param pitch_mm  周向通道间距（毫米）
     * @return          缺陷类型
     */
    DefectType classifyEvent(const DefectEvent& event, double pitch_mm);
    /**
     * 将DefectEvent 中的 candidates 按通道号升序排列
     * 同一通道存在多个DefectCandidate，将其合并为一个
     */
    void sortAndMergeCandidates(DefectEvent& event);
    /**
     * 将缺陷格式从每个通道的缺陷区间转换为多边形顶点格式
     */
    QVector<QPointF> buildPolygonFromDefectEvent(const DefectEvent &event);
    /**
     * 从文件中逐块读取数据并处理（模拟）
     * @param a_mm          采样间隔（毫米）
     * @param innerDiameter      内管径（毫米）
     * @param blockSize     每个块每个通道的采样点数（不含重叠）
     * @param overlap       块间重叠点数
     * @return              检测到的缺陷事件列表
     */
    //blockSize函数内固定为8000
    void detectDefectsFromBlocks(double a_mm,
        double innerDiameter,projectConfigure* CprjConfig);

    //Axis 为XYZ或涡流参数 X-1
    int readDataFromFile(QString qsfilePath, QVector<QVector<dataPoint> > &vecPoint2D, qint64 startPos, qint64 offset, qint64 &differ, int axis);
    int readBlockData(QString& qsfilePath,qint64 startPos,qint64 offset,projectConfigure* CprjConfig);

    //成员变量
    QVector<QVector<dataPoint>> m_vecPoint2D;
    int m_numChannels; //当前项目多个盒子的总传感器通道数

signals:
    void detectDefectComplete();

public slots:
    void handleStartDetectDefects(double a_mm, double innerDiameter, projectConfigure CprjConfig);

private:
    // 轴向距离阈值（毫米），用于判断两个候选是否属于同一缺陷，小于5厘米
    const double AXIAL_GAP_MM = 50.0;
};

/**
 * 将缺陷区域的点集数组转换为Json字符串
 */
QString buildJsonFromVertices(QVector<QPointF>& vertices);

#endif // DEFECTDETECTOR_H
