#include "eleaphrpc_packethandler.h"

EleaphRpcPacketHandler::EleaphRpcPacketHandler(QList<EleaphRpcPacketMetaEvent> events)
{
    qRegisterMetaType<EleaphRpcPacketHandler::EventResult>("EventResult");
    this->lstEvents = events;
}

void EleaphRpcPacketHandler::processPacket(QSharedPointer<EleaphRpcDelegate> delegate, EleaphRpcPacket packet)
{
    // simplefy some vars
    EleaphRpcDelegate *pDelegate = delegate.data();

    // process all "beforePacketProcess" Events, if one event result with false, abort packet process
    if(!this->processEvent(pDelegate, packet, EleaphRpcPacketMetaEvent::Type::Before)) {
        return;
    }

    // simplefy the delegate
    QObject* object = delegate->object;
    QByteArray method = delegate->method;

    // call delegate syncronly (because we are in the receiver thread)
    QMetaObject::invokeMethod(object, method.constData(), Qt::DirectConnection, Q_ARG(EleaphRpcPacket, packet));

    // process all "afterPacketProcess" Events
    this->processEvent(pDelegate, packet, EleaphRpcPacketMetaEvent::Type::After);
}

bool EleaphRpcPacketHandler::processEvent(EleaphRpcDelegate* delegate, EleaphRpcPacket packet, EleaphRpcPacketMetaEvent::Type type)
{
    // so let us loop all events
    foreach(EleaphRpcPacketMetaEvent event, this->lstEvents) {
        // only process needed events and valid ones
        if(event.type == EleaphRpcPacketMetaEvent::Type::Invalid || event.type != type) {
            continue;
        }

        // simplefy objects
        QObject* object = event.receiver;
        const QMetaObject* metaObject = object->metaObject();

        QString strEventMethodName = event.strEventSlotName;
        if(!strEventMethodName.isEmpty() && metaObject->indexOfMethod(QMetaObject::normalizedSignature((strEventMethodName + "(EleaphRpcDelegate*,EleaphRpcPacket,EleaphRpcPacketHandler::EventResult*)").toStdString().c_str())) != -1) {
            EventResult eventResult = EventResult::Ok;
            QMetaObject::invokeMethod(object, strEventMethodName.toStdString().c_str(), Qt::DirectConnection, Q_ARG(EleaphRpcDelegate*, delegate), Q_ARG(EleaphRpcPacket, packet), Q_ARG(EleaphRpcPacketHandler::EventResult*, &eventResult));

            // if event handler Results with false, ignore package
            if(!this->handleEventResult(eventResult, packet)) {
                return false;
            }
        }
    }

    // process event
    return true;
}

bool EleaphRpcPacketHandler::handleEventResult(EventResult eventResult, EleaphRpcPacket packet)
{
    // just ignore packet
    if(eventResult == EventResult::Ignore) {
        return false;
    }

    // kill peer on protocol violation and ignore package
    else if(eventResult == EventResult::ProtocolViolation) {
        packet.data()->ioPacketDevice->deleteLater();
        return false;
    }

    // ... otherwise everything is okay
    return true;
}
