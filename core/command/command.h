#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QStringList>
#include "commandengine.h"

class CommandEngine;
class CoreObject;
class CommandInput;

class Command : public QObject
{
    Q_OBJECT
public:
    Command(CommandEngine* parent);

    void setCommand(const QString command){
        mCommand = command;
    }

    const QString command(){
        return mCommand;
    }

    void setOwner(CoreObject* owner){
        mOwner = owner;
    }

    CoreObject* owner(){
        return mOwner;
    }

    void execute(CommandInput*);

private:
    CommandEngine* mEngine;

    QString mCommand;

    CoreObject* mOwner;

signals:
    void executed(CommandInput*);
};

#endif // COMMAND_H
