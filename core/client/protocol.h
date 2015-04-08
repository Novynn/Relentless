#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>
#include "shared/packet/packet.h"

class Protocol : public QObject
{
    Q_OBJECT
public:
    template<class PROTOCOL, class PACKET>
    PACKET* _serialize(uint packetId, QVariantHash data = QVariantHash()){
        QByteArrayBuilder* out = new QByteArrayBuilder();

        QString method = "serialize_" + Packet::packetIdToString<PACKET>(packetId);
        PROTOCOL::staticMetaObject.invokeMethod(PROTOCOL::instance(),
                                      method.toLatin1().constData(),
                                      Qt::DirectConnection,
                                      Q_ARG(QVariantHash, data),
                                      Q_ARG(QByteArrayBuilder*, out));

        typename PACKET::template PacketId p = static_cast<typename PACKET::PacketId>(packetId);
        PACKET* packet = new PACKET(p, *out);

        return packet;
    }


    template<class PROTOCOL, class PACKET>
    PACKET* _serialize(uint packetId, QString key, QVariant value){
        QVariantHash data;
        data.insert(key, value);
        return _serialize<PROTOCOL, PACKET>(packetId, data);
    }

    template<class PROTOCOL, class PACKET>
    QVariantHash* _deserialize(uint packetId, QByteArrayBuilder data){
        QVariantHash* out = new QVariantHash();

        QString method = "deserialize_" + Packet::packetIdToString<PACKET>(packetId);
        PROTOCOL::staticMetaObject.invokeMethod(PROTOCOL::instance(),
                                      method.toLatin1().constData(),
                                      Qt::DirectConnection,
                                      Q_ARG(QByteArrayBuilder, data),
                                      Q_ARG(QVariantHash*, out));
        return out;
    }

protected:
    // Purely static, so no class creation
    Protocol(){}

private:
};

#endif // PROTOCOL_H
