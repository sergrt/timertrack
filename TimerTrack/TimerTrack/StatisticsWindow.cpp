#include "stdafx.h"
#include "StatisticsWindow.h"
#include <QChartView>
#include <QLineSeries>
#include <QCategoryAxis>
#include <set>
#include "SqlLayer.h"
#include "Intervals.h"

static const int categoryCol = 0;
static const int todayCompletedCol = 1;
static const int todayInterruptedCol = 2;
static const int yesterdayCompletedCol = 3;
static const int yesterdayInterruptedCol = 4;
static const int curWeekCompletedCol = 5;
static const int curWeekInterruptedCol = 6;
static const int lastWeekCompletedCol = 7;
static const int lastWeekInterruptedCol = 8;
static const int curMonthCompletedCol = 9;
static const int curMonthInterruptedCol = 10;
static const int lastMonthCompletedCol = 11;
static const int lastMonthInterruptedCol = 12;
static const int curYearCompletedCol = 13;
static const int curYearInterruptedCol = 14;

extern const int pomodoroStatusCompleted;
extern const int pomodoroStatusInterrupted;

const int columnCount = 15;

static const QStringList headerLabels{
    "Category",
    "today+",
    "today-",
    "y'day+",
    "y'day-",
    "week+",
    "week-",
    "last\nweek+",
    "last\nweek-",
    "month+",
    "month-",
    "last\nmonth+",
    "last\nmonth-",
    "year+",
    "year-"
};

StatisticsWindow::StatisticsWindow(SqlLayer& sqlLayer, QWidget *parent)
    : QWidget(parent),
    sqlLayer_{ sqlLayer } {

    ui.setupUi(this);
    setWindowIcon(QIcon(":/TimerTrack/stopwatch.png"));
    ui.dateTimeEditFrom->setDate(QDateTime::currentDateTime().date().addDays(-7));
    ui.dateTimeEditTo->setDate(QDateTime::currentDateTime().date().addDays(1));

    ui.statisticsTable->setColumnCount(columnCount);
    ui.statisticsTable->setHorizontalHeaderLabels(headerLabels);
    ui.statisticsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    fillTable();

    fillCategories();
    // check all items to have initial chart view with all categories
    for (int i = 0; i < ui.categoriesList->count();++i)
        ui.categoriesList->item(i)->setCheckState(Qt::Checked);

    initChart();

    connect(ui.dateTimeEditFrom, &QDateTimeEdit::dateTimeChanged, this, &StatisticsWindow::dateChanged);
    connect(ui.dateTimeEditTo, &QDateTimeEdit::dateTimeChanged, this, &StatisticsWindow::dateChanged);
    connect(ui.categoriesList, &QListWidget::itemChanged, this, &StatisticsWindow::dateChanged);

    connect(ui.showCount, &QRadioButton::toggled, this, &StatisticsWindow::fillTable);
    connect(ui.showInMinutes, &QRadioButton::toggled, this, &StatisticsWindow::fillTable);

    dateChanged(); // update chart
}

void StatisticsWindow::categoriesChanged() {
    fillCategories();
    fillTable();
    dateChanged();
}

void StatisticsWindow::fillCategories() const {
    ui.categoriesList->clear();

    auto categories = sqlLayer_.readCategories();
    for (const auto& c : categories) {
        auto* item = new QListWidgetItem(c.createIcon(), c.name_);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, QVariant(c.id_));
        if (c.archived_)
            item->setTextColor(Qt::gray);
        ui.categoriesList->addItem(item);
    }
}

void StatisticsWindow::initChart() {
    auto* chart = new QChart();
    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->setTitle("Selected categories completed");

    chartView_ = new StatisticsChart(chart);
    chartView_->setRenderHint(QPainter::Antialiasing);

    auto* layout = new QGridLayout(ui.frame);
    ui.frame->setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView_);
}

void StatisticsWindow::fillTable() const {
    ui.statisticsTable->setRowCount(0);

    auto categories = sqlLayer_.readCategories();
    const auto inMinutes = ui.showInMinutes->isChecked();
    for (const auto& c : categories) {
        auto* item = new QTableWidgetItem();
        item->setIcon(c.createIcon());
        item->setText(c.name_);
        ui.statisticsTable->insertRow(ui.statisticsTable->rowCount());
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, categoryCol, item);

        // Today

        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, todayCompletedCol,
                                    new QTableWidgetItem(QString("%1").arg(getTodayStats(c, pomodoroStatusCompleted, inMinutes))));
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, todayInterruptedCol,
                                    new QTableWidgetItem(QString("%1").arg(getTodayStats(c, pomodoroStatusInterrupted, inMinutes))));

        // Yesterday
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, yesterdayCompletedCol,
                                    new QTableWidgetItem(QString("%1").arg(getYesterdayStats(c, pomodoroStatusCompleted, inMinutes))));
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, yesterdayInterruptedCol,
                                    new QTableWidgetItem(QString("%1").arg(getYesterdayStats(c, pomodoroStatusInterrupted, inMinutes))));

        // Current week
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, curWeekCompletedCol,
                                    new QTableWidgetItem(QString("%1").arg(getCurWeekStats(c, pomodoroStatusCompleted, inMinutes))));
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, curWeekInterruptedCol,
                                    new QTableWidgetItem(QString("%1").arg(getCurWeekStats(c, pomodoroStatusInterrupted, inMinutes))));

        // Last week
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, lastWeekCompletedCol,
                                    new QTableWidgetItem(QString("%1").arg(getLastWeekStats(c, pomodoroStatusCompleted, inMinutes))));
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, lastWeekInterruptedCol,
                                    new QTableWidgetItem(QString("%1").arg(getLastWeekStats(c, pomodoroStatusInterrupted, inMinutes))));

        // Current month        
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, curMonthCompletedCol,
                                    new QTableWidgetItem(QString("%1").arg(getCurMonthStats(c, pomodoroStatusCompleted, inMinutes))));
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, curMonthInterruptedCol,
                                    new QTableWidgetItem(QString("%1").arg(getCurMonthStats(c, pomodoroStatusInterrupted, inMinutes))));

        // Last month
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, lastMonthCompletedCol,
                                    new QTableWidgetItem(QString("%1").arg(getLastMonthStats(c, pomodoroStatusCompleted, inMinutes))));
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, lastMonthInterruptedCol,
                                    new QTableWidgetItem(QString("%1").arg(getLastMonthStats(c, pomodoroStatusInterrupted, inMinutes))));

        // Current year
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, curYearCompletedCol,
                                    new QTableWidgetItem(QString("%1").arg(getCurYearStats(c, pomodoroStatusCompleted, inMinutes))));
        ui.statisticsTable->setItem(ui.statisticsTable->rowCount() - 1, curYearInterruptedCol,
                                    new QTableWidgetItem(QString("%1").arg(getCurYearStats(c, pomodoroStatusInterrupted, inMinutes))));

    }
}

std::vector<int> StatisticsWindow::getSelectedCategories() const {
    std::vector<int> categories;
    for (auto i = 0; i < ui.categoriesList->count(); ++i) {
        const auto* item = ui.categoriesList->item(i);
        if (item->checkState() == Qt::Checked)
            categories.push_back(item->data(Qt::UserRole).toInt());
    }
    return categories;
}

void StatisticsWindow::dateChanged() const {
    const auto categories = getSelectedCategories();
    const auto from = QDateTime(ui.dateTimeEditFrom->date());
    const auto till = QDateTime(ui.dateTimeEditTo->date());
    const auto pts = sqlLayer_.getCompletedRecords(categories, from, till);

    // Fill days gaps
    // Create full days range
    const auto totalDays = from.date().daysTo(till.date());
    std::map<int, int> ptsFull;
    for (auto x = 0; x < totalDays; ++x) {
        const auto tmp = from.date().addDays(x);
        ptsFull[QDateTime(tmp).toSecsSinceEpoch()] = 0;
    }

    // Fill full days range with retrieved data
    for (const auto& p :pts) {
        const auto pointDate = QDateTime::fromSecsSinceEpoch(p.first).date();
        const auto pointTime = QDateTime(pointDate);
        ptsFull[pointTime.toSecsSinceEpoch()] = p.second;
    }

    // Create axis
    auto *axisX = new QCategoryAxis;
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);

    auto *axisY = new QCategoryAxis;
    axisY->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    

    // Create new line series
    auto* series = new QLineSeries();

    auto index{ 0 };
    auto minY{ 0 };
    auto maxY{ 0 };
    for (const auto& p : ptsFull) {
        axisX->append(QDateTime::fromSecsSinceEpoch(p.first).toString("dd-MM-yyyy"), index);
        series->append(index, p.second);
        if (p.second < minY)
            minY = p.second;
        if (p.second > maxY)
            maxY = p.second;

        ++index;
    }
    axisX->setMin(0);
    axisX->setMax(index - 1);

    for (auto i = minY; i <= maxY; ++i)
        axisY->append(QString("%1").arg(i), i);

    axisY->setMin(minY);
    axisY->setMax(maxY);

    auto* const chart = chartView_->chart();
    chart->removeAllSeries();
    chart->setAxisX(axisX);
    chart->setAxisY(axisY);
    chart->addSeries(series);
    chart->update();
}

QString StatisticsWindow::queryStats(const std::vector<int>& categores, const QDateTime& from, const QDateTime& till, int status, bool inMinutes) const {
    const auto res = sqlLayer_.getRecordsStats(categores, from, till, status, inMinutes);
    if (!inMinutes)
        return QString("%1").arg(res);

    return intervalToStr(std::chrono::seconds{ res });
}

QString StatisticsWindow::getTodayStats(const Category& category, int status, bool inMinutes) const {
    const std::vector<int> queryCategories{ category.id_ };
    const auto from = QDateTime(QDateTime::currentDateTime().date());
    const auto till = QDateTime(QDateTime::currentDateTime());
    return queryStats(queryCategories, from, till, status, inMinutes);
}

QString StatisticsWindow::getYesterdayStats(const Category& category, int status, bool inMinutes) const {
    const std::vector<int> queryCategories{ category.id_ };
    const auto from = QDateTime(QDateTime::currentDateTime().date().addDays(-1));
    const auto till = QDateTime(QDateTime::currentDateTime().date());
    return queryStats(queryCategories, from, till, status, inMinutes);
}

QString StatisticsWindow::getCurWeekStats(const Category& category, int status, bool inMinutes) const {
    const std::vector<int> queryCategories{ category.id_ };
    const auto from = QDateTime(QDateTime::currentDateTime().date().addDays(-QDateTime::currentDateTime().date().dayOfWeek() + 1));
    const auto till = QDateTime(QDateTime::currentDateTime().date());
    return queryStats(queryCategories, from, till, status, inMinutes);
}

QString StatisticsWindow::getLastWeekStats(const Category& category, int status, bool inMinutes) const {
    const std::vector<int> queryCategories{ category.id_ };
    const auto curDate = QDateTime::currentDateTime().date();
    const auto from = QDateTime(curDate.addDays(-QDateTime::currentDateTime().date().dayOfWeek() + 1 - 7));
    const auto till = QDateTime(curDate.addDays(-QDateTime::currentDateTime().date().dayOfWeek() + 1));
    return queryStats(queryCategories, from, till, status, inMinutes);
}

QString StatisticsWindow::getCurMonthStats(const Category& category, int status, bool inMinutes) const {
    const std::vector<int> queryCategories{ category.id_ };
    auto tmp = QDateTime::currentDateTime().date();
    tmp.setDate(tmp.year(), tmp.month(), 1);
    const auto from = QDateTime(tmp);
    const auto till = QDateTime(QDateTime::currentDateTime().date());
    return queryStats(queryCategories, from, till, status, inMinutes);
}

QString StatisticsWindow::getLastMonthStats(const Category& category, int status, bool inMinutes) const {
    const std::vector<int> queryCategories{ category.id_ };
    auto tmp = QDateTime::currentDateTime().date().addMonths(-1);
    tmp.setDate(tmp.year(), tmp.month(), 1);
    const auto from = QDateTime(tmp);

    tmp = QDateTime::currentDateTime().date();
    tmp.setDate(tmp.year(), tmp.month(), 1);
    const auto till = QDateTime(tmp);

    return queryStats(queryCategories, from, till, status, inMinutes);
}

QString StatisticsWindow::getCurYearStats(const Category& category, int status, bool inMinutes) const {
    const std::vector<int> queryCategories{ category.id_ };
    auto tmp = QDateTime::currentDateTime().date();
    tmp.setDate(tmp.year(), 1, 1);
    const auto from = QDateTime(tmp);
    const auto till = QDateTime(QDateTime::currentDateTime().date());
    return queryStats(queryCategories, from, till, status, inMinutes);
}
