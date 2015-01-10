#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QSettings>
#include <QElapsedTimer>
#include <QDir>
#include "logger.h"
#include "shared/MessageOrigin.h"
#include "shared/MessageType.h"

#include <iostream>

using namespace std;

class ClientCore;
class GameCore;

class Core : public QObject
{
    Q_OBJECT

    friend class CoreObject;
public:
    explicit Core(QObject *parent = 0);
    ~Core();

    static QString getClientConfigPath(){
        return QDir::currentPath().append("/clients/");
    }

    struct QueuedMessage {
        QString message;
        MessageOrigin origin;
        MessageType messageType;
    };
    
    inline void print(QString message, MessageOrigin origin = MessageOrigin("Core")){
        printMessage(message, origin, MessageType::Default);
    }

    inline void info(QString message, MessageOrigin origin = MessageOrigin("Core")){
        printMessage(message, origin, MessageType::Info);
    }

    inline void warning(QString message, MessageOrigin origin = MessageOrigin("Core")){
        printMessage(message, origin, MessageType::Warning);
    }

    inline void error(QString message, MessageOrigin origin = MessageOrigin("Core")){
        printMessage(message, origin, MessageType::Error);
    }


    void printMessage(QString message,
               MessageOrigin origin = MessageOrigin("Core"),
               MessageType messageType = MessageType::Default);

    ClientCore* getClientCore(){return clientCore;}
    GameCore* getGameCore(){return gameCore;}
    QSettings* settings(){return globalSettings;}

    quint64 uptime(){return uptimeTimer->elapsed();}
private:
    bool loadSettings();

    QElapsedTimer* uptimeTimer;

    ClientCore* clientCore;
    GameCore* gameCore;

    QSettings* globalSettings;

    void output(QString message, MessageOrigin origin, MessageType messageType);

    bool overrideHideInfo;
signals:
    void consoleMessage(QString, MessageOrigin);
};

#endif // CORE_H
