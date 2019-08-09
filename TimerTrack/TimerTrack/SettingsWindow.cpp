#include "stdafx.h"
#include "SettingsWindow.h"

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent) {

    ui.setupUi(this);

    settings_.load();
    updateUiToSettings();
    setupUiSettingsHandlers();
}

SettingsWindow::~SettingsWindow() = default;

void SettingsWindow::updateUiToSettings() const {
    ui.timerPattern->setText(settings_.timerPattern());
    const auto finishActions = settings_.finishActions();
    ui.finishActionPopup->setChecked(finishActions.find(Settings::FinishAction::Popup) != finishActions.end());
    ui.finishActionTooltip->setChecked(finishActions.find(Settings::FinishAction::Tooltip) != finishActions.end());
    ui.finishActionSound->setChecked(finishActions.find(Settings::FinishAction::Sound) != finishActions.end());
    ui.soundFileName->setText(settings_.soundFileName());
    ui.defaultCategoryId->addItem(QString("%1").arg(settings_.defaultCategoryId()));
}

void SettingsWindow::setupUiSettingsHandlers() {
    connect(ui.timerPattern, &QLineEdit::textChanged, this, [&]() {
        const auto s = ui.timerPattern->text();
        QPalette p;
        if (Settings::validateTimerPattern(s)) {
            p.setColor(QPalette::Text, Qt::black);
            settings_.setTimerPattern(s);
        } else {
            p.setColor(QPalette::Text, Qt::red);
        }
        ui.timerPattern->setPalette(p);
    });
    connect(ui.finishActionPopup, &QCheckBox::stateChanged, this, [&]() {
        settings_.updateFinishAction(Settings::FinishAction::Popup, ui.finishActionPopup->isChecked());
    });
    connect(ui.finishActionTooltip, &QCheckBox::stateChanged, this, [&]() {
        settings_.updateFinishAction(Settings::FinishAction::Tooltip, ui.finishActionTooltip->isChecked());
    });
    connect(ui.finishActionSound, &QCheckBox::stateChanged, this, [&]() {
        settings_.updateFinishAction(Settings::FinishAction::Sound, ui.finishActionSound->isChecked());
    });
}
