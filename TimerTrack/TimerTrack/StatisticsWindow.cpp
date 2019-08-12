#include "stdafx.h"
#include "StatisticsWindow.h"
#include <QChartView>
#include <QLineSeries>
#include "SqlLayer.h"
#include <QCategoryAxis>
#include <set>

StatisticsWindow::StatisticsWindow(SqlLayer& sqlLayer, QWidget *parent)
    : QWidget(parent),
    sqlLayer_{ sqlLayer } {

    ui.setupUi(this);

    QLineSeries *series = new QLineSeries();
    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);
    *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("Selected categories completed");
    chartView_ = new QChartView(chart);
    chartView_->setRenderHint(QPainter::Antialiasing);


    auto* layout = new QGridLayout(ui.frame);
    ui.frame->setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView_);

    connect(ui.dateTimeEditFrom, &QDateTimeEdit::dateTimeChanged, this, &StatisticsWindow::dateChanged);
    connect(ui.dateTimeEditTo, &QDateTimeEdit::dateTimeChanged, this, &StatisticsWindow::dateChanged);
}

std::vector<int> StatisticsWindow::getSelectedCategories() const {
    std::vector<int> categories{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    return categories;
}

void StatisticsWindow::dateChanged() {
    const auto categories = getSelectedCategories();
    const auto from = QDateTime(ui.dateTimeEditFrom->date());
    const auto till = QDateTime(ui.dateTimeEditTo->date());
    const auto pts = sqlLayer_.getCompletedPomodoros(categories, from, till);

    // Fill days gaps
    // Create full days range
    const auto totalDays = from.date().daysTo(till.date());
    std::map<int, int> ptsFull;
    for (auto x = 0; x < totalDays; ++x) {
        auto d = from.date().addDays(x);
        ptsFull[QDateTime(d).toSecsSinceEpoch()] = 0;
    }

    // Fill retrieved data
    for (const auto& p :pts) {
        auto pointDate = QDateTime::fromSecsSinceEpoch(p.first).date();
        const auto pointTime = QDateTime(pointDate);
        ptsFull[pointTime.toSecsSinceEpoch()] = p.second;
    }

    // Create new line series
    auto* series = new QLineSeries();

    // Create axis
    auto *axisX = new QCategoryAxis;
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);

    auto *axisY = new QCategoryAxis;
    axisY->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    axisY->setMin(0);
    axisY->setMax(0);

    int i = 0;
    for (const auto& p : ptsFull) {
        axisX->append(QDateTime::fromSecsSinceEpoch(p.first).toString("dd-MM-yyyy"), i);
        series->append(i, p.second);
        ++i;
    }
    axisX->setMin(0);
    axisX->setMax(i - 1);

    // Create set of Y axis values
    std::set<int> yValuesSet;
    for (const auto& p : pts)
        yValuesSet.insert(p.second);

    for (const auto& p : yValuesSet) {
        if (p < axisY->min())
            axisY->setMin(p);
        if (p > axisY->max())
            axisY->setMax(p);
        axisY->append(QString("%1").arg(p), p);
    }

    auto* chart = chartView_->chart();
    chart->removeAllSeries();

    chart->setAxisX(axisX);
    chart->setAxisY(axisY);
    chart->addSeries(series);
    chart->update();
}

StatisticsWindow::~StatisticsWindow() {
    delete chartView_;
}
