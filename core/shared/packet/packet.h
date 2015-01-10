#ifndef PACKET_H
#define PACKET_H

#include <QByteArray>
#include <QDateTime>
#include "shared/qbytearraybuilder.h"

class Packet : public QObject {
    Q_OBJECT
public:
    enum Protocol {
        PROTOCOL_W3GS = 0xF7,
        PROTOCOL_BNCS = 0xFF,
        PROTOCOL_BNLS = 0xAA
    };

    enum Locality {
        FROM_LOCAL,
        FROM_SERVER,
        FROM_INTERNAL
    };

    virtual QByteArray toPackedData() const = 0;
    virtual Protocol protocol() const = 0;
    virtual QString protocolString() const = 0;
    virtual uint packetId() const = 0;
    virtual QString packetIdString() const = 0;

    QByteArray data() const;
    void setData(QByteArray data);

    qint64 activeTime() const;
    void setActiveTime(qint64 time);

    Locality locality() const;
    void setLocality(Locality l);

    uint importance() const;
    void setImportance(uint i);
    inline void setImportant(){
        setImportance(100);
    }

private:
    virtual void stripHeader() = 0;
protected:
    Packet(QByteArray data = QByteArray(), Locality from = FROM_LOCAL, uint importance = 1, qint64 activeTime = QDateTime::currentMSecsSinceEpoch());

    QByteArray mData;
    Locality mLocality;
    uint mImportance;
    qint64 mActiveTime;
};

typedef Packet* PacketStar;
Q_DECLARE_METATYPE(PacketStar)

QDebug operator<<(QDebug dbg, const Packet* p);

#endif // PACKET_H
