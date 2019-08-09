#include "stdafx.h"
#include "TimerTrack.h"
#include <QtSql>
#include "Settings.h"
TimerTrack::TimerTrack(QWidget *parent)
    : QMainWindow(parent) {

    ui.setupUi(this);
    connect(ui.actionSettings, &QAction::triggered, this, [this]() {
        settingsWindow_.show();
    });
    connect(ui.actionStatistics, &QAction::triggered, this, [this]() {
        statisticsWindow_.show();
    });

    QSqlDatabase sdb = QSqlDatabase::addDatabase("QSQLITE");
    sdb.setDatabaseName("statistics.sqlite");
    if (sdb.open()) {
        auto query = sdb.exec("CREATE TABLE Categories(id int, Name varchar);");
        qDebug(query.lastError().text().toStdString().c_str());
    }

    Settings s;
    s.save();
}
