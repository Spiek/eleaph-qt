/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "eleaphrpc.h"

EleaphRpc::EleaphRpc(KeepAliveMode keepAliveMode, QObject *parent, uint keepAlivePingTime, uint keepAliveCloseTimeoutTime, quint32 maxDataLength) :
    IEleaph(keepAliveMode, keepAlivePingTime, keepAliveCloseTimeoutTime, maxDataLength, parent)
{
    // register the ProtoPacket
    qRegisterMetaType<EleaphRpcPacket>("EleaphRpcPacket");
}

//
// Public Access functions
//
/*
 * registerRPCMethod - register RPC Method for Async DataPacket handling
 */
void EleaphRpc::registerRpcMethod(QString strMethod, QObject *receiver, const char *member, bool singleShot,
                                       EleaphRpcPacketMetaEvent event0,
                                       EleaphRpcPacketMetaEvent event1,
                                       EleaphRpcPacketMetaEvent event2,
                                       EleaphRpcPacketMetaEvent event3,
                                       EleaphRpcPacketMetaEvent event4,
                                       EleaphRpcPacketMetaEvent event5,
                                       EleaphRpcPacketMetaEvent event6,
                                       EleaphRpcPacketMetaEvent event7)

{
    return this->registerRpcMethodLogicUnifier(strMethod, receiver, member, false, singleShot, event0, event1, event2, event3, event4, event5, event6, event7);
}

void EleaphRpc::registerRpcMethodWorker(QString strMethod, QObject *receiver, const char *member, bool singleShot,
                                       EleaphRpcPacketMetaEvent event0,
                                       EleaphRpcPacketMetaEvent event1,
                                       EleaphRpcPacketMetaEvent event2,
                                       EleaphRpcPacketMetaEvent event3,
                                       EleaphRpcPacketMetaEvent event4,
                                       EleaphRpcPacketMetaEvent event5,
                                       EleaphRpcPacketMetaEvent event6,
                                       EleaphRpcPacketMetaEvent event7)

{
    return this->registerRpcMethodLogicUnifier(strMethod, receiver, member, true, singleShot, event0, event1, event2, event3, event4, event5, event6, event7);
}


void EleaphRpc::registerRpcMethodLogicUnifier(QString strMethod, QObject *receiver, const char *member, bool isWorker, bool singleShot,
                                  EleaphRpcPacketMetaEvent event0,
                                  EleaphRpcPacketMetaEvent event1,
                                  EleaphRpcPacketMetaEvent event2,
                                  EleaphRpcPacketMetaEvent event3,
                                  EleaphRpcPacketMetaEvent event4,
                                  EleaphRpcPacketMetaEvent event5,
                                  EleaphRpcPacketMetaEvent event6,
                                  EleaphRpcPacketMetaEvent event7)

{
    // move all events to list
    QList<EleaphRpcPacketMetaEvent> events;
    if(event0.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event0);
    if(event1.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event1);
    if(event2.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event2);
    if(event3.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event3);
    if(event4.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event4);
    if(event5.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event5);
    if(event6.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event6);
    if(event7.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event7);

    // event thread check
    // every event have to be in the same thread as receiver object thread, if not we cannot call Events Syncronly, so remove event!
    for(int i = 0;i < events.length(); i++) {
        EleaphRpcPacketMetaEvent event = events.at(i);
        if(event.receiver->thread() != receiver->thread()) {
            qWarning("[%s %s::%s line:%i] Event-Receiver-Object of EleaphRpcPacketMetaEvent is not in the receivers Thread, \"%s\" event of \"%s\" method will not be processed!",
                     __FILE__,
                     typeid(*this).name(),
                     __func__ ,
                     __LINE__,
                     event.type == EleaphRpcPacketMetaEvent::Type::Before ? "before" : "after",
                     qPrintable(strMethod)
                     );
            events.removeAt(i);
        }
    }

    // normalize method
    QByteArray methodNormalized = this->extractMethodName(member);

    // create delegate which points on the receiver
    EleaphRpcDelegate* delegate = new EleaphRpcDelegate;
    delegate->object = receiver;
    delegate->method = methodNormalized;
    delegate->singleShot = singleShot;
    delegate->eventHandler.reset(new EleaphRpcPacketHandler(events));
    delegate->eventHandler->moveToThread(receiver->thread());

    // add rpc function to worker system
    if(isWorker) {
        // if receiver was destroyed, remove it's rpc methods
        this->connect(receiver, SIGNAL(destroyed()), this, SLOT(unregisterRpcWorkerObject()));

        // if not present, construct container
        if(!this->mapWorkerRpcFunctions.contains(strMethod)) {
            this->mapWorkerRpcFunctions.insert(strMethod, QSharedPointer<QMultiMap<QThread*, QSharedPointer<EleaphRpcDelegate> > >(new QMultiMap<QThread*, QSharedPointer<EleaphRpcDelegate> >));
        }

        // insert worker method
        this->mapWorkerRpcFunctions.value(strMethod)->insertMulti(receiver->thread(), QSharedPointer<EleaphRpcDelegate>(delegate));

        // save worker thread
        if(!this->queueWorkerThreads.contains(receiver->thread())) {
            this->queueWorkerThreads.enqueue(receiver->thread());
        }
    }

    // add rpc function to base system
    else {
        // if receiver was destroyed, remove it's rpc methods
        this->connect(receiver, SIGNAL(destroyed()), this, SLOT(unregisterRpcObject()));

        // insert base method
        this->mapRPCFunctions.insertMulti(strMethod, QSharedPointer<EleaphRpcDelegate>(delegate));
    }
}

void EleaphRpc::unregisterRpcMethod(QString strMethod, QObject *receiver, const char *member)
{
    // if no receiver was set, remove all registered procedures for given RPC-function-name
    if(!receiver) {
        this->mapRPCFunctions.remove(strMethod);
    }

    // otherwise we have to loop all registered procedures to determinate the right procedures for deletion
    else {
        // normalize method
        QByteArray methodNormalized = (member) ? this->extractMethodName(member) : QByteArray();

        // loop all registered delegates for rpc method
        foreach(QSharedPointer<EleaphRpcDelegate> delegate, this->mapRPCFunctions.values(strMethod)) {
            // - if member was set, remove only RPC methods which match on receiver and on the member
            // - if no member was set, remove only RPC methods which match only on the receiver
            if(delegate->object == receiver && (!member || delegate->method == methodNormalized)) {
                this->mapRPCFunctions.remove(strMethod, delegate);
            }
        }
    }
}

void EleaphRpc::unregisterRpcMethod(QObject *receiver, const char *member)
{
    // if receiver is not valid, exit
    if(!receiver) return;

    // loop all registered rpc methods
    foreach(QString strMethod, this->mapRPCFunctions.keys()) {
        this->unregisterRpcMethod(strMethod, receiver, member);
    }
}

void EleaphRpc::unregisterRpcMethodWorker(QString strMethod, QObject *receiver, const char *member)
{
    // if no receiver was set, remove all registered procedures for given RPC-function-name
    if(!receiver) {
        this->mapWorkerRpcFunctions.remove(strMethod);
    }

    // otherwise we have to loop all registered procedures to determinate the right procedures for deletion
    else {
        // normalize method
        QByteArray methodNormalized = (member) ? this->extractMethodName(member) : QByteArray();

        // loop all registered threads for given RPC-function-name
        QSharedPointer<QMultiMap<QThread*, QSharedPointer<EleaphRpcDelegate> > > delegateThreads = this->mapWorkerRpcFunctions.value(strMethod);
        foreach(QThread *delegateThread, delegateThreads->keys()) {
            // loop all registered procedures for given RPC-function-name
            foreach(QSharedPointer<EleaphRpcDelegate> delegate, delegateThreads->values(delegateThread)) {
                // - if member was set, remove only RPC methods which match on receiver and on the member
                // - if no member was set, remove only RPC methods which match only on the receiver
                if((!member || delegate->method == methodNormalized) && delegate->object == receiver) {
                    delegateThreads->remove(delegateThread, delegate);
                }
            }
        }

        // if delegateThread map thread is empty, remove it
        if(delegateThreads->empty()) this->mapWorkerRpcFunctions.remove(strMethod);

        // oterwhise reset the new modified map
        else this->mapWorkerRpcFunctions.insert(strMethod, delegateThreads);
    }
}

void EleaphRpc::unregisterRpcMethodWorker(QObject *receiver, const char *member)
{
    // if receiver is not valid, exit
    if(!receiver) return;

    // loop all registered worker rpc methods
    foreach(QString strMethod, this->mapWorkerRpcFunctions.keys()) {
        this->unregisterRpcMethodWorker(strMethod, receiver, member);
    }
}


/*
 *  sendRPCDataPacket - build rpc eleaph packet and send to device
 */
void EleaphRpc::sendRPCDataPacket(QString strProcedureName, char *data, int length)
{
    return this->sendRPCDataPacket(strProcedureName, QByteArray(data, length));
}

void EleaphRpc::sendRPCDataPacket(QString strProcedureName, std::string data)
{
    return this->sendRPCDataPacket(strProcedureName, QByteArray(data.c_str(), data.length()));
}

void EleaphRpc::sendRPCDataPacket(QString strProcedureName, QByteArray data)
{
    this->constructRpcPacket(strProcedureName, data);
    return this->sendDataPacket(&data);
}


void EleaphRpc::sendRPCDataPacket(QIODevice *device, QString strProcedureName, char *data, int length)
{
    return this->sendRPCDataPacket(device, strProcedureName, QByteArray(data, length));
}

void EleaphRpc::sendRPCDataPacket(QIODevice *device, QString strProcedureName, std::string data)
{
    return this->sendRPCDataPacket(device, strProcedureName, QByteArray(data.c_str(), data.length()));
}

void EleaphRpc::sendRPCDataPacket(QIODevice *device, QString strProcedureName, QByteArray data)
{
    this->constructRpcPacket(strProcedureName, data);
    return this->sendDataPacket(device, &data);
}

void EleaphRpc::constructRpcPacket(QString strProcedureName, QByteArray& data)
{
    // create content length with the help of Qt's Endian method qToBigEndian
    qint16 intDataLength = strProcedureName.length();
    intDataLength = qToBigEndian<qint16>(intDataLength);

    // prepend the content-length and method name to the data
    data.prepend(strProcedureName.toLatin1());
    data.prepend((char*)&intDataLength, sizeof(qint16));
}

EleaphRpcPacket EleaphRpc::waitAsyncForPacket(QString strMethod, int timeoutMs, QEventLoop* eventLoop)
{
    EleaphRpcAsyncPacketWaiter packetWaiter(this, strMethod);

    // wait for packet with the help of the event loop
    QEventLoop* loop = eventLoop ?: new QEventLoop;
    loop->connect(&packetWaiter, SIGNAL(packetReady()), loop, SLOT(quit()));
    if(timeoutMs > 0) QTimer::singleShot(timeoutMs, loop, &QEventLoop::quit);
    loop->exec();
    if(!eventLoop) delete loop;

    // return packet
    return packetWaiter.receivedDataPacket;
}


//
// Interface implementations
//

/*
 * newDataPacketReceived - parse the new received dataPacket and forward it to registered Delegate(s)
 */
void EleaphRpc::newDataPacketReceived(EleaphPacket *dataPacket)
{
    // extract rpc method name from packet with the help of Qt's Endian method qFromBigEndian
    qint16* ptrPacketLength = (qint16*)dataPacket->baRawPacketData->data();
    qint16 lenData = qFromBigEndian<qint16>(*ptrPacketLength);
    QString strMethodName = QString(dataPacket->baRawPacketData->mid(sizeof(qint16), lenData));

    // if given procedure name of the packet is not registered, then cleanup and exit
    if(!this->mapRPCFunctions.contains(strMethodName) && !this->mapWorkerRpcFunctions.contains(strMethodName)) {
        return;
    }

    // remove EleaphRPCProtocol::Packet from data
    int intRPCPacketLength = sizeof(qint16) + lenData;
    dataPacket->baRawPacketData->remove(0, intRPCPacketLength);
    dataPacket->intPacktLength -= intRPCPacketLength;

    // constuct rpc datapacket (and move all data from DataPacket to EleaphRPCDataPacket)
    EleaphRpcPacketData* rpcDataPacket = (EleaphRpcPacketData*)dataPacket->move(new EleaphRpcPacketData, true);
    EleaphRpcPacket watcher(rpcDataPacket);

    // set EleaphRPCDataPacket data
    rpcDataPacket->strMethodName = strMethodName;

    //
    // process all registered "base" and "worker" rpc methods
    // - for elimination of double code, we loop two times to process the events:
    //      - 1. run: process "base" events
    //      - 2. run: process "worker" events (only send this events to one worker of many workers!)
    //
    for(int i = 0;i < 2; i++) {
        // define default values
        QList<QSharedPointer<EleaphRpcDelegate> > lstRpcMethodsToProcess;
        QThread* workerThread = 0;

        // first loop, get all "base" rpc methods which should be called
        if(i == 0 && this->mapRPCFunctions.contains(strMethodName)) {
            lstRpcMethodsToProcess = this->mapRPCFunctions.values(strMethodName);
        }

        // second loop, get all "worker" rpc methods which should be called (but only process if at least on worker thread is available!)
        else if(this->mapWorkerRpcFunctions.contains(strMethodName)) {
            // get next worker thread (after getting a thread to process, enqueue the same thread again (at the end), so that we have an thread rotater!)
            workerThread = this->queueWorkerThreads.dequeue();
            this->queueWorkerThreads.enqueue(workerThread);

            // get all worker methods for given thread
            lstRpcMethodsToProcess = this->mapWorkerRpcFunctions.value(strMethodName)->values(workerThread);

            // exit loop (if we reach this position on loop one!)
            i = 2;
        }

        // loop all delegates and invoke them one by one
        foreach(QSharedPointer<EleaphRpcDelegate> delegate, lstRpcMethodsToProcess) {
            // let the EleaphAdditionalEventHandler do the rest (in the event loop of the receivers thread!)
            emit delegate->eventHandler->processPacket(delegate, watcher);

            // unregister single shot delegates (we can do it here, because eventHandler allready has all informations to handle the event!)
            if(delegate->singleShot) {
                // if we are in the first run, delete "base" delegate
                if(!workerThread) {
                    this->mapRPCFunctions.remove(strMethodName, delegate);
                }

                // if we are in the second run, delete "worker" delegate
                else {
                    this->mapWorkerRpcFunctions.value(strMethodName)->remove(workerThread, delegate);
                }
            }
        }
    }
}

void EleaphRpc::deviceAdded(QIODevice *device)
{
    emit this->sigDeviceAdded(device);
}

void EleaphRpc::deviceRemoved(QIODevice *device)
{
    emit this->sigDeviceRemoved(device);
}

void EleaphRpc::packetDownloadProcess(QIODevice *device, qint64 downloadedBytes, qint64 totalBytes)
{
	// if user don't want to receive packet download progress signals, exit
    if(!this->downloadProgress) return;

	// otherwise show download progress to external world
    emit this->sigDownloadProgress(device, downloadedBytes, totalBytes);
}

//
// private slots
//

void EleaphRpc::unregisterRpcObject()
{
    // take sender object
    QObject *objToUnregister = this->sender();

    // if object is valid, unregister all rpc methods which are connected to the object
    if(objToUnregister) {
        this->unregisterRpcMethod(objToUnregister);
    }
}

void EleaphRpc::unregisterRpcWorkerObject()
{
    // get sender object
    QObject *objToUnregister = this->sender();

    // if object is valid, unregister all rpc methods which are connected to the object
    if(objToUnregister) {
        this->unregisterRpcMethodWorker(objToUnregister);
    }
}

//
// Helper Methods
//

/*
 * extractMethodName - normalize SIGNAL and SLOT functionname to normal methodname
 */
QByteArray EleaphRpc::extractMethodName(const char *member)
{
    // code from qt source (4.8.2)
    // src: qtimer.cpp
    // line: 354
    const char* bracketPosition = strchr(member, '(');
    if (!bracketPosition || !(member[0] >= '0' && member[0] <= '3')) {
        return QByteArray();
    }
    return QByteArray(member+1, bracketPosition - 1 - member); // extract method name
}
