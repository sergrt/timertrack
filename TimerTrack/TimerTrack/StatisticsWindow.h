#pragma once

#include <QWidget>
#include "ui_StatisticsWindow.h"
#include "Category.h"
#include "StatisticsChart.h"

class SqlLayer;

class StatisticsWindow : public QWidget {
    Q_OBJECT

public:
    StatisticsWindow(SqlLayer& sqlLayer, QWidget *parent = Q_NULLPTR);
    ~StatisticsWindow() = default;
    StatisticsWindow(const StatisticsWindow&) = delete;
    StatisticsWindow(StatisticsWindow&&) = delete;
    StatisticsWindow& operator=(const StatisticsWindow&) = delete;
    StatisticsWindow& operator=(StatisticsWindow&&) = delete;

private:
    Ui::StatisticsWindow ui;
    StatisticsChart* chartView_ = nullptr;
    SqlLayer& sqlLayer_;

    void initChart();

    std::vector<int> getSelectedCategories() const;
    void fillCategories() const;

    int getTodayStats(const Category& category, int status, bool inMinutes) const;
    int getYesterdayStats(const Category& category, int status, bool inMinutes) const;
    int getCurWeekStats(const Category& category, int status, bool inMinutes) const;
    int getLastWeekStats(const Category& category, int status, bool inMinutes) const;
    int getCurMonthStats(const Category& category, int status, bool inMinutes) const;
    int getLastMonthStats(const Category& category, int status, bool inMinutes) const;
    int getCurYearStats(const Category& category, int status, bool inMinutes) const;

public slots:
    void dateChanged() const;
    void categoriesChanged();
    void fillTable() const;
};
