/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "eleaphrpc_packetmetaevent.h"

EleaphRpcPacketMetaEvent::EleaphRpcPacketMetaEvent(QObject* receiver, EleaphRpcPacketMetaEvent::Type type, QString eventSlotName)
{
    this->receiver = receiver;
    this->type = type;
    this->strEventSlotName = eventSlotName;
}
