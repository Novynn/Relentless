#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QSettings>
#include <QElapsedTimer>
#include <QDir>
#include "logger.h"
#include "shared/MessageOrigin.h"

#include <iostream>

using namespace std;

class ClientCore;
class GameCore;

class Core : public QObject
{
    Q_OBJECT
    Q_ENUMS(MessageType)

    friend class CoreObject;
public:
    explicit Core(QObject *parent = 0);
    ~Core();

    static QString getClientConfigPath(){
        return QDir::currentPath().append("/clients/");
    }

    enum MessageType {
        MESSAGE_TYPE_DEFAULT,
        MESSAGE_TYPE_INFO,
        MESSAGE_TYPE_WARNING,
        MESSAGE_TYPE_ERROR
    };

    struct QueuedMessage {
        QString message;
        MessageOrigin origin;
        MessageType messageType;
    };
    
    inline void print(QString message, MessageOrigin origin = MessageOrigin("Core")){
        printMessage(message, origin, MESSAGE_TYPE_DEFAULT);
    }

    inline void info(QString message, MessageOrigin origin = MessageOrigin("Core")){
        printMessage(message, origin, MESSAGE_TYPE_INFO);
    }

    inline void warning(QString message, MessageOrigin origin = MessageOrigin("Core")){
        printMessage(message, origin, MESSAGE_TYPE_WARNING);
    }

    inline void error(QString message, MessageOrigin origin = MessageOrigin("Core")){
        printMessage(message, origin, MESSAGE_TYPE_ERROR);
    }


    void printMessage(QString message,
               MessageOrigin origin = MessageOrigin("Core"),
               MessageType messageType = MESSAGE_TYPE_DEFAULT);

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
