#include "lobby.h"
#include "game.h"
#include "map/slotmap.h"

Lobby::Lobby(Game *parent) :
    QObject(parent), mGame(parent){
    mElapsedTimer = new QElapsedTimer;
    mElapsedTimer->start();
}

void Lobby::welcomePlayer(Player *player){
    player->addChat("Welcoming player...");

    // Find slot
    foreach(Slot* slot, slotMap()->getSlots()){
        if (slot->status() == Slot::SLOT_STATUS_OCCUPIED) continue;
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
            QJsonObject data;
            data.insert("player.id", p->playerId());
            data.insert("player.name", p->name());
            player->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_PLAYERINFO, data));
        }
        {
            // Send PLAYERINFO to all other players about the new player
            QJsonObject data;
            data.insert("player.id", player->playerId());
            data.insert("player.name", player->name());
            p->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_PLAYERINFO, data));
        }
    }

    {
        QJsonObject data;
        data.insert("filepath", map()->path());
        data.insert("filesize", (qint64) map()->size());
        data.insert("mapinfo", (qint64) map()->info());
        data.insert("mapcrc", (qint64) map()->CRC());
        data.insert("mapsha1", QString(map()->SHA1()));

        W3GSPacket* out = GameProtocol::serialize(W3GSPacket::W3GS_MAPCHECK, data);
        player->addChat("Checking player's map file...");
        player->sendPacket(out);
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

void Lobby::leavingPlayer(Player *player, uint reason){
    // Accept the player's leave request
    player->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_LEAVERS));

    // Now find their slot and clear it.
    foreach(Slot* slot, slotMap()->getSlots()){
        if (slot->playerId() != player->playerId()) continue;
        slot->setStatus(Slot::SLOT_STATUS_OPEN);
        slot->setComputer(false);
        slot->setPlayerId(0);
        break;
    }

    // Notify everyone that the player has left and why. Then update their slot information.
    foreach(Player* p, mGame->players()){
        // Skip the leaving player
        if (p == player) continue;
        {
            QVariantHash data;
            data.insert("player.id", player->playerId());
            data.insert("reason", reason);
            p->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_PLAYERLEFT));
        }


        p->sendPacket(Serialize_W3GS_SLOTINFO());
    }

    player->deleteLater();
}

void Lobby::ping(Player *p){
    p->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_PING_FROM_HOST));
}

void Lobby::pingAll(){
    foreach(Player* p, mGame->players()) ping(p);
}

void Lobby::handlePacket(Player* player, W3GSPacket* p){
    if (p->packetId() == W3GSPacket::W3GS_LEAVEREQ){
        QJsonObject* data = GameProtocol::deserialize((W3GSPacket::PacketId) p->packetId(), p->data());
        int reason = data->value("reason").toInt();
        leavingPlayer(player, reason);
    }
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







