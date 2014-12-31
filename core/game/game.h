#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QString>
#include <QTcpServer>
#include "client/client.h"
#include "lobby.h"
#include "instance.h"
#include "player.h"
#include "gamecore.h"
#include "map/map.h"
#include "gameprotocol.h"

class GameCore;
class Player;
class Client;
class Packet;

class Game : public QObject
{
    Q_OBJECT
public:
    explicit Game(GameCore *parent = 0);

    friend class Lobby;

    enum State {
        STATE_IDLE,
        STATE_CLOSED,
        STATE_LOBBY,
        STATE_INSTANCE,
        STATE_FINISHED
    };

    QHash<quint64, Client*> clients(){return mClients;}
    Map* map(){return mMap;}
    Lobby* lobby(){return mLobby;}
    Instance* instance(){return mInstance;}

    QString &hostname(){return mHostname;}

    bool setMap(Map* map);
    bool loadMap(QString mapFile);

    void setName(const QString name){mCurrentName = name;}

    const QString &initialName(){return mInitialName;}
    const QString &name(){return mCurrentName;}
    bool removePlayer(Player *p);
    bool addPlayer(Player *p);
    bool initializeClient(Client *c);
    QList<Client *> availableClients();
    quint64 getHostId(Client *client);
    bool hasHostId(quint64 hostId);
    void upgradeLobbyToInstance();
    void upgradeIdleToLobby();
    QList<quint64> hostIds();

    State state(){return mState;}

    void queuePlayerPacket(Player *player, W3GSPacket *packet);
    void handlePackets();

    int assignPlayerId();

    QList<Player*> players(){
        return mPlayers;
    }

    Player* playerById(int id) {
        for (Player* player : players()) {
            if (player->playerId() == id) {
                return player;
            }
        }
        return 0;
    }

    dword randomSeed(){
        return mRandomSeed;
    }

    SlotMap* slotMap(){
        return mSlotMap;
    }

private:
    GameCore* gameCore;
    QHash<quint64, Client*> mClients;
    Map* mMap;
    Lobby* mLobby;
    Instance* mInstance;

    QTimer* mTickTimer;

    SlotMap* mSlotMap;

    State mState;
    QList<Player*> mPlayers;
    QMultiHash<Player*, W3GSPacket*> queuedPackets;

    QString mInitialName;
    QString mCurrentName;

    QString gameCreator;
    QString gameOwner;

    QString mHostname;
    QString virtualHostName;

    int port;

    dword mRandomSeed;
signals:
    
public slots:
    void tick();
};

#endif // GAME_H
