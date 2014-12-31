#include "gameprotocol.h"

GameProtocol* GameProtocol::_instance = 0;

W3GSPacket* GameProtocol::serialize(W3GSPacket::PacketId packetId, QString key, QVariant value){
    QVariantHash data;
    data.insert(key, value);
    return serialize(packetId, data);
}

W3GSPacket* GameProtocol::serialize(W3GSPacket::PacketId packetId, QVariantHash data){
    QByteArrayBuilder* out = new QByteArrayBuilder();

    QString method = "serialize_" + W3GSPacket::packetIdToString(packetId);
    bool invoked = staticMetaObject.invokeMethod(instance(),
                                  method.toLatin1().constData(),
                                  Qt::DirectConnection,
                                  Q_ARG(QVariantHash, data),
                                  Q_ARG(QByteArrayBuilder*, out));

//    qDebug() << "INVOKED: " << invoked << method << "\n" << qPrintable(out->toReadableString());

    W3GSPacket* packet = new W3GSPacket(packetId, *out);
    return packet;
}

QVariantHash* GameProtocol::deserialize(W3GSPacket::PacketId packetId, QByteArrayBuilder data)
{
    QVariantHash* out = new QVariantHash();

    QString method = "deserialize_" + W3GSPacket::packetIdToString(packetId);
    bool invoked = staticMetaObject.invokeMethod(instance(),
                                  method.toLatin1().constData(),
                                  Qt::DirectConnection,
                                  Q_ARG(QByteArrayBuilder, data),
                                  Q_ARG(QVariantHash*, out));

//    qDebug() << "INVOKED: " << invoked << method << "\n" << out;

    return out;
}

void GameProtocol::serialize_W3GS_PING_FROM_HOST(QVariantHash data, QByteArrayBuilder* out){
    // Empty
    out->insertDWord(data.value("tickcount", 0).toUInt());
}

void GameProtocol::serialize_W3GS_SLOTINFOJOIN(QVariantHash data, QByteArrayBuilder* out){
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
void GameProtocol::serialize_W3GS_REJECTJOIN(QVariantHash data, QByteArrayBuilder* out){
    quint32 reason = (quint32) data.value("reason").toInt();
    out->insertDWord(reason);
}
void GameProtocol::serialize_W3GS_PLAYERINFO(QVariantHash data, QByteArrayBuilder* out){
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
    quint32 counter = (quint32) data.value("player.counter").toUInt();
    quint8 playerId = (quint8) data.value("player.id").toUInt();
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

void GameProtocol::serialize_W3GS_PLAYERLEFT(QVariantHash data, QByteArrayBuilder* out){
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
void GameProtocol::serialize_W3GS_PLAYERLOADED(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_SLOTINFO(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_COUNTDOWN_START(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_COUNTDOWN_END(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_INCOMING_ACTION(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_CHAT_FROM_HOST(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_LEAVERS(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_GAMEINFO(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_CREATEGAME(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_REFRESHGAME(QVariantHash data, QByteArrayBuilder* out){
}
void GameProtocol::serialize_W3GS_DECREATEGAME(QVariantHash data, QByteArrayBuilder* out){
}

void GameProtocol::deserialize_W3GS_REQJOIN(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_LEAVEREQ(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_GAMELOADED_SELF(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_OUTGOING_ACTION(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_OUTGOING_KEEPALIVE(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_CHAT_TO_HOST(QByteArrayBuilder data, QVariantHash *out)
{
//    (BYTE) Total

//    For each total:
//    (BYTE) To player number
//    (BYTE) From player number
//    (BYTE) Flags

//    For Flag 0x10:
//       (STRING) Message
//    For Flag 0x11:
//       (BYTE) Team
//    For Flag 0x12:
//       (BYTE) Color
//    For Flag 0x13:
//       (BYTE) Race
//    For Flag 0x14:
//       (BYTE) Handicap
//    For Flag 0x20:
//       (DWORD) Extra Flags
//       (STRING) Message

    quint8 total = data.getByte();
    out->insert("total", total);
    QVariantList messages;
    for (int i = 0; i < total; i++) {
        QVariantHash message;
        quint8 to = data.getByte();
        quint8 from = data.getByte();
        quint8 flag = data.getByte();
        message["to"] = to;
        message["from"] = from;
        message["flag"] = flag;
        switch (flag){
        case 0x10: // Normal chat (lobby)
            message["text"] = data.getString();
            break;
        case 0x11: // Switch Teams
            message["team"] = data.getByte(); // Team
            break;
        case 0x12: // Change Color
            message["color"] = data.getByte(); // Color
            break;
        case 0x13: // Change Race
            message["race"] = data.getByte(); // Race
            break;
        case 0x14: // Change Handicap
            message["handicap"] = data.getByte(); // Handicap
            break;
        case 0x20: // In-game Message
            message["extra_flags"] = data.getDWord(); // Extra flags
            message["text"] = data.getString(); // Message
            break;
        }
        messages.append(message);
    }
    out->insert("messages", messages);
}

void GameProtocol::deserialize_W3GS_SEARCHGAME(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_PING_FROM_OTHERS(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_PONG_TO_OTHERS(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_CLIENTINFO(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_STARTDOWNLOAD(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::serialize_W3GS_MAPCHECK(QVariantHash data, QByteArrayBuilder* out){
    /*
     * (DWORD) Unknown
     * (STRING) File Path
     * (DWORD) File size
     * (DWORD) Map info
     * (DWORD) File CRC encryption
     * (DWORD) File SHA-1 hash
    */
    QString filePath = data.value("filepath").toString();
    quint32 fileSize = data.value("filesize").toUInt();
    quint32 mapInfo = data.value("mapinfo").toUInt();
    quint32 mapCRC = data.value("mapcrc").toUInt();
    QByteArray mapSHA1 = data.value("mapsha1").toByteArray();

    out->insertDWord(1); // Unknown
    out->insertString(filePath);
    out->insertDWord(fileSize);
    out->insertDWord(mapInfo);
    out->insertDWord(mapCRC);
    out->insertVoid(mapSHA1);
}

void GameProtocol::serialize_W3GS_STARTDOWNLOAD(QVariantHash data, QByteArrayBuilder* out){
}

void GameProtocol::serialize_W3GS_MAPPART(QVariantHash data, QByteArrayBuilder* out){
}

void GameProtocol::serialize_W3GS_INCOMING_ACTION2(QVariantHash data, QByteArrayBuilder* out){
}

void GameProtocol::serialize_W3GS_SEARCHGAME(QVariantHash data, QByteArrayBuilder *out)
{

}

void GameProtocol::deserialize_W3GS_MAPSIZE(QByteArrayBuilder data, QVariantHash* out){
    /*
     * (DWORD) Unknown
     * (BYTE) Size Flag
     * (DWORD) Map size
     */
    out->insert("unknown", (qint64) data.getDWord());
    out->insert("sizeflag", data.getByte());
    out->insert("mapsize", (qint64) data.getDWord());
}

void GameProtocol::deserialize_W3GS_MAPPARTOK(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_MAPPARTNOTOK(QByteArrayBuilder data, QVariantHash *out)
{

}

void GameProtocol::deserialize_W3GS_PONG_TO_HOST(QByteArrayBuilder data, QVariantHash *out)
{
    out->insert("tickcount", data.getDWord());
}
