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
            // own the partial-read buffering. The request is NOT run inside this
            // readyRead emission but queued: a request that spins a nested event
            // loop (a modal) can see its client hang up and the socket's
            // deleteLater run -- returning into Qt's signal-emission frame of the
            // deleted socket was an access violation (doActivate, 2026-10-27).
            connect(sock, &QIODevice::readyRead, this, [this, sock] {
                QPointer<QTcpSocket> p(sock);
                QMetaObject::invokeMethod(this, [this, p] { if (p) drain(p); },
                                          Qt::QueuedConnection);
            });
            connect(sock, &QTcpSocket::disconnected, sock, &QObject::deleteLater);
        }
    });
    return true;
}

void QtCommandServer::drain(QTcpSocket* sock)
{
    if (_busy)
    {
        // Re-entered from a nested event loop inside the running request: keep
        // the lines buffered in the socket and come back when it is done.
        if (!_waiting.contains(sock))
            _waiting.append(sock);
        return;
    }

    _busy = true;
    QPointer<QTcpSocket> guard(sock);   // a nested loop may run its deleteLater
    while (guard && guard->canReadLine())
    {
        QByteArray line = guard->readLine();
        while (line.endsWith('\n') || line.endsWith('\r'))
            line.chop(1);
        if (line.isEmpty())
            continue;
        std::string reply = CbCommandServer::ProcessRequestLine(
            std::string(line.constData(), static_cast<size_t>(line.size())));
        if (!guard || guard->state() != QAbstractSocket::ConnectedState)
            break;                      // the client hung up meanwhile
        reply.push_back('\n');
        guard->write(reply.data(), static_cast<qint64>(reply.size()));
        guard->flush();
    }
    _busy = false;

    // Serve the sockets that got data while this one ran -- queued, so no deep
    // recursion and their own readyRead is not needed again.
    const QList<QPointer<QTcpSocket>> waiting = _waiting;
    _waiting.clear();
    for (const QPointer<QTcpSocket>& w : waiting)
        if (w)
            QMetaObject::invokeMethod(this, [this, w] { if (w) drain(w); },
                                      Qt::QueuedConnection);
}

void QtCommandServer::stop()
{
    if (!_server)
        return;
    _server->close();
    delete _server;   // deletes child sockets synchronously (event loop is ending)
    _server = nullptr;
}
