#include "stdafx.h"
#include "TimerTrack.h"

TimerTrack::TimerTrack(QWidget *parent)
    : QMainWindow(parent) {

    ui.setupUi(this);
    connect(ui.actionSettings, &QAction::triggered, this, [this]() {
        settingsWindow_.show();
    });
    connect(ui.actionStatistics, &QAction::triggered, this, [this]() {
        statisticsWindow_.show();
    });
}
