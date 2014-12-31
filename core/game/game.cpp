#include "game.h"

Game::Game(GameCore *parent) :
    QObject(parent), gameCore(parent), mTickTimer(new QTimer(this))
{
    mState = STATE_IDLE;

    mHostname = "Neco";
    mCurrentName = "necotest#2";
    mInitialName = mCurrentName;


    mRandomSeed = gameCore->core()->uptime();

    connect(mTickTimer, SIGNAL(timeout()), this, SLOT(tick()));
    mTickTimer->start(50);
}

void Game::upgradeIdleToLobby(){
    if (mState != STATE_IDLE) return;
    mLobby = new Lobby(this);
    mState = STATE_LOBBY;
}

void Game::upgradeLobbyToInstance(){
    if (mState != STATE_LOBBY) return;
    mInstance = new Instance(this);
    mState = STATE_INSTANCE;
}

bool Game::hasHostId(quint64 hostId){
    return mClients.contains(hostId);
}

QList<quint64> Game::hostIds(){
    return mClients.keys();
}

quint64 Game::getHostId(Client* client){
    return mClients.key(client, 0);
}

QList<Client*> Game::availableClients(){
    return mClients.values();
}

bool Game::initializeClient(Client *c){
    quint64 hId = gameCore->generateHostId();
    if (hId == 0) return false;
    mClients.insert(hId, c);
    return true;
}

bool Game::setMap(Map* map){
    if (map->valid()){
        qDebug() << "MAPLAYOUTSTYLE" << map->layoutStyle();
        mMap = map;
        mSlotMap = mMap->slotMap();
        return true;
    }
    return false;
}

bool Game::loadMap(QString mapFile){
    Map* map = new Map(mapFile, true);
    return setMap(map);
}

bool Game::addPlayer(Player *p){
    p->attachToGame(this);
    if (state() == STATE_LOBBY){
        p->setPlayerId(assignPlayerId());
        mPlayers.append(p);
        lobby()->welcomePlayer(p);
        //lobby()->introducePlayer(p);
        return true;
    }
    return false;
}

bool Game::removePlayer(Player *p){
    mPlayers.removeOne(p);
    if (state() == STATE_LOBBY && p->state() == Player::ATTACHED){
        lobby()->leavingPlayer(p);
    }
    return true;
}

void Game::queuePlayerPacket(Player* player, W3GSPacket* packet){
    queuedPackets.insert(player, packet);
}

void Game::tick(){
    handlePackets();

    if (state() == STATE_LOBBY){
        lobby()->tick();
    }
}

void Game::handlePackets(){
    foreach (Player* player, queuedPackets.keys()){
        foreach(W3GSPacket* p, queuedPackets.values(player)){
            if (p->protocol() != Packet::PROTOCOL_W3GS) continue;

            if (state() == STATE_LOBBY){
                lobby()->handlePacket(player, p);
            }
            else {
                // Unhandled for now, ignore!
            }
            queuedPackets.remove(player, p);
        }
    }
}

int Game::assignPlayerId(){
    QList<int> playerIds;
    foreach(Player* p, mPlayers){
        playerIds.append(p->playerId());
    }

    for(int i = 1; i < 12; i++){
        if (!playerIds.contains(i)) return i;
    }
    return 0;
}
