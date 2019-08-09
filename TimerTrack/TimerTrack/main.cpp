#include "stdafx.h"
#include "TimerTrack.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    TimerTrack w;
    w.show();
    return a.exec();
}
