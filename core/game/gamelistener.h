#ifndef GAMELISTENER_H
#define GAMELISTENER_H

#include <QTcpServer>

class GameListener : public QTcpServer {
    Q_OBJECT
public:
    explicit GameListener(QObject *parent = 0) : QTcpServer(parent){}
protected:
    void incomingConnection(qintptr socketDescriptor){ emit connectionRequest(socketDescriptor); }
signals:
    void connectionRequest(qintptr socketDescriptor);
};

#endif // GAMELISTENER_H
