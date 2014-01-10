#include "clientprotocol.h"

#include "game/game.h"

BNCSPacket* ClientProtocol::Serialize_SID_CHATCOMMAND(const QString message){
    QByteArrayBuilder out;
    out.insertString(message.simplified());
    return new BNCSPacket(BNCSPacket::SID_CHATCOMMAND, out);
}

BNCSPacket* ClientProtocol::Serialize_SID_STOPADV(){
    return new BNCSPacket(BNCSPacket::SID_STOPADV);
}

QVariantHash ClientProtocol::Deserialize_BNLS_CHOOSENLSREVISION(QByteArrayBuilder in){
    QVariantHash out;
    out["success"] = in.getDWord();
    return out;
}

QVariantHash ClientProtocol::Deserialize_SID_CHATEVENT(QByteArrayBuilder in){
    QVariantHash out;
    out["id"] = in.getDWord();
    out["flags"] = in.getDWord();
    out["ping"] = in.getDWord();
    in.getDWord(); // Defunct
    in.getDWord(); // Defunct
    in.getDWord(); // Defunct
    out["username"] = in.getString();
    out["text"] = in.getString();
    return out;
}

QVariantHash ClientProtocol::Deserialize_SID_STARTADVEX3(QByteArrayBuilder b){
    QVariantHash out;
    out["success"] = b.getByte();
    return out;
}

QVariantHash ClientProtocol::Deserialize_SID_ENTERCHAT(QByteArrayBuilder b){
    QVariantHash out;
    out["uname"] = b.getString();       // (STRING) Unique name
    out["statstring"] = b.getString();  // (STRING) Statstring
    out["aname"] = b.getString();       // (STRING) Account name
    return out;
}

BNCSPacket* ClientProtocol::Serialize_SID_JOINCHANNEL(const QString channel, quint32 flags){
    /* Flags
     *    0x00: NoCreate join
     *    0x01: First join
     *    0x02: Forced join
     *    0x05: D2 first join
    */
    QByteArrayBuilder out;
    out.insertDWord(flags);
    out.insertString(channel);
    return new BNCSPacket(BNCSPacket::SID_JOINCHANNEL, out);
}

BNCSPacket* ClientProtocol::Serialize_SID_ENTERCHAT(){
    QByteArrayBuilder out;
    out.insertByte(0); // Username (Null on WC3)
    out.insertByte(0); // Statstring (Null on CD-key products)
    return new BNCSPacket(BNCSPacket::SID_ENTERCHAT, out);
}

QVariantHash ClientProtocol::Deserialize_SID_ACCOUNTLOGONPROOF(QByteArrayBuilder in){
    QVariantHash out;
    out["result"] = in.getDWord();       // (DWORD) Status
    out["passwordproof"] = in.remove(0, 20);          // (BYTE) [20] Server Password Proof (M2)
    out["info"] = in.getString();     // (STRING) Additional information
    return out;
}

BNCSPacket* ClientProtocol::Serialize_SID_ACCOUNTLOGINPROOF(const QByteArray passwordProof){
    QByteArrayBuilder out;
    out.insertVoid(passwordProof);
    return new BNCSPacket(BNCSPacket::SID_AUTH_ACCOUNTLOGONPROOF, out);
}

QVariantHash ClientProtocol::Deserialize_BNLS_LOGONPROOF(QByteArrayBuilder in){
    QVariantHash out;
    out["passwordproof"] = in.getVoid(20);
    return out;
}

BNLSPacket* ClientProtocol::Serialize_BNLS_LOGONPROOF(const QByteArray salt, const QByteArray serverKey){
    QByteArrayBuilder out;
    out.insertVoid(salt);
    out.insertVoid(serverKey);
    return new BNLSPacket(BNLSPacket::BNLS_LOGONPROOF, out);
}

BNLSPacket *ClientProtocol::Serialize_BNLS_VERSIONCHECKEX2(const quint32 product, const QByteArray fileTime, const QString fileName, const QString valueString){
    QByteArrayBuilder out;
//    (DWORD) Product ID.*
//    (DWORD) Flags.**
//    (DWORD) Cookie.
//    (FILETIME) Timestamp for version check archive.
//    (STRING) Version check archive filename.
//    (STRING) Checksum formula.
    out.insertDWord(product); // 0x07, 0x08
    out.insertDWord(0);
    out.insertDWord(0);
    out.insertVoid(fileTime);
    out.insertString(fileName);
    out.insertString(valueString);
    return new BNLSPacket(BNLSPacket::BNLS_VERSIONCHECKEX2, out);
}

BNLSPacket *ClientProtocol::Serialize_BNLS_CDKEY_EX(const QStringList keys, quint32 serverToken){
    QByteArrayBuilder out;
//    (DWORD) Cookie
//    (BYTE) Number of CD-keys to encrypt
//    (DWORD) Flags
//    (DWORD) [] Server session key(s), depending on flags
//    (DWORD) [] Client session key(s), depending on flags
//    (STRING) [] CD-keys. No dashes or spaces
    out.insertDWord(0);
    out.insertByte(keys.count());
    out.insertDWord(0x01);          // CDKEY_SAME_SESSION_KEY
    out.insertDWord(serverToken);
    foreach(QString key, keys)
        out.insertString(key);
    return new BNLSPacket(BNLSPacket::BNLS_CDKEY_EX, out);
}

BNLSPacket *ClientProtocol::Serialize_BNLS_LOGONCHALLENGE(const QString username, const QString password){
    QByteArrayBuilder out;
    out.insertString(username);
    out.insertString(password);
    return new BNLSPacket(BNLSPacket::BNLS_LOGONCHALLENGE, out);
}

QVariantHash ClientProtocol::Deserialize_SID_ACCOUNT_LOGON(QByteArrayBuilder in){
    QVariantHash out;
    out["result"] = in.getDWord();   // (DWORD) Status
    out["salt"] = in.getVoid(32);    // (BYTE) [32] Salt (s)
    out["key"] = in.getVoid(32);     // (BYTE) [32] Server Key (B)
    return out;
}

BNCSPacket* ClientProtocol::Serialize_SID_AUTH_CHECK(quint32 clientToken,
                                 quint32 versionCode,
                                 quint32 versionHash,
                                 int keyCount,
                                 const QByteArray keyData,
                                 const QString checkString,
                                 const QString username){
    QByteArrayBuilder out;
    out.insertDWord(clientToken);
    out.insertDWord(versionCode);
    out.insertDWord(versionHash);
    out.insertDWord(keyCount);
    out.insertDWord(0x00);
    out.insertVoid(keyData);
    out.insertString(checkString);
    out.insertString(username);

    return new BNCSPacket(BNCSPacket::SID_AUTH_CHECK, out);
}

QVariantHash ClientProtocol::Deserialize_BNLS_CDKEY_EX(QByteArrayBuilder in){
    QVariantHash out;
    out["cookie"] = in.getDWord();       // (DWORD) Cookie
    out["rcount"] = in.getByte();        // (BYTE) Number of CD-keys requested
    out["scount"] = in.getByte();        // (BYTE) Number of successfully encrypted CD-keys
    out["bitmask"] = in.getDWord();      // (DWORD) Bit mask

    for(uint i = 0; i < out["scount"].toUInt(); ++i){                     // For each successful CD Key:
        out["clientkey" + QString::number(i)] = in.getDWord();  // (DWORD) Client session key
        out["keydata" + QString::number(i)] = in.getVoid(36);       // (DWORD) [9] CD-key data
    }
    return out;
}

BNCSPacket* ClientProtocol::Serialize_SID_ACCOUNTLOGON(const QByteArray clientKey, const QString username){
    QByteArrayBuilder out;
    out.insertVoid(clientKey);
    out.insertString(username);
    return new BNCSPacket(BNCSPacket::SID_AUTH_ACCOUNTLOGON, out);
}

QVariantHash ClientProtocol::Deserialize_BNLS_LOGONCHALLENGE(QByteArrayBuilder in){
    QVariantHash out;
    out["clientkey"] = in.getVoid(32);  // (DWORD)[8] Data for SID_AUTH_ACCOUNTLOGON
    return out;
}

QVariantHash ClientProtocol::Deserialize_BNLS_VERSIONCHECKEX2(QByteArrayBuilder in){
    QVariantHash out;
    out["success"] = in.getDWord();
    if (out["success"].toBool()){
        out["version"] = in.getDWord();
        out["checksum"] = in.getDWord();
        out["checkstring"] = in.getString();
        out["cookie"] = in.getDWord();
        out["versioncode"] = in.getDWord();
    }
    else {
        out["cookie"] = in.getDWord();
    }
    return out;
}

QVariantHash ClientProtocol::Deserialize_SID_AUTH_CHECK(QByteArrayBuilder in){
    QVariantHash out;
    out["result"] = in.getDWord();       //(DWORD) Result
    out["info"] = in.getString();    // (STRING) Additional Information
    return out;
}

BNLSPacket* ClientProtocol::Serialize_BNLS_CHOOSENLSREVISION(){
    QByteArrayBuilder out;
    out.insertDWord(0x02); // WC3
    return new BNLSPacket(BNLSPacket::BNLS_CHOOSENLSREVISION, out);
}

QVariantHash ClientProtocol::Deserialize_SID_AUTH_INFO(QByteArrayBuilder in){
    QVariantHash out;
    out["logontype"] = in.getDWord();
    out["servertoken"] = in.getDWord();
    out["udpvalue"] = in.getDWord();
    QByteArrayBuilder fileTime;
    for (int i = 0; i < 8;i++) fileTime.insertByte(in.getByte() + 1); // What??!?!?
    out["filetime"] = fileTime;
    out["filename"] = in.getString();
    out["valuestring"] = in.getString();
    return out;
}

BNCSPacket* ClientProtocol::Serialize_SID_PING(quint32 cookie){
    QByteArrayBuilder out;
    out.insertDWord(cookie);
    return new BNCSPacket(BNCSPacket::SID_PING, out);
}

QVariantHash ClientProtocol::Deserialize_SID_PING(QByteArrayBuilder in){
    QVariantHash out;
    out["cookie"] = in.getDWord();
    return out;
}

BNCSPacket* ClientProtocol::Serialize_SID_AUTH_INFO(){
    QByteArrayBuilder out;
    out.insertDWord(0);                     //    (DWORD) Protocol ID (0)
    out.insertString("68XI", false);        //    (DWORD) Platform ID
    // TODO
    if (true)
        out.insertString("PX3W", false);
    else
        out.insertString("3RAW", false);    //    (DWORD) Product ID
    out.insertDWord(0x1a);                  //    (DWORD) Version Byte
    out.insertDWord(0);                     //    (DWORD) Product language*
    //
    out.insertByte(0xc0);
    out.insertByte(0xa8);
    out.insertByte(0x01);
    out.insertByte(0x03);                   //    (DWORD) Local IP for NAT compatibility*
    //
    out.insertByte(0x30);
    out.insertByte(0xfd);
    out.insertByte(0xff);
    out.insertByte(0xff);                   //    (DWORD) Time zone bias*
    //
    out.insertByte(0x09);
    out.insertByte(0x04);
    out.insertByte(0x00);
    out.insertByte(0x00);                   //    (DWORD) Locale ID*
    //
    out.insertByte(0x09);
    out.insertByte(0x04);
    out.insertByte(0x00);
    out.insertByte(0x00);                   //    (DWORD) Language ID*
    //
    out.insertString("USA");                //    (STRING) Country abreviation
    out.insertString("United States");        //    (STRING) Country

    return new BNCSPacket(BNCSPacket::SID_AUTH_INFO, out);
}

BNCSPacket* ClientProtocol::Serialize_SID_STARTADVEX3(quint32 state,
                                                      quint32 uptime,
                                                      quint16 gameType,
                                                      quint16 subGameType,
                                                      quint32 ladderType,
                                                      QString gameName,
                                                      QString gamePassword,
                                                      QByteArray statString){
    /*
     * (DWORD) Game State
     * (DWORD) Game Uptime in seconds
     * (WORD) Game Type
     * (WORD) Sub Game Type
     * (DWORD) Provider Version Constant (0xFF)
     * (DWORD) Ladder Type
     * (STRING) Game Name
     * (STRING) Game Password
     * (STRING) Game Statstring
     */
    QByteArrayBuilder out;
    out.insertDWord(state);
    out.insertDWord(uptime);
    out.insertWord(gameType);
    out.insertWord(subGameType);
    out.insertDWord(1023);
    out.insertDWord(ladderType);
    out.insertString(gameName);
    out.insertString(gamePassword);
    out.insertVoid(statString);
    out.insertByte(0); // end of statString

    return new BNCSPacket(BNCSPacket::SID_STARTADVEX3, out);
}

BNCSPacket* ClientProtocol::Serialize_SID_NETGAMEPORT(int port){
    QByteArrayBuilder out;
    out.insertWord(port);
    return new BNCSPacket(BNCSPacket::SID_NETGAMEPORT, out);
}
