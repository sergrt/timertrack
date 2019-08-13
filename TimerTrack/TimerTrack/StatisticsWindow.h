#pragma once

#include <QWidget>
#include "ui_StatisticsWindow.h"
#include <QChartView>
#include "Category.h"

class SqlLayer;
using namespace QtCharts;

class StatisticsWindow : public QWidget {
    Q_OBJECT

public:
    StatisticsWindow(SqlLayer& sqlLayer, QWidget *parent = Q_NULLPTR);

private:
    StatisticsWindow(const StatisticsWindow&) = delete;
    StatisticsWindow(StatisticsWindow&&) = delete;
    StatisticsWindow& operator=(const StatisticsWindow&) = delete;
    StatisticsWindow& operator=(StatisticsWindow&&) = delete;

    Ui::StatisticsWindow ui;
    QChartView* chartView_ = nullptr;
    SqlLayer& sqlLayer_;

    void initChart();

    std::vector<int> getSelectedCategories() const;
    void fillTable() const;
    void fillCategories() const;

    int getTodayCount(const Category& category, int status) const;
    int getYesterdayCount(const Category& category, int status) const;
    int getCurWeekCount(const Category& category, int status) const;
    int getLastWeekCount(const Category& category, int status) const;
    int getCurMonthCount(const Category& category, int status) const;
    int getLastMonthCount(const Category& category, int status) const;
    int getCurYearCount(const Category& category, int status) const;

public slots:
    void dateChanged() const;
    void categoriesChanged();
};
