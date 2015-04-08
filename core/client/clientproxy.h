#ifndef CLIENTPROXY_H
#define CLIENTPROXY_H

#include "client.h"


class ClientProxy : public Client
{
    Q_OBJECT
public:
    ClientProxy(const QString identifier, ClientCore *parent, QSettings *set)
        : Client(identifier, parent, set)
    {
        warcraftListener = new QTcpServer(this);
        warcraftSocket = new QTcpSocket(this);

        connect (warcraftListener, &QTcpServer::newConnection, this, &ClientProxy::warcraftConnection);
    }

    bool load(){
        return Client::load();
    }

    bool connectClient() {
        info("Now listening for Warcraft III connections...");
        warcraftListener->listen(QHostAddress::LocalHost, 6112);
        return true;
    }

    void bncsConnected() {
        Client::bncsConnected();
        qDebug() << "Proxy: BNCS Connected";
    }

    bool validateSettings(){
        return true;
    }
protected:
    void startAuthentification() {}
private:
    QTcpServer* warcraftListener;
    QTcpSocket* warcraftSocket;

private slots:
    void warcraftConnection() {
        qDebug() << "Proxy: WAR3 Connected";
        QTcpSocket* socket = warcraftListener->nextPendingConnection();
         warcraftSocket->setSocketDescriptor(socket->socketDescriptor());

        connect(warcraftSocket, &QTcpSocket::connected, [this] (){
            qDebug() << "Proxy: WAR3->BNCS Connected";
        });
        connect(warcraftSocket, &QTcpSocket::disconnected, [this] (){
            qDebug() << "Proxy: WAR3->BNCS disconnected";
        });
        bncsConnect();
    }
};

#endif // CLIENTPROXY_H
