#pragma once

#include <QWidget>
#include "ui_SettingsWindow.h"
#include "Settings.h"

class SettingsWindow : public QWidget {
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent = Q_NULLPTR);
    ~SettingsWindow();

private:
    Ui::SettingsWindow ui;
    Settings settings_;

    void updateUiToSettings() const;
    void setupUiSettingsHandlers();
};
