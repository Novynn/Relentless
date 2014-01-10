! include( ../plugin.pri ) {
    error( Couldn\'t find plugin.pri! )
}

QT += core widgets

INCLUDEPATH += ../../core/
TARGET = MainWindow

SOURCES +=\
        mainwindow.cpp \
    mainwindowplugin.cpp

HEADERS  += mainwindow.h \
    mainwindowplugin.h

FORMS    += mainwindow.ui
