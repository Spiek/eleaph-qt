/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "ieleaph.h"

//
// Con's and Decon
//

/*
 * IEleaph - construct the PacketHandler
 *           NOTE: protected constructor for SINGELTON construction
 */
IEleaph::IEleaph(quint32 maxDataLength, QObject *parent) : QObject(parent)
{
    // save construct vars
    this->intMaxDataLength = maxDataLength;
}

/*
 * ~SQMPacketHandler - deconstructor
 */
IEleaph::~IEleaph()
{
    // cleanup
}



//
// SLOT section
//

/*
 * addDevice - add device for packet parsing
 */
void IEleaph::addDevice(QIODevice* device, DeviceForgetOptions forgetoptions, bool enableKeepAliveSystem, uint keepAlivePingTime, uint keepAliveCloseTimeoutTime)
{
    // if we have not a valid device, exit
    if(!device) return;

    // handle ready read
    this->connect(device, SIGNAL(readyRead()), this, SLOT(dataHandler()));

    // register signals for Forget Flags
    if((forgetoptions & ForgetDeviceOnClose) == ForgetDeviceOnClose) {
        this->connect(device, SIGNAL(aboutToClose()), this, SLOT(removeDevice()));
    }
    if((forgetoptions & ForgetDeviceOnDisconnect) == ForgetDeviceOnDisconnect) {
        QAbstractSocket* socket = qobject_cast<QAbstractSocket*>(device);
        if(socket) this->connect(socket, SIGNAL(disconnected()), this, SLOT(removeDevice()));
    }
    if((forgetoptions & ForgetKillDeviceOnClose) == ForgetKillDeviceOnClose) {
        this->connect(device, SIGNAL(aboutToClose()), device, SLOT(deleteLater()));
    }
    if((forgetoptions & ForgetKillDeviceOnDisconnect) == ForgetKillDeviceOnDisconnect) {
        QAbstractSocket* socket = qobject_cast<QAbstractSocket*>(device);
        if(socket) this->connect(socket, SIGNAL(disconnected()), device, SLOT(deleteLater()));
    }

    // if wanted, activate keep alive system
    if(enableKeepAliveSystem) {
        device->setProperty(PROPERTYNAME_KEEPALIVE, QDateTime::currentMSecsSinceEpoch());
        QTimer* timerKeepAlive = device->property(PROPERTYNAME_KEEPALIVETIMER).value<QTimer*>() ?: new QTimer(device);
        device->setProperty(PROPERTYNAME_KEEPALIVETIMER, QVariant::fromValue<QTimer*>(timerKeepAlive));
        QObject::connect(timerKeepAlive, &QTimer::timeout, [keepAliveCloseTimeoutTime,keepAlivePingTime,device,this]() {
            if(device->property(PROPERTYNAME_KEEPALIVE).value<qint64>() + keepAliveCloseTimeoutTime < QDateTime::currentMSecsSinceEpoch()) {
                return device->close();
            }

            // send empty ping packet
            this->sendDataPacket(device, "");
        });
        timerKeepAlive->setInterval(keepAlivePingTime);
        timerKeepAlive->start();
    }

    // register device
    this->lstDevices.insert(device);

    // call user implementation
    this->deviceAdded(device);
}

/*
 * removeDevice - remove device from packet parsing
 */
void IEleaph::removeDevice(QIODevice *device)
{
    // aquire device by param or by casting the sender, if not possible, exit
    QIODevice *ioPacketDevice = !device ? qobject_cast<QIODevice*>(this->sender()) : device;
    if(!ioPacketDevice) {
        return;
    }

    // remove all connection of device (device --> this) and (this --> device)
    ioPacketDevice->disconnect(this);
    this->disconnect(ioPacketDevice);

    // delete (if present) used property for packet cache and the cached DataPacket
    QVariant variantStoredPackage = ioPacketDevice->property(PROPERTYNAME_PACKET);
    EleaphPacket *packet = (variantStoredPackage.type() == QVariant::Invalid) ? (EleaphPacket*)0 : (EleaphPacket*)variantStoredPackage.value<void *>();
    ioPacketDevice->setProperty(PROPERTYNAME_PACKET, QVariant(QVariant::Invalid));
    if(packet) {
        delete packet;
    }

    // deactivate keep alive system (if activated
    QVariant timer = ioPacketDevice->property(PROPERTYNAME_KEEPALIVETIMER);
    if(timer.isValid()) {
        timer.value<QTimer*>()->deleteLater();
        ioPacketDevice->setProperty(PROPERTYNAME_KEEPALIVE, QVariant::Invalid);
        ioPacketDevice->setProperty(PROPERTYNAME_KEEPALIVETIMER, QVariant::Invalid);
    }

    // register device
    this->lstDevices.remove(device);

    // call user implementation
    this->deviceRemoved(ioPacketDevice);
}

/*
 * dataHandler - packet parser logic, will called by every device on which data is available
 */
void IEleaph::dataHandler()
{
    // get the sending QIODevice and exit if it's not a valid
    QIODevice *ioPacketDevice = qobject_cast<QIODevice*>(this->sender());
    if(!ioPacketDevice) {
        return;
    }

    // loop as long device contains data
    while(PACKETLENGTHTYPE intAvailableDataLength = !ioPacketDevice->isOpen() ? 0 : ioPacketDevice->bytesAvailable()) {

        /// <Aquire Data Packet>
        // get the exesting data packet, or if it doesn't exist a 0 Pointer
        QVariant variantStoredPackage = ioPacketDevice->property(PROPERTYNAME_PACKET);
        EleaphPacket *packet = variantStoredPackage.type() == QVariant::Invalid ? (EleaphPacket*)0 : (EleaphPacket*)variantStoredPackage.value<void *>();

        // create new data packet if data packet doesn't allready exist
        if(!packet) {
            packet = new EleaphPacket;

            // initialize default values and add new packet to progress map
            packet->intPacktLength = 0;
            packet->baRawPacketData = 0;
            packet->ioPacketDevice = ioPacketDevice;
            ioPacketDevice->setProperty(PROPERTYNAME_PACKET, qVariantFromValue((void *) packet));
        }

        /// </Aquire Data Packet> <-- Packet was successfull aquired!
        /// <Read Header>

        // read header if it is not present, yet
        if(!packet->intPacktLength) {
            // if not enough data available to read complete header, exit here and wait for more data!
            if(intAvailableDataLength < sizeof(PACKETLENGTHTYPE)) {
                return;
            }

            // otherwise, enough data is present to read the complete header, so do it :-)
            // read content length with the help of Qt's Endian method qFromBigEndian
            QByteArray baPacketLength = ioPacketDevice->read(sizeof(PACKETLENGTHTYPE));
            PACKETLENGTHTYPE* ptrPacketLength = (PACKETLENGTHTYPE*)baPacketLength.constData();
            packet->intPacktLength = qFromBigEndian<PACKETLENGTHTYPE>(*ptrPacketLength);
            intAvailableDataLength -= sizeof(PACKETLENGTHTYPE);

			// if empty ping packet received (keep alive packet), update keepalive time
            if(!baPacketLength.isEmpty() && packet->intPacktLength == 0 && ioPacketDevice->property(PROPERTYNAME_KEEPALIVETIMER).isValid()) {
                ioPacketDevice->setProperty(PROPERTYNAME_KEEPALIVE, QDateTime::currentMSecsSinceEpoch());
            }

            // security check:
            // if content length is greater than the allowed intMaxDataLength, kill the device immediately
            if(packet->intPacktLength > this->intMaxDataLength) goto kill;
        }

        /// </Read Header> <-- Header read complete!
        /// <Read Content>

        // cleanup and exit on empty packets
        if(packet->intPacktLength == 0) goto cleanup_packet;

        // inform the outside world about packet download process
        this->packetDownloadProcess(ioPacketDevice, (intAvailableDataLength > packet->intPacktLength ? packet->intPacktLength : intAvailableDataLength ), packet->intPacktLength);

        // if not enough data is present to read complete content, exit here and wait for more data!
        if(intAvailableDataLength < packet->intPacktLength) {
            return;
        }

        // read the complete content of packet
        packet->baRawPacketData = new QByteArray(ioPacketDevice->read(packet->intPacktLength));

        // call user implementation (IMPORTANT: user implementation has to delete the DataPacket after use!)
        this->newDataPacketReceived(packet);

        // at this point the entire packet was read and sent:
        // now we delete used Packet-Cache property
        ioPacketDevice->setProperty(PROPERTYNAME_PACKET, QVariant(QVariant::Invalid));
        continue;

        /// </Read Content> <-- Content read complete!

        /// Error handling
        kill:
           ioPacketDevice->close();

        cleanup_packet:
           ioPacketDevice->setProperty(PROPERTYNAME_PACKET, QVariant(QVariant::Invalid));
           delete packet;
    }
}

/*
 * newTcpHost - add new connected tcp host to packet parser
 *              and make sure that the socket will deleted properly
 */
void IEleaph::newTcpHost()
{
    // exit if we have no pending connection
    if(!this->serverTcp || !this->serverTcp->hasPendingConnections()) return;

    // acquire socket from tcpServer
    QTcpSocket *socket = this->serverTcp->nextPendingConnection();

    // enables or disables keep alive system
    socket->setSocketOption(QAbstractSocket::KeepAliveOption, this->boolKeepConnectedHostsAlive ? 1 : 0);

    // add the device to packet parser and kill the device if it's destroyed
    this->addDevice(socket, IEleaph::ForgetKillDeviceOnDisconnect);
}




//
// Public section
//

/*
 * startTcpListening - start listenening on given adress and port
 */
bool IEleaph::startTcpListening(quint16 port, QHostAddress address, bool keepConnectedHostsAlive, bool useSSL, QString pathCrt, QString pathKey, bool verifyPeer)
{
    // save keepConnectedHostsAlive
    this->boolKeepConnectedHostsAlive = keepConnectedHostsAlive;

    // (re)construct QTcpServer or SslTcpServer
    if(this->serverTcp) this->serverTcp->deleteLater();
    this->serverTcp = useSSL ? new SslTcpServer(pathCrt, pathKey, verifyPeer, this) : new QTcpServer(this);

    // handle new connected tcp clients
    this->connect(this->serverTcp, SIGNAL(newConnection()), this, SLOT(newTcpHost()));
    return this->serverTcp->listen(address, port);
}




//
// Static Helper section
//
/*
 * sendDataPacket - send given data to last registered IODevice
 */
void IEleaph::sendDataPacket(std::string strDatatoSend)
{
    if(this->lstDevices.isEmpty()) return;
    QByteArray baData(strDatatoSend.c_str(), strDatatoSend.length());
    return this->sendDataPacket(*--this->lstDevices.end(), &baData);
}

/*
 * sendDataPacket - send given data to last registered IODevice
 */
void IEleaph::sendDataPacket(QByteArray *baDatatoSend)
{
    if(this->lstDevices.isEmpty()) return;
    return this->sendDataPacket(*--this->lstDevices.end(), baDatatoSend);
}

/*
 * sendDataPacket - send given data to given IODevice including packet length
 */
void IEleaph::sendDataPacket(QIODevice *device, std::string strDatatoSend)
{
    QByteArray baData(strDatatoSend.c_str(), strDatatoSend.length());
    return this->sendDataPacket(device, &baData);
}

/*
 * sendDataPacket - send given data to given IODevice including packet length
 */
void IEleaph::sendDataPacket(QIODevice *device, QByteArray *baDatatoSend)
{
    // if device or data is null or device is not registered, exit
    if(!device || !baDatatoSend || !this->lstDevices.contains(device)) return;

    // create content length with the help of Qt's Endian method qToBigEndian
    PACKETLENGTHTYPE intDataLength = baDatatoSend->length();
    intDataLength = qToBigEndian<PACKETLENGTHTYPE>(intDataLength);

    // send the content-length and data
    device->write((char*)&intDataLength, sizeof(PACKETLENGTHTYPE));
    device->write(*baDatatoSend);
}

//
// Empty Virtual Events
//
void IEleaph::deviceAdded(QIODevice* device) { Q_UNUSED(device); }

void IEleaph::deviceRemoved(QIODevice* device) { Q_UNUSED(device); }

void IEleaph::packetDownloadProcess(QIODevice *device, qint64 downloadedBytes, qint64 totalBytes) { Q_UNUSED(device); Q_UNUSED(downloadedBytes); Q_UNUSED(totalBytes); }
