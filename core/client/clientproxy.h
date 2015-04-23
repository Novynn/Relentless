#ifndef CLIENTPROXY_H
#define CLIENTPROXY_H

#include "client.h"
#include <qtcpserverex.h>

class ClientProxy : public Client
{
    Q_OBJECT
public:
    ClientProxy(const QString identifier, ClientCore *parent, QSettings *set)
        : Client(identifier, parent, set)
    {
        warcraftListener = new QTcpServerEx(this);
        warcraftSocket = new QTcpSocket(this);

        connect(warcraftSocket, &QTcpSocket::connected, this, &ClientProxy::war3Connected);
        connect(warcraftSocket, &QTcpSocket::disconnected, this, &ClientProxy::war3Disconnected);
        connect(warcraftSocket, &QTcpSocket::readyRead, this, &ClientProxy::war3ReadyRead);

        connect (warcraftListener, &QTcpServerEx::incomingConnectionHandle, this, &ClientProxy::warcraftConnection);

        isConnected = false;
    }

    bool load(){
        return Client::load();
    }

    bool connectClient() {
        info("Now listening for Warcraft III connections...");
        warcraftListener->listen(QHostAddress::LocalHost, 6112);
        return true;
    }

    void disconnectClient() {
        isConnected = false;
        setStatus(Client::CLIENT_IDLE);
        Client::disconnectClient();
        warcraftSocket->close();
    }

    int getDelay(int size, bool free) {
        Q_UNUSED(size);
        Q_UNUSED(free);
        return 0;
    }

    void handleIncomingPackets(){
        while (!incomingDataQueue.isEmpty()){
            Packet* p = incomingDataQueue.dequeue();

            if (p->protocol() != Packet::PROTOCOL_BNCS) {
                delete p;
                p = 0;
                continue;
            }

            BNCSPacket* packet = dynamic_cast<BNCSPacket*>(p);
            qDebug() << "BNCS->PROX " << packet->packetIdString();
            Client::handleIncomingPacket(p);

            QByteArrayBuilder data = packet->toPackedData();
            qDebug() << "PROX->WAR3 " << packet->packetIdString();
            warcraftSocket->write(data);

            delete packet;
        }
    }

    bool validateSettings(){
        return true;
    }

    void proxySend(Packet* p) {
        p->setImportance(99); // Instant send when 99
        Client::send(p);
    }

    void send(Packet* p) {
        p->setImportance(0); // Disable sending

        if (p->packetId() == BNCSPacket::SID_CHATCOMMAND) {
            p->setImportance(99);

            QString message = QByteArrayBuilder(p->data()).getString();

            if (!message.startsWith("/")) {
                // Do a write-back to WC3 to show that we sent a message
                BNCSPacket* backP = new BNCSPacket(BNCSPacket::SID_CHATEVENT, QByteArray(), Packet::FROM_INTERNAL);
                QByteArrayBuilder b;
                b.insertDWord(0x05); // ID
                b.insertDWord(0x01); // Flags
                b.insertDWord(0x00); // Ping
                b.insertDWord(0x00);
                b.insertDWord(0x00);
                b.insertDWord(0x00);
                b.insertString(username());
                b.insertString(message);

                backP->setData(b);
                b = backP->toPackedData();
                warcraftSocket->write(b);
                delete backP;
            }
        }
        Client::send(p);
    }


    void handleOutgoingPackets(){
        if (!isConnected) return;
        Client::handleOutgoingPackets();
    }

    void onOutgoingPacket(const BNCSPacket* p) {
        if (p->packetId() == BNCSPacket::SID_CHATCOMMAND) {
            QString message = QByteArrayBuilder(p->data()).getString();

            if (!message.startsWith("/")) {
                QVariantHash callback;
                callback.insert("username", username());
                callback.insert("text", message);
                clientCore->emitEvent(this, "chat_usertalk", callback);
            }
        }
    }

    void addAdditionalConnection(QTcpSocket* sock) {
//        connect(sock, &QTcpSocket::disconnected, [this, sock] () {
//           qDebug() << "Additional socket #" << additionalSockets.indexOf(sock) << " disconnected.";
//           additionalSockets.removeOne(sock);
//           sock->disconnect();
//           sock->deleteLater();
//        });
//        connect(sock, &QTcpSocket::readyRead, [this, sock] () {
//           socketReadyRead(sock);
//        });


//        additionalSockets.append(sock);
    }

protected:
    void startAuthentification() {
        qDebug() << "Proxy: BNCS Connected";

        isConnected = true;
    }


private:
    QTcpServerEx* warcraftListener;
    QTcpSocket* warcraftSocket;
    QList<QTcpSocket*> additionalSockets;

    bool isConnected;

public slots:
    void bncsConnected() {
        Client::bncsConnected();

    }

private slots:
    void warcraftConnection(qintptr handle) {
        if (warcraftSocket->isOpen() && warcraftSocket->state() == QTcpSocket::ConnectedState) {
            qDebug() << "Recieved additional request!";
            QTcpSocket* sock = new QTcpSocket();
            sock->setSocketDescriptor(handle);
            addAdditionalConnection(sock);
            return;
        }
        qDebug() << "Proxy: WAR3 Connected";
        warcraftSocket->setSocketDescriptor(handle);

        bncsConnect();
    }

    void war3Connected(){
        qDebug() << "Proxy: WAR3->BNCS Connected";
    }

    void war3Disconnected(){
        qDebug() << "Proxy: WAR3->BNCS disconnected";
        disconnectClient();
    }

    void socketReadyRead(QTcpSocket* sock) {
        QByteArrayBuilder b = sock->peek(1);
        if (b.peekByte() == 0x01) {
            sock->read(1);

        }
        if (sock->bytesAvailable() < 4){
            return;
        }
        while (sock->bytesAvailable() >= 4){
            QByteArrayBuilder block = sock->peek(4);
            if (block.size() < 4 )
                break;
            if (block.peekByte() != (byte) Packet::PROTOCOL_BNCS)
                break;

            int size = block.peekWord(2);
            int id = block.peekByte(1);

            if (sock->bytesAvailable() < size) {
                break;
            }

            QByteArray data = sock->read(size);
            BNCSPacket* p = new BNCSPacket((BNCSPacket::PacketId) id, data, Packet::FROM_LOCAL);
            p->stripHeader();
            qDebug() << "WAR3->PROX (SOCK#?) " << p->packetIdString();
            delete p;
        }
    }

    void war3ReadyRead(){
        QByteArrayBuilder b = warcraftSocket->peek(1);
        if (b.peekByte() == 0x01) {
            warcraftSocket->read(1);

        }
        if (warcraftSocket->bytesAvailable() < 4){
            return;
        }
        while (warcraftSocket->bytesAvailable() >= 4){
            QByteArrayBuilder block = warcraftSocket->peek(4);
            if (block.size() < 4 )
                break;
            if (block.peekByte() != (byte) Packet::PROTOCOL_BNCS)
                break;

            int size = block.peekWord(2);
            int id = block.peekByte(1);

            if (warcraftSocket->bytesAvailable() < size) {
                break;
            }

            QByteArray data = warcraftSocket->read(size);
            BNCSPacket* p = new BNCSPacket((BNCSPacket::PacketId) id, data, Packet::FROM_LOCAL);
            p->stripHeader();
            qDebug() << "WAR3->PROX " << p->packetIdString();
            onOutgoingPacket(p);
            proxySend(p);
            handleOutgoingPackets();
        }
    }

public slots:
    void outgoingPacket(Packet* p) {
        if (p->protocol() != Packet::PROTOCOL_BNCS){
            p->setImportance(0);
        }
        else {
            qDebug() << "PROX->BNCS " << Packet::packetIdToString<BNCSPacket>(p->packetId());
        }

        Client::outgoingPacket(p);
    }
};

#endif // CLIENTPROXY_H
