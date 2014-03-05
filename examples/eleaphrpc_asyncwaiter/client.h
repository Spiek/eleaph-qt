/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef MYELEAPHCLIENT_H
#define MYELEAPHCLIENT_H

// qt libs
#include <QTimer>

// qt test libs
#include <QtTest/QTest>

// include eleaph system
#include "EleaphRpc"

class MySyncronEleaphClient : public QObject
{
    Q_OBJECT
    public:
        MySyncronEleaphClient(QObject* parent) : QObject(parent) {}

    public slots:
        void appStarted()
        {
            // init client
            // connect tcp socket to server on localhost:123, and wait until socket is successfull connected
            QTcpSocket *socketServer = new QTcpSocket(this);
            socketServer->connectToHost("localhost", 1234);
            socketServer->waitForConnected();

            // init eleaph and let eleaph receive datapackets on socket
            EleaphRpc* eleaphRPC = new EleaphRpc(this);
            eleaphRPC->addDevice(socketServer);

            // start (a)sync client/server ping/pong game :-P
            while(true) {
                // send ping to server
                eleaphRPC->sendRPCDataPacket(socketServer, "ping", QByteArray("This is just a test content!"));

                // inform user about sent packages
                qDebug("[Client] Ping Packet sent to server port %i...", socketServer->peerPort());

                // wait (a)sync for new package
                EleaphRpcPacket packetResponse = eleaphRPC->waitAsyncForPacket("pong");

                // inform user about received package
                qDebug("[Client] Ping Packet received from server on local port %i...\n", ((QTcpSocket*)packetResponse->ioPacketDevice)->localPort());

                // wait (a)syncron 1 second
                QTest::qWait(1000);
            }
        }
};

#endif // MYELEAPHCLIENT_H
