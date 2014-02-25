/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef ELEAPHPROTORPC_H
#define ELEAPHPROTORPC_H

// eleaph interface
#include "ieleaph.h"

// qt core libs
#include <QtCore/QMultiMap>
#include <QtCore/QMetaObject>
#include <QtCore/QEventLoop>
#include <QtCore/QSharedPointer>

// forward declaration of file classes (some modules need classes from this module)
class EleaphProtoRPC;

// rpc modules
#include "eleaphrpc_packet.h"
#include "eleaphrpc_packetmetaevent.h"
#include "eleaphrpc_asyncpacketwaiter.h"

// forward declarations
class EleaphRpcPacketHandler;

//
// Main Eleaphrpc Packet System
//
class EleaphProtoRPC : public IEleaph
{
    Q_OBJECT
    signals:
        void sigDeviceAdded(QIODevice* device);
        void sigDeviceRemoved(QIODevice* device);

    public:
        // Delegate (defines a registered RPC Method)
        struct Delegate
        {
            QObject* object;
            QByteArray method;
            QSharedPointer<EleaphRpcPacketHandler> eventHandler;
            bool singleShot;
        };

        // con / decon
        EleaphProtoRPC(QObject *parent = 0, quint32 maxDataLength = 20971520);

        //
        // RPC funtions
        //

        // register/unregister
        void registerRPCMethod(QString strMethod, QObject* receiver, const char *member, bool singleShot = false, EleaphRpcPacketMetaEvent event0 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event1 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event2 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event3 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event4 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event5 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event6 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event7 = EleaphRpcPacketMetaEvent());
        void unregisterRPCMethod(QString strMethod, QObject* receiver = 0, const char *member = 0);
        void unregisterRPCMethod(QObject* receiver, const char *member = 0);

        // sending
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, std::string);
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, char* data, int length);
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, QByteArray data = QByteArray());

        // asyncron wait
        EleaphRpcPacket waitAsyncForPacket(QString strMethod);

    protected:
        // interface implementation
        virtual void newDataPacketReceived(EleaphPacket *dataPacket);
        virtual void deviceAdded(QIODevice* device);
        virtual void deviceRemoved(QIODevice* device);

    private slots:
        void unregisterRPCObject();

    private:
        // rpc members
        QMultiMap<QString, QSharedPointer<EleaphProtoRPC::Delegate> > mapRPCFunctions;

        // helper methods
        QByteArray extractMethodName(const char* method);
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
        EleaphRpcPacketHandler(QList<EleaphRpcPacketMetaEvent> events) { this->lstEvents = events; qRegisterMetaType<EleaphRpcPacketHandler::EventResult>("EventResult"); }

    public slots:
        void processPacket(QSharedPointer<EleaphProtoRPC::Delegate> delegate, EleaphRpcPacket packet)
        {
            // simplefy some vars
            EleaphProtoRPC::Delegate *pDelegate = delegate.data();

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

    private:
        QList<EleaphRpcPacketMetaEvent> lstEvents;

        bool processEvent(EleaphProtoRPC::Delegate* delegate, EleaphRpcPacket packet, EleaphRpcPacketMetaEvent::Type type)
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
                if(!strEventMethodName.isEmpty() && metaObject->indexOfMethod(QMetaObject::normalizedSignature((strEventMethodName + "(EleaphProtoRPC::Delegate*,EleaphRpcPacket,EleaphRpcPacketHandler::EventResult*)").toStdString().c_str())) != -1) {
                    EventResult eventResult = EventResult::Ok;
                    QMetaObject::invokeMethod(object, strEventMethodName.toStdString().c_str(), Qt::DirectConnection, Q_ARG(EleaphProtoRPC::Delegate*, delegate), Q_ARG(EleaphRpcPacket, packet), Q_ARG(EleaphRpcPacketHandler::EventResult*, &eventResult));

                    // if event handler Results with false, ignore package
                    if(!this->handleEventResult(eventResult, packet)) {
                        return false;
                    }
                }
            }

            // process event
            return true;
        }

        bool handleEventResult(EventResult eventResult, EleaphRpcPacket packet)
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
};

#endif // ELEAPHPROTORPC_H
