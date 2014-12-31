#include "gamecore.h"

#include "game/game.h"
#include "game/player.h"

GameCore::GameCore(Core *parent) :
    CoreObject(parent), listener(new GameListener(this)){

}

void GameCore::load(){
    core()->settings()->beginGroup("Games");
    if (core()->settings()->value("listen", true).toBool())
        listen(core()->settings()->value("start", 6112).toInt(),
               core()->settings()->value("end", 6119).toInt());
    core()->settings()->endGroup();
    print("GameCore initialized!");
}

QList<Game*> GameCore::games(){
    return mGames;
}

QList<Game*> GameCore::games(QString name){
    QList<Game*> result;
    foreach(Game* game, mGames){
        if (game->name() == name)
            result.append(game);
    }
    return result;
}

bool GameCore::listen(int fromPort, int toPort){
    listener->close();
    disconnect(listener);

    int port = fromPort;
    if (fromPort > toPort){
        port = toPort;
        toPort = fromPort;
        fromPort = toPort;
    }

    while(!listener->listen(QHostAddress::Any, port)){
        if (port >= toPort) {port = -1; break;}
        warning("Could not listen on port [" + QString::number(port) + "]...");
        ++port;
    }
    if (port != -1){
        print("Listening for games on port [" + QString::number(port) + "]...");
        connect(listener, SIGNAL(connectionRequest(qintptr)), this, SLOT(incomingConnection(qintptr)));
        connect(listener, SIGNAL(newConnection()), this, SLOT(newConnection()));
        return true;
    }
    error("Could not listen on ports [6112-6119]...");
    error(" > Additional Info: [" + listener->errorString() + "]");
    return false;
}

Game* GameCore::gameWithHostId(quint64 hostId){
    foreach(Game* game, mGames){
        if (game->hasHostId(hostId)) return game;
    }
    return 0;
}

quint64 GameCore::generateHostId(){
    quint64 hostId = 1;
    if (!mGames.isEmpty()){
        QList<quint64> currList;
        foreach(Game* game, mGames){
            currList.append(game->hostIds());
        }
        while(currList.contains(hostId)){
            hostId++;
        }
    }
    return hostId;
}

void GameCore::newGame(Game* game){
    info("Looking for clients available for broadcasting...");
    QStringList foundRealms;
    QStringList pClients = core()->getClientCore()->getClientAliases();
    QList<Client*> clients;
    pClients.sort();
    foreach(QString alias, pClients){
        Client* client = core()->getClientCore()->getClient(alias);
        QString userId = alias + "." + client->username();
        if (!client){
            error(QString("%1@%2 \t\t [%3]").arg(userId, "UNKNOWN", "ERROR"));
            continue;
        }
        if (client->status() != Client::CLIENT_CONNECTED
                && client->status() != Client::CLIENT_CONNECTED_HOSTING){
            warning(QString("%1@%2 \t\t [%3]").arg(userId, client->realm(), "OFFLINE"));
            continue;
        }
        if (client->status() == Client::CLIENT_DISABLED){
            warning(QString("%1@%2 \t\t [%3]").arg(userId, client->realm(), "DISABLED"));
            continue;
        }
        if (client->status() == Client::CLIENT_CONNECTED_HOSTING ||
            client->status() == Client::CLIENT_CONNECTED_BEGIN_HOSTING){
            warning(QString("%1@%2 \t\t [%3]").arg(userId, client->realm(), "BUSY"));
            continue;
        }
        if (foundRealms.contains(client->realm())){
            warning(QString("%1@%2 \t\t [%3]").arg(userId, client->realm(), "SKIPPED"));
            continue;
        }
        info(QString("%1@%2 \t\t [%3]").arg(userId, client->realm(), "OK"));
        foundRealms.append(client->realm());
        clients.append(client);
    }
    info(QString("%1 available clients found! Sending broadcast signal...").arg(clients.count()));

    mGames.append(game);
    game->upgradeIdleToLobby();
    foreach(Client* client, clients){
        if (game->initializeClient(client)){
            client->command("Hosting [" + game->name() + "]...");
            client->beginHosting(game);
        }
    }
}

//bool GameCore::registerNewGame(Game* game){
//    mGames.insert(game->hostId(), game);
//}

//bool GameCore::unregisterGame(Game* game){
//    if (!mGames.contains(game->hostId())) return false;
//    if (mGames.remove(game->hostId()) == 0) return false;

//    return true;
//}

//bool GameCore::unregisterGame(int gid){

//}

//void GameCore::newGame(){
//    Game* game = new Game(this);
//    game->loadMap("");
//}

//void GameCore::getGame(){

//}

//void GameCore::cancelGame(){

//}

void GameCore::newConnection(){
}

void GameCore::incomingConnection(qintptr socketDescriptor){
    if (mGames.isEmpty()) return;
    potientialConnection((int) socketDescriptor);
}

void GameCore::potientialConnection(int socketDescriptor){
    info("New potiential connection.");

    Player* player = new Player(this);
    if (player->initialize(socketDescriptor)){
        info("Player socket assigned. Awaiting join request...");
        connect(player, SIGNAL(joinRequest(Player*,quint64)), this, SLOT(attachPlayerToGame(Player*, quint64)));
        return;
    }
    error("Unable to create player object.");
    delete player;
}

void GameCore::joinRequest(Player* player, QByteArrayBuilder data){
    player->Recv_W3GS_REQJOIN(data);
}

void GameCore::attachPlayerToGame(Player* player, quint64 hostCounter){
    player->disconnect(this); // We no longer want to hear from the player

    Game* game = gameWithHostId(hostCounter);
    if (game){
        info("[" + player->name() + "] is attempting to join [" + game->name() + "]...");
        if (game->state() == Game::STATE_LOBBY)
            game->addPlayer(player);
        else if (game->state() == Game::STATE_INSTANCE)
            player->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_REJECTJOIN,
                                                       "reason", 0x10)); // REJECTJOIN_STARTED
        else
            player->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_REJECTJOIN,
                                                       "reason", 0x09)); // REJECTJOIN_FULL
    }
    else {
        info("[" + player->name() + "] is attempting to join invalid game with host id [" + QString::number(hostCounter) + "]...");

        player->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_REJECTJOIN,
                                                   "reason", 0x07)); // REJECTJOIN_INVALID
    }
}
