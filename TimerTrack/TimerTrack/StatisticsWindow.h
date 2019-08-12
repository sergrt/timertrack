#pragma once

#include <QWidget>
#include "ui_StatisticsWindow.h"
#include <QChartView>

class SqlLayer;
using namespace QtCharts;

class StatisticsWindow : public QWidget {
    Q_OBJECT

public:
    StatisticsWindow(SqlLayer& sqlLayer, QWidget *parent = Q_NULLPTR);
    ~StatisticsWindow();

private:
    StatisticsWindow(const StatisticsWindow&) = delete;
    StatisticsWindow(StatisticsWindow&&) = delete;
    StatisticsWindow& operator=(const StatisticsWindow&) = delete;
    StatisticsWindow& operator=(StatisticsWindow&&) = delete;

    Ui::StatisticsWindow ui;
    QChartView* chartView_ = nullptr;
    SqlLayer& sqlLayer_;

    std::vector<int> getSelectedCategories() const;

public slots:
    void dateChanged();
};
