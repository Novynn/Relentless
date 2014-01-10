#include "bnlsproxy.h"

BNLSProxy::BNLSProxy(QObject *parent, int socketDescriptor) :
    QObject(parent)
{


    incomingSocket = new QSslSocket(this);
    QByteArray key;
    QByteArray cert;

    QFile fileKey("ca.key");
    if(fileKey.open(QIODevice::ReadOnly)){
        key = fileKey.readAll();
        fileKey.close();
    }
    else{
        qDebug() << fileKey.errorString();
    }

    QFile fileCert("ca.crt");
    if(fileCert.open(QIODevice::ReadOnly)){
        cert = fileCert.readAll();
        fileCert.close();
    }
    else{
        qDebug() << fileCert.errorString();
    }

    qDebug() << key + "\n" + cert;

    QSslKey sslKey(key, QSsl::Rsa);
    QSslCertificate sslCert(cert);

    incomingSocket->setPrivateKey(sslKey);
    incomingSocket->setLocalCertificate(sslCert);
    incomingSocket->setProtocol(QSsl::SslV3);



    incomingSocket->setSocketDescriptor(socketDescriptor);

    incomingSocket->startServerEncryption();
    connect(incomingSocket, SIGNAL(encrypted()), this, SLOT(ready()));
    connect(incomingSocket, SIGNAL(readyRead()), this, SLOT(incomingData()));
    connect(incomingSocket, SIGNAL(disconnected()), this, SLOT(idisconnected()));
    connect(incomingSocket, SIGNAL(sslErrors(const QList<QSslError> &)),
        this, SLOT(sslError(const QList<QSslError> &)));

    qDebug() << "New connection from [" << incomingSocket->peerAddress() << "]";

    outgoingSocket = new QTcpSocket(this);

}

void BNLSProxy::ready(){
    // Ready to start
    // QHostAddress::LocalHost
    outgoingSocket->connectToHost("jbls.novynn.com", 9367);
    qDebug() << "Encryption handshake finished.";
    connect(outgoingSocket, SIGNAL(readyRead()), this, SLOT(outgoingData()));
    connect(outgoingSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(outgoingSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void BNLSProxy::incomingData(){
    // To send to BNLS server
    if (incomingSocket->bytesAvailable() <= 3) return;
    QByteArray data = incomingSocket->readAll();

    outgoingSocket->write(data);
}
void BNLSProxy::outgoingData(){
    // To send to Client
    if (outgoingSocket->bytesAvailable() <= 3) return;
    QByteArray data = outgoingSocket->readAll();

    incomingSocket->write(data);
}

void BNLSProxy::connected(){
    // Connection to BNLS established
    qDebug() << "Connected to remote BNLS server.";
}

void BNLSProxy::sslError(const QList<QSslError> & error){
    incomingSocket->ignoreSslErrors();
}

void BNLSProxy::idisconnected(){
    qDebug() << "Erros:" << incomingSocket->errorString() << outgoingSocket->errorString();
    qDebug() << "sslErrors: " << incomingSocket->sslErrors();
    disconnected();
}

void BNLSProxy::disconnected(){
    // Disconnected from the BNLS server
    qDebug() << "Disconnected.";
    outgoingSocket->close();
    incomingSocket->close();
    deleteLater();
}
