#ifndef GAMEPROTOCOL_H
#define GAMEPROTOCOL_H

#include "shared/packet/w3gspacket.h"
#include <QVariantHash>

class GameProtocol : public QObject
{
    Q_OBJECT
public:
    static GameProtocol* _instance;
    static GameProtocol* instance(){
        if (_instance == 0){
            _instance = new GameProtocol();
        }
        return _instance;
    }

    static W3GSPacket* serialize(W3GSPacket::PacketId packetId, QVariantHash data = QVariantHash());
    static W3GSPacket* serialize(W3GSPacket::PacketId packetId, QString key, QVariant value);
    static QVariantHash* deserialize(W3GSPacket::PacketId packetId, QByteArrayBuilder data);

    // Outgoing Data
    Q_INVOKABLE void serialize_W3GS_PING_FROM_HOST(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_SLOTINFOJOIN(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_REJECTJOIN(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_PLAYERINFO(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_PLAYERLEFT(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_PLAYERLOADED(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_SLOTINFO(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_COUNTDOWN_START(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_COUNTDOWN_END(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_INCOMING_ACTION(QVariantHash data, QByteArrayBuilder* out); //TODO
    Q_INVOKABLE void serialize_W3GS_CHAT_FROM_HOST(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_LEAVERS(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_MAPCHECK(QVariantHash data, QByteArrayBuilder* out); //TODO
    Q_INVOKABLE void serialize_W3GS_STARTDOWNLOAD(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_MAPPART(QVariantHash data, QByteArrayBuilder* out); //TODO
    Q_INVOKABLE void serialize_W3GS_INCOMING_ACTION2(QVariantHash data, QByteArrayBuilder* out); //TODO

    Q_INVOKABLE void serialize_W3GS_SEARCHGAME(QVariantHash data, QByteArrayBuilder* out); // UDP
    Q_INVOKABLE void serialize_W3GS_GAMEINFO(QVariantHash data, QByteArrayBuilder* out); //UDP
    Q_INVOKABLE void serialize_W3GS_CREATEGAME(QVariantHash data, QByteArrayBuilder* out); //UDP
    Q_INVOKABLE void serialize_W3GS_REFRESHGAME(QVariantHash data, QByteArrayBuilder* out); //UDP
    Q_INVOKABLE void serialize_W3GS_DECREATEGAME(QVariantHash data, QByteArrayBuilder* out); //UDP

    // Incoming Data
    Q_INVOKABLE void deserialize_W3GS_REQJOIN(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_LEAVEREQ(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_GAMELOADED_SELF(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_OUTGOING_ACTION(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_OUTGOING_KEEPALIVE(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_CHAT_TO_HOST(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_SEARCHGAME(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_PING_FROM_OTHERS(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_PONG_TO_OTHERS(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_CLIENTINFO(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_STARTDOWNLOAD(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_MAPSIZE(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_MAPPARTOK(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_MAPPARTNOTOK(QByteArrayBuilder data, QVariantHash* out);
    Q_INVOKABLE void deserialize_W3GS_PONG_TO_HOST(QByteArrayBuilder data, QVariantHash* out);

private:
    // Purely static, so no class creation
    GameProtocol(){}
};

#endif // GAMEPROTOCOL_H
