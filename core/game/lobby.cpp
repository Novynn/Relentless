#include "lobby.h"
#include "game.h"
#include "map/slotmap.h"

Lobby::Lobby(Game *parent) :
    QObject(parent), mGame(parent){
    mElapsedTimer = new QElapsedTimer;
    mElapsedTimer->start();
}

void Lobby::welcomePlayer(Player *player){
    qDebug() << "Welcoming player...";

    // Find slot
    foreach(Slot* slot, slotMap()->getSlots()){
        if (slot->status() == Slot::SLOT_STATUS_OCCUPIED) continue;
        qDebug() << "TEAM: " << (slot->team() + 1) << "/" << map()->numTeams();
        slot->setStatus(Slot::SLOT_STATUS_OCCUPIED);
        slot->setComputer(false);
        slot->setPlayerId(player->playerId());
        break;
    }

    {
//        QVariantHash data;
//        data.insert("player.id", player->playerId());
//        data.insert("player.name", player->name());
        player->sendPacket(Serialize_W3GS_SLOTINFOJOIN(player));
    }

    foreach(Player* p, game()->players()){
        if (p == player) continue;
        {
            // Send PLAYERINFO to new player
            QVariantHash data;
            data.insert("player.id", p->playerId());
            data.insert("player.name", p->name());
            player->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_PLAYERINFO, data));
        }
        {
            // Send PLAYERINFO to all other players about the new player
            QVariantHash data;
            data.insert("player.id", player->playerId());
            data.insert("player.name", player->name());
            p->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_PLAYERINFO, data));
        }
    }

    {
        QString filePath = map()->path();
        quint32 fileSize = map()->size();
        quint32 mapInfo = map()->info();
        quint32 mapCRC = map()->CRC();
        QByteArray mapSHA1 = map()->SHA1();
        qDebug() << "Sending W3GS_MAPCHECK";
        player->sendPacket(GameProtocol::serialize_W3GS_MAPCHECK(filePath, fileSize, mapInfo, mapCRC, mapSHA1));
    }

    foreach(Player* p, mGame->players()){
//        QVariantHash data;
//        data.insert("slots.data", slotMap()->encode());
//        data.insert("randomseed", mGame->randomSeed());
//        data.insert("map.layout", mGame->map()->layoutStyle());
//        data.insert("slots.count", mGame->map()->numPlayers());
//        p->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_SLOTINFO, data));
        p->sendPacket(Serialize_W3GS_SLOTINFO());
    }
}

void Lobby::leavingPlayer(Player *player){
    foreach(Slot* slot, slotMap()->getSlots()){
        if (slot->playerId() != player->playerId()) continue;
        slot->setStatus(Slot::SLOT_STATUS_OPEN);
        slot->setComputer(false);
        slot->setPlayerId(0);
        break;
    }

    foreach(Player* p, mGame->players()){
        if (p == player) continue;
        p->sendPacket(Serialize_W3GS_PLAYERLEFT(player));
        p->sendPacket(Serialize_W3GS_SLOTINFO());
    }
}


W3GSPacket* Lobby::Serialize_W3GS_PLAYERLEFT(Player* player){
    /*
     *(BYTE) Player number
(DWORD) Reason
*/
    QByteArrayBuilder out;
    out.insertByte(player->playerId());
    out.insertDWord(0x01);
    return new W3GSPacket(W3GSPacket::W3GS_PLAYERLEFT, out);
}

SlotMap *Lobby::slotMap(){
    return mGame->slotMap();
}

Game *Lobby::game(){
    return mGame;
}

Map *Lobby::map(){
    return mGame->map();
}

W3GSPacket* Lobby::Serialize_W3GS_PLAYERINFO(Player* player){
    /*
     * (DWORD) Player Counter
     * (BYTE) Player number
     * (STRING) Player name
     * (WORD) Unknown (1)
     * (WORD) AF_INET (2)
     * (WORD) Port
     * (DWORD) External IP
     * (DWORD) Unknown (0)
     * (DWORD) Unknown (0)
     * (WORD) AF_INET (2)
     * (WORD) Port
     * (DWORD) Internal IP
     * (DWORD) Unknown (0)
     * (DWORD) Unknown (0)
    */
    QByteArrayBuilder out;
    out.insertDWord(2);
    out.insertByte(player->playerId());
    out.insertString(player->name());
    out.insertWord(1);
    out.insertWord(2);
    out.insertWord(0); // Port
    out.insertDWord(0); // External
    out.insertDWord(0);
    out.insertDWord(0);
    out.insertWord(2);
    out.insertWord(0); // Port
    out.insertDWord(0); // Internal
    out.insertDWord(0);
    out.insertDWord(0);
    return new W3GSPacket(W3GSPacket::W3GS_PLAYERINFO, out);
}


W3GSPacket* Lobby::Serialize_W3GS_SLOTINFOJOIN(Player* player){
    /*
     * (WORD) Length of Slot Info
     * (BYTE) Number of slots
     * (BYTE)[] Slot data
     * (DWORD) Random seed
     * (BYTE) Game type
     * (BYTE) Number of player slots without observers
     * (BYTE) Player number
     * (DWORD) Port
     * (DWORD) External IP
     * (DWORD) Unknown (0)
     * (DWORD) Unknown (0)
     *
     * For each slot:
     * (BYTE) Player number
     * (BYTE) Download status
     * (BYTE) Slot status
     * (BYTE) Computer status
     * (BYTE) Team
     * (BYTE) Color
     * (BYTE) Race
     * (BYTE) Computer type
     * (BYTE) Handicap
*/
    QByteArrayBuilder slotData;

    int count = mGame->slotMap()->getSlots().count();

    slotData.insertByte(count);
    foreach(Slot* slot, mGame->slotMap()->getSlots()){
        slotData.insertVoid(slot->toByteArray());
    }
    slotData.insertDWord(mGame->randomSeed());
    slotData.insertByte(mGame->map()->layoutStyle());
    slotData.insertByte(mGame->map()->numPlayers());

    QByteArrayBuilder out;

    out.insertWord(slotData.size());
    out.insertVoid(slotData);
    out.insertByte(player->playerId());
    out.insertDWord(player->socket->peerPort());
    // Peer address
    //out.insertDWord(player->socket->peerAddress().);
    dword peerAddress = player->socket->peerAddress().toIPv4Address();
    out.insertDWord(peerAddress);
    // End
    out.insertDWord(0);
    out.insertDWord(0);
    return new W3GSPacket(W3GSPacket::W3GS_SLOTINFOJOIN, out);
}

W3GSPacket* Lobby::Serialize_W3GS_SLOTINFO(){
    /*
     * (WORD) Length of Slot Info

     * Slot Info:
     * (BYTE) Player number
     * (BYTE) Download status
     * (BYTE) Slot status
     * (BYTE) Computer status
     * (BYTE) Team
     * (BYTE) Color
     * (BYTE) Race
     * (BYTE) Computer type
     * (BYTE) Handicap
     */
    QByteArrayBuilder slotData;

    int count = mGame->slotMap()->getSlots().count();

    slotData.insertByte(count);
    qDebug() << "count: " << count;
    foreach(Slot* slot, mGame->slotMap()->getSlots()){
        slotData.insertVoid(slot->toByteArray());
        qDebug() << slot->team() << slot->colour() << slot->playerId() << slot->status();
    }
    slotData.insertDWord(mGame->randomSeed());
    slotData.insertByte(mGame->map()->layoutStyle());
    slotData.insertByte(mGame->map()->numPlayers());
    qDebug() << "data: " << mGame->randomSeed() << mGame->map()->layoutStyle() << mGame->map()->numPlayers();

    QByteArrayBuilder out;
    out.insertWord(slotData.size());
    out.insertVoid(slotData);

    return new W3GSPacket(W3GSPacket::W3GS_SLOTINFO, out);
}







