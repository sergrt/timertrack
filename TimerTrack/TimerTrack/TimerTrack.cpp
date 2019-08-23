#include "stdafx.h"
#include "TimerTrack.h"
#include "Intervals.h"
#include "Record.h"

static const auto defaultTimerLabelText = QString(QApplication::tr("--:--", "TimerTrackDefaultLabelText"));
static const auto defaultTrayIconTooltip = QString(QApplication::tr("TimerTrack - no interval is active", "TimerTrackDefaultLabelTooltip"));
static const auto labelUpdateInterval{ 200 };

TimerTrack::TimerTrack(QWidget *parent)
    : QMainWindow(parent),
    settingsWindow_(sqlLayer_, settings_),
    statisticsWindow_(sqlLayer_) {

    ui.setupUi(this);
    ui.timerLabel->setText(defaultTimerLabelText);
    setupTrayIcon();

    this->setWindowFlags(this->windowFlags() | Qt::Tool | Qt::FramelessWindowHint);
    this->setWindowFlag(Qt::WindowStaysOnTopHint, settings_.alwaysOnTop());

    connect(&settingsWindow_, &SettingsWindow::contextMenuChanged, this, &TimerTrack::updateContextMenu);
    this->resize(settings_.width(), settings_.height());
    this->move(settings_.pos());
    this->setStyleSheet(settings_.stylesheet());

    // Context menu
    updateContextMenu();
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QMainWindow::customContextMenuRequested, this, [&](const QPoint& p) { popupMenu_.exec(mapToGlobal(p)); });

    connect(&settingsWindow_, &SettingsWindow::categoriesChanged, &statisticsWindow_, &StatisticsWindow::categoriesChanged);
    connect(&settingsWindow_, &SettingsWindow::categoriesChanged, this, &TimerTrack::updateContextMenu);

    connect(ui.closeButton, &QPushButton::clicked, this, [&]() {hide();});

    // Timer
    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this, &TimerTrack::timerFinished);

    labelTimer_.setInterval(labelUpdateInterval);
    connect(&labelTimer_, &QTimer::timeout, this, &TimerTrack::updateLabel);
}

TimerTrack::~TimerTrack() {
    interruptTimer();
}

void TimerTrack::setupTrayIcon() {
    trayIcon_.setIcon(QIcon(":/TimerTrack/stopwatch.png"));
    trayIcon_.setToolTip(defaultTrayIconTooltip);
    trayIcon_.setContextMenu(&popupMenu_);
    trayIcon_.show();
    connect(&trayIcon_, &QSystemTrayIcon::activated, this, &TimerTrack::iconActivated);
}

void TimerTrack::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            hide();
            event->ignore();
        } else {
            event->accept();
        }
    }
    QMainWindow::changeEvent(event);
}

void TimerTrack::closeEvent(QCloseEvent* event) {
    QApplication::exit();
}

void TimerTrack::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        if (isHidden())
            show();
        else
            hide();
    }
}

void TimerTrack::startTimer(IntervalInfo intervalInfo) {
    labelTimer_.start();

    timer_.start(intervalInfo.interval.count());
    activeIntervalInfo_ = std::move(intervalInfo);
    interruptAction_->setDisabled(false);
    setTooltips();
}

void TimerTrack::setTooltips() {
    if (!activeIntervalInfo_) {
        trayIcon_.setToolTip(defaultTrayIconTooltip);
        if (settings_.enableTooltip())
            ui.timerLabel->setToolTip(QString());
    } else {
        // Escape ampersand
        auto tooltipStr = QString(tr(R"(TimerTrack - "%1" [%2] timer is active)"))
            .arg(activeIntervalInfo_->categoryName)
            .arg(intervalToStr(activeIntervalInfo_->interval));
        tooltipStr.replace("&", "&&&");
        trayIcon_.setToolTip(tooltipStr);

        if (settings_.enableTooltip()) {
            ui.timerLabel->setToolTip(QString(tr(R"("%1" [%2])"))
                                      .arg(activeIntervalInfo_->categoryName)
                                      .arg(intervalToStr(activeIntervalInfo_->interval)));
        }
    }
}

void TimerTrack::stopTimer() {
    timer_.stop();
    ui.timerLabel->setText(defaultTimerLabelText);

    labelTimer_.stop();
    activeIntervalInfo_.reset();
    interruptAction_->setDisabled(true);
    setTooltips();
}

void TimerTrack::timerFinished() {
    if (!activeIntervalInfo_)
        throw std::runtime_error("Something bad happened");

    sqlLayer_.finishRecord(activeIntervalInfo_->recordId);
    const auto categoryName = activeIntervalInfo_->categoryName;
    const auto intervalLength = activeIntervalInfo_->interval;
    stopTimer();
    executeFinishActions(categoryName, intervalLength);

    if (!intervals_.empty())
        startNextInterval();
}

void TimerTrack::executeFinishActions(const QString& categoryName, const std::chrono::milliseconds& length) {
    const auto finishActions = settings_.finishActions();
    for (const auto& a : finishActions) {
        if (a == Settings::FinishAction::Popup) {
            auto* m = new QMessageBox(QDesktopWidget().screen());
            m->setObjectName("finishMessageBox");
            m->setStyleSheet(settings_.stylesheet());
            m->setIcon(QMessageBox::Information);
            m->setAttribute(Qt::WA_DeleteOnClose, true);
            m->setWindowFlags(m->windowFlags() | Qt::WindowStaysOnTopHint | Qt::Tool);
            m->setWindowTitle(tr("Finished", "MessageBoxHeader"));
            m->setText(QString(tr(R"(Interval "%1" [%2] finished)", "MessageBoxText"))
                       .arg(categoryName)
                       .arg(intervalToStr(length)));
            m->show();
        } else if (a == Settings::FinishAction::Tooltip) {
            trayIcon_.showMessage(tr("Finished", "TrayIconHeader"), QString(tr(R"(Interval "%1" [%2] finished)", "TrayMessage"))
                                  .arg(categoryName)
                                  .arg(intervalToStr(length)));
        } else if (a == Settings::FinishAction::Sound) {
            mediaPlayer_.setMedia(QUrl::fromLocalFile(settings_.soundFileName()));
            mediaPlayer_.play();
        }
    }
}

void TimerTrack::startTimerSequence() {
    if (activeIntervalInfo_)
        interruptTimer();

    const auto times = sequenceToIntervals(settings_.timerSequence());
    intervals_.clear();
    auto odd = true;
    for (auto t : times) {
        intervals_.push_back({ t, odd });
        odd = !odd;
    }
    if (!intervals_.empty())
        startNextInterval();
}

void TimerTrack::updateContextMenu() {
    popupMenu_.clear();
    interruptAction_ = nullptr;

    const auto* startPattenAction = popupMenu_.addAction(tr("Start sequence"));
    connect(startPattenAction, &QAction::triggered, this, &TimerTrack::startTimerSequence);

    const auto contextMenuIntervals = entriesToIntervals(settingsWindow_.getContextMenuEntries());
    const auto categories = sqlLayer_.readCategories();

    for (const auto& interval : contextMenuIntervals) {
        auto* menu = popupMenu_.addMenu(interval.second);
        for (const auto& category : categories) {
            if (!category.archived_) {
                // Replace ampersand
                auto categoryName = category.name_;
                categoryName.replace("&", "&&");
                auto* action = menu->addAction(categoryName);
                action->setData(interval.first.count());
                action->setIcon(category.createIcon());

                connect(action, &QAction::triggered, this, [&, categoryId = category.id_, timerInterval = interval.first]() {
                    if (activeIntervalInfo_)
                        interruptTimer();
                    intervals_ = decltype(intervals_){{ timerInterval, true }};
                    startNextInterval(categoryId);
                });
            }
        }
    }
    interruptAction_ = popupMenu_.addAction(tr("Interrupt current timer"));
    connect(interruptAction_, &QAction::triggered, this, &TimerTrack::interruptTimer);
    interruptAction_->setDisabled(!timer_.isActive());

    popupMenu_.addSeparator();
    const auto* actionSettings = popupMenu_.addAction(tr("Settings..."));
    const auto* actionStatistics = popupMenu_.addAction(tr("Statistics..."));

    connect(actionSettings, &QAction::triggered, this, [this]() { settingsWindow_.show(); });
    connect(actionStatistics, &QAction::triggered, this, [this]() { statisticsWindow_.show(); });

    popupMenu_.addSeparator();
    const auto* actionExit = popupMenu_.addAction(tr("Exit"));
    connect(actionExit, &QAction::triggered, this, []() { QApplication::quit(); });
}

void TimerTrack::startNextInterval(std::optional<int> categoryId) {
    const auto curInterval = intervals_.front();
    if (!categoryId)
        categoryId = curInterval.isOdd ? settings_.defaultCategoryId() : sqlLayer_.restingCategoryId();

    qDebug(QString("Starting interval of %1ms of category id = %2").arg(curInterval.interval.count()).arg(*categoryId).toStdString().c_str());

    Record r;
    r.category_ = *categoryId;
    r.status_ = Record::Status::Started;
    r.startTime_ = std::chrono::system_clock::now();
    r.plannedTime_ = curInterval.interval;
    r.passedTime_ = std::chrono::milliseconds{ 0 };

    const auto recordId = sqlLayer_.addRecord(r);

    startTimer({ recordId, sqlLayer_.getCategory(*categoryId).name_, curInterval.interval });
    intervals_.erase(begin(intervals_));
}

void TimerTrack::interruptTimer() {
    if (!activeIntervalInfo_)
        return;

    sqlLayer_.interruptRecord(activeIntervalInfo_->recordId);
    stopTimer();
}

void TimerTrack::updateLabel() const {
    if (const auto remaining = timer_.remainingTime(); remaining != -1)
        ui.timerLabel->setText(intervalToStr(std::chrono::milliseconds{remaining}));
}

void TimerTrack::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        localMousePos_ = event->globalPos() - pos();

    QMainWindow::mousePressEvent(event);
}

void TimerTrack::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        const auto newPos = event->globalPos() - localMousePos_;
        settings_.setPos(newPos);
        move(newPos);
    }
    QMainWindow::mouseMoveEvent(event);
}
