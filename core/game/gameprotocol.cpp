#include "gameprotocol.h"

GameProtocol* GameProtocol::_instance = 0;

W3GSPacket* GameProtocol::serialize(W3GSPacket::PacketId packetId, QString key, QVariant value){
    QJsonObject data;
    data.insert(key, QJsonValue::fromVariant(value));
    return serialize(packetId, data);
}

W3GSPacket* GameProtocol::serialize(W3GSPacket::PacketId packetId, QJsonObject data){
    QByteArrayBuilder* out;

    QString method = "serialize_" + W3GSPacket::packetIdToString(packetId);
    bool invoked = staticMetaObject.invokeMethod(instance(),
                                  method.toLatin1().constData(),
                                  Qt::DirectConnection,
                                  Q_ARG(QJsonObject, data),
                                  Q_ARG(QByteArrayBuilder*, out));

    qDebug() << "INVOKED: " << invoked << method << "\n" << qPrintable(out->toReadableString());

    W3GSPacket* packet = new W3GSPacket(packetId, *out);
    return packet;

//    switch(packetId){
//    case W3GSPacket::W3GS_PING_FROM_HOST:
//        packet = GameProtocol::serialize_W3GS_PING_FROM_HOST();
//        break;
//    case W3GSPacket::W3GS_SLOTINFOJOIN:
//        packet = GameProtocol::serialize_W3GS_SLOTINFOJOIN(QByteArray(), 0, 0, 0);
//        break;
//    case W3GSPacket::W3GS_REJECTJOIN:
//        packet = GameProtocol::serialize_W3GS_REJECTJOIN(data.value("reason").toInt(0x07));
//        break;
//    case W3GSPacket::W3GS_PLAYERINFO:
//        packet = GameProtocol::serialize_W3GS_PLAYERINFO(data.value("player.counter").toInt(0x02),
//                                                         data.value("player.id").toInt(),
//                                                         data.value("player.name").toString("Unknown"));
//        break;
//    case W3GSPacket::W3GS_PLAYERLEFT:
//        packet = GameProtocol::serialize_W3GS_PLAYERLEFT(data.value("player.id").toInt(),
//                                                         data.value("reason").toInt());
//        break;
//    case W3GSPacket::W3GS_PLAYERLOADED:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_SLOTINFO:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_COUNTDOWN_START: //*
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_COUNTDOWN_END: //*
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_INCOMING_ACTION:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_CHAT_FROM_HOST:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_LEAVERS:
//        // No data
//        break;
//    case W3GSPacket::W3GS_REQJOIN:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_LEAVEREQ:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_GAMELOADED_SELF:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_OUTGOING_ACTION:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_OUTGOING_KEEPALIVE:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_CHAT_TO_HOST:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_SEARCHGAME:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_GAMEINFO:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_CREATEGAME:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_REFRESHGAME:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_DECREATEGAME:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_PING_FROM_OTHERS:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_PONG_TO_OTHERS:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_CLIENTINFO:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_MAPCHECK:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_STARTDOWNLOAD:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_MAPSIZE:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_MAPPART:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_MAPPARTOK:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_MAPPARTNOTOK:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_PONG_TO_HOST:
//        packet = packet;
//        break;
//    case W3GSPacket::W3GS_INCOMING_ACTION2:
//        packet = packet;
//        break;
//    default:
//        return 0;
//    }


    return packet;
}

QJsonObject GameProtocol::deserialize(W3GSPacket::PacketId packetId, QByteArrayBuilder data)
{
    QJsonObject* out;

    QString method = "deserialize_" + W3GSPacket::packetIdToString(packetId);
    bool invoked = staticMetaObject.invokeMethod(instance(),
                                  method.toLatin1().constData(),
                                  Qt::DirectConnection,
                                  Q_ARG(QByteArrayBuilder, data),
                                  Q_ARG(QJsonObject*, out));

    qDebug() << "INVOKED: " << invoked << method << "\n" << out;

    return *out;
}

void GameProtocol::serialize_W3GS_PING_FROM_HOST(QJsonObject data, QByteArrayBuilder* out){
    // Empty
}

void GameProtocol::serialize_W3GS_SLOTINFOJOIN(QJsonObject data, QByteArrayBuilder* out){
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
    QByteArray slotInfo;
    quint32 seed;
    quint8 layout;
    quint8 players;
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

//    QByteArrayBuilder out;
//    out.insertWord(slotInfo.size());
//    out.insertVoid(slotData);
//    slotData.insertDWord(game->randomSeed());
//    slotData.insertByte(game->map()->layoutStyle());
//    slotData.insertByte(game->map()->numPlayers());

//    return new W3GSPacket(W3GSPacket::W3GS_SLOTINFOJOIN, out);
}
void GameProtocol::serialize_W3GS_REJECTJOIN(QJsonObject data, QByteArrayBuilder* out){
    quint32 reason = (quint32) data.value("reason").toInt();
    out->insertDWord(reason);
}
void GameProtocol::serialize_W3GS_PLAYERINFO(QJsonObject data, QByteArrayBuilder* out){
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
    quint32 counter = (quint32) data.value("player.counter").toInt(0x02);
    quint8 playerId = (quint8) data.value("player.id").toInt();
    QString playerName = data.value("player.name").toString();

    out->insertDWord(counter);
    out->insertByte(playerId);
    out->insertString(playerName);
    out->insertWord(1);
    out->insertWord(2);
    out->insertWord(0); // Port
    out->insertDWord(0); // External
    out->insertDWord(0);
    out->insertDWord(0);
    out->insertWord(2);
    out->insertWord(0); // Port
    out->insertDWord(0); // Internal
    out->insertDWord(0);
    out->insertDWord(0);
}

void GameProtocol::serialize_W3GS_PLAYERLEFT(QJsonObject data, QByteArrayBuilder* out){
    /*
     *  0x00 Kicked
        0x01 PLAYERLEAVE_DISCONNECT
        0x07 PLAYERLEAVE_LOST
        0x08 PLAYERLEAVE_LOSTBUILDINGS
        0x09 PLAYERLEAVE_WON
        0x0A PLAYERLEAVE_DRAW
        0x0B PLAYERLEAVE_OBSERVER
        0x0D PLAYERLEAVE_LOBBY
    */

    quint8 playerId = (quint8) data.value("player.id").toInt();
    quint32 reason = (quint32) data.value("reason").toInt();

    out->insertByte(playerId);
    out->insertDWord(reason);
}
void GameProtocol::serialize_W3GS_PLAYERLOADED(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_SLOTINFO(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_COUNTDOWN_START(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_COUNTDOWN_END(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_INCOMING_ACTION(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_CHAT_FROM_HOST(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_LEAVERS(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_GAMEINFO(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_CREATEGAME(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_REFRESHGAME(QJsonObject data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_DECREATEGAME(QJsonObject data, QByteArrayBuilder* out){
}

void GameProtocol::deserialize_W3GS_REQJOIN(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_LEAVEREQ(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_GAMELOADED_SELF(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_OUTGOING_ACTION(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_OUTGOING_KEEPALIVE(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_CHAT_TO_HOST(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_SEARCHGAME(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_PING_FROM_OTHERS(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_PONG_TO_OTHERS(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_CLIENTINFO(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_STARTDOWNLOAD(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::serialize_W3GS_MAPCHECK(QJsonObject data, QByteArrayBuilder* out){
    /*
     * (DWORD) Unknown
     * (STRING) File Path
     * (DWORD) File size
     * (DWORD) Map info
     * (DWORD) File CRC encryption
     * (DWORD) File SHA-1 hash
    */
    QString filePath = data.value("filepath").toString();
    quint32 fileSize = (quint32) data.value("filesize").toInt();
    quint32 mapInfo = (quint32) data.value("mapinfo").toInt();
    quint32 mapCRC = (quint32) data.value("mapcrc").toInt();
    QByteArray mapSHA1 = data.value("mapsha1").toString().toLatin1();

    out->insertDWord(1); // Unknown
    out->insertString(filePath);
    out->insertDWord(fileSize);
    out->insertDWord(mapInfo);
    out->insertDWord(mapCRC);
    out->insertVoid(mapSHA1);
}

void GameProtocol::serialize_W3GS_STARTDOWNLOAD(QJsonObject data, QByteArrayBuilder* out){
}

void GameProtocol::serialize_W3GS_MAPPART(QJsonObject data, QByteArrayBuilder* out){
}

void GameProtocol::serialize_W3GS_INCOMING_ACTION2(QJsonObject data, QByteArrayBuilder* out){
}

void GameProtocol::serialize_W3GS_SEARCHGAME(QJsonObject data, QByteArrayBuilder *out)
{

}

void GameProtocol::deserialize_W3GS_MAPSIZE(QByteArrayBuilder data, QJsonObject* out){
    /*
     * (DWORD) Unknown
     * (BYTE) Size Flag
     * (DWORD) Map size
     */
    out->insert("unknown", (qint64) data.getDWord());
    out->insert("sizeflag", data.getByte());
    out->insert("mapsize", (qint64) data.getDWord());
}

void GameProtocol::deserialize_W3GS_MAPPARTOK(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_MAPPARTNOTOK(QByteArrayBuilder data, QJsonObject *out)
{

}

void GameProtocol::deserialize_W3GS_PONG_TO_HOST(QByteArrayBuilder data, QJsonObject *out)
{

}
