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
    TimerTrack(const TimerTrack&) = delete;
    TimerTrack(TimerTrack&&) = delete;
    TimerTrack& operator=(const TimerTrack&) = delete;
    TimerTrack& operator=(TimerTrack&&) = delete;

private:
    Ui::timerTrackWindow ui;
    SqlLayer sqlLayer_;
    Settings settings_;
    SettingsWindow settingsWindow_;
    StatisticsWindow statisticsWindow_;
    QMenu popupMenu_;

    struct IntervalOddEven {
        std::chrono::milliseconds interval;
        bool isOdd;
    };
    std::vector<IntervalOddEven> intervals_; //TODO: consider queue
    void startNextInterval(std::optional<int> categoryId = std::nullopt);
    QTimer timer_;
    QTimer labelTimer_;

    struct IntervalInfo {
        int recordId;
        QString categoryName;
        std::chrono::milliseconds interval;
    };

    std::optional<IntervalInfo> activeIntervalInfo_;
    QAction* interruptAction_ = nullptr;
    QSystemTrayIcon trayIcon_;
    QMediaPlayer mediaPlayer_;

    void setupTrayIcon();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void executeFinishActions(const QString& categoryName, const std::chrono::milliseconds& length);
    void stopTimer();
    void startTimer(IntervalInfo intervalInfo);
    void setTooltips();

protected:
    void changeEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    QPoint localMousePos_; // position inside window to calculate offset

public slots:
    void updateContextMenu();
    void interruptTimer();
    void timerFinished();
    void startTimerSequence();
    void updateLabel() const;
};
