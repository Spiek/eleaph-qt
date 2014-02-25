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
            eleaphRPC->registerRPCMethod("pong", this, SLOT(handlePongPacket(EleaphRpcPacket)));

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
