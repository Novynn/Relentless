#ifndef CLIENTCORE_H
#define CLIENTCORE_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QString>
#include <QTemporaryFile>
#include <QCryptographicHash>
#include "coreobject.h"

class Client;

class ClientCore : public CoreObject
{
    Q_OBJECT
public:
    explicit ClientCore(Core *parent = 0);

    void load();

    const QString alias() {
        return "CC";
    }

    void clientOutput(QString message, QString identifier, Core::MessageType type = Core::MESSAGE_TYPE_DEFAULT);
    
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
private:
    QHash<QString, Client*> clients;
    quint16 maxClients;
    bool multiClientMode;

signals:
    void on_newClient(QString);
};

#endif // CLIENTCORE_H
