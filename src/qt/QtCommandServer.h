// qt/QtCommandServer.h -- cross-platform transport for the JSON command server.
//
// A QTcpServer bound to 127.0.0.1 (port from CB_CMD_PORT, default 51777), driven
// entirely by the Qt event loop on the GUI thread. Each newline-delimited JSON
// request is handed to CbCommandServer::ProcessRequestLine ON THE GUI THREAD --
// so handlers touch the model exactly like a menu action, with NO worker thread
// and NO platform-specific marshaling. This replaces the old Win32 named-pipe +
// SendMessage(WM_CB_COMMAND) server; the same client/protocol/docs now work on
// Windows, macOS, and Linux unchanged.
//
// NOT re-entrant: a handler that spins a nested event loop (a modal dialog, a
// wait cursor's processEvents) would otherwise dispatch the next request INSIDE
// the running one. While a request runs, sockets with data are only queued and
// drained when it is done; a socket that closed meanwhile is never written to.
// Requests run from a queued call, never inside the socket's readyRead emission
// (the socket may be deleted by the time a nested loop returns).
#pragma once

#include <QList>
#include <QObject>
#include <QPointer>

class QTcpServer;
class QTcpSocket;

class QtCommandServer : public QObject
{
    Q_OBJECT
public:
    explicit QtCommandServer(QObject* parent = nullptr);
    ~QtCommandServer() override;

    // Listen on 127.0.0.1:<port>. port==0 means "use CB_CMD_PORT, else 51777".
    // Returns false if the bind fails (e.g. a stable instance already holds the
    // port). Registers the command table on first start.
    bool start(quint16 port = 0);
    void stop();

private:
    void drain(QTcpSocket* sock);           // handle every complete line on sock

    QTcpServer* _server = nullptr;
    bool _busy = false;                     // a request is being handled
    QList<QPointer<QTcpSocket>> _waiting;   // sockets that got data meanwhile
};
