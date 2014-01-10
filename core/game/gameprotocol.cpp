#include "gameprotocol.h"

W3GSPacket* GameProtocol::serialize(W3GSPacket::PacketId packetId, QVariantHash data){
    W3GSPacket* packet;
    switch(packetId){
    case W3GSPacket::W3GS_PING_FROM_HOST:
        // No data
        break;
    case W3GSPacket::W3GS_SLOTINFOJOIN:
        packet = GameProtocol::serialize_W3GS_SLOTINFOJOIN(QByteArray(), 0, 0, 0);
        break;
    case W3GSPacket::W3GS_REJECTJOIN:
        packet = GameProtocol::serialize_W3GS_REJECTJOIN(data.value("reason", 0x07).toUInt());
        break;
    case W3GSPacket::W3GS_PLAYERINFO:
        packet = GameProtocol::serialize_W3GS_PLAYERINFO(data.value("player.counter", 0x02).toUInt(),
                                                         data.value("player.id", 0).toUInt(),
                                                         data.value("player.name", "Unknown").toString());
        break;
    case W3GSPacket::W3GS_PLAYERLEFT:
        packet = packet;
        break;
    case W3GSPacket::W3GS_PLAYERLOADED:
        packet = packet;
        break;
    case W3GSPacket::W3GS_SLOTINFO:
        packet = packet;
        break;
    case W3GSPacket::W3GS_COUNTDOWN_START: //*
        packet = packet;
        break;
    case W3GSPacket::W3GS_COUNTDOWN_END: //*
        packet = packet;
        break;
    case W3GSPacket::W3GS_INCOMING_ACTION:
        packet = packet;
        break;
    case W3GSPacket::W3GS_CHAT_FROM_HOST:
        packet = packet;
        break;
    case W3GSPacket::W3GS_LEAVERS:
        packet = packet;
        break;
    case W3GSPacket::W3GS_REQJOIN:
        packet = packet;
        break;
    case W3GSPacket::W3GS_LEAVEREQ:
        packet = packet;
        break;
    case W3GSPacket::W3GS_GAMELOADED_SELF:
        packet = packet;
        break;
    case W3GSPacket::W3GS_OUTGOING_ACTION:
        packet = packet;
        break;
    case W3GSPacket::W3GS_OUTGOING_KEEPALIVE:
        packet = packet;
        break;
    case W3GSPacket::W3GS_CHAT_TO_HOST:
        packet = packet;
        break;
    case W3GSPacket::W3GS_SEARCHGAME:
        packet = packet;
        break;
    case W3GSPacket::W3GS_GAMEINFO:
        packet = packet;
        break;
    case W3GSPacket::W3GS_CREATEGAME:
        packet = packet;
        break;
    case W3GSPacket::W3GS_REFRESHGAME:
        packet = packet;
        break;
    case W3GSPacket::W3GS_DECREATEGAME:
        packet = packet;
        break;
    case W3GSPacket::W3GS_PING_FROM_OTHERS:
        packet = packet;
        break;
    case W3GSPacket::W3GS_PONG_TO_OTHERS:
        packet = packet;
        break;
    case W3GSPacket::W3GS_CLIENTINFO:
        packet = packet;
        break;
    case W3GSPacket::W3GS_MAPCHECK:
        packet = packet;
        break;
    case W3GSPacket::W3GS_STARTDOWNLOAD:
        packet = packet;
        break;
    case W3GSPacket::W3GS_MAPSIZE:
        packet = packet;
        break;
    case W3GSPacket::W3GS_MAPPART:
        packet = packet;
        break;
    case W3GSPacket::W3GS_MAPPARTOK:
        packet = packet;
        break;
    case W3GSPacket::W3GS_MAPPARTNOTOK:
        packet = packet;
        break;
    case W3GSPacket::W3GS_PONG_TO_HOST:
        packet = packet;
        break;
    case W3GSPacket::W3GS_INCOMING_ACTION2:
        packet = packet;
        break;
    default:
        return 0;
    }


    return packet;
}

W3GSPacket *GameProtocol::serialize_W3GS_PING_FROM_HOST(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_PING_FROM_HOST, out);
}

W3GSPacket* GameProtocol::serialize_W3GS_SLOTINFOJOIN(QByteArray slotInfo, quint32 seed, quint8 layout, quint8 players){
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
//    QByteArrayBuilder slotData;

//    int count = game->slotMap()->getSlots().count();

//    slotData.insertByte(count);
//    qDebug() << "count: " << count;
//    foreach(Slot* slot, game->slotMap()->getSlots()){
//        slotData.insertVoid(slot->toByteArray());
//        qDebug() << slot->team() << slot->colour() << slot->playerId() << slot->status();
//    }
//    slotData.insertDWord(game->randomSeed());
//    slotData.insertByte(game->map()->layoutStyle());
//    slotData.insertByte(game->map()->numPlayers());
//    qDebug() << "data: " << game->randomSeed() << game->map()->layoutStyle() << game->map()->numPlayers();

    QByteArrayBuilder out;
//    out.insertWord(slotInfo.size());
//    out.insertVoid(slotData);
//    slotData.insertDWord(game->randomSeed());
//    slotData.insertByte(game->map()->layoutStyle());
//    slotData.insertByte(game->map()->numPlayers());

    return new W3GSPacket(W3GSPacket::W3GS_SLOTINFOJOIN, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_REJECTJOIN(uint reason){
    QByteArrayBuilder out;
    out.insertDWord(reason);
    return new W3GSPacket(W3GSPacket::W3GS_REJECTJOIN, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_PLAYERINFO(quint32 counter, quint8 playerId, QString playerName){
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
    out.insertDWord(counter);
    out.insertByte(playerId);
    out.insertString(playerName);
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

W3GSPacket *GameProtocol::serialize_W3GS_PLAYERLEFT(quint8 playerId, quint32 reason){
    /*
        0x01 PLAYERLEAVE_DISCONNECT
        0x07 PLAYERLEAVE_LOST
        0x08 PLAYERLEAVE_LOSTBUILDINGS
        0x09 PLAYERLEAVE_WON
        0x0A PLAYERLEAVE_DRAW
        0x0B PLAYERLEAVE_OBSERVER
        0x0D PLAYERLEAVE_LOBBY
    */

    QByteArrayBuilder out;
    out.insertByte(playerId);
    out.insertDWord(reason);
    return new W3GSPacket(W3GSPacket::W3GS_PLAYERLEFT, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_PLAYERLOADED(quint8 playerId){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_PLAYERLOADED, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_SLOTINFO(QByteArray slotInfo, quint32 seed, quint8 layout, quint8 players){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_SLOTINFO, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_COUNTDOWN_START(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_COUNTDOWN_START, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_COUNTDOWN_END(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_COUNTDOWN_END, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_INCOMING_ACTION(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_INCOMING_ACTION, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_CHAT_FROM_HOST(QList<quint8> playerIds, quint8 fromPlayer, quint8 flags,
                                                        quint32 extraFlags, QString message){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_CHAT_FROM_HOST, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_LEAVERS(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_LEAVERS, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_GAMEINFO(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_GAMEINFO, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_CREATEGAME(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_CREATEGAME, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_REFRESHGAME(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_REFRESHGAME, out);
}
W3GSPacket* GameProtocol::serialize_W3GS_DECREATEGAME(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_DECREATEGAME, out);
}

QVariantHash GameProtocol::deserialize_W3GS_MAPSIZE(QByteArrayBuilder data){
    /*
     * (DWORD) Unknown
     * (BYTE) Size Flag
     * (DWORD) Map size
     */
    QVariantHash out;
    out.insert("unknown", data.getDWord());
    out.insert("sizeflag", data.getByte());
    out.insert("mapsize", data.getDWord());
    return out;
}

W3GSPacket* GameProtocol::serialize_W3GS_MAPCHECK(QString filePath, quint32 fileSize, quint32 mapInfo,
                                                  quint32 mapCRC, QByteArray mapSHA1){
    /*
     * (DWORD) Unknown
     * (STRING) File Path
     * (DWORD) File size
     * (DWORD) Map info
     * (DWORD) File CRC encryption
     * (DWORD) File SHA-1 hash
    */
    QByteArrayBuilder out;
    out.insertDWord(1); // Unknown
    out.insertString(filePath);
    out.insertDWord(fileSize);
    out.insertDWord(mapInfo);
    out.insertDWord(mapCRC);
    out.insertVoid(mapSHA1);
    return new W3GSPacket(W3GSPacket::W3GS_MAPCHECK, out);
}

W3GSPacket* GameProtocol::serialize_W3GS_MAPPART(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_MAPPART, out);
}

W3GSPacket* GameProtocol::serialize_W3GS_INCOMING_ACTION2(){
    QByteArrayBuilder out;
    return new W3GSPacket(W3GSPacket::W3GS_INCOMING_ACTION2, out);
}
