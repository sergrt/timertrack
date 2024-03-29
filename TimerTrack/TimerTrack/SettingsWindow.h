#pragma once

#include <QWidget>
#include "ui_SettingsWindow.h"

class SqlLayer;
class Settings;

class SettingsWindow : public QWidget {
    Q_OBJECT

public:
    SettingsWindow(SqlLayer& sqlLayer, Settings& settings, QWidget *parent = Q_NULLPTR);
    ~SettingsWindow();
    SettingsWindow(const SettingsWindow&) = delete;
    SettingsWindow(SettingsWindow&&) = delete;
    SettingsWindow& operator=(const SettingsWindow&) = delete;
    SettingsWindow& operator=(SettingsWindow&&) = delete;

    QString getContextMenuEntries() const;

private:
    Ui::SettingsWindow ui;
    SqlLayer& sqlLayer_;
    Settings& settings_;

    void updateUiToSettings() const;
    void setupUiSettingsHandlers();
    void updateUiToDatabase() const;
    void reloadCategories() const;
    void selectCategoryInListById(int id) const;

signals:
    void contextMenuChanged() const;
    void categoriesChanged() const;

public slots:
    void browseSoundFile();
    void timerSequenceChanged() const;
    void contextMenuItemsChanged() const;
    void addCategory() const;
    void activateCategory() const;
    void updateCategory() const;
    void deleteCategory() const;
    void pickCategoryColor() const;
    void categoryListSelChanged() const;
};
