#pragma once

#include <QWidget>
#include "ui_SettingsWindow.h"
#include "Settings.h"

class SqlLayer;

class SettingsWindow : public QWidget {
    Q_OBJECT

public:
    SettingsWindow(SqlLayer& sqlLayer, QWidget *parent = Q_NULLPTR);
    ~SettingsWindow();

    QString getContextMenuEntries() const;
private:
    Ui::SettingsWindow ui;
    SqlLayer& sqlLayer_;
    Settings settings_;

    void updateUiToSettings() const;
    void setupUiSettingsHandlers();
    void updateCategories() const;
signals:
    void contextMenuChanged();
};
