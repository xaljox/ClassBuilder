#ifndef _CB_COMMAND_SERVER_H
#define _CB_COMMAND_SERVER_H

#include "json.hpp"   // third_party/json (on the include path)
#include "CbString.h"
#include <string>
#include <map>
#include <functional>

struct CbCommandRequest
{
    std::string    command;       // dispatcher key, e.g. "find_class"
    nlohmann::json params;        // input parameters from the JSON request
    nlohmann::json response;      // filled by the command function
    bool           ok;            // success flag (set by DispatchOnUiThread)
    std::string    error;         // error message when !ok
};

// A command handler runs on the UI thread. It reads `params`, fills `response`
// on success, or fills `error` on failure (leaving `ok` to be set by the
// dispatcher based on whether `error` is empty).
typedef std::function<void(const nlohmann::json& params,
                           nlohmann::json& response,
                           std::string& error)> CbCommandFn;

class CbCommandServer
{
public:
    // Populate the command table once (idempotent). Call before serving any
    // request. The TRANSPORT is a separate, platform-specific layer (the Qt TCP
    // server in qt/QtCommandServer), so this dispatch core is Qt-free and has no
    // sockets/threads of its own.
    static void EnsureRegistered();

    // Parse ONE newline-stripped JSON request line, run the command on the
    // CALLING thread (the GUI thread -- handlers may freely touch the model and
    // views, exactly like a menu action), and return the reply JSON as a single
    // line (no trailing newline). The transport-agnostic entry point.
    static std::string ProcessRequestLine(const std::string& jsonLine);

    // Looks up the command for pRequest->command and invokes it (fills ok /
    // response / error). Used by ProcessRequestLine; runs on the caller's thread.
    static void DispatchOnUiThread(CbCommandRequest* pRequest);

    // Adds or replaces a command in the dispatcher table.
    static void Register(const std::string& name, CbCommandFn fn);

    // Pending-params slot for pipe-driven `new_model_basic` /
    // `new_model_serialize`. The pipe handler sets this before calling
    // OpenDocumentFile(NULL), then clears it. CClassBuilderDoc::OnNewDocument
    // reads it (via GetPendingNewModelParams) and, when non-null, bypasses
    // the DataModel new-model wizard by applying these values directly.
    struct PendingNewModelParams
    {
        CbString name;        // DataModel name (also the doc title)
        CbString hFile;       // master header — defaults to name + ".h"
        CbString className;   // document class for serialize variant; "" otherwise
        bool    serialize;   // true → InitSerialize runs (Xxx + XxxObject + CbObject)
        bool    undoRedo;    // requires serialize; InitUndoRedo runs
    };
    static PendingNewModelParams* GetPendingNewModelParams();
    static void                   SetPendingNewModelParams(PendingNewModelParams* p);
};

#endif
