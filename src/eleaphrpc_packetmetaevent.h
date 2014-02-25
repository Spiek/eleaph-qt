/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef ELEAPHRPC_PACKETMETAEVENT_H
#define ELEAPHRPC_PACKETMETAEVENT_H

#include <QtCore/QObject>


//
// Eleaphrpc Packet Event Meta System
//
// if registering a new rpc method over "EleaphProtoRPC::registerRPCMethod", it's possible to handle additional "Meta Events" for the given packet type!
// Meta Events are events which are processed "before" or/and "after" the call of the regular registered method!
//
// For example:
// before the "get.user.profile"-Packet become processed (by the regular registered method), you want first check if the user is successfull logged in.
// This could be arranged by registering the before event and check if the user is logged in, if not you're able to "just ignore" the packet (meaning the registered method will not be called)
// or to ignore and to delete the device which has send the packet (ProtocolViolation Type).
//
// Important:
// All meta events which are registered, have to be in the SAME thread like the main receiver object!
//
// Background Info:
// To implement this kind of eventsystem, the main packet handling philosophy have to be changed.
// We need to call the before and the after Event Syncronly (to catch the return value!).
// To be able to do this (in a multi threaded application!) the complete call process must be moved to a new class called "EleaphRpcPacketHandler".
// This class will be created for each registered method and will be moved directly to the method receiver Object Thread.
// If now a package arrives, the packet parser call "EleaphRpcPacketHandler::processPacket" Asyncronly.
// The Eventhandler then call all Meta events Syncronly in the Receiver Thread including the main method call.
//
// Because of this little Workaround all meta events which are registered, have to be in the same thread like the main receiver object!
//
class EleaphRpcPacketMetaEvent
{
    public:
        enum class Type {
            Invalid = 0,
            Before = 1,
            After = 2
        };
        EleaphRpcPacketMetaEvent(QObject* receiver = 0, EleaphRpcPacketMetaEvent::Type type = EleaphRpcPacketMetaEvent::Type::Invalid, QString eventSlotName = "");
        EleaphRpcPacketMetaEvent::Type type;
        QObject* receiver;
        QString strEventSlotName;
};

class EleaphRpcPacketMetaEvent_Before : public EleaphRpcPacketMetaEvent {
    public:
        EleaphRpcPacketMetaEvent_Before(QObject* receiver, QString eventSlotName = "beforePacketProcess") : EleaphRpcPacketMetaEvent(receiver, EleaphRpcPacketMetaEvent::Type::Before, eventSlotName) { }
};

class EleaphRpcPacketMetaEvent_After : public EleaphRpcPacketMetaEvent {
    public:
        EleaphRpcPacketMetaEvent_After(QObject* receiver, QString eventSlotName = "afterPacketProcess") : EleaphRpcPacketMetaEvent(receiver, EleaphRpcPacketMetaEvent::Type::After, eventSlotName) { }
};

#endif // ELEAPHRPC_PACKETMETAEVENT_H
