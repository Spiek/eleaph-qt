/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef ELEAPHRPC_PACKETHANDLER_H
#define ELEAPHRPC_PACKETHANDLER_H

#include <QtCore/QObject>

// forward declaration (we need forward this declration very early because of circle including)
class EleaphRpcPacketHandler;
class EleaphRpcDelegate;

// rpc modules
#include "eleaphrpc_packetmetaevent.h"
#include "eleaphrpc.h"

// Delegate (defines a registered RPC Method)
struct EleaphRpcDelegate
{
    QObject* object;
    QByteArray method;
    QSharedPointer<EleaphRpcPacketHandler> eventHandler;
    bool singleShot;
};

class EleaphRpcPacketHandler : public QObject
{
    Q_OBJECT
    public:
        enum class EventResult {
            Ok = 0,
            Ignore = 1,
            ProtocolViolation = 2
        };
        EleaphRpcPacketHandler(QList<EleaphRpcPacketMetaEvent> events);

    public slots:
        void processPacket(QSharedPointer<EleaphRpcDelegate> delegate, EleaphRpcPacket packet);

    private:
        QList<EleaphRpcPacketMetaEvent> lstEvents;

        bool processEvent(EleaphRpcDelegate* delegate, EleaphRpcPacket packet, EleaphRpcPacketMetaEvent::Type type);
        bool handleEventResult(EventResult eventResult, EleaphRpcPacket packet);
};

#endif // ELEAPHRPC_PACKETHANDLER_H
