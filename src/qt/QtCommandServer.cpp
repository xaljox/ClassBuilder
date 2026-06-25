// qt/QtCommandServer.cpp -- see QtCommandServer.h.

#include "QtCommandServer.h"
#include "CbCommandServer.h"   // EnsureRegistered / ProcessRequestLine (Qt-free dispatch core)

#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <string>

QtCommandServer::QtCommandServer(QObject* parent) : QObject(parent) {}

QtCommandServer::~QtCommandServer() { stop(); }

bool QtCommandServer::start(quint16 port)
{
    if (_server)
        return true;   // idempotent

    if (port == 0)
        port = static_cast<quint16>(qEnvironmentVariableIntValue("CB_CMD_PORT"));
    if (port == 0)
        port = 51777;

    CbCommandServer::EnsureRegistered();

    _server = new QTcpServer(this);
    if (!_server->listen(QHostAddress::LocalHost, port))   // loopback only -- never off-box
    {
        delete _server;
        _server = nullptr;
        return false;
    }

    connect(_server, &QTcpServer::newConnection, this, [this] {
        while (QTcpSocket* sock = _server->nextPendingConnection())   // child of _server
        {
            // One request line at a time, on the GUI thread. canReadLine/readLine
            // own the partial-read buffering; the handler runs inline (sock as the
            // connection context auto-disconnects the slot when the socket dies).
            connect(sock, &QIODevice::readyRead, sock, [sock] {
                while (sock->canReadLine())
                {
                    QByteArray line = sock->readLine();
                    while (line.endsWith('\n') || line.endsWith('\r'))
                        line.chop(1);
                    if (line.isEmpty())
                        continue;
                    std::string reply = CbCommandServer::ProcessRequestLine(
                        std::string(line.constData(), static_cast<size_t>(line.size())));
                    reply.push_back('\n');
                    sock->write(reply.data(), static_cast<qint64>(reply.size()));
                    sock->flush();
                }
            });
            connect(sock, &QTcpSocket::disconnected, sock, &QObject::deleteLater);
        }
    });
    return true;
}

void QtCommandServer::stop()
{
    if (!_server)
        return;
    _server->close();
    delete _server;   // deletes child sockets synchronously (event loop is ending)
    _server = nullptr;
}
