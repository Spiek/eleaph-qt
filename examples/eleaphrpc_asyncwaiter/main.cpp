/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include <QCoreApplication>

#include "server.h"
#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // init server
    MyEleaphServer *server = new MyEleaphServer;
    Q_UNUSED(server);

    // init (a)syncron eleaph client
    MySyncronEleaphClient *client = new MySyncronEleaphClient(&a);

    // start the client over qt's event loop (because we need a working event loop!)
    emit client->appStarted();

    return a.exec();
}
