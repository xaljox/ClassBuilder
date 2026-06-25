// qt/QtCommandServer.h -- cross-platform transport for the JSON command server.
//
// A QTcpServer bound to 127.0.0.1 (port from CB_CMD_PORT, default 51777), driven
// entirely by the Qt event loop on the GUI thread. Each newline-delimited JSON
// request is handed to CbCommandServer::ProcessRequestLine ON THE GUI THREAD --
// so handlers touch the model exactly like a menu action, with NO worker thread
// and NO platform-specific marshaling. This replaces the old Win32 named-pipe +
// SendMessage(WM_CB_COMMAND) server; the same client/protocol/docs now work on
// Windows, macOS, and Linux unchanged.
#pragma once

#include <QObject>

class QTcpServer;

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
    QTcpServer* _server = nullptr;
};
