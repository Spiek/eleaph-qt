/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef ELEAPHRPC_H
#define ELEAPHRPC_H

// eleaph interface
#include "ieleaph.h"

// qt core libs
#include <QtCore/QMultiMap>
#include <QtCore/QMetaObject>
#include <QtCore/QEventLoop>
#include <QtCore/QSharedPointer>

// forward declaration of file classes (some modules need classes from this module)
class EleaphRpc;

// rpc modules
#include "eleaphrpc_packet.h"
#include "eleaphrpc_packetmetaevent.h"
#include "eleaphrpc_asyncpacketwaiter.h"
#include "eleaphrpc_packethandler.h"

//
// Main Eleaphrpc Packet System
//
class EleaphRpc : public IEleaph
{
    Q_OBJECT
    signals:
        void sigDeviceAdded(QIODevice* device);
        void sigDeviceRemoved(QIODevice* device);

    public slots:
        void unregisterRpcObject();

    public:
        // con / decon
        EleaphRpc(QObject *parent = 0, quint32 maxDataLength = 20971520);

        //
        // RPC funtions
        //

        // register/unregister
        void registerRpcMethod(QString strMethod, QObject* receiver, const char *member, bool singleShot = false, EleaphRpcPacketMetaEvent event0 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event1 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event2 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event3 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event4 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event5 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event6 = EleaphRpcPacketMetaEvent(), EleaphRpcPacketMetaEvent event7 = EleaphRpcPacketMetaEvent());
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

    private:
        // rpc members
        QMultiMap<QString, QSharedPointer<EleaphRpcDelegate> > mapRPCFunctions;

        // helper methods
        QByteArray extractMethodName(const char* method);
};


#endif // ELEAPHRPC_H
