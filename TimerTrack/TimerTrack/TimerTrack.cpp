#include "stdafx.h"
#include "TimerTrack.h"
#include "Intervals.h"
#include "Record.h"

TimerTrack::TimerTrack(QWidget *parent)
    : QMainWindow(parent),
    settingsWindow_(sqlLayer_, settings_) {

    ui.setupUi(this);

    trayIcon_.setIcon(this->style()->standardIcon(QStyle::SP_ComputerIcon));
    trayIcon_.setToolTip("TimerTrack");
    trayIcon_.show();

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

    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this, [&]() {
        if (!activeRecord_)
            throw std::runtime_error("Something bad happened");

        sqlLayer_.finishRecord(*activeRecord_);
        activeRecord_.reset();
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
        startTimer();
    });
}

TimerTrack::~TimerTrack() {
    interruptTimer();
}

void TimerTrack::updateContextMenu() {
    popupMenu_.clear();

    const auto* startPattenAction = popupMenu_.addAction("Start pattern");
    connect(startPattenAction, &QAction::triggered, this, [&]() {
        //intervals_ = patternToIntervals(settings_.timerPattern());
        const auto times = patternToIntervals(settings_.timerPattern());
        intervals_.clear();
        auto odd = true;
        for (auto t : times) {
            intervals_.emplace_back(t, odd);
            odd = !odd;
        }
        startTimer();
    });

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
                    intervals_ = decltype(intervals_){
                        { timerInterval, true }};
                    startTimer(categoryId);
                });
            }
        }
    }
    interruptAction_.reset(popupMenu_.addAction("Interrupt current timer"));
    connect(interruptAction_.get(), &QAction::triggered, this, &TimerTrack::interruptTimer);
    interruptAction_->setDisabled(!timer_.isActive());
}

void TimerTrack::startTimer(std::optional<int> categoryId) {
    if (intervals_.empty())
        return;

    const auto interval = intervals_.front();
    if (!categoryId) {
        if (interval.second)
            categoryId = settings_.defaultOddCategoryId();
        else
            categoryId = settings_.defaultEvenCategoryId();
    }

    qDebug(QString("Starting interval of %1ms of category id = %2").arg(interval.first.count()).arg(*categoryId).toStdString().c_str());

    const auto now = std::chrono::system_clock::now();
    Record r;
    r.type_ = Record::Type::Pomodoro;
    r.category_ = *categoryId;
    r.status_ = Record::Status::Started;
    r.startTime_ = now;
    r.plannedTime_ = interval.first;
    r.passedTime_ = std::chrono::milliseconds{ 0 };
    const auto id = sqlLayer_.addRecord(r);
    activeRecord_ = id;

    timer_.start(interval.first.count());
    intervals_.erase(begin(intervals_));
    interruptAction_->setDisabled(!timer_.isActive());
}

void TimerTrack::interruptTimer() {
    if (!activeRecord_)
        return;
    timer_.stop();
    sqlLayer_.interruptRecord(*activeRecord_);
    activeRecord_.reset();
}
