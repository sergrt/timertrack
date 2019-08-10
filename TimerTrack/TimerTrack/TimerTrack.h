#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TimerTrack.h"
#include "SettingsWindow.h"
#include "StatisticsWindow.h"
#include <QList>
#include <chrono>
#include "SqlLayer.h"

class TimerTrack : public QMainWindow {
    Q_OBJECT

public:
    TimerTrack(QWidget *parent = Q_NULLPTR);

private:
    Ui::TimerTrackClass ui;
    SqlLayer sqlLayer_;
    SettingsWindow settingsWindow_;
    StatisticsWindow statisticsWindow_;
    QMenu popupMenu_;

    QList<std::chrono::milliseconds> intervals_;
};
