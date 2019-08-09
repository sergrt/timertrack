#pragma once

#include <QWidget>
#include "ui_StatisticsWindow.h"

class StatisticsWindow : public QWidget
{
    Q_OBJECT

public:
    StatisticsWindow(QWidget *parent = Q_NULLPTR);
    ~StatisticsWindow();

private:
    Ui::StatisticsWindow ui;
};
