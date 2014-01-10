#ifndef COREOBJECT_H
#define COREOBJECT_H

#include <QObject>
#include "core.h"

class CoreObject : public QObject
{
    Q_OBJECT
public:
    CoreObject();
    explicit CoreObject(Core *parent = 0);

    virtual void load() = 0;
    virtual const QString alias() = 0;

    Core* core(){
        return mCore;
    }

    inline void print(QString message){
        core()->print(message, MessageOrigin(alias()));
    }

    inline void info(QString message){
        core()->info(message, MessageOrigin(alias()));
    }

    inline void warning(QString message){
        core()->warning(message, MessageOrigin(alias()));
    }

    inline void error(QString message){
        core()->error(message, MessageOrigin(alias()));
    }

    inline void printMessage(QString message, Core::MessageType messageType = Core::MESSAGE_TYPE_DEFAULT){
        core()->printMessage(message, MessageOrigin(alias()), messageType);
    }

    inline void log(QString message){
        Logger::instance()->log(message, MessageOrigin(alias()));
    }

private:
    Core* mCore;
};

#endif // COREOBJECT_H
