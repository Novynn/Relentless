#ifndef CLIENTPROTOCOL_H
#define CLIENTPROTOCOL_H

#include <client/protocol.h>
#include <shared/packet/bncspacket.h>

class Game;

class ClientProtocol : public Protocol
{
    Q_OBJECT
public:
    enum Product {
        PRODUCT_WAR3,
        PRODUCT_W3XP
    };

    static ClientProtocol* instance() {
        if (_instance == 0) {
            _instance = new ClientProtocol();
        }
        return _instance;
    }

    static Packet* serialize(uint p, QString s, QVariant d) {
        return instance()->_serialize<ClientProtocol, BNCSPacket>(p, s, d);
    }

    static Packet* serialize(uint p, QVariantHash data = QVariantHash()) {
        return instance()->_serialize<ClientProtocol, BNCSPacket>(p, data);
    }

    static QVariantHash* deserialize(uint p, QByteArrayBuilder data) {
        return instance()->_deserialize<ClientProtocol, BNCSPacket>(p, data);
    }

    BNCSPacket* Serialize_SID_AUTH_INFO();
    BNCSPacket* Serialize_SID_AUTH_CHECK(quint32 clientToken, quint32 versionCode, quint32 versionHash, int keyCount, const QByteArray keyData, const QString checkString, const QString username);
    BNCSPacket* Serialize_SID_ACCOUNTLOGON(const QByteArray clientKey, const QString username);
    BNCSPacket* Serialize_SID_ACCOUNTLOGINPROOF(const QByteArray passwordProof);
    BNCSPacket* Serialize_SID_JOINCHANNEL(const QString channel, quint32 flags = 0x00);
    BNCSPacket* Serialize_SID_PING(quint32 cookie);
    BNCSPacket* Serialize_SID_CHATCOMMAND(const QString message);
    BNCSPacket* Serialize_SID_ENTERCHAT();
    BNCSPacket* Serialize_SID_STOPADV();
    BNCSPacket* Serialize_SID_STARTADVEX3(quint32 state, quint32 uptime, quint16 gameType, quint16 subGameType, quint32 ladderType, QString gameName, QString gamePassword, QByteArray statString);
    BNCSPacket* Serialize_SID_NETGAMEPORT(int port);
    BNCSPacket* Serialize_SID_GETADVLISTEX(quint16 gameType, quint16 subGameType,
                                           quint32 filter, quint32 count,
                                           QString name, QString password,
                                           QString statstring);

    QVariantHash Deserialize_SID_PING(QByteArrayBuilder b);
    QVariantHash Deserialize_SID_AUTH_INFO(QByteArrayBuilder b);
    QVariantHash Deserialize_SID_AUTH_CHECK(QByteArrayBuilder b);
    QVariantHash Deserialize_SID_ACCOUNT_LOGON(QByteArrayBuilder b);
    QVariantHash Deserialize_SID_ACCOUNTLOGONPROOF(QByteArrayBuilder in);
    QVariantHash Deserialize_SID_ENTERCHAT(QByteArrayBuilder b);
    QVariantHash Deserialize_SID_CHATEVENT(QByteArrayBuilder b);
    QVariantHash Deserialize_SID_STARTADVEX3(QByteArrayBuilder b);
    QVariantHash Deserialize_SID_GETADVLISTEX(QByteArrayBuilder b);
    QVariantHash Deserialize_SID_CLANINFO(QByteArrayBuilder b);
    QVariantHash Deserialize_SID_FRIENDSLIST(QByteArrayBuilder in);
private:

    static ClientProtocol* _instance;
};

#endif // CLIENTPROTOCOL_H
