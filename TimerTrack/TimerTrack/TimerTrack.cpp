#include "stdafx.h"
#include "TimerTrack.h"


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

    popupMenu_.addAction("Start pattern");
    auto* a = popupMenu_.addAction("05:00");
    auto* m = popupMenu_.addMenu("10:00");
    m->addAction("Default");
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
