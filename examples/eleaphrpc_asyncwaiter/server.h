/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef MYELEAPHSERVER_H
#define MYELEAPHSERVER_H

// include eleaph system
#include "EleaphRpc"

class MyEleaphServer : public QObject
{
    Q_OBJECT

    public:
        MyEleaphServer() : QObject()
        {
            // Create Eleaph Rpc
            // - with max packet size of 65536 Bytes
            // - and with a tcp listener on port 1234
            this->eleaphRPC = new EleaphRpc(this, 65536);
            this->eleaphRPC->startTcpListening(1234);

            // register ping packettype (which will be sent from the client)
            eleaphRPC->registerRpcMethod("ping", this, SLOT(handlePingPacket(EleaphRpcPacket)));
        }

    public slots:
        void handlePingPacket(EleaphRpcPacket dataPacket)
        {
            // inform user
            qDebug("[Server] Pong Packet from client on local port %i received, sent pong back to client...", ((QTcpSocket*)dataPacket->ioPacketDevice)->localPort());

            // send pong packet back
            this->eleaphRPC->sendRPCDataPacket(dataPacket->ioPacketDevice, "pong", QByteArray("This is just a test content!"));
        }

    private:
         EleaphRpc* eleaphRPC;
};

#endif // MYELEAPHSERVER_H
