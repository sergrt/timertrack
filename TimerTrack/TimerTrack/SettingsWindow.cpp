#include "stdafx.h"
#include "SettingsWindow.h"
#include "SqlLayer.h"
#include "Settings.h"
#include "Intervals.h"

SettingsWindow::SettingsWindow(SqlLayer& sqlLayer, Settings& settings, QWidget *parent)
    : QWidget(parent),
    sqlLayer_{ sqlLayer },
    settings_{ settings } {

    ui.setupUi(this);
    updateCategories();
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

    ui.contextMenuEntries->setText(settings_.contextMenuEntries());
}

void SettingsWindow::setupUiSettingsHandlers() {
    connect(ui.timerPattern, &QLineEdit::textChanged, this, [&]() {
        const auto s = ui.timerPattern->text();
        QPalette p;
        if (validateTimeEntries(s)) {
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
        sqlLayer_.addCategory(c);
        updateCategories();
        updateUiToSettings();
    });

    connect(ui.delCategory, &QPushButton::clicked, this, [&]() {
        if (const auto* item = ui.categoriesList->currentItem()) {
            const int id = item->data(Qt::UserRole).toInt();

            if (sqlLayer_.isCategoryUsed(id))
                sqlLayer_.archiveCategory(id);
            else
                sqlLayer_.deleteCategory(id);

            updateCategories();
            updateUiToSettings();
        }
    });

    connect(ui.contextMenuEntries, &QLineEdit::textChanged, this, [&]() {
        const auto s = ui.contextMenuEntries->text();
        QPalette p;
        if (validateTimeEntries(s)) {
            p.setColor(QPalette::Text, Qt::black);
            settings_.setContextMenuEntries(s);
            emit contextMenuChanged();
        } else {
            p.setColor(QPalette::Text, Qt::red);
        }
        ui.contextMenuEntries->setPalette(p);
     });
}

void SettingsWindow::updateCategories() const {
    const auto blocker = QSignalBlocker(ui.defaultCategoryId);

    ui.defaultCategoryId->clear();
    ui.categoriesList->clear();

    const auto categories = sqlLayer_.readCategories();
    for (const auto& c : categories) {
        if (!c.archived_) {
            ui.defaultCategoryId->addItem(c.name_, QVariant(c.id_));
            const auto idx = ui.defaultCategoryId->count() - 1;
            ui.defaultCategoryId->setItemIcon(idx, c.createIcon());
            //if (c.archived_) ui.defaultCategoryId->setItemData(idx, QBrush(Qt::gray), Qt::TextColorRole);

            QListWidgetItem* item = new QListWidgetItem(c.createIcon(), c.name_);
            item->setData(Qt::UserRole, QVariant(c.id_));
            //if (c.archived_) item->setTextColor(Qt::gray);
            ui.categoriesList->addItem(item);
        }
    }
}

QString SettingsWindow::getContextMenuEntries() const {
    return settings_.contextMenuEntries();
}
