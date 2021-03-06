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
        LEAVING,
        UNKNOWN
    };

    Player(GameCore *parent = 0);
    Player(Game *game);
    
    QTcpSocket* socket;
    bool initialize(int socketDescriptor);
    void addChat(QString s, MessageType type = MessageType::Default);

    QString name(){return mPlayerName;}

    State state(){return mState;}
    void attachToGame(Game* game){
        mState = ATTACHED;
        mGame = game;
    }
    void setLeaving() {
        mState = LEAVING;
    }

    void setPlayerId(unsigned char pId){
        mPId = pId;
    }

    unsigned char playerId(){
        return mPId;
    }

    void sendPacket(Packet *packet);
    void Recv_W3GS_LEAVEREQ(QByteArrayBuilder b);
    void Send_W3GS_LEAVERS();
    void Recv_W3GS_REQJOIN(QByteArrayBuilder b);
    void close();
private:
    QQueue<Packet*> packetQueue;
    QString mPlayerName;

    GameCore* gameCore;

    void extractPackets();
    void handlePackets();
    bool assignLength(QByteArray &content, int offset = 2) const;


    bool isVirtual;

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
