#include "player.h"
#include "game.h"

Player::Player(GameCore *parent) :
    QObject(parent), gameCore(parent)
{
    socket = new QTcpSocket(this);
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    mPlayerName = "Unknown";
    mPId = 1;
    isVirtual = false;
}

Player::Player(Game* game)
    : QObject(game->getGameCore()), gameCore(game->getGameCore()){
    socket = 0;

    mPlayerName = "Host";
    mPId = 12;
    isVirtual = true;
}

void Player::close() {
    if (socket && socket->isOpen())
        socket->close();
}

void Player::disconnected(){
    addChat("Player disconnected.", Core::MESSAGE_TYPE_DEFAULT);
    mGame->removePlayer(this);
    deleteLater();
}

void Player::readyRead(){
    extractPackets();
}

void Player::addChat(QString s, Core::MessageType type){
    gameCore->printMessage(s.prepend("[%1] ").arg(mPlayerName), type);
}

bool Player::initialize(int socketDescriptor){
    if (!socket || !socket->setSocketDescriptor(socketDescriptor)) return false;
    return true;
}

bool Player::assignLength( QByteArray &content, int offset ) const{
    QByteArrayBuilder lengthBytes;
    if( content.size( ) >= 3 && content.size( ) <= 65535 ){
        lengthBytes.insertWord(content.size());
        content[offset] = lengthBytes[0];
        content[offset + 1] = lengthBytes[1];
        return true;
    }
    return false;
}

void Player::extractPackets(){
    if (!socket) return;
    if (socket->bytesAvailable() < 4)
        return;
    while (socket->bytesAvailable() >= 4){
        QByteArrayBuilder block = socket->peek(4);
        if (block.size() < 4 )
            break;
        if (block.peekByte() != (byte) Packet::PROTOCOL_W3GS)
            break;

        int size = block.peekWord(2);
        int id = block.peekByte(1);
        if (socket->bytesAvailable() < size) {
            break;
        }
        QByteArray data = socket->read(size);

        W3GSPacket* packet = new W3GSPacket((W3GSPacket::PacketId) id, data, Packet::FROM_SERVER);

        if (state() == ATTACHED && mGame) {
            mGame->queuePlayerPacket(this, packet);
        }
        else if (packet->packetId() == W3GSPacket::W3GS_REQJOIN){
            gameCore->joinRequest(this, packet->data());
        }
        else {
            addChat("Unhandled packet (0x" + QString::number(packet->packetId(), 16) + ") recieved while idle...",
                Core::MESSAGE_TYPE_ERROR);
        }
    }
}

void Player::Recv_W3GS_LEAVEREQ(QByteArrayBuilder b){
    b.getDWord();

    Send_W3GS_LEAVERS();
}

void Player::Send_W3GS_LEAVERS(){
    addChat("Farewelling player...");
    W3GSPacket* packet = new W3GSPacket(W3GSPacket::W3GS_LEAVERS);
    socket->write(packet->toPackedData());
    socket->waitForBytesWritten();
    socket->close();
}

void Player::Recv_W3GS_REQJOIN(QByteArrayBuilder b){
    quint32 hostCounter = b.getDWord();
    quint64 actualHostCounter;
    {
        QString hostCounterString = QString::number(hostCounter, 16);;
        QByteArray cstr = hostCounterString.toLatin1();
        std::reverse(cstr.begin(), cstr.end());
        hostCounterString = cstr;
        bool ok;
        actualHostCounter = hostCounterString.toUInt(&ok, 16);
    }
    b.getDWord(); // (DWORD) Tick count (0 on b.net)
    b.getByte(); // (BYTE) 0
    b.getWord(); // (DWORD) Listen port
    b.getDWord(); // (DWORD) Peer key
    mPlayerName = b.getString(); // (STRING) Username
    b.getDWord(); // (DWORD) Unknown
    b.getWord(); // (WORD) Internal port
    b.getDWord(); // (DWORD) Internal IP

    addChat("Player set: " +
            QString("%1 [%2:%3]")
                .arg(socket->peerName())
                .arg(socket->peerAddress().toString())
                .arg(socket->peerPort()),
            Core::MESSAGE_TYPE_INFO);

    emit joinRequest(this, actualHostCounter);
}

void Player::sendPacket(W3GSPacket* packet){
    if (socket && !isVirtual) {
        socket->write(packet->toPackedData());
        socket->flush();
    }
}
