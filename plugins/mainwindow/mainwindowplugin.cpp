#include "mainwindowplugin.h"

void MainWindowPlugin::load(){
    ShowWindow( GetConsoleWindow(), SW_HIDE );
    window = new MainWindow;

    window->tabifyDockWidget(window->clientsDock, window->gamesDock);
    window->setTabPosition(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea, QTabWidget::East);

    window->show();

    connect(window, SIGNAL(quit()), this, SLOT(unloadNow()));
}

void MainWindowPlugin::unloadNow(){
    unload();
}

void MainWindowPlugin::unload(){
    window->close();
    delete window;
    window = 0;
    ShowWindow( GetConsoleWindow(), SW_SHOW );
    emit unloading();
}

void MainWindowPlugin::newClient(QString client){
    QWidget* widget = new QWidget(window);
    QHBoxLayout* hLayout = new QHBoxLayout(widget);
    hLayout->setSpacing(2);
    hLayout->setContentsMargins(0, 0, 0, 0);

    QTextEdit* edit = new QTextEdit(widget);
    edit->setReadOnly(true);

    hLayout->addWidget(edit);


    window->tabWidget->addTab(widget, client);
}

void MainWindowPlugin::consoleMessage(QString message, MessageOrigin origin)
{
    if (origin.length() == 1)
        window->textEdit->append(origin.toString() + "|" + message);
    else {
        if (origin.first() == "CC"){
            for(int i = 0; i < window->tabWidget->count(); i++){
                QString text = window->tabWidget->tabText(i);
                if (text != origin.at(1)) continue;

                QObjectList children = window->tabWidget->widget(i)->children();
                foreach(QObject* child, children){
                    if (QString::fromLatin1(child->metaObject()->className()) != "QTextEdit") continue;

                    QTextEdit* edit = qobject_cast<QTextEdit*>(child);
                    if (edit){
                        edit->append(message);
                        break;
                    }
                }
                break;
            }
        }
    }
}
