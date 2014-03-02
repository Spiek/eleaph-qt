/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef MYELEAPHCLIENT_H
#define MYELEAPHCLIENT_H

// qt libs
#include <QTimer>

// include eleaph system
#include "EleaphRpc"

class MyEleaphClient : public QObject
{
    Q_OBJECT

    public:
        MyEleaphClient() : QObject()
        {
            // connect tcp socket to server on localhost:123, and wait until socket is successfull connected
            this->socketServer = new QTcpSocket(this);
            this->socketServer->connectToHost("localhost", 1234);
            this->socketServer->waitForConnected();

            // init eleaph and let eleaph receive datapackets on socket
            this->eleaphRPC = new EleaphRpc(this);
            this->eleaphRPC->addDevice(this->socketServer);

            // register pong packettype (which will be sent back from the server)
            eleaphRPC->registerRpcMethod("pong", this, SLOT(handlePongPacket(EleaphRpcPacket)));

            // init simple One Second timer
            this->connect(&this->timer, SIGNAL(timeout()), this, SLOT(timerReached()));
            this->timer.start(1000);
        }

    public slots:
        void timerReached()
        {
            // send ping packet to server
            this->eleaphRPC->sendRPCDataPacket(this->socketServer, "ping", QByteArray("Client -> Server: Pong"));
        }

        void handlePongPacket(EleaphRpcPacket dataPacket)
        {
            // print debug message
            QTcpSocket *socketServer = (QTcpSocket*)dataPacket->ioPacketDevice;
            qDebug("[Client] Pong Packet from server on Port %i received!\r\n", socketServer->localPort());
        }

    private:
         QTcpSocket *socketServer;
         EleaphRpc* eleaphRPC;
         QTimer timer;
};

#endif // MYELEAPHCLIENT_H
