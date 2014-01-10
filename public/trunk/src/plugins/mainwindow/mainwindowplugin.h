#ifndef MAINWINDOWPLUGIN_H
#define MAINWINDOWPLUGIN_H

#include "plugins/plugin.h"
#include "mainwindow.h"
#ifdef Q_OS_WIN
#include "windows.h"
#endif

class MainWindowPlugin : public Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.relentless.mainwindow" FILE "mainwindow.json")
    Q_INTERFACES(Plugin)

public:
    QString getName(){return "Featherstone Interface";}
    QString getAuthor(){return "Novynn";}
    QString getDescription(){return "Generic Interface.";}

    QString getVersion(){return "1.0.0";}

    void load();
    void unload();
public slots:
    void newClient(QString client);
    void consoleMessage(QString message, MessageOrigin origin);
    void unloadNow();
private:
    MainWindow* window;
};

#endif // MAINWINDOWPLUGIN_H
