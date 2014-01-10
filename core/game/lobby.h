#ifndef LOBBY_H
#define LOBBY_H

#include <QObject>
#include <QElapsedTimer>
#include "shared/packet/w3gspacket.h"

class Game;
class Map;
class Player;
class SlotMap;

class Lobby : public QObject
{
    Q_OBJECT
public:
    explicit Lobby(Game* parent);

    quint32 elapsed(){return (quint32) mElapsedTimer->elapsed();}
    quint32 state(){return 16;}

    void welcomePlayer(Player* player);
    void leavingPlayer(Player* player);
    W3GSPacket *Serialize_W3GS_PLAYERINFO(Player *player);
    W3GSPacket *Serialize_W3GS_SLOTINFO();
    W3GSPacket *Serialize_W3GS_SLOTINFOJOIN(Player *player);
    W3GSPacket *Serialize_W3GS_PLAYERLEFT(Player *player);

    SlotMap *slotMap();

    Game* game();
    Map* map();

private:
    QElapsedTimer* mElapsedTimer;

    Game* mGame;
signals:
    
public slots:
    
};

#endif // LOBBY_H
