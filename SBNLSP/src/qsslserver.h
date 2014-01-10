#ifndef QSSLSERVER_H
#define QSSLSERVER_H

#include <QTcpServer>
#include <QDebug>
#include "bnlsproxy.h"

class QSslServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit QSslServer(QObject *parent = 0);
protected:
    void incomingConnection(int handle);
private:
    QList<BNLSProxy*> clientList;
    
signals:
    
public slots:
    
};

#endif // QSSLSERVER_H
