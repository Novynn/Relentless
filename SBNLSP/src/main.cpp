#include <QCoreApplication>
#include "qsslserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSslServer server;
    server.listen(QHostAddress::Any, 9368);
    
    return a.exec();
}
