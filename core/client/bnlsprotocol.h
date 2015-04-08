#ifndef BNLSPROTOCOL_H
#define BNLSPROTOCOL_H

#include <client/protocol.h>
#include <shared/packet/bnlspacket.h>


class BNLSPacket;

class BNLSProtocol : public Protocol
{
    Q_OBJECT
public:
    static BNLSProtocol* instance() {
        if (_instance == 0) {
            _instance = new BNLSProtocol();
        }
        return _instance;
    }

    static Packet* serialize(uint p, QString s, QVariant d) {
        return instance()->_serialize<BNLSProtocol, BNLSPacket>(p, s, d);
    }

    static Packet* serialize(uint p, QVariantHash data = QVariantHash()) {
        return instance()->_serialize<BNLSProtocol, BNLSPacket>(p, data);
    }

    static QVariantHash* deserialize(uint p, QByteArrayBuilder data) {
        return instance()->_deserialize<BNLSProtocol, BNLSPacket>(p, data);
    }

private:
    // BNLS Packet handling
    Q_INVOKABLE void serialize_BNLS_CHOOSENLSREVISION(QVariantHash data, QByteArrayBuilder* out);
    Q_INVOKABLE void serialize_BNLS_LOGONPROOF(QVariantHash data, QByteArrayBuilder *out);
    Q_INVOKABLE void serialize_BNLS_VERSIONCHECKEX2(QVariantHash data, QByteArrayBuilder *out);
    Q_INVOKABLE void serialize_BNLS_CDKEY_EX(QVariantHash data, QByteArrayBuilder *out);
    Q_INVOKABLE void serialize_BNLS_LOGONCHALLENGE(QVariantHash data, QByteArrayBuilder *out);
    Q_INVOKABLE void deserialize_BNLS_CHOOSENLSREVISION(QByteArrayBuilder data, QVariantHash *out);
    Q_INVOKABLE void deserialize_BNLS_CDKEY_EX(QByteArrayBuilder data, QVariantHash *out);
    Q_INVOKABLE void deserialize_BNLS_VERSIONCHECKEX2(QByteArrayBuilder data, QVariantHash *out);
    Q_INVOKABLE void deserialize_BNLS_LOGONCHALLENGE(QByteArrayBuilder data, QVariantHash *out);
    Q_INVOKABLE void deserialize_BNLS_LOGONPROOF(QByteArrayBuilder data, QVariantHash *out);

private:
    // Purely static, so no class creation
    BNLSProtocol(){}

    static BNLSProtocol* _instance;
};

#endif // BNLSPROTOCOL_H
