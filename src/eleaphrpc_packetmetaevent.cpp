#include "eleaphrpc_packetmetaevent.h"

EleaphRpcPacketMetaEvent::EleaphRpcPacketMetaEvent(QObject* receiver, EleaphRpcPacketMetaEvent::Type type, QString eventSlotName)
{
    this->receiver = receiver;
    this->type = type;
    this->strEventSlotName = eventSlotName;
}
