#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <qendian.h>
#include "game/gamecore.h"
#include "shared/qbytearraybuilder.h"
#include "shared/packet/w3gspacket.h"

class GameCore;
class Game;

class Player : public QObject
{
    Q_OBJECT
public:
    enum State {
        IDLE,
        ATTACHED,
        UNKNOWN
    };
    explicit Player(GameCore *parent = 0);
    
    QTcpSocket* socket;
    bool initialize(int socketDescriptor);
    void addChat(QString s, Core::MessageType type = Core::MESSAGE_TYPE_DEFAULT);

    void Send_W3GS_PING_FROM_HOST();
    void Send_W3GS_REJECTJOIN(quint32 result);

    QString name(){return mPlayerName;}

    State state(){return mState;}
    void attachToGame(Game* game){
        mState = ATTACHED;
        mGame = game;
    }

    void setPlayerId(unsigned char pId){
        mPId = pId;
    }

    unsigned char playerId(){
        return mPId;
    }

    void sendPacket(W3GSPacket *packet);
    void Recv_W3GS_LEAVEREQ(QByteArrayBuilder b);
    void Send_W3GS_LEAVERS();
private:
    QQueue<Packet*> packetQueue;
    QString mPlayerName;

    GameCore* gameCore;

    void extractPackets();
    void handlePackets();
    bool assignLength(QByteArray &content, int offset = 2) const;


    void Recv_W3GS_REQJOIN(QByteArrayBuilder b);

    unsigned char mPId;

    // GAME

    State mState;
    Game* mGame;

signals:
    void joinRequest(Player*, quint64);
public slots:
    void disconnected();
    void readyRead();
};

#endif // PLAYER_H
