#ifndef BNLSPACKET_H
#define BNLSPACKET_H

#include <QObject>
#include <QMetaEnum>
#include "packet.h"

class BNLSPacket : public Packet {
    Q_OBJECT
public:
    enum PacketId {
        BNLS_NULL                   = 0x00,
        BNLS_CDKEY                  = 0x01,
        BNLS_LOGONCHALLENGE         = 0x02,
        BNLS_LOGONPROOF             = 0x03,
        BNLS_CREATEACCOUNT          = 0x04,
        BNLS_CHANGECHALLENGE        = 0x05,
        BNLS_CHANGEPROOF            = 0x06,
        BNLS_UPGRADECHALLENGE       = 0x07,
        BNLS_UPGRADEPROOF           = 0x08,
        BNLS_VERSIONCHECK           = 0x09,
        BNLS_CONFIRMLOGON           = 0x0A,
        BNLS_HASHDATA               = 0x0B,
        BNLS_CDKEY_EX               = 0x0C,
        BNLS_CHOOSENLSREVISION      = 0x0D,
        BNLS_AUTHORIZE              = 0x0E,
        BNLS_AUTHORIZEPROOF         = 0x0F,
        BNLS_REQUESTVERSIONBYTE     = 0x10,
        BNLS_VERIFYSERVER           = 0x11,
        BNLS_RESERVESERVERSLOTS     = 0x12,
        BNLS_SERVERLOGONCHALLENGE   = 0x13,
        BNLS_SERVERLOGONPROOF       = 0x14,
        BNLS_VERSIONCHECKEX         = 0x18,
        BNLS_VERSIONCHECKEX2        = 0x1A,
        BNLS_WARDEN                 = 0x7D,
        BNLS_IPBAN                  = 0xFF
    };

    Q_ENUMS(PacketId)

    explicit BNLSPacket(PacketId pId, QByteArray data = QByteArray(), Locality from = Packet::FROM_LOCAL)
        : Packet(data, from), mPId(pId){
        if (locality() == Packet::FROM_SERVER)
            stripHeader();
    }

    uint packetId() const {
        return (uint) mPId;
    }

    QString packetIdString() const {
        QMetaObject metaObject = BNLSPacket::staticMetaObject;
        QMetaEnum metaEnum = metaObject.enumerator(metaObject.indexOfEnumerator("PacketId"));
        return QString(metaEnum.valueToKey(mPId));
    }

    Protocol protocol() const {
        return PROTOCOL_BNLS;
    }

    QString protocolString() const {
        return "BNLS";
    }

    QByteArray toPackedData() const {
        QByteArrayBuilder out;
        out.insertWord(mData.size() + 3);
        out.insertByte(mPId);
        out.insertVoid(mData);
        return out;
    }

private:
    void stripHeader(){
        QByteArrayBuilder content = mData;
        if (content.size() < 3 || content.size() > 65535) return;
        word s = content.getWord(); if (s != mData.size()) return;
        byte i = content.getByte(); if (i != packetId()) return;

        mData = content;
    }

    PacketId mPId;
};

#endif // BNLSPACKET_H
