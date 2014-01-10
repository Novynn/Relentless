#ifndef COMMANDENGINE_H
#define COMMANDENGINE_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QDebug>
#include <QTime>
#include <QQueue>
#include <QTimer>
#include "core.h"

class Command;
class CommandInput;

class Core;

class CommandEngine : public QObject
{
    Q_OBJECT
public:
    explicit CommandEngine(Core *parent = 0);
    
    Command* newCommand(CoreObject *owner, QString command);
    Command* getCommand(QString command);
    bool processInput(CoreObject *from, QString fromCookie, QString who, QString input);
    bool hasCommand(QString command);

    void queueCommandInput(CommandInput *input);
    Core* core(){return mCore;}
private:
    QHash<QString, Command*> commands;
    QQueue<CommandInput*> commandQueue;

    QTimer* queueTick;

    Core* mCore;
signals:
    
public slots:
    void handleQueue();
};

#endif // COMMANDENGINE_H
