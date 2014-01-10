#include "qsslserver.h"

QSslServer::QSslServer(QObject *parent) :
    QTcpServer(parent)
{
    qDebug() << "Initializing SBNLSP server...";
}

void QSslServer::incomingConnection(int socketDescriptor){
    BNLSProxy* p = new BNLSProxy(this, socketDescriptor);
    clientList.append(p);
}
