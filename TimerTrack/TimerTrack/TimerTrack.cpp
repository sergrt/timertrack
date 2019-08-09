#include "stdafx.h"
#include "TimerTrack.h"
#

TimerTrack::TimerTrack(QWidget *parent)
    : QMainWindow(parent) {

    ui.setupUi(this);
    connect(ui.actionSettings, &QAction::triggered, this, [this]() {
        settingsWindow_.show();
    });
}
