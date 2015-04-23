#ifndef QTCPSERVEREX
#define QTCPSERVEREX

#include <qtcpserver.h>


class QTcpServerEx : public QTcpServer {
    Q_OBJECT
public:
    QTcpServerEx(QObject *parent = 0) : QTcpServer(parent) {}
protected:
    void incomingConnection(qintptr handle) {
        emit incomingConnectionHandle(handle);
        //QTcpServer::incomingConnection(handle);
    }

signals:
    void incomingConnectionHandle(qintptr handle);
};

#endif // QTCPSERVEREX

