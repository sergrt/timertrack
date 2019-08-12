#include "stdafx.h"
#include "TimerTrack.h"
#include "Intervals.h"
#include "Record.h"

TimerTrack::TimerTrack(QWidget *parent)
    : QMainWindow(parent),
    settingsWindow_(sqlLayer_, settings_),
    statisticsWindow_(sqlLayer_) {

    ui.setupUi(this);
    setupTrayIcon();

    connect(&settingsWindow_, &SettingsWindow::contextMenuChanged, this, &TimerTrack::updateContextMenu);

    // Context menu
    updateContextMenu();
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QMainWindow::customContextMenuRequested, this, [&](const QPoint& p) { popupMenu_.exec(mapToGlobal(p)); });

    // Timer
    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this, &TimerTrack::timerFinished);
}

TimerTrack::~TimerTrack() {
    interruptTimer();
}

void TimerTrack::setupTrayIcon() {
    trayIcon_.setIcon(this->style()->standardIcon(QStyle::SP_ComputerIcon));
    trayIcon_.setToolTip("TimerTrack");
    trayIcon_.setContextMenu(&popupMenu_);
    trayIcon_.show();
    connect(&trayIcon_, &QSystemTrayIcon::activated, this, &TimerTrack::iconActivated);
}

void TimerTrack::closeEvent(QCloseEvent* event) {
    event->ignore();
    hide();
}

void TimerTrack::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger)
        this->setVisible(!this->isVisible());
}

void TimerTrack::startTimer(std::chrono::milliseconds interval, int recordId) {
    timer_.start(interval.count());
    activeRecord_ = recordId;
    interruptAction_->setDisabled(false);
}

void TimerTrack::stopTimer() {
    timer_.stop();
    activeRecord_.reset();
    interruptAction_->setDisabled(true);
}

void TimerTrack::timerFinished() {
    if (!activeRecord_)
        throw std::runtime_error("Something bad happened");

    sqlLayer_.finishRecord(*activeRecord_);
    stopTimer();
    executeFinishActions();

    if (!intervals_.empty())
        startTimer();
}

void TimerTrack::executeFinishActions() {
    const auto finishActions = settings_.finishActions();
    for (const auto& a : finishActions) {
        if (a == Settings::FinishAction::Popup) {
            QMessageBox box(QMessageBox::Information, "Finished", "Interval finished", QMessageBox::NoButton, this);
            box.exec();
        } else if (a == Settings::FinishAction::Tooltip) {
            trayIcon_.showMessage("Finished", "Interval finished");
        } else if (a == Settings::FinishAction::Sound) {
            mediaPlayer_.setMedia(QUrl::fromLocalFile(settings_.soundFileName()));
            mediaPlayer_.play();
        }
    }
}

void TimerTrack::startTimerPattern() {
    if (activeRecord_)
        interruptTimer();

    const auto times = patternToIntervals(settings_.timerPattern());
    intervals_.clear();
    auto odd = true;
    for (auto t : times) {
        intervals_.emplace_back(t, odd);
        odd = !odd;
    }
    startTimer();
}

void TimerTrack::updateContextMenu() {
    popupMenu_.clear();

    const auto* startPattenAction = popupMenu_.addAction("Start pattern");
    connect(startPattenAction, &QAction::triggered, this, &TimerTrack::startTimerPattern);

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
                    if (activeRecord_)
                        interruptTimer();
                    intervals_ = decltype(intervals_){{ timerInterval, true }};
                    startTimer(categoryId);
                });
            }
        }
    }
    interruptAction_.reset(popupMenu_.addAction("Interrupt current timer"));
    connect(interruptAction_.get(), &QAction::triggered, this, &TimerTrack::interruptTimer);
    interruptAction_->setDisabled(!timer_.isActive());

    popupMenu_.addSeparator();
    const auto* actionSettings = popupMenu_.addAction("Settings...");
    const auto* actionStatistics = popupMenu_.addAction("Statistics...");

    connect(actionSettings, &QAction::triggered, this, [this]() { settingsWindow_.show(); });
    connect(actionStatistics, &QAction::triggered, this, [this]() { statisticsWindow_.show(); });
}

void TimerTrack::startTimer(std::optional<int> categoryId) {
    const auto interval = intervals_.front();
    if (!categoryId)
        categoryId = interval.second ? settings_.defaultCategoryId() : sqlLayer_.restingCategoryId();

    qDebug(QString("Starting interval of %1ms of category id = %2").arg(interval.first.count()).arg(*categoryId).toStdString().c_str());

    Record r;
    r.category_ = *categoryId;
    r.status_ = Record::Status::Started;
    r.startTime_ = std::chrono::system_clock::now();
    r.plannedTime_ = interval.first;
    r.passedTime_ = std::chrono::milliseconds{ 0 };

    const auto id = sqlLayer_.addRecord(r);

    startTimer(interval.first, id);
    intervals_.erase(begin(intervals_));
}

void TimerTrack::interruptTimer() {
    if (!activeRecord_)
        return;

    sqlLayer_.interruptRecord(*activeRecord_);
    stopTimer();
}
