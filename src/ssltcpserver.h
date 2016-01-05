#ifndef SSLTCPSERVER_H
#define SSLTCPSERVER_H

#include <QTcpServer>
#include <QtNetwork/QSslSocket>

class SslTcpServer : public QTcpServer
{
    Q_OBJECT
    public:
        SslTcpServer(QString pathCrt, QString pathKey, bool verifyPeer = true, QObject *parent = 0);
        virtual QSslSocket* nextPendingConnection();

    protected:
        virtual void incomingConnection(qintptr socketDescriptor);

    private slots:
        void handleSocketEncryption();
        void handleSSLError();

    private:
        QString pathCrt;
        QString pathKey;
        bool verifyPeer;
};

#endif // SSLTCPSERVER_H
