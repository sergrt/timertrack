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
    updateUiToDatabase();
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
    ui.defaultCategoryId->setCurrentIndex(idx == -1 ? 0 : idx);
    ui.contextMenuEntries->setText(settings_.contextMenuEntries());
}

// Update supplied 'lineEdit' palette depending of the validity of the content,
// checked as time entries.
void paintToContentValidity(QLineEdit& lineEdit) {
    const auto text = lineEdit.text();
    QPalette p;
    if (validateTimeEntries(text))
        p.setColor(QPalette::Text, Qt::black);
    else
        p.setColor(QPalette::Text, Qt::red);

    lineEdit.setPalette(p);
}

void SettingsWindow::setupUiSettingsHandlers() {
    connect(ui.timerPattern, &QLineEdit::textChanged, this, &SettingsWindow::timerPatternChanged);
    connect(ui.defaultCategoryId, &QComboBox::currentTextChanged, this, [&]() {
        settings_.setDefaultCategoryId(ui.defaultCategoryId->currentData().toInt());
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
    connect(ui.browseSoundFile, &QPushButton::clicked, this, &SettingsWindow::browseSoundFile);

    connect(ui.contextMenuEntries, &QLineEdit::textChanged, this, &SettingsWindow::contextMenuItemsChanged);

    connect(ui.addCategory, &QPushButton::clicked, this, &SettingsWindow::addCategory);
    connect(ui.activateCategory, &QPushButton::clicked, this, &SettingsWindow::activateCategory);
    connect(ui.updateCategory, &QPushButton::clicked, this, &SettingsWindow::updateCategory);
    connect(ui.delCategory, &QPushButton::clicked, this, &SettingsWindow::deleteCategory);
    connect(ui.addCategoryPickColor, &QPushButton::clicked, this, &SettingsWindow::pickCategoryColor);
    connect(ui.categoriesList, &QListWidget::currentItemChanged, this, &SettingsWindow::categoryListSelChanged);
}

void SettingsWindow::selectCategoryInListById(int id) const {
    for (auto i = 0; i < ui.categoriesList->count(); ++i) {
        if (ui.categoriesList->item(i)->data(Qt::UserRole).toInt() == id) {
            ui.categoriesList->setCurrentItem(ui.categoriesList->item(i));
            return;
        }
    }
    ui.categoriesList->setCurrentItem(nullptr);
}

void SettingsWindow::updateUiToDatabase() const {
    reloadCategories();
}

void SettingsWindow::reloadCategories() const {
    const auto blockerOdd = QSignalBlocker(ui.defaultCategoryId);

    ui.defaultCategoryId->clear();
    ui.categoriesList->clear();

    const auto categories = sqlLayer_.readCategories();
    for (const auto& c : categories) {
        if (!c.archived_ && c.role_ != Category::Role::Resting) {
            ui.defaultCategoryId->addItem(c.name_, QVariant(c.id_));
            const auto idx = ui.defaultCategoryId->count() - 1;
            ui.defaultCategoryId->setItemIcon(idx, c.createIcon());
            //if (c.archived_) ui.defaultCategoryId->setItemData(idx, QBrush(Qt::gray), Qt::TextColorRole);
        }

        auto* item = new QListWidgetItem(c.createIcon(), c.name_);
        item->setData(Qt::UserRole, QVariant(c.id_));
        if (c.archived_)
            item->setTextColor(Qt::gray);
        ui.categoriesList->addItem(item);
    }
}

QString SettingsWindow::getContextMenuEntries() const {
    return settings_.contextMenuEntries();
}

void SettingsWindow::timerPatternChanged() const {
    paintToContentValidity(*ui.timerPattern);
    const auto text = ui.timerPattern->text();
    if (validateTimeEntries(text))
        settings_.setTimerPattern(text);
}

void SettingsWindow::browseSoundFile() {
    QFileDialog dlg(this, "Select file name", QString(), "Sound files (*.wav *.mp3)");
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setDirectory(QDir().filePath(settings_.soundFileName()));
    if (dlg.exec() == QDialog::DialogCode::Accepted) {
        const auto f = dlg.selectedFiles()[0];
        ui.soundFileName->setText(f);
        settings_.setSoundFileName(f);
    }
}

void SettingsWindow::contextMenuItemsChanged() const {
    paintToContentValidity(*ui.contextMenuEntries);
    const auto text = ui.contextMenuEntries->text();
    if (validateTimeEntries(text)) {
        settings_.setContextMenuEntries(text);
        emit contextMenuChanged();
    }
}

void SettingsWindow::addCategory() const {
    const auto category = Category{ 0, ui.addCategoryName->text(), QColor(ui.addCategoryColor->text()) };
    const auto id = sqlLayer_.addCategory(category);
    reloadCategories();
    updateUiToSettings();
    selectCategoryInListById(id);
    emit categoriesChanged();
}

void SettingsWindow::activateCategory() const {
    if (const auto* item = ui.categoriesList->currentItem()) {
        const auto id = item->data(Qt::UserRole).toInt();
        if (sqlLayer_.isCategoryArchived(id)) {
            sqlLayer_.unarchiveCategory(id);
            reloadCategories();
            updateUiToSettings();
            selectCategoryInListById(id);
            emit categoriesChanged();
        }
    }
}

void SettingsWindow::updateCategory() const {
    if (const auto* item = ui.categoriesList->currentItem()) {
        const auto id = item->data(Qt::UserRole).toInt();
        sqlLayer_.updateCategory(id, ui.addCategoryName->text(), QColor(ui.addCategoryColor->text()));
        reloadCategories();
        updateUiToSettings();
        selectCategoryInListById(id);
        emit categoriesChanged();
    }
}

void SettingsWindow::deleteCategory() const {
    if (const auto* item = ui.categoriesList->currentItem()) {
        const auto id = item->data(Qt::UserRole).toInt();
        if (sqlLayer_.isCategoryPersistent(id)) {
            QMessageBox msg(QMessageBox::Information, "", "Category is persistent and cannot be deleted.", QMessageBox::Ok);
            msg.exec();
        } else if (sqlLayer_.isCategoryArchived(id)) {
            QMessageBox msg(QMessageBox::Information, "", "Category is archived and cannot be deleted. Some records in the database use it.", QMessageBox::Ok);
            msg.exec();
        } else {

            if (sqlLayer_.isCategoryUsed(id))
                sqlLayer_.archiveCategory(id);
            else
                sqlLayer_.deleteCategory(id);

            reloadCategories();
            updateUiToSettings();
            selectCategoryInListById(id); // selecting deleted id should produce no selection
            emit categoriesChanged();
        }
    }
}

void SettingsWindow::pickCategoryColor() const {
    QColorDialog dlg;
    if (dlg.exec() == QDialog::DialogCode::Accepted)
        ui.addCategoryColor->setText(dlg.currentColor().name());
}

void SettingsWindow::categoryListSelChanged() const {
    if (const auto* item = ui.categoriesList->currentItem()) {
        const auto id = item->data(Qt::UserRole).toInt();
        const auto categories = sqlLayer_.readCategories();
        const auto c = std::find_if(begin(categories), end(categories), [id](const auto& cat) { return cat.id_ == id; });
        if (c != end(categories)) {
            ui.addCategoryName->setText(c->name_);
            ui.addCategoryColor->setText(c->color_.name());
            ui.activateCategory->setEnabled(c->archived_);
            ui.delCategory->setEnabled(c->role_ != Category::Role::Resting && c->role_ != Category::Role::Default);
            ui.updateCategory->setEnabled(true);
        }
    } else {
        ui.delCategory->setEnabled(false);
        ui.updateCategory->setEnabled(false);
        ui.activateCategory->setEnabled(false);
    }
}
