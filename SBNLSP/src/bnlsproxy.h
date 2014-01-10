#ifndef BNLSPROXY_H
#define BNLSPROXY_H

#include <QObject>
#include <QSslSocket>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>
#include <QDebug>

class BNLSProxy : public QObject
{
    Q_OBJECT
public:
    explicit BNLSProxy(QObject *parent, int socketDescriptor);
private:
    QSslSocket* incomingSocket;
    QTcpSocket* outgoingSocket;
signals:

public slots:
    void ready();

    void incomingData();
    void outgoingData();

    void connected();
    void disconnected();

    void idisconnected();
    void sslError(const QList<QSslError> & error);
};

#endif // BNLSPROXY_H
