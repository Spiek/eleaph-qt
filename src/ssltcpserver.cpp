#include "ssltcpserver.h"

SslTcpServer::SslTcpServer(QString pathCrt, QString pathKey, bool verifyPeer, QObject *parent) : QTcpServer(parent)
{
    this->pathCrt = pathCrt;
    this->pathKey = pathKey;
    this->verifyPeer = verifyPeer;
}

QSslSocket* SslTcpServer::nextPendingConnection()
{
    return qobject_cast<QSslSocket*>(QTcpServer::nextPendingConnection());
}

void SslTcpServer::incomingConnection(qintptr socketDescriptor)
{
    // construct ssl socket with given cert and key information
    QSslSocket *socket = new QSslSocket(this);
    socket->setCaCertificates(QSslCertificate::fromPath(this->pathCrt));
    socket->setPrivateKey(this->pathKey);
    socket->setLocalCertificate(this->pathCrt);
    socket->setProtocol(QSsl::TlsV1_0);
    if(this->verifyPeer) {
        socket->setPeerVerifyMode(QSslSocket::VerifyPeer);
        socket->setPeerVerifyDepth(1);
    }

    // bind socket descriptor to socket, delete on socket on fail
    if(socket->setSocketDescriptor(socketDescriptor)) {
        QObject::connect(socket, SIGNAL(encrypted()), this, SLOT(handleSocketEncryption()));
        QObject::connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(handleSSLError()));
        socket->startServerEncryption();
    } else {
        delete socket;
    }
}

void SslTcpServer::handleSocketEncryption()
{
    // exit if we cannot acquire the socket
    QSslSocket* socket = qobject_cast<QSslSocket*>(this->sender());
    if(!socket) return;

    // add pending connection and emit newConnection signal to the outside world
    this->addPendingConnection(socket);
    emit this->newConnection();
}

void SslTcpServer::handleSSLError()
{
    // try to acquire socket, exit if not possible or if no ssl errors exists
    QSslSocket* socket = qobject_cast<QSslSocket*>(this->sender());
    if(!socket || socket->sslErrors().isEmpty()) return;

    // build ssl errror string and print it
    QString sslErrors = "";
    for(QSslError sslError : socket->sslErrors()) {
        if(!sslErrors.isEmpty()) sslErrors += ", ";
        sslErrors.append(sslError.errorString());
    }
    qDebug("[SSLError][%s:%i] %s", qPrintable(socket->peerAddress().toString()), socket->peerPort(), qPrintable(sslErrors));
}
