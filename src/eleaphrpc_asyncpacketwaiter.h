/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef ELEAPHRPC_ASYNCPACKETWAITER_H
#define ELEAPHRPC_ASYNCPACKETWAITER_H

// qt core libs
#include <QtCore/QObject>
#include <QtCore/QString>

// rpc modules
#include "eleaphrpc_packet.h"
#include "eleaphrpc.h"

class EleaphRpcAsyncPacketWaiter : public QObject
{
    Q_OBJECT
    signals:
        void packetReady();

    public:
        EleaphRpcPacket receivedDataPacket;
        EleaphRpcAsyncPacketWaiter(EleaphRpc *eleaphProto, QString strMethod);

    public slots:
        void packetReceived(EleaphRpcPacket dataPacket);
};

#endif // ELEAPHRPC_ASYNCPACKETWAITER_H
