#ifndef GAMEPROTOCOL_H
#define GAMEPROTOCOL_H

#include "shared/packet/w3gspacket.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

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

    static W3GSPacket* serialize(W3GSPacket::PacketId packetId, QJsonObject data = QJsonObject());
    static W3GSPacket* serialize(W3GSPacket::PacketId packetId, QString key, QVariant value);
    static QJsonObject* deserialize(W3GSPacket::PacketId packetId, QByteArrayBuilder data);

    // Outgoing Data
    Q_INVOKABLE void serialize_W3GS_PING_FROM_HOST(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_SLOTINFOJOIN(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_REJECTJOIN(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_PLAYERINFO(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_PLAYERLEFT(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_PLAYERLOADED(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_SLOTINFO(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_COUNTDOWN_START(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_COUNTDOWN_END(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_INCOMING_ACTION(QJsonObject data, QByteArrayBuilder* out); //TODO
    Q_INVOKABLE void serialize_W3GS_CHAT_FROM_HOST(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_LEAVERS(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_MAPCHECK(QJsonObject data, QByteArrayBuilder* out); //TODO
    Q_INVOKABLE void serialize_W3GS_STARTDOWNLOAD(QJsonObject data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_W3GS_MAPPART(QJsonObject data, QByteArrayBuilder* out); //TODO
    Q_INVOKABLE void serialize_W3GS_INCOMING_ACTION2(QJsonObject data, QByteArrayBuilder* out); //TODO

    Q_INVOKABLE void serialize_W3GS_SEARCHGAME(QJsonObject data, QByteArrayBuilder* out); // UDP
    Q_INVOKABLE void serialize_W3GS_GAMEINFO(QJsonObject data, QByteArrayBuilder* out); //UDP
    Q_INVOKABLE void serialize_W3GS_CREATEGAME(QJsonObject data, QByteArrayBuilder* out); //UDP
    Q_INVOKABLE void serialize_W3GS_REFRESHGAME(QJsonObject data, QByteArrayBuilder* out); //UDP
    Q_INVOKABLE void serialize_W3GS_DECREATEGAME(QJsonObject data, QByteArrayBuilder* out); //UDP

    // Incoming Data
    Q_INVOKABLE void deserialize_W3GS_REQJOIN(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_LEAVEREQ(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_GAMELOADED_SELF(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_OUTGOING_ACTION(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_OUTGOING_KEEPALIVE(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_CHAT_TO_HOST(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_SEARCHGAME(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_PING_FROM_OTHERS(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_PONG_TO_OTHERS(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_CLIENTINFO(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_STARTDOWNLOAD(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_MAPSIZE(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_MAPPARTOK(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_MAPPARTNOTOK(QByteArrayBuilder data, QJsonObject* out);
    Q_INVOKABLE void deserialize_W3GS_PONG_TO_HOST(QByteArrayBuilder data, QJsonObject* out);

private:
    // Purely static, so no class creation
    GameProtocol(){}
};

#endif // GAMEPROTOCOL_H
