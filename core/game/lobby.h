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
    void leavingPlayer(Player* player, uint reason = 0x00);

    void ping(Player* p);
    void pingAll();

    void tick();

    W3GSPacket *Serialize_W3GS_SLOTINFO();
    W3GSPacket *Serialize_W3GS_SLOTINFOJOIN(Player *player);

    SlotMap *slotMap();

    Game* game();
    Map* map();

    QMap<Player*, quint32> playerTickCounts;

    void handlePacket(Player *player, W3GSPacket *p);
    void slotChangeRequest(Player *player, QVariantHash data);
    void sendMessageToPlayer(Player *from, Player *to, QString text, quint8 flag, quint32 extraFlags = 0);
    void sendMessageToPlayers(Player *from, QList<Player *> to, QString text, quint8 flag, quint32 extraFlags = 0);
    void handleLeaveRequest(Player *player, W3GSPacket *p);
    void handlePong(Player *player, W3GSPacket *p);
    void handleChat(Player *player, W3GSPacket *p);
private:
    QElapsedTimer* mElapsedTimer;
    QDateTime lastTick;

    Game* mGame;
};

#endif // LOBBY_H
