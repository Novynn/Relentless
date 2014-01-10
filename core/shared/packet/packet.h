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

    QByteArray data() const {
        return mData;
    }
    void setData(QByteArray data){
        mData = data;
    }

    qint64 activeTime() const {
        return mActiveTime;
    }
    void setActiveTime(qint64 time){
        mActiveTime = time;
    }

    Locality locality() const {
        return mLocality;
    }

    void setLocality(Locality l){
        mLocality = l;
    }

    uint importance() const {
        return mImportance;
    }

    void setImportance(uint i){
        mImportance = i;
    }

    inline void setImportant(){
        setImportance(100);
    }

private:
    virtual void stripHeader() = 0;
protected:
    Packet(QByteArray data = QByteArray(), Locality from = FROM_LOCAL, uint importance = 1, qint64 activeTime = QDateTime::currentMSecsSinceEpoch())
        : mData(data), mLocality(from), mImportance(importance), mActiveTime(activeTime){}

    QByteArray mData;
    Locality mLocality;
    uint mImportance;
    qint64 mActiveTime;
};

QDebug operator<<(QDebug dbg, const Packet* p);

#endif // PACKET_H
