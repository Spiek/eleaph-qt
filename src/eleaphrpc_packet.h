/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

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
