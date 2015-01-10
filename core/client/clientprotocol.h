#ifndef CLIENTPROTOCOL_H
#define CLIENTPROTOCOL_H

#include <QTcpSocket>
#include <QSslSocket>
#include <QSslCipher>
#include <QCryptographicHash>
#include "shared/packet/bncspacket.h"
#include "shared/packet/bnlspacket.h"
#include "shared/functions.h"

class Game;

class ClientProtocol {
public:
    enum Product {
        PRODUCT_WAR3,
        PRODUCT_W3XP
    };

    // BNLS Packet handling
    BNLSPacket* Serialize_BNLS_CHOOSENLSREVISION();
    BNLSPacket* Serialize_BNLS_LOGONPROOF(const QByteArray salt, const QByteArray serverKey);
    BNLSPacket* Serialize_BNLS_VERSIONCHECKEX2(const dword product, const QByteArray fileTime,
                                               const QString fileName, const QString valueString);
    BNLSPacket* Serialize_BNLS_CDKEY_EX(const QStringList keys, quint32 serverToken);
    BNLSPacket* Serialize_BNLS_LOGONCHALLENGE(const QString username, const QString password);
    QVariantHash Deserialize_BNLS_CHOOSENLSREVISION(QByteArrayBuilder b);
    QVariantHash Deserialize_BNLS_CDKEY_EX(QByteArrayBuilder b);
    QVariantHash Deserialize_BNLS_VERSIONCHECKEX2(QByteArrayBuilder b);
    QVariantHash Deserialize_BNLS_LOGONCHALLENGE(QByteArrayBuilder b);
    QVariantHash Deserialize_BNLS_LOGONPROOF(QByteArrayBuilder b);

    // BNCS Packet handling
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

};

#endif // CLIENTPROTOCOL_H
