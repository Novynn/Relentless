#include <qsystemdetection.h>
#ifdef Q_OS_WIN
#include <QApplication>
#else
#include <QCoreApplication>
#endif

#include "core.h"

int main(int argc, char *argv[]){
#ifdef Q_OS_WIN
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
#else
    QCoreApplication a(argc, argv);
#endif

    Core c;
    return a.exec();
}
