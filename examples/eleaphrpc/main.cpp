#include <QCoreApplication>

#include "server.h"
#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // init server
    MyEleaphServer *server = new MyEleaphServer;
    Q_UNUSED(server);

    // init client
    MyEleaphClient *client = new MyEleaphClient;
    Q_UNUSED(client);

    return a.exec();
}
