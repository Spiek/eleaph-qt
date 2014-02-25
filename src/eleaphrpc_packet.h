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
struct ElaphRpcPacketData : EleaphPacket
{
    QString strMethodName;
};
typedef QSharedPointer<ElaphRpcPacketData> EleaphRpcPacket;


#endif // ELAPHRPC_PACKET_H
