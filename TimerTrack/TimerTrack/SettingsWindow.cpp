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

    const auto idxOdd = ui.defaultOddCategoryId->findData(settings_.defaultOddCategoryId());
    if (idxOdd != -1)
        ui.defaultOddCategoryId->setCurrentIndex(idxOdd);
    else
        ui.defaultOddCategoryId->setCurrentIndex(0);

    const auto idxEven = ui.defaultEvenCategoryId->findData(settings_.defaultEvenCategoryId());
    if (idxOdd != -1)
        ui.defaultEvenCategoryId->setCurrentIndex(idxOdd);
    else
        ui.defaultEvenCategoryId->setCurrentIndex(0);

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
    connect(ui.defaultOddCategoryId, &QComboBox::currentTextChanged, this, [&]() {
        settings_.setDefaultOddCategoryId(ui.defaultOddCategoryId->currentData().toInt());
    });
    connect(ui.defaultEvenCategoryId, &QComboBox::currentTextChanged, this, [&]() {
        settings_.setDefaultEvenCategoryId(ui.defaultEvenCategoryId->currentData().toInt());
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
            if (sqlLayer_.isCategoryArchived(id)) {
                QMessageBox msg(QMessageBox::Information, "", "Category is archived and cannot be deleted. Some records in the database use it.", QMessageBox::Ok, this);
                msg.exec();
            } else {
                // Check if it is last not archived category
                if (sqlLayer_.checkIdIsLastNotArchived(id)) {
                    // show notification
                    QMessageBox msg(QMessageBox::Information, "", "Unable to delete last active category", QMessageBox::Ok, this);
                    msg.exec();
                } else {
                    if (sqlLayer_.isCategoryUsed(id))
                        sqlLayer_.archiveCategory(id);
                    else
                        sqlLayer_.deleteCategory(id);

                    updateCategories();
                    updateUiToSettings();
                }
            }
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

    connect(ui.addCategoryPickColor, &QPushButton::clicked, this, [&]() {
        QColorDialog dlg;
        if (dlg.exec() == QDialog::DialogCode::Accepted) {
            ui.addCategoryColor->setText(dlg.currentColor().name());
        }
    });

    connect(ui.activateCategory, &QPushButton::clicked, this, [&]() {
        if (const auto* item = ui.categoriesList->currentItem()) {
            const int id = item->data(Qt::UserRole).toInt();
            if (sqlLayer_.isCategoryArchived(id)) {
                sqlLayer_.unarchiveCategory(id);
                updateCategories();
                updateUiToSettings();

                selectCategoryInListById(id);
            }
        }
    });

    connect(ui.updateCategory, &QPushButton::clicked, this, [&]() {
        if (const auto* item = ui.categoriesList->currentItem()) {
            const int id = item->data(Qt::UserRole).toInt();
            sqlLayer_.updateCategory(id, ui.addCategoryName->text(), QColor(ui.addCategoryColor->text()));
            updateCategories();
            updateUiToSettings();

            selectCategoryInListById(id);
        }
    });

    connect(ui.categoriesList, &QListWidget::currentItemChanged, this, [&]() {
        if (const auto* item = ui.categoriesList->currentItem()) {
            const int id = item->data(Qt::UserRole).toInt();
            const auto categories = sqlLayer_.readCategories();
            const auto c = std::find_if(begin(categories), end(categories), [id](const auto& cat) {
                return cat.id_ == id;
            });
            if (c != end(categories)) {
                ui.addCategoryName->setText(c->name_);
                ui.addCategoryColor->setText(c->color_.name());

                ui.activateCategory->setEnabled(c->archived_);
            }
            ui.delCategory->setEnabled(true);
            ui.updateCategory->setEnabled(true);
            
        } else {
            ui.delCategory->setEnabled(false);
            ui.updateCategory->setEnabled(false);
            ui.activateCategory->setEnabled(false);
        }
    });

    connect(ui.browseSoundFile, &QPushButton::clicked, this, [&]() {
        QFileDialog dlg(this, "Select file name", QString(), "Sound files (*.wav *.mp3)");
        dlg.setFileMode(QFileDialog::ExistingFile);
        dlg.setDirectory(QDir().filePath(settings_.soundFileName()));
        if (dlg.exec() == QDialog::DialogCode::Accepted) {
            const auto f = dlg.selectedFiles()[0];
            ui.soundFileName->setText(f);
            settings_.setSoundFileName(f);
        }
    });
}

void SettingsWindow::selectCategoryInListById(int id) const {
    for (auto i = 0; i < ui.categoriesList->count(); ++i) {
        if (ui.categoriesList->item(i)->data(Qt::UserRole).toInt() == id) {
            ui.categoriesList->setCurrentItem(ui.categoriesList->item(i));
            break;
        }
    }
}

void SettingsWindow::updateCategories() const {
    const auto blockerOdd = QSignalBlocker(ui.defaultOddCategoryId);
    const auto blockerEven = QSignalBlocker(ui.defaultEvenCategoryId);

    ui.defaultOddCategoryId->clear();
    ui.defaultEvenCategoryId->clear();
    ui.categoriesList->clear();

    const auto categories = sqlLayer_.readCategories();
    for (const auto& c : categories) {
        //if (!c.archived_)
        {
            if (!c.archived_) {
                ui.defaultOddCategoryId->addItem(c.name_, QVariant(c.id_));
                const auto idxOdd = ui.defaultOddCategoryId->count() - 1;
                ui.defaultOddCategoryId->setItemIcon(idxOdd, c.createIcon());
                //if (c.archived_) ui.defaultCategoryId->setItemData(idx, QBrush(Qt::gray), Qt::TextColorRole);

                ui.defaultEvenCategoryId->addItem(c.name_, QVariant(c.id_));
                const auto idxEven = ui.defaultEvenCategoryId->count() - 1;
                ui.defaultEvenCategoryId->setItemIcon(idxEven, c.createIcon());
            }

            QListWidgetItem* item = new QListWidgetItem(c.createIcon(), c.name_);
            item->setData(Qt::UserRole, QVariant(c.id_));
            if (c.archived_) item->setTextColor(Qt::gray);
            ui.categoriesList->addItem(item);
        }
    }
}

QString SettingsWindow::getContextMenuEntries() const {
    return settings_.contextMenuEntries();
}
