#include "stdafx.h"
#include "TimerTrack.h"
#include "Intervals.h"

TimerTrack::TimerTrack(QWidget *parent)
    : QMainWindow(parent),
    settingsWindow_(sqlLayer_) {

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

    /*
    connect(ui.startPattern, &QPushButton::clicked, this, [&]() {
        intervals_ = splitPattern();

    });
    */
}

void TimerTrack::updateContextMenu() {
    popupMenu_.clear();
    // TODO: disconnect all signal/slot connections

    popupMenu_.addAction("Start pattern");

    const auto entries = settingsWindow_.getContextMenuEntries();
    const auto contextMenuIntervals = entriesToIntervals(entries);
    const auto categories = sqlLayer_.readCategories();

    for (const auto& interval : contextMenuIntervals) {
        auto* menu = popupMenu_.addMenu(interval.second);
        //auto* action = popupMenu_.addAction(interval.second);
        //action->setData(interval.first.count());
        for (const auto& category : categories) {
            if (!category.archived_) {
                auto* action = menu->addAction(category.name_);
                action->setData(interval.first.count());
                action->setIcon(category.createIcon());
            }
        }
    }
    /*
    auto* a = popupMenu_.addAction("05:00");
    auto* m = popupMenu_.addMenu("10:00");
    m->addAction("Default");
    */
}

