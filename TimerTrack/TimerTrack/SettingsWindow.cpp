#include "stdafx.h"
#include "SettingsWindow.h"
#include "SqlLayer.h"

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent) {

    ui.setupUi(this);
    updateCategories();
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

    const auto idx = ui.defaultCategoryId->findData(settings_.defaultCategoryId());
    ui.defaultCategoryId->setCurrentIndex(idx);
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
    connect(ui.defaultCategoryId, &QComboBox::currentTextChanged, this, [&]() {
        settings_.setDefaultCategoryId(ui.defaultCategoryId->currentData().toInt());
    });

    connect(ui.addCategory, &QPushButton::clicked, this, [&]() {
        Category c;
        c.name_ = ui.addCategoryName->text();
        c.color_ = QColor(ui.addCategoryColor->text());
        SqlLayer s;
        s.addCategory(c);
        updateCategories();
        updateUiToSettings();
    });

    connect(ui.delCategory, &QPushButton::clicked, this, [&]() {
        if (const auto* item = ui.categoriesList->currentItem()) {
            const int id = item->data(Qt::UserRole).toInt();
            SqlLayer s;
            s.deleteCategory(id);
            updateCategories();
            updateUiToSettings();
        }
    });
}

void SettingsWindow::updateCategories() const {
    ui.defaultCategoryId->blockSignals(true);
    ui.defaultCategoryId->clear();
    ui.categoriesList->clear();

    // TODO: Subject to refactor
    // There should be dependency injection - SqlLayer should be injected here
    SqlLayer s;
    const auto c = s.readCategories();
    for (const auto& i : c) {
        ui.defaultCategoryId->addItem(i.name_, QVariant(i.id_));
        const auto idx = ui.defaultCategoryId->count() - 1;
        QPixmap p(128, 128);
        p.fill(i.color_);
        QIcon icon(p);
        ui.defaultCategoryId->setItemIcon(idx, icon);

        QListWidgetItem* item = new QListWidgetItem(icon, i.name_);
        item->setData(Qt::UserRole, QVariant(i.id_));
        ui.categoriesList->addItem(item);
    }
    ui.defaultCategoryId->blockSignals(false);
}

