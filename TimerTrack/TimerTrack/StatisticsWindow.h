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

    QString getTodayStats(const Category& category, int status, bool inMinutes) const;
    QString getYesterdayStats(const Category& category, int status, bool inMinutes) const;
    QString getCurWeekStats(const Category& category, int status, bool inMinutes) const;
    QString getLastWeekStats(const Category& category, int status, bool inMinutes) const;
    QString getCurMonthStats(const Category& category, int status, bool inMinutes) const;
    QString getLastMonthStats(const Category& category, int status, bool inMinutes) const;
    QString getCurYearStats(const Category& category, int status, bool inMinutes) const;

    QString queryStats(const std::vector<int>& categores, const QDateTime& from, const QDateTime& till, int status, bool inMinutes) const;
public slots:
    void dateChanged() const;
    void categoriesChanged();
    void fillTable() const;
};
