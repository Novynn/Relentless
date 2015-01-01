#include "lobby.h"
#include "game.h"
#include "map/slotmap.h"

Lobby::Lobby(Game *parent) :
    QObject(parent), mGame(parent){
    mElapsedTimer = new QElapsedTimer;
    mElapsedTimer->start();

    lastTick = QDateTime::fromMSecsSinceEpoch(0);
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
        QVariantHash data;
        data.insert("filepath", map()->path());
        data.insert("filesize", map()->size());
        data.insert("mapinfo", map()->info());
        data.insert("mapcrc", map()->CRC());
        data.insert("mapsha1", map()->SHA1());

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
        if (p != player){
            QVariantHash data;
            data.insert("player.id", player->playerId());
            data.insert("reason", reason);
            p->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_PLAYERLEFT, data));
            p->sendPacket(Serialize_W3GS_SLOTINFO());
        }
    }
    player->setLeaving();
    game()->removePlayer(player);
    player->deleteLater();
}

void Lobby::tick() {
    if (lastTick.msecsTo(QDateTime::currentDateTime()) < 30000) return;

    foreach(Client* client, game()->clients()){
        client->hostRefresh();
    }

    pingAll();

    lastTick = QDateTime::currentDateTime();
}

void Lobby::ping(Player *p){
    p->addChat("Sent Ping");
    p->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_PING_FROM_HOST, "tickcount", elapsed()));
    playerTickCounts.insert(p, elapsed());
}

void Lobby::pingAll(){
    foreach(Player* p, mGame->players()){
        ping(p);
    }
}

void Lobby::handlePacket(Player* player, W3GSPacket* p){
    if (p->packetId() == W3GSPacket::W3GS_LEAVEREQ){
        QVariantHash* data = GameProtocol::deserialize((W3GSPacket::PacketId) p->packetId(), p->data());
        int reason = data->value("reason").toUInt();
        leavingPlayer(player, reason);
    }
    if (p->packetId() == W3GSPacket::W3GS_CHAT_TO_HOST){
        QVariantHash* data = GameProtocol::deserialize((W3GSPacket::PacketId) p->packetId(), p->data());
        for (QVariant m : data->value("messages").toList()) {
            QVariantHash message = m.toHash();
            if (message.contains("text")) {
                QString text = message.value("text", "").toString();
                Player* to = game()->playerById(message.value("to", -1).toInt());
                quint8 flag = message.value("flag", 0).toUInt();
                quint32 extraFlags = message.value("extra_flags", 0).toUInt();
                if (to != 0) {
                    sendMessageToPlayer(player, to, text, flag, extraFlags);
                }
            }
            else {
                slotChangeRequest(player, message);
            }
        }
    }
    if (p->packetId() == W3GSPacket::W3GS_PONG_TO_HOST) {
        QVariantHash* data = GameProtocol::deserialize((W3GSPacket::PacketId) p->packetId(), p->data());
        quint32 tick = data->value("tickcount", 0).toUInt();
        quint32 lastTick = playerTickCounts.value(player, 0);
        quint32 roundTrip = elapsed();
        playerTickCounts.remove(player);
    }
}

void Lobby::sendMessageToPlayers(Player* from, QList<Player*> to, QString text, quint8 flag, quint32 extraFlags) {
    QVariantHash data;
    data["from"] = from->playerId();
    QVariantList players;
    for (Player* player : to) {
        players.append(player->playerId());
    }
    data["to"] = players;
    data["text"] = text;
    data["flag"] = flag;
    data["extra_flags"] = extraFlags;

    for (Player* player : to) {
        player->sendPacket(GameProtocol::serialize(W3GSPacket::W3GS_CHAT_FROM_HOST, data));
    }
}

void Lobby::sendMessageToPlayer(Player* from, Player* to, QString text, quint8 flag, quint32 extraFlags) {
    sendMessageToPlayers(from, QList<Player*>{to}, text, flag, extraFlags);
}

void Lobby::slotChangeRequest(Player* player, QVariantHash data) {
    Slot* currSlot = slotMap()->getSlotFromPlayerId(player->playerId());
    quint8 flag = data.value("flag", 0x00).toUInt();
    quint8 val;

    if (currSlot == 0) return;

    switch(flag) {
    case 0x11:
        val = data.value("team", 0).toUInt();
        foreach(Slot* slot, slotMap()->getSlots()){
            if (slot->team() == val && slot->status() == Slot::SLOT_STATUS_OPEN){
                currSlot->setStatus(Slot::SLOT_STATUS_OPEN);
                currSlot->setPlayerId(0);
                slot->setStatus(Slot::SLOT_STATUS_OCCUPIED);
                slot->setPlayerId(player->playerId());
                break;
            }
        }
        break;
    case 0x12:
        val = data.value("color", 0).toUInt();
        if (val > 11) return;
        currSlot->setColour(val);
        break;
    case 0x13:
        val = data.value("race", 0).toUInt();
        if (val != Slot::SLOT_RACE_HUMAN &&
            val != Slot::SLOT_RACE_NIGHTELF &&
            val != Slot::SLOT_RACE_ORC &&
            val != Slot::SLOT_RACE_UNDEAD &&
            val != Slot::SLOT_RACE_RANDOM){
            return;
        }
        currSlot->setRace(val);
        break;
    case 0x14:
        val = data.value("handicap", 0).toUInt();
        if (val > 100 || val < 50 || val % 10 != 0) return;
        currSlot->setHandicap(val);
        break;
    default:
        // Invalid, ignore
        break;
    }

    // Update!
    foreach(Player* p, game()->players()){
        p->sendPacket(Serialize_W3GS_SLOTINFO());
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
    slotData.reset();
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
    foreach(Slot* slot, mGame->slotMap()->getSlots()){
        slotData.insertVoid(slot->toByteArray());
    }
    slotData.insertDWord(mGame->randomSeed());
    slotData.insertByte(mGame->map()->layoutStyle());
    slotData.insertByte(mGame->map()->numPlayers());

    QByteArrayBuilder out;
    slotData.reset();
    out.insertWord(slotData.size());
    out.insertVoid(slotData);

    return new W3GSPacket(W3GSPacket::W3GS_SLOTINFO, out);
}







