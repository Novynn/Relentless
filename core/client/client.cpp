#include "client.h"

Client::Client(const QString identifier, ClientCore *parent, QSettings *set)
    : QObject(parent), mIdentifier(identifier), clientCore(parent), settings(set){

    queueStartingCredits = 200;
    queueThreshholdBytes = 200;
    queueCostPerByte = 5;
    queueCostPerPacket = 200;
    queueCostPerByteOverThreshhold = 8;
    queueMaxCredits = 600;
    queueCreditRate = 7;
    queueTimeLastSent = QDateTime::currentMSecsSinceEpoch();

    queueCredits = queueStartingCredits;

    setStatus(CLIENT_DISABLED);
    currentGame = 0;

    clientProtocol = new ClientProtocol;
    bncs = new QTcpSocket(this);
    QObject::connect(bncs, SIGNAL(connected()), this, SLOT(bncsConnected()));
    QObject::connect(bncs, SIGNAL(disconnected()), this, SLOT(bncsDisconnected()));
    QObject::connect(bncs, SIGNAL(readyRead()), this, SLOT(bncsReadyRead()));
    bnls = new QSslSocket(this);
    bnls->setProtocol(QSsl::SslV3);
    QObject::connect(bnls, SIGNAL(connected()), this, SLOT(bnlsConnected()));
    QObject::connect(bnls, SIGNAL(disconnected()), this, SLOT(bnlsDisconnected()));
    QObject::connect(bnls, SIGNAL(encrypted()), this, SLOT(bnlsReady()));
    QObject::connect(bnls, SIGNAL(readyRead()), this, SLOT(bnlsReadyRead()));
    QObject::connect(bnls, SIGNAL(sslErrors(const QList<QSslError> &)),
                                     this, SLOT(bnlsSSLError(const QList<QSslError> &)));

    queueTimer = new QTimer(this);
    queueTimer->setInterval(30);
    QObject::connect(queueTimer, SIGNAL(timeout()), this, SLOT(readySend()));
}

inline void Client::print(QString message){
    clientCore->clientOutput(message, getIdentifier());
}
inline void Client::info(QString message){
    clientCore->clientOutput(message, getIdentifier(), Core::MESSAGE_TYPE_INFO);
}
inline void Client::warning(QString message){
    clientCore->clientOutput(message, getIdentifier(), Core::MESSAGE_TYPE_WARNING);
}
inline void Client::error(QString message){
    clientCore->clientOutput(message, getIdentifier(), Core::MESSAGE_TYPE_ERROR);
}
inline void Client::printMessage(QString message, Core::MessageType messageType){
    clientCore->clientOutput(message, getIdentifier(), messageType);
}
void Client::command(QString message){
    clientProtocol->Serialize_SID_CHATCOMMAND(message);
}

bool Client::load(){
    emit eventInitializing();
    QFileInfo info = QFileInfo(settings->fileName());
    QString fileName = info.fileName();
    QString tempMessage = "";
    if (info.absoluteDir() == QDir::tempPath()){
        fileName = fileName.mid(0, fileName.length() - 14);
        tempMessage = "complex ";
    }
    print("Loading " + tempMessage + "config file [" + fileName + "]...");

    settings->beginGroup("Main");
    mExpansion  = settings->value("w3xp", false).toBool();
    mKey        = settings->value("key", "").toString().toLower();
    mKeyEx      = settings->value("keyex", "").toString().toLower();
    mUsername   = settings->value("username", "").toString();
    mPassword   = settings->value("password", "").toString();
    bncsHost    = settings->value("host", "").toString().toLower();
    mRealm      = settings->value("realm", "").toString().toLower();
    bncsPort    = settings->value("port", 6112).toInt();
    settings->endGroup();

    settings->beginGroup("BNLS");
    bnlsHost    = settings->value("host", "bnls.mattkv.net").toString().toLower();
    bnlsPort    = settings->value("port", 9367).toInt();
    bnlsSecure    = settings->value("secure", false).toBool();
    settings->endGroup();

    settings->beginGroup("MISC");
    pingMode    = settings->value("pingmode", 0).toInt();
    settings->endGroup();

    if (!mRealm.isEmpty()){
        // "azeroth" to "Azeroth" for example
        mRealm[0] = mRealm.at(0).toUpper();
    }

    if (!mExpansion && !mKeyEx.isEmpty()){
        warning("Expansion key present. Did you mean to connect as W3XP?");
        mExpansion = true;
    }

    if (mKey.isEmpty() || mKey.length() != 26){
        error("Invalid key");
        return false;
    }
    if (mExpansion){
        if (mKeyEx.isEmpty() || mKeyEx.length() != 26){
            error("Invalid Expansion Key");
            return false;
        }
    }
    if (mUsername.isEmpty()){
        error("Invalid username");
        return false;
    }
    if (mPassword.isEmpty()){
        error("Invalid password");
        return false;
    }
    if (mRealm.isEmpty()){
        if (bncsHost == "useast.battle.net"){
            mRealm = "Azeroth";
        }
        else if (bncsHost == "uswest.battle.net"){
            mRealm = "Lordaeron";
        }
        else if (bncsHost == "europe.battle.net"){
            mRealm = "Northrend";
        }
        else if (bncsHost == "asia.battle.net"){
            mRealm = "Kalimdor";
        }
        else{
            error("Could not determine the correct Battle.net realm.");
            error("If you are using a PvPGN server, please remember to fill out the \"realm\" value in your config file.");
            return false;
        }
    }
    if (bncsHost.isEmpty()){
        // Let's try to get a hostname from the provided realm...
        if (mRealm == "Azeroth"){
            bncsHost = "useast.battle.net";
        }
        else if (mRealm == "Lordaeron"){
            bncsHost = "uswest.battle.net";
        }
        else if (mRealm == "Northrend"){
            bncsHost = "europe.battle.net";
        }
        else if (mRealm == "Kalimdor"){
            bncsHost = "asia.battle.net";
        }
        else{
            error("Could not determine the correct hostname from the provided Battle.net realm.");
            error("If you are using a PvPGN server, please remember to fill out "
                  "the \"host\" value as well as the \"realm\" value in your config file.");
            return false;
        }
    }
    queueTimer->stop();
    queueTimer->start();

    setStatus(CLIENT_IDLE);

    return true;
}

void Client::unload(){
    queueTimer->stop();
    if (status() == CLIENT_CONNECTING
            || status() == CLIENT_CONNECTED
            || status() == CLIENT_CONNECTED_HOSTING) disconnect();
    setStatus(CLIENT_DISABLED);
    emit eventTerminating();
}

void Client::bnlsSSLError(const QList<QSslError> &list){
    QString str("BNLS [%1:%2]: An SSL error occured, but it will not affect login.");
    foreach(QSslError e, list){
        if (e.errorString() != "No error")
            str.append("\n> " + e.errorString());
    }
    warning(str.arg(bnlsHost).arg(bnlsPort));
    bnls->ignoreSslErrors();
}

bool Client::connect(){
    info("Connecting to bnls...");
    setStatus(CLIENT_CONNECTING);
    emit eventConnecting();
    bnls->close();
    if (bnlsSecure)
        bnls->connectToHostEncrypted(bnlsHost, bnlsPort);
    else
        bnls->connectToHost(bnlsHost, bnlsPort);
    return true;
}

void Client::disconnect(){
    setStatus(Client::CLIENT_DISCONNECTED);
    emit eventDisconnected();
    if (bnls->state() != QAbstractSocket::UnconnectedState) bnls->close();
    if (bncs->state() != QAbstractSocket::UnconnectedState) bncs->close();
}

void Client::bncsConnect(){
    BNLSPacket* packet = clientProtocol->Serialize_BNLS_CHOOSENLSREVISION();
    packet->setImportant();
    send(packet);
    bncs->connectToHost(bncsHost, bncsPort);
}

void Client::bncsConnected(){
    info(QString("BNCS [%1:%2]: Connected!").arg(bncsHost).arg(bncsPort));
    emit eventConnected();
    loginStarted();

    QByteArrayBuilder b;
    b.insertByte(1);
    bncs->write(b);
    bncs->waitForBytesWritten();

    BNCSPacket* packet = clientProtocol->Serialize_SID_AUTH_INFO();
    send(packet);

    if (pingMode == 1){
        BNCSPacket* p = clientProtocol
            ->Serialize_SID_PING(0);
        send(p);
    }
}

void Client::bncsDisconnected(){
    warning(QString("BNCS [%1:%2]: Disconnected!").arg(bncsHost).arg(bncsPort));
    if (bncs->state() != QAbstractSocket::UnconnectedState)
        disconnect();
}

void Client::bncsReadyRead(){
    if (bncs->bytesAvailable() < 4)
        return;
    while (bncs->bytesAvailable() >= 4){
        QByteArrayBuilder block = bncs->peek(4);
        if (block.size() < 4 )
            break;
        if (block.peekByte() != (byte) Packet::PROTOCOL_BNCS)
            break;

        int size = block.peekWord(2);
        int id = block.peekByte(1);

        if (bncs->bytesAvailable() < size) {
            break;
        }

        QByteArray data = bncs->read(size);
        incomingDataQueue.append(new BNCSPacket((BNCSPacket::PacketId) id, data, Packet::FROM_SERVER));
    }
    handlePackets();
}

void Client::bnlsReady(){
    if (!bnlsSecure) return;
    info(QString("BNLS [%1:%2]: Secure connection established!").arg(bnlsHost).arg(bnlsPort));
    bncsConnect();
}

void Client::bnlsConnected(){
    info(QString("BNLS [%1:%2]: Connected!").arg(bnlsHost).arg(bnlsPort));
    if (!bnlsSecure)
        bncsConnect();
}

void Client::bnlsDisconnected(){
    if (status() == CLIENT_CONNECTED
            || status() == CLIENT_CONNECTED_HOSTING) return;
    if (status() == CLIENT_DISCONNECTED) return;

    error(QString("BNLS [%1:%2]: Unexpectedly disconnected").arg(bnlsHost).arg(bnlsPort));
    disconnect();
}

void Client::bnlsReadyRead(){
    if (bnls->bytesAvailable() < 3)
        return;
    while (bnls->bytesAvailable() >= 3){
        QByteArrayBuilder block = bnls->peek(3);
        if (block.size() < 3 )
            break;
        int size = block.peekWord();
        int id = block.peekByte(2);
        QByteArray data = bnls->read(size);

        incomingDataQueue.append(new BNLSPacket((BNLSPacket::PacketId) id, data, Packet::FROM_SERVER));
    }
    handlePackets();
}

int Client::getProjectedDelay(int size){
    int byteCost = queueCostPerByte;
    int iQueueCredits = queueCredits;

    qint64 currTime = QDateTime::currentMSecsSinceEpoch();
    if (iQueueCredits < queueMaxCredits){ // Adjust credits up
        iQueueCredits += ((currTime - queueTimeLastSent) / queueCreditRate);
        if (queueCredits > queueMaxCredits)
            iQueueCredits = queueMaxCredits;
    }

    int thisByteDelay = byteCost;
    if (size > queueThreshholdBytes)
        thisByteDelay = queueCostPerByteOverThreshhold;
    int thisPacketCost = queueCostPerPacket + (thisByteDelay * size);

    int requiredDelay = 0;
    if (iQueueCredits < 0)
        requiredDelay = (-1 * iQueueCredits) * queueCreditRate;
    if (thisPacketCost > iQueueCredits)
        requiredDelay = (-1 * (iQueueCredits - thisPacketCost)) * thisByteDelay;

    return requiredDelay;
}

int Client::getDelay(int size){
    int byteCost = queueCostPerByte;

    qint64 currTime = QDateTime::currentMSecsSinceEpoch();
    if (queueCredits < queueMaxCredits){ // Adjust credits up
        queueCredits += ((currTime - queueTimeLastSent) / queueCreditRate);
        if (queueCredits > queueMaxCredits)
            queueCredits = queueMaxCredits;
    }
    queueTimeLastSent = currTime;

    int thisByteDelay = byteCost;
    if (size > queueThreshholdBytes)
        thisByteDelay = queueCostPerByteOverThreshhold;
    int thisPacketCost = queueCostPerPacket + (thisByteDelay * size);

    int requiredDelay = 0;
    if (queueCredits < 0)
        requiredDelay = (-1 * queueCredits) * queueCreditRate;
    if (thisPacketCost > queueCredits)
        requiredDelay = (-1 * (queueCredits - thisPacketCost)) * thisByteDelay;

    queueCredits -= thisPacketCost;
    return requiredDelay;
}

void Client::send(Packet* p){
    if (status() == CLIENT_AUTHENTICATING) p->setImportance(100);
    if (p->importance() == 100){
        sendImmediately(p);
    }
    else {
        qint64 delay = getDelay(p->data().size());
        qint64 when = (QDateTime::currentMSecsSinceEpoch() + delay);
        p->setActiveTime(when);
        outgoingDataQueue.enqueue(p);
    }
}

bool Client::sendImmediately(Packet *p){
    outgoingPacket(p);

    QAbstractSocket* socket = 0;
    if (p->protocol() == Packet::PROTOCOL_BNCS) socket = bncs;
    else if (p->protocol() == Packet::PROTOCOL_BNLS) socket = bnls;
    if (socket == 0) return false;
    QByteArray data = QByteArray(p->toPackedData());
    delete p;
    p = 0;
    socket->write(data);
    return true;
}

void Client::readySend(){
    for (int i = 0; i < outgoingDataQueue.count(); i++){
        Packet* p = outgoingDataQueue.at(i);
        if (QDateTime::currentMSecsSinceEpoch() < p->activeTime()) continue;
        if (sendImmediately(p)) outgoingDataQueue.removeOne(p);
    }
}

void Client::handlePackets(){
    while (!incomingDataQueue.isEmpty()){
        Packet* p = incomingDataQueue.dequeue();
        incomingPacket(p);

        if (p->protocol() == Packet::PROTOCOL_BNCS){
            switch(p->packetId()){
            case BNCSPacket::SID_NULL:
                // Just keeping the connection alive
                break;
            case BNCSPacket::SID_CHATEVENT:
                Recv_SID_CHATEVENT(p->data());
                break;
            case BNCSPacket::SID_PING:
                Recv_SID_PING(p->data());
                break;
            case BNCSPacket::SID_AUTH_INFO:
                Recv_SID_AUTH_INFO(p->data());
                break;
            case BNCSPacket::SID_AUTH_CHECK:
                Recv_SID_AUTH_CHECK(p->data());
                break;
            case BNCSPacket::SID_AUTH_ACCOUNTLOGON:
                Recv_SID_ACCOUNT_LOGON(p->data());
                break;
            case BNCSPacket::SID_AUTH_ACCOUNTLOGONPROOF:
                Recv_SID_ACCOUNTLOGONPROOF(p->data());
                break;
            case BNCSPacket::SID_ENTERCHAT:
                Recv_SID_ENTERCHAT(p->data());
                break;
            case BNCSPacket::SID_STARTADVEX3:
                Recv_SID_STARTADVEX3(p->data());
                break;
            case BNCSPacket::SID_GETADVLISTEX:
                Recv_SID_GETADVLISTEX(p->data());
                break;
            case 0x4C: // Extra Work
                // We can safely ignore this.
                break;
            default:
                warning("Unknown Packet!");
                qDebug() << p;
                break;
            }
        }
        else if (p->protocol() == Packet::PROTOCOL_BNLS){ // Made up BNLS Protocol Header..
            switch(p->packetId()){
            case BNLSPacket::BNLS_CDKEY_EX:
                Recv_BNLS_CDKEY_EX(p->data());
                break;
            case BNLSPacket::BNLS_CHOOSENLSREVISION:
                Recv_BNLS_CHOOSENLSREVISION(p->data());
                break;
            case BNLSPacket::BNLS_LOGONCHALLENGE:
                Recv_BNLS_LOGONCHALLENGE(p->data());
                break;
            case BNLSPacket::BNLS_LOGONPROOF:
                Recv_BNLS_LOGONPROOF(p->data());
                break;
            case BNLSPacket::BNLS_VERSIONCHECKEX2:
                Recv_BNLS_VERSIONCHECKEX2(p->data());
                break;
            default:
                warning("Unknown Packet!");
                qDebug() << p;
                break;
            }
        }

        delete p;
    }
}

void Client::Recv_BNLS_CHOOSENLSREVISION(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_BNLS_CHOOSENLSREVISION(b);
    b.reset();

    uint success = data.value("success").toUInt();

    if (success == 1) return;
    error("The BNLS server couldn't set the NLS revision (error code: " + QString::number(success) + ").");
    qDebug() << b.toReadableString();
    disconnect();
}

void Client::Recv_BNLS_CDKEY_EX(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_BNLS_CDKEY_EX(b);
    //qDebug() << data;

    uint keyCount = data.value("scount").toUInt();
    if (keyCount != data.value("rcount").toUInt()){
        error("The BNLS server could not solve the provided CD-Keys, make sure they are correct.");
        disconnect();
    }

    QByteArray keyData;

    for(uint i = 0; i < keyCount; i++){
        mClientToken = data.value("clientkey" + QString::number(i)).toUInt();
        keyData.append(data.value("keydata" + QString::number(i)).toByteArray());
    }

    BNCSPacket* packet = clientProtocol
            ->Serialize_SID_AUTH_CHECK(mClientToken,
                            mVersionCode,
                            mVersionHash,
                            keyCount, keyData,
                            mCheckString,
                            mUsername);

    send(packet);
}

void Client::Recv_BNLS_VERSIONCHECKEX2(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_BNLS_VERSIONCHECKEX2(b);
    //qDebug() << data;

    mVersionHash = data.value("checksum").toUInt();
    mCheckString = data.value("checkstring").toString();
    mVersionCode = data.value("versioncode").toUInt();

    QStringList keys;
    keys << mKey;
    if (mExpansion) keys << mKeyEx;

    BNLSPacket* p = clientProtocol
            ->Serialize_BNLS_CDKEY_EX(keys, mServerToken);

    send(p);
}

void Client::Recv_BNLS_LOGONCHALLENGE(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_BNLS_LOGONCHALLENGE(b);

    //qDebug() << data;

    QByteArray clientKey = data.value("clientkey").toByteArray();

    BNCSPacket* p = clientProtocol
            ->Serialize_SID_ACCOUNTLOGON(clientKey, mUsername);
    send(p);
}

void Client::Recv_BNLS_LOGONPROOF(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_BNLS_LOGONPROOF(b);

    //qDebug() << data;

    QByteArray passwordProof = data.value("passwordproof").toByteArray();

    BNCSPacket* p = clientProtocol
            ->Serialize_SID_ACCOUNTLOGINPROOF(passwordProof);

    send(p);
}

void Client::Recv_SID_PING(QByteArrayBuilder b){
    if (pingMode == 2) return;
    QVariantHash data = clientProtocol->Deserialize_SID_PING(b);

    quint32 cookie = data.value("cookie").toUInt();

    BNCSPacket* p = clientProtocol
            ->Serialize_SID_PING(cookie);

    send(p);
}

void Client::Recv_SID_AUTH_INFO(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_SID_AUTH_INFO(b);
    //qDebug() << data;

    mServerToken = data.value("servertoken").toUInt();

    BNLSPacket* p = clientProtocol
            ->Serialize_BNLS_VERSIONCHECKEX2(0x08, data.value("filetime").toByteArray(),
                                             data.value("filename").toString(),
                                             data.value("valuestring").toString());

    send(p);
}

void Client::Recv_SID_AUTH_CHECK(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_SID_AUTH_CHECK(b);

    uint result = data.value("result").toUInt();

    if (result != 0){
        QString info = data.value("info").toString();
        error("Could not log in due to [0x" + QString::number(result, 16) + "] [" + info + "]");
        disconnect();
        return;

    }
    info("CD-Keys accepted!");

    BNLSPacket* p = clientProtocol
            ->Serialize_BNLS_LOGONCHALLENGE(mUsername, mPassword);
    send(p);
}

void Client::Recv_SID_ACCOUNT_LOGON(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_SID_ACCOUNT_LOGON(b);

    //qDebug() << data;

    QByteArray salt = data.value("salt").toByteArray();
    QByteArray key = data.value("key").toByteArray();

    BNLSPacket* p = clientProtocol
            ->Serialize_BNLS_LOGONPROOF(salt, key);

    send(p);
}

void Client::Recv_SID_ACCOUNTLOGONPROOF(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_SID_ACCOUNTLOGONPROOF(b);
//    (DWORD) Status
//    (BYTE) [20] Server Password Proof (M2)
//    (STRING) Additional information

//    Remarks:	Statuses:
//    0x00: Logon successful.
//    0x02: Incorrect password.
//    0x06: Account closed.
//    0x0E: An email address should be registered for this account.
//    0x0F: Custom error. A string at the end of this message contains the error.
    uint result =  data.value("result").toUInt();

    if (result != 0){
        switch(result){
        case 0x02:
            error("Incorrect password.");
            break;
        case 0x06:
            error("Account closed.");
            break;
        case 0x0E:
            error("Email address required.");
            break;
        case 0x0F:
            error("Unknown Error [" + data.value("info").toString() + "]");
            break;
        }
        disconnect();
        return;
    }
    loginFinished();

    BNCSPacket* p = clientProtocol->Serialize_SID_ENTERCHAT();

    send(p);
}

void Client::RequestGameList() {
    BNCSPacket* p = clientProtocol->Serialize_SID_GETADVLISTEX(0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00);
    send(p);
}

void Client::Recv_SID_ENTERCHAT(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_SID_ENTERCHAT(b);

    mUsername = data.value("uname", mUsername).toString();

    info("Entered chat with the unique username [" + username() + "]");

    emit eventEnteredChat(username());

    settings->beginGroup("Main");
    QString channel = settings->value("firstchannel", "necotest").toString();
    settings->endGroup();

    BNCSPacket* p = clientProtocol->Serialize_SID_JOINCHANNEL(channel, 0x02);
    send(p);

    RequestGameList();
//    GameCore* gCore = clientCore->core()->getGameCore();
//    //if (gCore->games().count() == 0){

//        Game* game = new Game(gCore);
//        game->loadMap("uxIslandDefense3.w3x");
//        game->setName("necotest");
//        QString &hostname = game->hostname();
//        hostname = username();

//        gCore->newGame(game);
    //}
}

void Client::Recv_SID_CHATEVENT(QByteArrayBuilder b){
/*    qDebug() << "CHAT EVENT: " << clientProtocol->Deserialize_SID_CHATEVENT(b);
    [0x01] EID_SHOWUSER: User in channel
    [0x02] EID_JOIN: User joined channel
    [0x03] EID_LEAVE: User left channel
    [0x04] EID_WHISPER: Recieved whisper
    [0x05] EID_TALK: Chat text
    [0x06] EID_BROADCAST: Server broadcast
    [0x07] EID_CHANNEL: Channel information
    [0x09] EID_USERFLAGS: Flags update
    [0x0A] EID_WHISPERSENT: Sent whisper
    [0x0D] EID_CHANNELFULL: Channel full
    [0x0E] EID_CHANNELDOESNOTEXIST: Channel doesn't exist
    [0x0F] EID_CHANNELRESTRICTED: Channel is restricted
    [0x12] EID_INFO: Information
    [0x13] EID_ERROR: Error message
    [0x17] EID_EMOTE: Emote
      */

    QVariantHash data = clientProtocol->Deserialize_SID_CHATEVENT(b);
    int id = data.value("id", 0).toInt();

    QString username = data.value("username", "").toString();
    QString text = data.value("text", "").toString();
    QString flags = data.value("flags", "").toString();

    switch (id){
    case (BNCSPacket::EID_SHOWUSER):
        info(QString("channel.%0> %1 (%2) is in the channel.").arg(currentChannel, username, text));
        emit eventUserInChannel(currentChannel, username, flags);
        break;
    case (BNCSPacket::EID_JOIN):
        info(QString("channel.%0.join> %1 (%2) has joined the channel.").arg(currentChannel, username, text));
        emit eventUserJoins(currentChannel, username, flags);
        break;
    case (BNCSPacket::EID_LEAVE):
        info(QString("channel.%0.leave> %1 (%2) has left the channel.").arg(currentChannel, username));
        emit eventUserLeaves(currentChannel, username, flags);
        break;
    case (BNCSPacket::EID_WHISPER):
        info(QString("whisper.from.%0> %1").arg(username, text));
        emit eventUserWhisper(username, text);
        break;
    case (BNCSPacket::EID_TALK):
        info(QString("channel.%0.%1> %2").arg(currentChannel, username, text));
        emit eventUserTalk(currentChannel, username, text);
        break;
    case (BNCSPacket::EID_BROADCAST):
        info(QString("server.broadcast.%0> %1").arg(username, text));
        emit eventServerInfo(username, text);
        break;
    case (BNCSPacket::EID_CHANNEL):
        if (!currentChannel.isEmpty()){
            emit eventChannelLeave(currentChannel);
        }
        currentChannel = text;
        info(QString("channel.%0.joined>").arg(currentChannel));
        emit eventChannelJoin(currentChannel);
        break;
    case (BNCSPacket::EID_USERFLAGS):
        info(QString("channel.%0> %1 (%2) has updated.").arg(currentChannel, username, text));
        emit eventFlagUpdate(currentChannel, username, flags);
        break;
    case (BNCSPacket::EID_WHISPERSENT):
        info(QString("whisper.to.%0> %1").arg(username, text));
        emit eventWhisperSent(username, text);
        break;
    case (BNCSPacket::EID_CHANNELFULL):
        info(QString("channel.%0.full> %1").arg(username, text));
        emit eventServerError(username, text);
        break;
    case (BNCSPacket::EID_CHANNELDOESNOTEXIST):
        info(QString("channel.%0.nonexistant> %1").arg(username, text));
        emit eventServerError(username, text);
        break;
    case (BNCSPacket::EID_CHANNELRESTRICTED):
        info(QString("channel.%0.restricted> %1").arg(username, text));
        emit eventServerError(username, text);
        break;
    case (BNCSPacket::EID_INFO):
        info(QString("server.info.%0> %1").arg(username, text));
        emit eventServerInfo(username, text);
        break;
    case (BNCSPacket::EID_ERROR):
        info(QString("server.error.%0> %1").arg(username, text));
        emit eventServerError(username, text);
        break;
    case (BNCSPacket::EID_EMOTE):
        info(QString("channel.%0.%1.emote> %2").arg(currentChannel, username, text));
        emit eventUserEmote(currentChannel, username, text);
        break;
    default:
        break;
    }
}

void Client::Recv_SID_STARTADVEX3(QByteArrayBuilder b){
    QVariantHash data = clientProtocol->Deserialize_SID_STARTADVEX3(b);
    int success = data.value("success").toUInt();
    if (success == 0){
        if (refreshDue){
            //info("Game [" + currentGame->name() + "] refreshed.");
            refreshDue = false;
        }
        else {
            setStatus(CLIENT_CONNECTED_HOSTING);
            info("Now hosting [" + currentGame->name() + "]");
        }
    }
    else {
        setStatus(CLIENT_CONNECTED);
        error("Unable to host [" + currentGame->name() + "]");
        refreshDue = false;
    }
}

void Client::Recv_SID_GETADVLISTEX(QByteArrayBuilder b)
{
    QVariantHash data = clientProtocol->Deserialize_SID_GETADVLISTEX(b);
    QVariantList games = data.value("games").toList();
    for (QVariant g : games) {
        QVariantHash game = g.toHash();
        qDebug() << game.value("name", "").toString();
    }
}

// END

void Client::beginHosting(Game* game){
    print("Broadcasting...");
    currentGame = game;
    refreshDue = false;

    setStatus(CLIENT_CONNECTED_BEGIN_HOSTING);

    Map* map = game->map();
    if (map == 0 || !map->valid()){
        error("The map file is invalid...");
        return;
    }

    quint64 hostCounter = game->getHostId(this);
    if (hostCounter == 0){
        error("Recieved an invalid host ID...");
        return;
    }

    QString hostCounterString = QString::number(hostCounter, 16);
    hostCounterString.prepend(QString(8 - hostCounterString.size(), '0'));

    info(QString("HGID = %1 (%2)").arg(hostCounterString).arg(hostCounter));

    quint32 state = game->lobby()->state();
    quint32 uptime = 0; // Game just created
    quint16 gameType = map->gameType();
    gameType |= MAPGAMETYPE_UNKNOWN0;
    quint16 subGameType = 32840; // 8048
    quint32 ladderType = 0;
    QString gameName = game->name();
    QString gamePassword = "";

    QByteArray statString = map->generateStatstring(hostCounterString, username());
    BNCSPacket* packet = clientProtocol->Serialize_SID_STARTADVEX3(state, uptime, gameType, subGameType, ladderType, gameName, gamePassword, statString);

    send(packet);
}

void Client::hostRefresh(){
    if (refreshDue){
        return;
    }
    if (status() != CLIENT_CONNECTED_HOSTING){
        error("Not hosting...");
        endHosting();
        return;
    }
    if (!currentGame){
        error("currentGame is null");
        endHosting();
        return;
    }

    refreshDue = true;

    Map* map = currentGame->map();
    quint64 hostCounter = currentGame->getHostId(this);

    QString hostCounterString = QString::number(hostCounter, 16);
    hostCounterString.prepend(QString(8 - hostCounterString.size(), '0'));

    quint32 state = currentGame->lobby()->state();
    quint32 uptime = 0; // currentGame->lobby()->elapsed()
    quint16 gameType = map->gameType();
    gameType |= MAPGAMETYPE_UNKNOWN0;
    quint16 subGameType = 32840; // 8048
    quint32 ladderType = 0;
    QString gameName = currentGame->name();
    QString gamePassword = "";

    QByteArray statString = map->generateStatstring(hostCounterString, username());
    BNCSPacket* packet = clientProtocol->Serialize_SID_STARTADVEX3(state, uptime, gameType, subGameType, ladderType, gameName, gamePassword, statString);

    send(packet);

    //info("Game refreshing...");
}

void Client::endHosting(){
    if (status() != CLIENT_CONNECTED_HOSTING) return;
    refreshDue = false;
    clientProtocol->Serialize_SID_STOPADV();
    clientProtocol->Serialize_SID_ENTERCHAT();
    print("No longer broadcasting");
    setStatus(CLIENT_CONNECTED);
    currentGame = 0;
}

void Client::loginStarted(){
    print("Login started.");
    loginStart = QDateTime::currentMSecsSinceEpoch();
    setStatus(CLIENT_AUTHENTICATING);
    emit eventLoggingIn();
}

void Client::loginFinished(){
    print("Login finished.");
    qint64 loginTime = QDateTime::currentMSecsSinceEpoch() - loginStart;

    info("Login took [" + QString::number(loginTime) + "] milliseconds");
    setStatus(CLIENT_CONNECTED);
    emit eventLoggedIn();

    BNCSPacket* p = clientProtocol->Serialize_SID_NETGAMEPORT(6112);
    send(p);
}

void Client::incomingPacket(Packet *p){
    emit eventIncomingData(p);
}

void Client::outgoingPacket(Packet *p){
    emit eventOutgoingData(p);
}

void Client::channelJoin(QString channel){
    print("Joined channel: " + channel);
}
