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
    void linkClientEvents(Client *client);
    void unlinkClientEvents(Client *client);
private:
    QHash<QString, Client*> clients;
    quint16 maxClients;
    bool multiClientMode;

signals:
    void eventNewClient(QString id);
    void eventClientConnecting(QString id);
    void eventClientConnected(QString id);
    void eventClientLoggingIn(QString id);
    void eventClientLoggedIn(QString id);
    void eventClientEnteredChat(QString id, QString username);
    void eventClientError(QString id);
    void eventClientWarning(QString id);
    void eventClientInfo(QString id);
    void eventClientDisconnected(QString id);


    void eventClientIncomingData(QString id, Packet* p);
    void eventClientOutgoingData(QString id, Packet* p);

    void eventClientInitializing(QString id);
    void eventClientTerminating(QString id);

    void eventClientServerInfo(QString id, QString username, QString text);
    void eventClientServerError(QString id, QString username, QString text);
    void eventClientUserTalk(QString id, QString channel, QString username, QString text);
    void eventClientUserEmote(QString id, QString channel, QString username, QString text);
    void eventClientUserWhisper(QString id, QString username, QString text);
    void eventClientWhisperSent(QString id, QString username, QString text);
    void eventClientUserJoins(QString id, QString channel, QString username, QString flags);
    void eventClientUserLeaves(QString id, QString channel, QString username, QString flags);
    void eventClientUserInChannel(QString id, QString channel, QString username, QString flags);
    void eventClientFlagUpdate(QString id, QString channel, QString username, QString flags);
    void eventClientChannelJoin(QString id, QString joiningChannel);
    void eventClientChannelLeave(QString id, QString leavingChannel);
    void eventClientMessagePrepared(QString id); // "PressedEnter"
    void eventClientMessageSent(QString id);
    void eventClientMessageQueued(QString id);
    void eventClientClanInfo(QString id);
    void eventClientClanMemberList(QString id);
    void eventClientClanMemberUpdate(QString id);
    void eventClientClanMemberLeaves(QString id);
    void eventClientClanMOTD(QString id);
    void eventClientClanCandidateList(QString id);
    void eventClientClanPromoteUserReply(QString id);
    void eventClientClanDemoteUserReply(QString id);
    void eventClientClanRemoveUserReply(QString id);
    void eventClientClanDisbandReply(QString id);
    void eventClientClanInviteUserReply(QString id);
    void eventClientClanInvitation(QString id);
    void eventClientBotClanInfo(QString id);
    void eventClientBotRemovedFromClan(QString id);
    void eventClientBotClanRankChanged(QString id);
    void eventClientBotJoinedClan(QString id);
    void eventClientChannelList(QString id);
};

#endif // CLIENTCORE_H
