#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QMetaEnum>
#include <QTimer>
#include <QQueue>
#include <QSslSocket>
#include <QSslError>
#include "clientcore.h"
#include "game/game.h"
#include "clientprotocol.h"
#include "friend.h"
#include "shared/packet/packet.h"
#include <client/bncsnlshandler.h>

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
        CLIENT_CONNECTED_BEGIN_HOSTING,
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

    const QString username(){return mUsername;}
    const QString realm(){return mRealm;}

    virtual bool load();
    virtual void unload();
    virtual bool connectClient();
    virtual void disconnectClient();

    virtual void bncsConnect();
    virtual void handleIncomingPackets();
    virtual void handleOutgoingPackets();

    BNCSNLSHandler* loginHandler;

    QSettings* getSettings() {
        return settings;
    }

    void print(QString message);
    void info(QString message);
    void warning(QString message);
    void error(QString message);
    void printMessage(QString message, MessageType messageType = MessageType::Default);

    void command(QString message);

    void beginHosting(Game *game);
    void hostRefresh();
    void endHosting();
    void RequestGameList();
    void Recv_SID_CLANINFO(QByteArrayBuilder b);
    virtual bool validateSettings();
    virtual bool loadSettings();
//protected:
    void emitEvent(QString event, QVariantHash data = QVariantHash());
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
    void Recv_SID_GETADVLISTEX(QByteArrayBuilder b);
    void Recv_SID_FRIENDSLIST(QByteArrayBuilder b);
    //
    QString mIdentifier;

    virtual void send(Packet* p);
    bool sendImmediately(Packet* p);
    virtual int getDelay(int size, bool free=false);

    int queueCredits;
    int queueStartingCredits;
    int queueThreshholdBytes;
    int queueCostPerByte;
    int queueCostPerPacket;
    int queueCostPerByteOverThreshhold;
    int queueMaxCredits;
    int queueCreditRate;
    qint64 queueTimeLastSent;

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

    QList<Friend*> _friends;
    QTimer* _friendsUpdateTimer;

    virtual void handleIncomingPacket(Packet *p);
public slots:
    void readySend();
    virtual void bncsConnected();
    void bncsDisconnected();
    void bncsReadyRead();

    void bnlsReady();
    void bnlsConnected();
    void bnlsDisconnected();
    void bnlsReadyRead();
    void bnlsSSLError(const QList<QSslError> &);

    virtual void incomingPacket(Packet*);
    virtual void outgoingPacket(Packet*);

    void channelJoin(QString);

    void loginStarted();
    void loginFinished();
protected:
    virtual void startAuthentification();
};

Q_DECLARE_METATYPE(Client*)

#endif // CLIENT_H
