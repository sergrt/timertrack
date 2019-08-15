#include "stdafx.h"
#include "StatisticsChart.h"
#include <QCategoryAxis>

static const int xLabelOffset = 15;
static const int yLabelOffset = 29;

StatisticsChart::StatisticsChart(QChart* chart)
    : QChartView(chart) {
    xLabel_ = new QGraphicsSimpleTextItem();
    yLabel_ = new QGraphicsSimpleTextItem();

    xLabel_->setBrush(QBrush(Qt::blue));
    yLabel_->setBrush(QBrush(Qt::blue));

    scene()->addItem(xLabel_);
    scene()->addItem(yLabel_);
}

void StatisticsChart::mouseMoveEvent(QMouseEvent * event) {
    const auto x = event->pos().x();
    const auto y = event->pos().y();
    auto* chart_ = chart();

    if (chart_) {
        const auto axisX = dynamic_cast<QCategoryAxis*>(chart_->axisX());
        const auto axisY = dynamic_cast<QCategoryAxis*>(chart_->axisY());
        if (axisX && axisY) {
            const auto xVal = chart_->mapToValue(event->pos()).x();
            const auto yVal = chart_->mapToValue(event->pos()).y();

            if (xVal <= axisX->max() && xVal >= axisX->min() && yVal <= axisY->max() && yVal >= axisY->min()) {
                const auto xPosOnAxis = chart_->mapToPosition(QPointF(x, 0));
                const auto yPosOnAxis = chart_->mapToPosition(QPointF(0, y));

                xLabel_->setPos(x, xPosOnAxis.y() + xLabelOffset);
                yLabel_->setPos(yPosOnAxis.x() - yLabelOffset, y);

                QString prevLabelText = "";
                for (const auto& label : axisX->categoriesLabels()) {
                    if (prevLabelText.isEmpty())
                        prevLabelText = label;

                    if (axisX->endValue(label) >= xVal) {
                        xLabel_->setText(prevLabelText);
                        break;
                    }
                    prevLabelText = label;
                }
                yLabel_->setText(QString("%1").arg(static_cast<int>(yVal)));
            }
        }
    }

    QChartView::mouseMoveEvent(event);
}