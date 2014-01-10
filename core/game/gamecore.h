#ifndef GAMECORE_H
#define GAMECORE_H

#include <QObject>
#include <QTcpServer>
#include "coreobject.h"
#include "gamelistener.h"

class Player;
class Game;

class GameCore : public CoreObject
{
    Q_OBJECT
public:
    explicit GameCore(Core *parent = 0);

    void load();

    const QString alias() {
        return "GC";
    }

    QList<Game*> games();
    QList<Game*> games(QString name);

    void newGame(Game *game);
    quint64 generateHostId();
    Game* gameWithHostId(quint64 hostId);
    bool listen(int fromPort, int toPort);
    bool listening(){return listener->isListening();}
    quint16 port(){return listener->serverPort();}
private:
    GameListener* listener;
    void potientialConnection(int socketDescriptor);

    QList<Player*> mPotientialPlayers;
    QList<Player*> mPlayers;

    QList<Game*> mGames;
protected:

signals:
    
public slots:
    void attachPlayerToGame(Player *player, quint64 hostCounter);
    void incomingConnection(qintptr socketDescriptor);
    void newConnection();
};

#endif // GAMECORE_H
