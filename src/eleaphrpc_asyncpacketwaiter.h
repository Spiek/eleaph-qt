#ifndef ELEAPHRPC_ASYNCPACKETWAITER_H
#define ELEAPHRPC_ASYNCPACKETWAITER_H

// qt core libs
#include <QtCore/QObject>
#include <QtCore/QString>

// rpc modules
#include "eleaphrpc_packet.h"
#include "eleaphprotorpc.h"

class EleaphRpcAsyncPacketWaiter : public QObject
{
    Q_OBJECT
    signals:
        void packetReady();

    public:
        EleaphRpcPacket receivedDataPacket;
        EleaphRpcAsyncPacketWaiter(EleaphProtoRPC *eleaphProto, QString strMethod);

    public slots:
        void packetReceived(EleaphRpcPacket dataPacket);
};

#endif // ELEAPHRPC_ASYNCPACKETWAITER_H
