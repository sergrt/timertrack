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
#include <QSystemTrayIcon>
#include <QMediaPlayer>

class TimerTrack : public QMainWindow {
    Q_OBJECT

public:
    TimerTrack(QWidget *parent = Q_NULLPTR);
    ~TimerTrack();

private:
    Ui::TimerTrackClass ui;
    SqlLayer sqlLayer_;
    Settings settings_;
    SettingsWindow settingsWindow_;
    StatisticsWindow statisticsWindow_;
    QMenu popupMenu_;

    using IntervalOddEven = std::pair<std::chrono::milliseconds, bool>;
    std::vector<IntervalOddEven> intervals_; //TODO: consider queue
    void startTimer(std::optional<int> categoryId = std::nullopt);
    QTimer timer_;
    std::optional<int> activeRecord_;
    std::unique_ptr<QAction> interruptAction_;
    QSystemTrayIcon trayIcon_;
    QMediaPlayer mediaPlayer_;

    void setupTrayIcon();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void executeFinishActions();
    void stopTimer();
    void startTimer(std::chrono::milliseconds interval, int recordId);
protected:
    void closeEvent(QCloseEvent* event) override;

public slots:
    void updateContextMenu();
    void interruptTimer();
    void timerFinished();
    void startTimerPattern();
};
