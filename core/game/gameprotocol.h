#ifndef GAMEPROTOCOL_H
#define GAMEPROTOCOL_H

#include "shared/packet/w3gspacket.h"

class GameProtocol
{
public:
    static W3GSPacket* serialize(W3GSPacket::PacketId packetId, QVariantHash data = QVariantHash());
    static QVariantHash deserialize(W3GSPacket::PacketId packetId, QByteArrayBuilder data);

    // Outgoing Data
    static W3GSPacket* serialize_W3GS_PING_FROM_HOST();
    static W3GSPacket* serialize_W3GS_SLOTINFOJOIN(QByteArray slotInfo, quint32 seed, quint8 layout, quint8 players);
    static W3GSPacket* serialize_W3GS_REJECTJOIN(quint32 reason);
    static W3GSPacket* serialize_W3GS_PLAYERINFO(quint32 counter, quint8 playerId, QString playerName);
    static W3GSPacket* serialize_W3GS_PLAYERLEFT(quint8 playerId, quint32 reason);
    static W3GSPacket* serialize_W3GS_PLAYERLOADED(quint8 playerId);
    static W3GSPacket* serialize_W3GS_SLOTINFO(QByteArray slotInfo, quint32 seed, quint8 layout, quint8 players);
    static W3GSPacket* serialize_W3GS_COUNTDOWN_START();
    static W3GSPacket* serialize_W3GS_COUNTDOWN_END();
    static W3GSPacket* serialize_W3GS_INCOMING_ACTION(); //TODO
    static W3GSPacket* serialize_W3GS_CHAT_FROM_HOST(QList<quint8> playerIds, quint8 fromPlayer, quint8 flags,
                                                     quint32 extraFlags, QString message);
    static W3GSPacket* serialize_W3GS_LEAVERS();
    static W3GSPacket* serialize_W3GS_MAPCHECK(QString filePath, quint32 fileSize, quint32 mapInfo, quint32 mapCRC, QByteArray mapSHA1); //TODO
    static W3GSPacket* serialize_W3GS_MAPPART(); //TODO
    static W3GSPacket* serialize_W3GS_INCOMING_ACTION2(); //TODO

    static W3GSPacket* serialize_W3GS_GAMEINFO(); //UDP
    static W3GSPacket* serialize_W3GS_CREATEGAME(); //UDP
    static W3GSPacket* serialize_W3GS_REFRESHGAME(); //UDP
    static W3GSPacket* serialize_W3GS_DECREATEGAME(); //UDP

    // Incoming Data
    static QVariantHash deserialize_W3GS_REQJOIN(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_LEAVEREQ(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_GAMELOADED_SELF(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_OUTGOING_ACTION(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_OUTGOING_KEEPALIVE(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_CHAT_TO_HOST(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_SEARCHGAME(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_PING_FROM_OTHERS(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_PONG_TO_OTHERS(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_CLIENTINFO(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_STARTDOWNLOAD(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_MAPSIZE(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_MAPPARTOK(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_MAPPARTNOTOK(QByteArrayBuilder data);
    static QVariantHash deserialize_W3GS_PONG_TO_HOST(QByteArrayBuilder data);

private:
    // Purely static, so no class creation
    GameProtocol(){}

    // Disable Copying
    GameProtocol(const GameProtocol &);
    GameProtocol& operator=(const GameProtocol &);
};

#endif // GAMEPROTOCOL_H
