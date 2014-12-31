#ifndef W3GSPACKET_H
#define W3GSPACKET_H

#include <QObject>
#include <QMetaEnum>
#include "packet.h"

class W3GSPacket : public Packet {
    Q_OBJECT
public:
    enum PacketId {
        W3GS_PING_FROM_HOST = 0x01,
        W3GS_SLOTINFOJOIN = 0x04,
        W3GS_REJECTJOIN = 0x05,
        W3GS_PLAYERINFO = 0x06,
        W3GS_PLAYERLEFT = 0x07,
        W3GS_PLAYERLOADED = 0x08,
        W3GS_SLOTINFO = 0x09,
        W3GS_COUNTDOWN_START = 0x0A,
        W3GS_COUNTDOWN_END = 0x0B,
        W3GS_INCOMING_ACTION = 0x0C,
        W3GS_CHAT_FROM_HOST = 0x0F,
        W3GS_LEAVERS = 0x1B,
        W3GS_REQJOIN = 0x1E,
        W3GS_LEAVEREQ = 0x21,
        W3GS_GAMELOADED_SELF = 0x23,
        W3GS_OUTGOING_ACTION = 0x26,
        W3GS_OUTGOING_KEEPALIVE = 0x27,
        W3GS_CHAT_TO_HOST = 0x28,
        W3GS_SEARCHGAME = 0x2F,
        W3GS_GAMEINFO = 0x30,
        W3GS_CREATEGAME = 0x31,
        W3GS_REFRESHGAME = 0x32,
        W3GS_DECREATEGAME = 0x33,
        W3GS_PING_FROM_OTHERS = 0x35,
        W3GS_PONG_TO_OTHERS = 0x36,
        W3GS_CLIENTINFO = 0x37,
        W3GS_MAPCHECK = 0x3D,
        W3GS_STARTDOWNLOAD = 0x3F,
        W3GS_MAPSIZE = 0x42,
        W3GS_MAPPART = 0x43,
        W3GS_MAPPARTOK = 0x44,
        W3GS_MAPPARTNOTOK = 0x45,
        W3GS_PONG_TO_HOST = 0x46,
        W3GS_INCOMING_ACTION2 = 0x48
    };

    Q_ENUMS(PacketId)

    W3GSPacket(PacketId pId, QByteArray data = QByteArray(), Locality from = Packet::FROM_LOCAL)
        : Packet(data, from), mPId(pId){
        if (locality() == Packet::FROM_SERVER)
            stripHeader();
    }

    W3GSPacket(uint pId, QByteArray data = QByteArray(), Locality from = Packet::FROM_LOCAL)
        : W3GSPacket((PacketId) pId, data, from){
    }

    uint packetId() const {
        return (uint) mPId;
    }

    QString packetIdString() const {
        return W3GSPacket::packetIdToString(mPId);
    }

    static QString packetIdToString(PacketId p) {
        QMetaObject metaObject = W3GSPacket::staticMetaObject;
        QMetaEnum metaEnum = metaObject.enumerator(metaObject.indexOfEnumerator("PacketId"));
        return QString(metaEnum.valueToKey(p));
    }

    Protocol protocol() const {
        return PROTOCOL_W3GS;
    }

    QString protocolString() const {
        return "W3GS";
    }

    QByteArray toPackedData() const {
        QByteArrayBuilder out;
        out.insertByte(PROTOCOL_W3GS);
        out.insertByte(mPId);
        out.insertWord(mData.size() + 4);
        out.insertVoid(mData);
        return out;
    }

private:
    void stripHeader(){
        QByteArrayBuilder content = mData;
        if (content.size() < 4 || content.size() > 65535) {
            qDebug() << "ERROR: Packet out of range.";
            return;
        }
        byte p = content.getByte();
        if (p != PROTOCOL_W3GS) {
            qDebug() << "ERROR: Packet not a BNCS packet.";
            return;
        }
        byte i = content.getByte();
        if (i != packetId()){
            qDebug() << "ERROR: Packet reported the wrong Iid.";
            return;
        }
        word s = content.getWord();
        if (s != mData.size()){
            qDebug() << "ERROR: Packet size incorrect. Reported: " << s << " Actual: " << mData.size();
            return;
        }
        mData = content.getVoid(content.size());
    }

    PacketId mPId;
};

#endif // W3GSPACKET_H
