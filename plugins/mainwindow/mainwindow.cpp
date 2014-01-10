#include "mainwindow.h"
#include <QCloseEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
}

MainWindow::~MainWindow(){

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    emit quit();
    event->ignore();
}
