#pragma once
#include <QChartView>

using namespace QtCharts;

class StatisticsChart : public QChartView {
public:
    StatisticsChart(QChart* chart);

private:
    QGraphicsSimpleTextItem* xLabel_ = nullptr;
    QGraphicsSimpleTextItem* yLabel_ = nullptr;

protected:
    void mouseMoveEvent(QMouseEvent * event) override;
};
