#ifndef ELAPHRPC_PACKET_H
#define ELAPHRPC_PACKET_H

// qt core libs
#include <QtCore/QSharedPointer>

// own modules
#include "ieleaph.h"

//
// Packet data
// Extent the eleaph data packet for additional rpc philosophy
//
struct EleaphRpcPacketData : EleaphPacket
{
    QString strMethodName;
};
typedef QSharedPointer<EleaphRpcPacketData> EleaphRpcPacket;


#endif // ELAPHRPC_PACKET_H
