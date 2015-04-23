#include "packet.h"

QDebug operator<<(QDebug dbg, const Packet* p){
    QString message = QString("[%1] [%2] Packet {\n\tPacketId: %3\n\tSize: %4\n\tDecayed: %5ms\n\tImportance: %6%\n\tData:\n%7\n}\n")
            .arg((p->locality() == Packet::FROM_SERVER) ? "Incoming" : "Outgoing")
            .arg(p->protocolString())
            .arg(p->packetIdString())
            .arg(p->data().size())
            .arg(QDateTime::currentMSecsSinceEpoch() - p->activeTime())
            .arg(p->importance())
            .arg(QByteArrayBuilder(p->toPackedData()).toReadableString());

//    QString message = QString("[%1] [%2] Packet {\n\tPacketId: %3\n\tSize: %4\n\tDecayed: %5ms\n\tImportance: %6%\n}\n")
//            .arg((p->locality() == Packet::FROM_SERVER) ? "Incoming" : "Outgoing")
//            .arg(p->protocolString())
//            .arg(p->packetIdString())
//            .arg(p->data().size())
//            .arg(QDateTime::currentMSecsSinceEpoch() - p->activeTime())
//            .arg(p->importance());

    dbg.nospace() << qPrintable(message);
    return dbg.space();
}


QByteArray Packet::data() const {
    return mData;
}

void Packet::setData(QByteArray data){
    mData = data;
}

qint64 Packet::activeTime() const {
    return mActiveTime;
}

void Packet::setActiveTime(qint64 time){
    mActiveTime = time;
}

Packet::Locality Packet::locality() const {
    return mLocality;
}

void Packet::setLocality(Packet::Locality l){
    mLocality = l;
}

uint Packet::importance() const {
    return mImportance;
}

void Packet::setImportance(uint i){
    mImportance = i;
}

Packet::Packet(const QByteArray &data, Packet::Locality from, uint importance, qint64 activeTime)
    : mData(data), mLocality(from), mImportance(importance), mActiveTime(activeTime){}
