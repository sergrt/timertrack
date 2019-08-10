#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TimerTrack.h"
#include "SettingsWindow.h"
#include "StatisticsWindow.h"
#include <vector>
#include <chrono>
#include "SqlLayer.h"
#include "Settings.h"
#include <optional>

class TimerTrack : public QMainWindow {
    Q_OBJECT

public:
    TimerTrack(QWidget *parent = Q_NULLPTR);

private:
    Ui::TimerTrackClass ui;
    SqlLayer sqlLayer_;
    Settings settings_;
    SettingsWindow settingsWindow_;
    StatisticsWindow statisticsWindow_;
    QMenu popupMenu_;

    std::vector<std::chrono::milliseconds> intervals_; //TODO: consider queue
    void startTimer(std::optional<int> categoryId = std::nullopt);
    QTimer timer_;

public slots:
    void updateContextMenu();
};
