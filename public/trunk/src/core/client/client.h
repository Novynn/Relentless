#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QMetaEnum>
#include <QTimer>
#include <QQueue>
#include "clientcore.h"
#include "game/game.h"
#include "clientprotocol.h"

#include "shared/packet/packet.h"

class ClientCore;
class ClientProtocol;
class Game;

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(const QString identifier, ClientCore *parent = 0, QSettings *set = 0);

    enum Status {
        CLIENT_DISABLED,
        CLIENT_IDLE,
        CLIENT_DISCONNECTED,
        CLIENT_CONNECTING,
        CLIENT_AUTHENTICATING,
        CLIENT_CONNECTED,
        CLIENT_CONNECTED_HOSTING
    };

    Q_ENUMS(Status)

    static QString statusToString(Status status){
        QMetaObject metaObject = Client::staticMetaObject;
        QMetaEnum metaEnum = metaObject.enumerator(metaObject.indexOfEnumerator("Status"));
        return QString(metaEnum.valueToKey(status));
    }

    Status status() const {return mStatus;}
    void setStatus(Status s){
        //info(QString("[%2] => [%1]").arg(statusToString(s)).arg(statusToString(mStatus)));
        mStatus = s;
    }

    Q_INVOKABLE const QString getIdentifier(){return mIdentifier;}
    void setIdentifier(const QString identifier){mIdentifier = identifier;}
    QString mIdentifier;

    const QString username(){return mUsername;}
    const QString realm(){return mRealm;}

    bool load();
    void unload();
    bool connect();
    void disconnect();

    void bncsConnect();
    void handlePackets();

    void print(QString message);
    void info(QString message);
    void warning(QString message);
    void error(QString message);
    void printMessage(QString message, Core::MessageType messageType = Core::MESSAGE_TYPE_DEFAULT);

    void command(QString message);

    void beginHosting(Game *game);
    void hostRefresh();
    void endHosting();
private:
    // BNLS Packet handling
    void Recv_BNLS_CHOOSENLSREVISION(QByteArrayBuilder b);
    void Recv_BNLS_CDKEY_EX(QByteArrayBuilder b);
    void Recv_BNLS_VERSIONCHECKEX2(QByteArrayBuilder b);
    void Recv_BNLS_LOGONCHALLENGE(QByteArrayBuilder b);
    void Recv_BNLS_LOGONPROOF(QByteArrayBuilder b);

    // BNCS Packet handling
    void Recv_SID_PING(QByteArrayBuilder b);
    void Recv_SID_AUTH_INFO(QByteArrayBuilder b);
    void Recv_SID_AUTH_CHECK(QByteArrayBuilder b);
    void Recv_SID_ACCOUNT_LOGON(QByteArrayBuilder b);
    void Recv_SID_ACCOUNTLOGONPROOF(QByteArrayBuilder b);
    void Recv_SID_ENTERCHAT(QByteArrayBuilder b);
    void Recv_SID_CHATEVENT(QByteArrayBuilder b);
    void Recv_SID_STARTADVEX3(QByteArrayBuilder b);
    //

    void send(Packet* p);
    bool sendImmediately(Packet* p);
    int getDelay(int size);
    int getProjectedDelay(int size);

    int queueCredits;
    int queueStartingCredits;
    int queueThreshholdBytes;
    int queueCostPerByte;
    int queueCostPerPacket;
    int queueCostPerByteOverThreshhold;
    int queueMaxCredits;
    int queueCreditRate;
    qint64 queueTimeLastSent;

private:
    ClientCore* clientCore;
    ClientProtocol* clientProtocol;

    QTcpSocket *bncs;
    QSslSocket *bnls;
    QQueue<Packet*> incomingDataQueue;
    QQueue<Packet*> outgoingDataQueue;
    QTimer* queueTimer;

    // + GAME HOSTING
    Game* currentGame;
    bool refreshDue;
    // - GAME HOSTING

    QSettings* settings;

    qint64 loginStart;

    QString currentChannel;
    QString mRealm;
    bool mExpansion;
    uint mVersionHash;
    QString mCheckString;
    uint mVersionCode;

    QString bnlsHost;
    int bnlsPort;
    bool bnlsSecure;

    QString bncsHost;
    int bncsPort;

    QString mKey;
    QString mKeyEx;
    QString mUsername;
    QString mPassword;

    quint32 mServerToken;
    quint32 mClientToken;
    int pingMode;

    Status mStatus;
signals:
    void eventConnecting();
    void eventConnected();
    void eventLogin();
    void eventError();
    void eventWarning();
    void eventInfo();
    void eventDisconnected();


    void eventIncomingData();
    void eventOutgoingData();

    void eventInitializing();
    void eventTerminating();

    void eventServerInfo();
    void eventServerError();
    void eventUserTalk();
    void eventUserEmote();
    void eventUserWhisper();
    void eventUserJoins();
    void eventUserLeaves();
    void eventUserInChannel();
    void eventFlagUpdate();
    void eventChannelJoin(QString);
    void eventChannelLeave();
    void eventMessagePrepared(); // "PressedEnter"
    void eventMessageSent();
    void eventMessageQueued();
    void eventClanInfo();
    void eventClanMemberList();
    void eventClanMemberUpdate();
    void eventClanMemberLeaves();
    void eventClanMOTD();
    void eventClanCandidateList();
    void eventClanPromoteUserReply();
    void eventClanDemoteUserReply();
    void eventClanRemoveUserReply();
    void eventClanDisbandReply();
    void eventClanInviteUserReply();
    void eventClanInvitation();
    void eventBotClanInfo();
    void eventBotRemovedFromClan();
    void eventBotClanRankChanged();
    void eventBotJoinedClan();
    void eventChannelList();

public slots:
    void readySend();
    void bncsConnected();
    void bncsDisconnected();
    void bncsReadyRead();

    void bnlsReady();
    void bnlsConnected();
    void bnlsDisconnected();
    void bnlsReadyRead();
    void bnlsSSLError(const QList<QSslError> &);

    void incomingPacket(Packet*);
    void outgoingPacket(Packet*);

    void channelJoin(QString);

    void loginStarted();
    void loginFinished();
};

Q_DECLARE_METATYPE(Client*)

#endif // CLIENT_H
