#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TimerTrack.h"
#include "SettingsWindow.h"
#include "StatisticsWindow.h"

class TimerTrack : public QMainWindow {
    Q_OBJECT

public:
    TimerTrack(QWidget *parent = Q_NULLPTR);

private:
    Ui::TimerTrackClass ui;
    SettingsWindow settingsWindow_;
    StatisticsWindow statisticsWindow_;
};
