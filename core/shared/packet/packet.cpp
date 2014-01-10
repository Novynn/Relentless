#include "packet.h"

QDebug operator<<(QDebug dbg, const Packet* p){
//    QString message = QString("[%1] [%2] Packet {\n\tPacketId: %3\n\tSize: %4\n\tDecayed: %5ms\n\tImportance: %6%\n\tData:\n%7\n}\n")
//            .arg((p->locality() == Packet::FROM_SERVER) ? "Incoming" : "Outgoing")
//            .arg(p->protocolString())
//            .arg(p->packetIdString())
//            .arg(p->data().size())
//            .arg(QDateTime::currentMSecsSinceEpoch() - p->activeTime())
//            .arg(p->importance())
//            .arg(QByteArrayBuilder(p->data()).toReadableString());

    QString message = QString("[%1] [%2] Packet {\n\tPacketId: %3\n\tSize: %4\n\tDecayed: %5ms\n\tImportance: %6%\n}\n")
            .arg((p->locality() == Packet::FROM_SERVER) ? "Incoming" : "Outgoing")
            .arg(p->protocolString())
            .arg(p->packetIdString())
            .arg(p->data().size())
            .arg(QDateTime::currentMSecsSinceEpoch() - p->activeTime())
            .arg(p->importance());

    dbg.nospace() << qPrintable(message);
    return dbg.space();
}
