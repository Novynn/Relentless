#ifndef PLUGIN_H
#define PLUGIN_H

#include <QtPlugin>
#include <QObject>
#include <QString>
#include <QJsonObject>
#include "shared/MessageOrigin.h"

class Packet;

class Plugin : public QObject
{
    Q_OBJECT
public:
    virtual ~Plugin(){}

    virtual QString getName() = 0;
    virtual QString getAuthor() = 0;
    virtual QString getDescription() = 0;

    virtual QString getVersion() = 0;

    virtual QString getAbout(){return QString();}
    virtual QString getWebsite(){return QString();}

    virtual void load() = 0;
    virtual void unload() = 0;
public slots:
    void newClient(QString client){Q_UNUSED(client)}
    void consoleMessage(QString message, MessageOrigin origin){Q_UNUSED(message) Q_UNUSED(origin)}
    void eventClientIncomingData(QString client, Packet* packet) {Q_UNUSED(client) Q_UNUSED(packet)}
    void eventClientOutgoingData(QString client, Packet* packet) {Q_UNUSED(client) Q_UNUSED(packet)}
signals:
    void unloading();
};

Q_DECLARE_INTERFACE(Plugin, "com.Relentless.Plugin/1.0")

#endif // PLUGIN_H
