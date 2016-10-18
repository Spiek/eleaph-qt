/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef SQMPACKETHANDLER_H
#define SQMPACKETHANDLER_H

// qt core libs
#include <QtCore/QObject>
#include <QtCore/QIODevice>
#include <QtCore/QVariant>
#include <QtCore/QtEndian>
#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtCore/QQueue>
#include <QtCore/QSet>

// qt network libs
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

// include ansi libs
#include <string>

// own libs
#include "ssltcpserver.h"

//
// PACKETLENGTHTYPE
//
// The datatype will read at first from datastream to determinate the content length
// NOTE: this value has to be the same on client and server, if not, the packet parser will not work properly
//
#define PACKETLENGTHTYPE quint32

//
// PROPERTYNAME
//
// The property key will be used to store the non finished packet in the QObject meta system of the sending IODevice
// NOTE: don't use key names which start with _q_, because this will be used by Qt itself
//
#define PROPERTYNAME_PACKET "SQMPacketHandler_packet"
#define PROPERTYNAME_KEEPALIVE "eleaph_keepalive"
#define PROPERTYNAME_KEEPALIVETIMER "eleaph_keepalivetimer"
#define PROPERTYNAME_WRITEDATACACHE "eleaph_writedatacache"


struct EleaphPacket
{
    public:
        // data
        QIODevice* ioPacketDevice;
        QByteArray* baRawPacketData;
        PACKETLENGTHTYPE intPacktLength;

        // if we inherit from this class,
        // we want to move the data from the old object to the new
        EleaphPacket* move(EleaphPacket *moveTo, bool deleteMySelf = false)
        {
            moveTo->ioPacketDevice = this->ioPacketDevice;
            moveTo->baRawPacketData = this->baRawPacketData;
            moveTo->intPacktLength = this->intPacktLength;

            // reset all values to default
            this->resetToDefault();

            // if user want to delete this object, so do it
            if(deleteMySelf) {
                delete this;
            }

            // return the object where all data where moved to
            return moveTo;
        }

        void resetToDefault()
        {
            this->ioPacketDevice = 0;
            this->baRawPacketData = 0;
            this->intPacktLength = 0;
        }

        ~EleaphPacket()
        {
            // exit if there is nothing to delete
            if(!this->baRawPacketData) {
                return;
            }

            // delete all content
            delete baRawPacketData;
        }
};


class IEleaph : public QObject
{
    Q_OBJECT
    public:
        // device forget options for device addings
        enum DeviceForgetOptions {
            ForgetDeviceOnClose          = 1 << 0,
            ForgetDeviceOnDisconnect     = 1 << 1,
            ForgetKillDeviceOnClose      = ForgetDeviceOnClose      + (1 << 2),
            ForgetKillDeviceOnDisconnect = ForgetDeviceOnDisconnect + (1 << 3)
        };
        enum class KeepAliveMode {
            Disabled = 0,
            EleaphServer = 1,
            EleaphClient = 2
        };

        // start tcp listening
        bool startTcpListening(quint16 port, QHostAddress address = QHostAddress::Any, bool useSSL = false, QString pathCrt = "", QString pathKey = "", bool verifyPeer = true);

        // datapacket send functions
        void sendDataPacket(QByteArray *baDatatoSend);
        void sendDataPacket(std::string strDatatoSend);
        void sendDataPacket(QIODevice* device, QByteArray *baDatatoSend);
        void sendDataPacket(QIODevice* device, std::string strDatatoSend);

		// helper functions
        bool isDeviceRegistered(QIODevice* device);

    public slots:
        void addDevice(QIODevice* device, IEleaph::DeviceForgetOptions forgetoptions = IEleaph::ForgetDeviceOnClose);
        void removeDevice(QIODevice *device = 0);

    protected:
        // protected con and decon for inhertance override
        // set max length by default to 20MB (20971520 Bytes)
        IEleaph(KeepAliveMode keepAliveMode, uint keepAlivePingTime = 1000, uint keepAliveCloseTimeoutTime = 300000, quint32 maxDataLength = 20971520, QObject *parent = 0);
        ~IEleaph();

    protected:
        // virtual methods
        virtual void deviceAdded(QIODevice* device);
        virtual void deviceRemoved(QIODevice* device);
        virtual void packetDownloadProcess(QIODevice* device, qint64 downloadedBytes, qint64 totalBytes);
        virtual void newDataPacketReceived(EleaphPacket *dataPacket) = 0;

    private slots:
        void newTcpHost();
        void dataHandler();

    private:
        // dynamic members
        quint32 intMaxDataLength;

        // members for tcpserver feature
        QTcpServer* serverTcp = 0;
        QSet<QIODevice*> lstDevices;

        // keep alive system
        KeepAliveMode keepAliveMode;
        uint keepAlivePingTime;
        uint keepAliveCloseTimeoutTime;
};

#endif // SQMPACKETHANDLER_H
