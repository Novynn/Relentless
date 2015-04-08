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

QVariantHash ClientProtocol::Deserialize_SID_GETADVLISTEX(QByteArrayBuilder b)
{
    QVariantHash out;
    quint32 count = b.peekDWord();
    out["count"] = count;

    if (count > 20) {
        // Something went wrong!
        qDebug() << "ERROR: Malformed games count returned.";
        return out;
    }
    b.getDWord();
    if (count == 0) {
        out["status"] = b.getDWord();
    }
    else {
        QVariantList games;
        QVariantHash game;
        for (uint i = 0; i < count; i++) {
            game.clear();
            game["type"] = b.getDWord();
            game["language"] = b.getDWord();
            game["address_family"] = b.getWord();
            game["port"] = b.getWord();
            game["ip"] = b.getDWord();
            b.getDWord(); // 0
            b.getDWord(); // 0
            game["status"] = b.getDWord(); // 0x10 = Public, 0x11 = Private
            game["elapsed"] = b.getDWord(); // In seconds
            game["name"] = b.getString();
            game["password"] = b.getString();
            game["slots"] = b.getByte();
            game["counter"] = b.getString(8);

            QByteArray statstring = b.getString().toLocal8Bit();
            game["statstring"] = Functions::decodeStatString(statstring);
            game["encoded_statstring"] = statstring;
            games.append(game);
        }
        out["games"] = games;
    }
    return out;
}

QVariantHash ClientProtocol::Deserialize_SID_CLANINFO(QByteArrayBuilder b)
{
    QVariantHash out;
    b.getByte();
    QString clan = b.getVoid(4);
    QString reversed = "";
    reversed.reserve(4);
    for (int i = clan.size() - 1; i >= 0; i--) {
        reversed.insert(0, clan.at(i));
    }

    out["clan"] = reversed;
    out["rank"] = b.getByte();
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



QVariantHash ClientProtocol::Deserialize_SID_FRIENDSLIST(QByteArrayBuilder in) {
    QVariantHash out;
    quint8 total = in.getByte();
    out["total"] = total;

    QVariantList friends;
    for (int i = 0; i < total; i++) {
        QVariantHash f;
        f["account"] = in.getString();
        f["location"] = in.getByte();
        f["status"] = in.getByte();
        f["product"] = in.getString(4);
        f["location_detail"] = in.getString();
        friends.append(f);
    }
    out["friends"] = friends;

    return out;
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



BNCSPacket* ClientProtocol::Serialize_SID_ACCOUNTLOGON(const QByteArray clientKey, const QString username){
    QByteArrayBuilder out;
    out.insertVoid(clientKey);
    out.insertString(username);
    return new BNCSPacket(BNCSPacket::SID_AUTH_ACCOUNTLOGON, out);
}

QVariantHash ClientProtocol::Deserialize_SID_AUTH_CHECK(QByteArrayBuilder in){
    QVariantHash out;
    out["result"] = in.getDWord();       //(DWORD) Result
    out["info"] = in.getString();    // (STRING) Additional Information
    return out;
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
    out.insertString("68XI", 4);        //    (DWORD) Platform ID
    // TODO
    if (true)
        out.insertString("PX3W", 4);
    else
        out.insertString("3RAW", 4);    //    (DWORD) Product ID
    out.insertDWord(0x1a);                  //    (DWORD) Version Byte
    out.insertDWord(0);                     //    (DWORD) Product language*
    //
    out.insertByte(0x7F);
    out.insertByte(0x00);
    out.insertByte(0x00);
    out.insertByte(0x01);                   //    (DWORD) Local IP for NAT compatibility*
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

BNCSPacket *ClientProtocol::Serialize_SID_GETADVLISTEX(quint16 gameType, quint16 subGameType, quint32 filter, quint32 count, QString name, QString password, QString statstring)
{
    QByteArrayBuilder out;
    out.insertWord(gameType);
    out.insertWord(subGameType);
    out.insertDWord(filter);
    out.insertDWord(0x00);
    out.insertDWord(count);
    out.insertString(name);
    out.insertString(password);
    out.insertString(statstring);
    return new BNCSPacket(BNCSPacket::SID_GETADVLISTEX, out);
}
