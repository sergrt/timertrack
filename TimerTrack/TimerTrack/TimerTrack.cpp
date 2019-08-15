#include "stdafx.h"
#include "TimerTrack.h"
#include "Intervals.h"
#include "Record.h"

static const auto defaultTimerLabelText = QString("--:--");

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

    labelTimer_.setInterval(1000);
    connect(&labelTimer_, &QTimer::timeout, this, &TimerTrack::updateLabel);
}

TimerTrack::~TimerTrack() {
    interruptTimer();
}

void TimerTrack::setupTrayIcon() {
    trayIcon_.setIcon(QIcon(":/TimerTrack/stopwatch.png"));
    trayIcon_.setToolTip("TimerTrack");
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

void TimerTrack::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        if (isHidden())
            show();
        else
            hide();
    }
}

void TimerTrack::startTimer(std::chrono::milliseconds interval, int recordId) {
    ui.timerLabel->setText(intervalToStr(interval));
    labelTimer_.start();

    timer_.start(interval.count());
    activeRecord_ = recordId;
    interruptAction_->setDisabled(false);
}

void TimerTrack::stopTimer() {
    timer_.stop();
    ui.timerLabel->setText(defaultTimerLabelText);

    labelTimer_.stop();
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
        startNextInterval();
}

void TimerTrack::executeFinishActions() {
    const auto finishActions = settings_.finishActions();
    for (const auto& a : finishActions) {
        if (a == Settings::FinishAction::Popup) {
            auto* m = new QMessageBox(QDesktopWidget().screen());
            m->setIcon(QMessageBox::Information);
            m->setFont(this->font());
            m->setAttribute(Qt::WA_DeleteOnClose, true);
            m->setWindowFlags(m->windowFlags() | Qt::WindowStaysOnTopHint | Qt::Tool);
            m->setWindowTitle("Finished");
            m->setText("Interval finished");
            m->show();
        } else if (a == Settings::FinishAction::Tooltip) {
            trayIcon_.showMessage("Finished", "Interval finished");
        } else if (a == Settings::FinishAction::Sound) {
            mediaPlayer_.setMedia(QUrl::fromLocalFile(settings_.soundFileName()));
            mediaPlayer_.play();
        }
    }
}

void TimerTrack::startTimerSequence() {
    if (activeRecord_)
        interruptTimer();

    const auto times = sequenceToIntervals(settings_.timerSequence());
    intervals_.clear();
    auto odd = true;
    for (auto t : times) {
        intervals_.emplace_back(t, odd);
        odd = !odd;
    }
    if (!intervals_.empty())
        startNextInterval();
}

void TimerTrack::updateContextMenu() {
    popupMenu_.clear();
    interruptAction_ = nullptr;

    const auto* startPattenAction = popupMenu_.addAction("Start sequence");
    connect(startPattenAction, &QAction::triggered, this, &TimerTrack::startTimerSequence);

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
                    startNextInterval(categoryId);
                });
            }
        }
    }
    interruptAction_ = popupMenu_.addAction("Interrupt current timer");
    connect(interruptAction_, &QAction::triggered, this, &TimerTrack::interruptTimer);
    interruptAction_->setDisabled(!timer_.isActive());

    popupMenu_.addSeparator();
    const auto* actionSettings = popupMenu_.addAction("Settings...");
    const auto* actionStatistics = popupMenu_.addAction("Statistics...");

    connect(actionSettings, &QAction::triggered, this, [this]() { settingsWindow_.show(); });
    connect(actionStatistics, &QAction::triggered, this, [this]() { statisticsWindow_.show(); });

    popupMenu_.addSeparator();
    const auto* actionExit = popupMenu_.addAction("Exit");
    connect(actionExit, &QAction::triggered, this, []() { QApplication::quit(); });
}

void TimerTrack::startNextInterval(std::optional<int> categoryId) {
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

void TimerTrack::updateLabel() const {
    if (auto interval = strToInterval(ui.timerLabel->text())) {
        *interval -= std::chrono::seconds{ 1 };
        ui.timerLabel->setText(intervalToStr(*interval));
    }
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
