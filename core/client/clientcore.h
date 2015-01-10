#ifndef CLIENTCORE_H
#define CLIENTCORE_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QString>
#include <QTemporaryFile>
#include <QCryptographicHash>
#include "coreobject.h"
#include <QQueue>

class Client;
class Packet;

class ClientCore : public CoreObject
{
    Q_OBJECT
public:
    explicit ClientCore(Core *parent = 0);

    void load();

    const QString alias() {
        return "CC";
    }

    void clientOutput(Client* client, QString message, MessageType type = MessageType::Default);
    
    QString clientConfigPath;
    bool removeClient(const QString clientAlias);
    bool removeClient(Client *client);
    bool newClient(const QString configFile);
    bool hasClient(const QString clientAlias);
    bool hasClient(Client *client);
    Client *getClient(const QString clientAlias);
    QStringList getClientAliases();

    static QString getClientConfigPath(){
        return QDir::currentPath().append("/clients/");
    }

    int getClientsCount(){
        return clients.count();
    }

    void sendConnectSignal();
    void sendDisconnectSignal();
    void loadClients();
    void unloadClients();

    void unloadClient(Client *client);

    QString parseConfigFile(const QString configFile);
    QHash<QString, QString> getBlocks(const QString fileName, const QString parentName = QString());
    void linkClientEvents(Client *client);
    void unlinkClientEvents(Client *client);
    void emitEvent(Client *client, QString event, QVariantHash data = QVariantHash());

    QQueue<QVariantHash> getEventLog() {
        return eventLog;
    }

private:
    QHash<QString, Client*> clients;
    quint16 maxClients;
    bool multiClientMode;

    QQueue<QVariantHash> eventLog;
public slots:
    void clientRequest(QString event, QString id, QVariantHash data);
signals:
    void clientEvent(QString event, QString id, QVariantHash data = QVariantHash());
};

#endif // CLIENTCORE_H
