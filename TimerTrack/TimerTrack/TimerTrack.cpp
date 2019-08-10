#include "stdafx.h"
#include "TimerTrack.h"
#include "Intervals.h"

TimerTrack::TimerTrack(QWidget *parent)
    : QMainWindow(parent),
    settingsWindow_(sqlLayer_, settings_) {

    ui.setupUi(this);
    connect(ui.actionSettings, &QAction::triggered, this, [this]() {
        settingsWindow_.show();
    });
    connect(ui.actionStatistics, &QAction::triggered, this, [this]() {
        statisticsWindow_.show();
    });
    connect(&settingsWindow_, &SettingsWindow::contextMenuChanged, this, &TimerTrack::updateContextMenu);
    updateContextMenu();
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QMainWindow::customContextMenuRequested, this, [&](const QPoint& p) {
        popupMenu_.exec(mapToGlobal(p));
    });

    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this, [&]() {
        system("pause"); // TODO: finish action here
        startTimer();
    });
}

void TimerTrack::updateContextMenu() {
    popupMenu_.clear();

    const auto* startPattenAction = popupMenu_.addAction("Start pattern");
    connect(startPattenAction, &QAction::triggered, this, [&]() {
        intervals_ = patternToIntervals(settings_.timerPattern());
        startTimer();
    });

    const auto contextMenuIntervals = entriesToIntervals(settingsWindow_.getContextMenuEntries());
    const auto categories = sqlLayer_.readCategories();

    for (const auto& interval : contextMenuIntervals) {
        auto* menu = popupMenu_.addMenu(interval.second);
        for (const auto& category : categories) {
            if (!category.archived_) {
                auto* action = menu->addAction(category.name_);
                action->setData(interval.first.count());
                action->setIcon(category.createIcon());

                connect(action, &QAction::triggered, this, [&, categoryId = category.id_, timerInterval = interval.first]() {
                    intervals_ = std::vector<std::chrono::milliseconds>{ timerInterval };
                    startTimer(categoryId);
                });
            }
        }
    }
}

void TimerTrack::startTimer(std::optional<int> categoryId) {
    if (intervals_.empty())
        return;

    const auto interval = intervals_.front();
    if (!categoryId)
        categoryId = settings_.defaultCategoryId();
    qDebug(QString("Starting interval of %1ms of category id = %2").arg(interval.count()).arg(*categoryId).toStdString().c_str());

    timer_.start(interval.count());
    intervals_.erase(begin(intervals_));
}
