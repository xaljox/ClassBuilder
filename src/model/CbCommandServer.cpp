// CbCommandServer.cpp — the JSON command DISPATCH CORE for ClassBuilder.
//
// This file is the portable, Qt-free, transport-free core: the command table
// (g_dispatcher) + all handlers + ProcessRequestLine(jsonLine) -> replyLine. It
// runs on whatever thread calls it -- in practice the GUI thread -- so handlers
// may freely touch the data model and views, exactly like a menu action.
//
// The TRANSPORT lives separately in the Qt layer (qt/QtCommandServer, a
// QTcpServer on 127.0.0.1) so it is cross-platform with NO worker thread and NO
// Win32 marshaling: the Qt event loop delivers each request line on the GUI
// thread and calls ProcessRequestLine directly. (The old design -- a Win32
// worker thread reading a named pipe and SendMessage(WM_CB_COMMAND)-ing onto the
// UI thread -- is gone; that marshaling was the part that wasn't portable.)

#include "StdAfx.h"
#include "CbCommandServer.h"
#include "ClassBuilderDoc.h"
#include "CbShellHooks.h"
#include "CbColor.h"        // CbColorRef (set_*_color commands)
#include <sstream>
#include <fstream>
#include <set>
#include <vector>
#include <functional>
#include <regex>
#include "CbZstdStream.h"
#include "SourceLogInterface.h"
#include "ParseLogInterface.h"
#ifdef _WIN32
#include <direct.h>   // _chdir (POSIX chdir via CbWinTypes.h on non-Windows)
#endif

using nlohmann::json;

namespace
{
    // The command table. Populated once by RegisterBuiltinCommands (via
    // EnsureRegistered); read by DispatchOnUiThread / ProcessRequestLine. No
    // transport state lives here any more -- that's the Qt TCP server's job.
    std::map<std::string, CbCommandFn> g_dispatcher;

    // Converts a CbString / const char* to std::string for JSON responses
    // (CbString converts to const char* implicitly).
    inline std::string ToStd(const char* s)
    {
        return s ? std::string(s) : std::string();
    }

    // Headless log sinks for write_source / read_source — the pipe equivalents
    // of the Qt Save/Read-source dialogs' SourceLogInterface / ParseLogInterface
    // implementations. They discard progress and keep counts + the warning/error
    // text so the JSON reply can surface codegen / parse problems.
    struct PipeSourceLog : public SourceLogInterface
    {
        int logs = 0, warnings = 0, errors = 0;
        std::vector<std::string> warningMsgs, errorMsgs;
        void AddLog(const CbString&) override          { ++logs; }
        void AddLogWarning(const CbString& s) override { ++warnings; warningMsgs.push_back(ToStd(s)); }
        void AddLogError(const CbString& s) override   { ++errors;   errorMsgs.push_back(ToStd(s)); }
        void StepProgress() override {}
    };

    struct PipeParseLog : public ParseLogInterface   // methods take const char*
    {
        int logs = 0, warnings = 0, errors = 0;
        std::vector<std::string> warningMsgs, errorMsgs;
        void AddLog(const char*) override            { ++logs; }
        void AddLogWarning(const char* s) override   { ++warnings; warningMsgs.push_back(ToStd(s)); }
        void AddLogError(const char* s) override     { ++errors;   errorMsgs.push_back(ToStd(s)); }
        void StepProgress() override {}
    };
}

// ---------------------------------------------------------------------------
// Helpers — active doc lookup + JSON marshalling for CB data-model types
// ---------------------------------------------------------------------------

namespace
{
    // Server-side STICKY document target set by select_document. null = follow the
    // GUI's active model (the long-standing implicit behaviour). When set, it is
    // honoured strictly: every doc-targeting command uses it instead of GUI focus,
    // so a script can drive one model while the user edits another.
    CClassBuilderDoc* g_selectedDoc = nullptr;

    // The document every doc-targeting command operates on. If a select_document
    // target is set, honour it strictly: return it while it's still open, else null
    // (commands then report "no active document" until a new select_document) -- we
    // deliberately do NOT silently fall back to GUI focus once a target was chosen.
    // With no target set, follow the GUI's active model.
    CClassBuilderDoc* GetActiveDoc()
    {
        if (g_selectedDoc)
            return Cb_IsDocumentOpen(g_selectedDoc) ? g_selectedDoc : nullptr;
        return Cb_ActiveDoc();
    }

    DataModel* GetActiveDataModel()
    {
        CClassBuilderDoc* pDoc = GetActiveDoc();
        return pDoc ? pDoc->GetDataModelDoc().GetDataModel() : NULL;
    }

    // Decomposes a JSON type string like "const CbArchive&" into the bare
    // type name (looked up via DataModelDoc::FindType) and modifier flags
    // applied to the resulting Variable/Argument/Method.
    struct ParsedType
    {
        std::string bareName;
        int isConst;
        int isReference;
        int isPointer;
        int isPointerPointer;
        int isArray;
        unsigned int arraySize;       // 0 if open ([])
        std::string arraySizeStr;     // non-numeric size expression, if any
    };

    static void Trim(std::string& s)
    {
        while (!s.empty() && isspace((unsigned char)s.back()))  s.pop_back();
        size_t i = 0;
        while (i < s.size() && isspace((unsigned char)s[i])) ++i;
        if (i) s.erase(0, i);
    }

    ParsedType ParseTypeString(const std::string& in)
    {
        ParsedType pt = {in, 0, 0, 0, 0, 0, 0, std::string()};
        Trim(pt.bareName);

        // Leading "const "
        if (pt.bareName.compare(0, 6, "const ") == 0)
        {
            pt.isConst = 1;
            pt.bareName.erase(0, 6);
            Trim(pt.bareName);
        }

        // Trailing array suffix: "[]", "[N]", or "[expr]". Strip first so
        // any trailing '*' under the array applies to the element type.
        if (!pt.bareName.empty() && pt.bareName.back() == ']')
        {
            size_t open = pt.bareName.rfind('[');
            if (open != std::string::npos)
            {
                std::string inside = pt.bareName.substr(
                    open + 1, pt.bareName.size() - open - 2);
                Trim(inside);
                pt.isArray = 1;
                if (!inside.empty())
                {
                    char* end = NULL;
                    unsigned long n = strtoul(inside.c_str(), &end, 10);
                    if (end && *end == 0)
                        pt.arraySize = (unsigned int)n;
                    else
                        pt.arraySizeStr = inside;
                }
                pt.bareName.erase(open);
                Trim(pt.bareName);
            }
        }

        // Trailing modifier — order matters, check ** before *
        if (pt.bareName.size() >= 2 &&
            pt.bareName.compare(pt.bareName.size() - 2, 2, "**") == 0)
        {
            pt.isPointerPointer = 1;
            pt.bareName.erase(pt.bareName.size() - 2);
        }
        else if (!pt.bareName.empty() && pt.bareName.back() == '&')
        {
            pt.isReference = 1;
            pt.bareName.pop_back();
        }
        else if (!pt.bareName.empty() && pt.bareName.back() == '*')
        {
            pt.isPointer = 1;
            pt.bareName.pop_back();
        }
        Trim(pt.bareName);

        return pt;
    }

    // Applies parsed modifier flags onto a Variable. Through the Variable*
    // parameter this calls Variable::SetConst (the type's constness),
    // distinct from Method::SetConst which represents the method-itself
    // postfix const.
    void ApplyTypeModifiers(Variable* pTarget, const ParsedType& pt)
    {
        pTarget->SetConst(pt.isConst);
        pTarget->SetReference(pt.isReference);
        pTarget->SetPointer(pt.isPointer);
        pTarget->SetPointerPointer(pt.isPointerPointer);
        pTarget->SetArray(pt.isArray);
        pTarget->SetArraySize(pt.arraySize);
        pTarget->SetArraySizeStr(CbString(pt.arraySizeStr.c_str()));
    }

    const char* AccessName(AccessType a)
    {
        switch (a)
        {
        case PUBLIC:    return "public";
        case PROTECTED: return "protected";
        case PRIVATE:   return "private";
        default:        return "none";
        }
    }

    bool ParseAccess(const std::string& s, AccessType& out)
    {
        if      (s == "public")    { out = PUBLIC;    return true; }
        else if (s == "protected") { out = PROTECTED; return true; }
        else if (s == "private")   { out = PRIVATE;   return true; }
        return false;
    }

    json ToJson(Argument* pArg)
    {
        json j;
        j["name"]    = ToStd(pArg->GetVariableName());
        j["type"]    = ToStd(pArg->GetTypeName());
        j["default"] = ToStd(pArg->GetDefault());
        return j;
    }

    json ToJson(Method* pMethod)
    {
        json j;
        j["id"]          = (unsigned)pMethod->GetId();
        j["name"]        = ToStd(pMethod->GetName());
        j["return_type"] = ToStd(pMethod->GetTypeName());
        j["access"]      = AccessName(pMethod->GetAccess());
        j["static"]      = (bool)pMethod->GetStatic();
        j["virtual"]     = (bool)pMethod->GetVirtual();
        j["const"]       = (bool)pMethod->GetConst();
        j["pure"]        = (bool)pMethod->GetPure();
        j["dll_export"]  = (bool)pMethod->GetDllExport();

        json args = json::array();
        Method::ArgumentIterator iArg(pMethod);
        while (++iArg)
            args.push_back(ToJson(iArg));
        j["args"] = args;

        return j;
    }

    json ToJson(Member* pMember)
    {
        json j;
        j["name"] = ToStd(pMember->GetVariableName());
        j["type"] = ToStd(pMember->GetTypeName());
        return j;
    }

    // Rich member record — separate from ToJson(Member) to keep `list_members`
    // and `MemberRecord` lightweight. Used by `get_member`.
    json ToJsonFull(Member* pMember)
    {
        json j;
        j["name"]           = ToStd(pMember->GetVariableName());
        j["prefixed_name"]  = ToStd(pMember->GetPrefixedName());
        j["type"]           = ToStd(pMember->GetTypeName());
        j["access"]         = AccessName(pMember->GetAccess());
        j["static"]         = (bool)pMember->GetStatic();
        j["serialize"]      = (bool)pMember->GetSerialize();
        j["initialization"] = ToStd(pMember->GetInitialization());
        j["note"]           = ToStd(pMember->GetNote());

        // Getter / setter access — "none" when the method doesn't exist.
        j["getter"] = pMember->GetGetMemberMethod()
                          ? AccessName(pMember->GetGetMemberMethod()->GetAccess())
                          : "none";
        j["setter"] = pMember->GetSetMemberMethod()
                          ? AccessName(pMember->GetSetMemberMethod()->GetAccess())
                          : "none";
        return j;
    }

    json ToJson(BaseClass* pBaseClass)
    {
        json j;
        j["name"] = ToStd(pBaseClass->GetName());
        const char* kind = "BaseClass";
        if (pBaseClass->IsClass())            kind = "Class";
        else if (pBaseClass->IsExternClass()) kind = "ExternClass";
        j["kind"] = kind;
        return j;
    }

    // Resolve a Method by either {id:N} (preferred for overloads) or
    // {name:"X"}. Sets `error` and returns NULL on miss / bad params.
    Method* ResolveMethod(BaseClass* pBC, const json& params, std::string& error)
    {
        if (params.contains("id"))
        {
            UINT id = (UINT)params["id"].get<unsigned>();
            Method* m = pBC->FindMethodWithId(id);
            if (!m) error = "no method with that id on the class";
            return m;
        }
        std::string name = params.value("name", std::string());
        if (name.empty())
        { error = "missing 'id' or 'name' for method lookup"; return NULL; }
        Method* m = pBC->FindMethodWithName(CbString(name.c_str()));
        if (!m) error = "no method with that name on the class";
        return m;
    }

    // Find an Argument on a Method by `arg` (name) or by `arg_index` (0-based).
    Argument* ResolveArgument(Method* pMethod, const json& params, std::string& error)
    {
        if (params.contains("arg_index"))
        {
            int wanted = params["arg_index"].get<int>();
            int idx = 0;
            Method::ArgumentIterator iArg(pMethod);
            while (++iArg)
            {
                if (idx == wanted) return iArg;
                ++idx;
            }
            error = "arg_index out of range";
            return NULL;
        }
        std::string argName = params.value("arg", std::string());
        if (argName.empty())
        { error = "missing 'arg' or 'arg_index'"; return NULL; }
        Argument* pArg = pMethod->FindArgument(CbString(argName.c_str()));
        if (!pArg) error = "no argument with that name";
        return pArg;
    }
}

// ---------------------------------------------------------------------------
// Built-in commands (registered in Start)
// ---------------------------------------------------------------------------

static void RegisterBuiltinCommands()
{
    // Sanity-check command for end-to-end pipe / dispatcher / UI-thread path.
    CbCommandServer::Register("ping",
        [](const json& params, json& response, std::string& /*error*/)
        {
            response["pong"] = true;
            // Build stamp of this server translation unit -- lets a client detect a
            // STALE binary still owning the pipe (the "extend ping" diagnostic gap).
            response["build"] = __DATE__ " " __TIME__;
            if (params.contains("echo"))
                response["echo"] = params["echo"];
        });

    // list_commands() -- meta: the names of every registered command (sorted;
    // g_dispatcher is a std::map). For discovery / tooling. No document needed.
    // Runs after all commands are registered, so the list is complete.
    CbCommandServer::Register("list_commands",
        [](const json& /*params*/, json& response, std::string& /*error*/)
        {
            json arr = json::array();
            for (const auto& kv : g_dispatcher)
                arr.push_back(kv.first);
            response["commands"] = arr;
            response["count"]    = (int)arr.size();
        });

    // -- Document selection -----------------------------------------------
    // The pipe targets ONE document. By default that's the GUI's active model;
    // select_document overrides it with a sticky server-side target decoupled from
    // GUI focus (see GetActiveDoc + g_selectedDoc).

    // list_documents() -> {documents:[{title,path,modified,gui_active,selected}], count}
    CbCommandServer::Register("list_documents",
        [](const json& /*params*/, json& response, std::string& /*error*/)
        {
            CClassBuilderDoc* pGuiActive = Cb_ActiveDoc();
            CClassBuilderDoc* pTarget    = GetActiveDoc();   // resolved sticky target (null if closed)
            json arr = json::array();
            int n = Cb_DocumentCount();
            for (int i = 0; i < n; ++i)
            {
                CClassBuilderDoc* p = Cb_DocumentAt(i);
                if (!p) continue;
                json e;
                e["title"]      = ToStd(p->GetTitle());
                e["path"]       = ToStd(p->GetPathName());
                e["modified"]   = p->IsModified() ? true : false;
                e["gui_active"] = (p == pGuiActive);
                e["selected"]   = (p == pTarget);
                arr.push_back(e);
            }
            response["documents"] = arr;
            response["count"]     = n;
        });

    // select_document({title | path}) — set the sticky server-side target for
    // subsequent commands. Does NOT change the GUI's active model. Error if no open
    // document matches.
    CbCommandServer::Register("select_document",
        [](const json& params, json& response, std::string& error)
        {
            std::string title = params.value("title", std::string());
            std::string path  = params.value("path",  std::string());
            if (title.empty() && path.empty())
            { error = "missing 'title' or 'path'"; return; }

            int n = Cb_DocumentCount();
            CClassBuilderDoc* found = nullptr;
            for (int i = 0; i < n; ++i)
            {
                CClassBuilderDoc* p = Cb_DocumentAt(i);
                if (!p) continue;
                if (!title.empty() && ToStd(p->GetTitle())    == title) { found = p; break; }
                if (!path.empty()  && ToStd(p->GetPathName()) == path)  { found = p; break; }
            }
            if (!found) { error = "no open document matches"; return; }
            g_selectedDoc = found;
            response["title"]    = ToStd(found->GetTitle());
            response["path"]     = ToStd(found->GetPathName());
            response["selected"] = true;
        });

    // current_document() -> {title, path, modified, explicit} of the targeted doc,
    // or null if none / it was closed. explicit=false means it's following GUI focus.
    CbCommandServer::Register("current_document",
        [](const json& /*params*/, json& response, std::string& /*error*/)
        {
            CClassBuilderDoc* p = GetActiveDoc();
            if (!p) { response = nullptr; return; }
            response["title"]    = ToStd(p->GetTitle());
            response["path"]     = ToStd(p->GetPathName());
            response["modified"] = p->IsModified() ? true : false;
            response["explicit"] = (g_selectedDoc != nullptr);
        });

    // close_document({title | path, save?}) — close a document headlessly (no save
    // prompt). save:true writes the .cbz first, but only if it already has a path
    // (untitled docs close without saving). If the closed doc was the selected
    // target, the target is cleared (commands follow GUI focus again until a new
    // select_document). For script cleanup.
    CbCommandServer::Register("close_document",
        [](const json& params, json& response, std::string& error)
        {
            std::string title = params.value("title", std::string());
            std::string path  = params.value("path",  std::string());
            if (title.empty() && path.empty())
            { error = "missing 'title' or 'path'"; return; }
            bool save = params.value("save", false);

            int n = Cb_DocumentCount();
            CClassBuilderDoc* found = nullptr;
            for (int i = 0; i < n; ++i)
            {
                CClassBuilderDoc* p = Cb_DocumentAt(i);
                if (!p) continue;
                if (!title.empty() && ToStd(p->GetTitle())    == title) { found = p; break; }
                if (!path.empty()  && ToStd(p->GetPathName()) == path)  { found = p; break; }
            }
            if (!found) { error = "no open document matches"; return; }

            std::string closedTitle = ToStd(found->GetTitle());
            std::string closedPath  = ToStd(found->GetPathName());
            bool wasSelected = (found == g_selectedDoc);
            if (!Cb_CloseDocument(found, save))
            { error = "close failed"; return; }
            if (wasSelected) g_selectedDoc = nullptr;   // target gone -> follow GUI focus
            response["closed"] = true;
            response["title"]  = closedTitle;
            response["path"]   = closedPath;
        });

    // undo / redo -- the same model-side funnel every Undo/Redo button uses
    // (DataModelDoc::Undo/Redo: RAII view lock + replay + all-views refresh
    // when anything was restored). Returns the restored-object count.
    CbCommandServer::Register("undo",
        [](const json&, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            response["restored"] = pDoc->GetDataModelDoc().Undo();
            response["canUndo"]  = pDoc->CanUndo();
            response["canRedo"]  = pDoc->CanRedo();
        });

    CbCommandServer::Register("redo",
        [](const json&, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            response["restored"] = pDoc->GetDataModelDoc().Redo();
            response["canUndo"]  = pDoc->CanUndo();
            response["canRedo"]  = pDoc->CanRedo();
        });

    // Creates a new untitled document. Mirrors File / New in the menu.
    CbCommandServer::Register("new_model",
        [](const json& /*params*/, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = Cb_ShellNewDocument();
            if (!pDoc)
            {
                error = "new document was cancelled or failed";
                return;
            }
            response["title"] = ToStd(pDoc->GetTitle());
            response["path"]  = ToStd(pDoc->GetPathName());
        });

    // new_model_basic({name, h_file?}) — creates a new model with serialize
    // OFF, bypassing the New-Model wizard. Only the default ExternClasses /
    // OtherTypes / Actors groups + scalar types are populated. Once created
    // this way, serialize cannot be enabled later (matches GUI invariant).
    //
    // h_file defaults to name + ".h" if omitted/empty.
    CbCommandServer::Register("new_model_basic",
        [](const json& params, json& response, std::string& error)
        {
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            std::string hFile = params.value("h_file", std::string());
            if (hFile.empty()) hFile = name + ".h";

            CbCommandServer::PendingNewModelParams p;
            p.name      = CbString(name.c_str());
            p.hFile     = CbString(hFile.c_str());
            p.className = "";
            p.serialize = false;
            p.undoRedo  = false;
            CbCommandServer::SetPendingNewModelParams(&p);
            CClassBuilderDoc* pDoc = Cb_ShellNewDocument();
            CbCommandServer::SetPendingNewModelParams(NULL);

            if (!pDoc) { error = "new document failed"; return; }
            response["title"] = ToStd(pDoc->GetTitle());
            response["path"]  = ToStd(pDoc->GetPathName());
        });

    // new_model_serialize({name, document_class, h_file?, undo_redo?}) —
    // creates a new model with serialize ON, bypassing the wizard.
    // InitSerialize creates `CbObject` (extern), `<document_class>`,
    // `<document_class>Object`, the 1-to-many relation, _membersOnly,
    // SerializeMembersOnly, etc. With undo_redo=true, InitUndoRedo also runs.
    //
    // Once created with serialize ON it cannot be turned off; undo_redo can
    // be enabled later but never disabled (matches GUI invariant).
    //
    // h_file defaults to name + ".h" if omitted/empty.
    CbCommandServer::Register("new_model_serialize",
        [](const json& params, json& response, std::string& error)
        {
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            std::string className = params.value("document_class", std::string());
            if (className.empty()) { error = "missing 'document_class'"; return; }
            std::string hFile = params.value("h_file", std::string());
            if (hFile.empty()) hFile = name + ".h";
            bool undoRedo = params.value("undo_redo", false);

            CbCommandServer::PendingNewModelParams p;
            p.name      = CbString(name.c_str());
            p.hFile     = CbString(hFile.c_str());
            p.className = CbString(className.c_str());
            p.serialize = true;
            p.undoRedo  = undoRedo;
            CbCommandServer::SetPendingNewModelParams(&p);
            CClassBuilderDoc* pDoc = Cb_ShellNewDocument();
            CbCommandServer::SetPendingNewModelParams(NULL);

            if (!pDoc) { error = "new document failed"; return; }
            response["title"] = ToStd(pDoc->GetTitle());
            response["path"]  = ToStd(pDoc->GetPathName());
        });

    // open_doc({path}) — opens an existing .cbz via the same code path as
    // File / Open.
    CbCommandServer::Register("open_doc",
        [](const json& params, json& response, std::string& error)
        {
            std::string path = params.value("path", std::string());
            if (path.empty()) { error = "missing 'path'"; return; }

            CClassBuilderDoc* pDoc = Cb_ShellOpenDocument(path.c_str());
            if (!pDoc) { error = "open failed for: " + path; return; }
            response["title"] = ToStd(pDoc->GetTitle());
            response["path"]  = ToStd(pDoc->GetPathName());
        });

    // -- Read commands -----------------------------------------------------

    // Returns info about the active MDI document (or null if none open).
    CbCommandServer::Register("active_doc",
        [](const json& /*params*/, json& response, std::string& /*error*/)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc)
            {
                response = nullptr;
                return;
            }
            response["title"]    = ToStd(pDoc->GetTitle());
            response["path"]     = ToStd(pDoc->GetPathName());
            response["modified"] = (bool)pDoc->IsModified();
        });

    // list_classes() — names of all Class objects in the active model's
    // DataModel. Walks the DataModel→Class multi relation.
    CbCommandServer::Register("list_classes",
        [](const json& /*params*/, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            json arr = json::array();
            DataModel::ClassIterator iClass(pDataModel);
            while (++iClass)
                arr.push_back(ToStd(iClass->GetName()));
            response = arr;
        });

    // find_class({name}) — typed Class lookup via DataModel::FindClass.
    // Returns null if not present.
    CbCommandServer::Register("find_class",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            Class* pClass = pDataModel->FindClass(CbString(name.c_str()));
            response = pClass ? ToJson(pClass) : json(nullptr);
        });

    // find_extern_class({name}) — BaseClass lookup, narrowed to ExternClass
    // (or Class, since Class : ExternClass : BaseClass).
    CbCommandServer::Register("find_extern_class",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(name.c_str()));
            if (pBC && !pBC->IsExternClass())
                pBC = NULL;
            response = pBC ? ToJson(pBC) : json(nullptr);
        });

    // get_class({name}) — rich class record: serialize, files, dll_export,
    // note, inherits[], members[], methods[]. Use this for full class
    // inspection; `find_class` returns only {name, kind} for existence checks.
    CbCommandServer::Register("get_class",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            Class* pClass = pDataModel->FindClass(CbString(name.c_str()));
            if (!pClass) { response = nullptr; return; }

            response["name"]          = ToStd(pClass->GetName());
            response["kind"]          = "Class";
            response["serialize"]     = (bool)pClass->GetSerialize();
            response["h_file"]        = ToStd(pClass->GetHFile());
            response["cpp_file"]      = ToStd(pClass->GetCppFile());
            response["dll_export"]    = (bool)pClass->GetDllExport();
            response["note"]          = ToStd(pClass->GetNote());
            response["member_prefix"] = ToStd(pClass->GetMemberPrefix());

            json inherits = json::array();
            // Outgoing edges (the bases this class inherits from) live on
            // ExternClass's InheritIterator, NOT BaseClass's. BaseClass's
            // iterator yields *incoming* edges — classes that derive from
            // `this`. See ClassShape.cpp:83-102 for both sides side-by-side.
            ExternClass::InheritIterator iInherit(pClass);
            while (++iInherit)
            {
                BaseClass* pBase = iInherit->GetBaseClass();
                if (!pBase) continue;
                json e;
                e["name"]    = ToStd(pBase->GetName());
                e["virtual"] = (bool)iInherit->GetVirtual();
                inherits.push_back(e);
            }
            response["inherits"] = inherits;

            json members = json::array();
            BaseClass::MemberIterator iMember(pClass);
            while (++iMember)
                members.push_back(ToJson(iMember));
            response["members"] = members;

            json methods = json::array();
            BaseClass::MethodIterator iMethod(pClass);
            while (++iMethod)
                methods.push_back(ToJson(iMethod));
            response["methods"] = methods;
        });

    // list_members({class}) — granular alternative to get_class for clients
    // that only want the member list. Returns [] for an empty class.
    CbCommandServer::Register("list_members",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { response = nullptr; return; }

            json arr = json::array();
            BaseClass::MemberIterator iMember(pBC);
            while (++iMember)
                arr.push_back(ToJson(iMember));
            response = arr;
        });

    // list_inherits({class}) — outgoing inheritance edges of the named
    // class. Each entry is {name, virtual} where `name` is the base class
    // (the target of the inherit). Returns [] for a class with no bases.
    CbCommandServer::Register("list_inherits",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { response = nullptr; return; }

            // Outgoing inherits live on ExternClass; bare BaseClass entries
            // can't have outgoing inherits, so an empty array is correct.
            ExternClass* pEC = dynamic_cast<ExternClass*>(pBC);
            json arr = json::array();
            if (pEC)
            {
                ExternClass::InheritIterator iInherit(pEC);
                while (++iInherit)
                {
                    BaseClass* pBase = iInherit->GetBaseClass();
                    if (!pBase) continue;
                    json e;
                    e["name"]    = ToStd(pBase->GetName());
                    e["virtual"] = (bool)iInherit->GetVirtual();
                    arr.push_back(e);
                }
            }
            response = arr;
        });

    // add_class({name, serialize?, h_file?}) — creates a new Class on the
    // active model. `serialize` defaults to the model's own serialize flag.
    // When serialize=true the new class auto-inherits the model's
    // DocumentObject (which transitively inherits CbObject), mirroring the
    // GUI's DataModel::OnAddClass path. Rejects serialize=true on a
    // non-serialize model (no DocumentObject to inherit from).
    CbCommandServer::Register("add_class",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            CbString cname(name.c_str());
            if (pDataModel->GetDataModelDoc()->FindBaseClass(cname))
            { error = "a class with that name already exists"; return; }

            bool modelHasSerialize = (pDataModel->GetSerialize() != 0);
            bool wantSerialize = params.value("serialize", modelHasSerialize);
            if (wantSerialize && !modelHasSerialize)
            { error = "cannot create a serialize class in a non-serialize model"; return; }

            std::string hFile = params.value("h_file", std::string());

            pDataModel->GetDataModelDoc()->MarkLastUndo();

            Class* pClass = new Class(pDataModel);
            pClass->SetOrder(pDataModel->GetClassCount() - 1);

            // Reconcile serialize state if it differs from the model default
            // (Class ctor initializes _serialize from the model's flag).
            if (wantSerialize != (bool)pClass->GetSerialize())
                pClass->SetSerialize(wantSerialize ? 1 : 0);

            // Ensure the docObject inherit exists when serialize is on. If
            // SetSerialize ran the off->on branch above it already added one;
            // for the model-default-serialize case it didn't, so add manually
            // (mirrors DataModel::OnAddClass).
            if (pClass->GetSerialize() && pClass->GetInheritCount() == 0)
            {
                Inherit* pInherit = new Inherit(pClass, pDataModel->GetDocumentObject());
                pInherit->SetVirtual(1);
            }

            pClass->SetName(cname);
            if (!hFile.empty())
                pClass->SetHFile(CbString(hFile.c_str()));

            pClass->Add();

            response["name"]      = ToStd(pClass->GetName());
            response["kind"]      = "Class";
            response["serialize"] = (bool)pClass->GetSerialize();
            response["h_file"]    = ToStd(pClass->GetHFile());
        });

    // delete_class({name}) — removes a Class from the active model. Honors
    // the GUI invariants (cannot delete the document or document-object
    // class, cannot delete a class that has its own outgoing inheritances —
    // the latter matches Class::OnDelete's existing guard) but skips the
    // confirmation dialog.
    CbCommandServer::Register("delete_class",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            Class* pClass = pDataModel->FindClass(CbString(name.c_str()));
            if (!pClass) { error = "no such class"; return; }

            if (pClass == pDataModel->GetDocument())
            { error = "cannot delete the document class"; return; }
            if (pClass == pDataModel->GetDocumentObject())
            { error = "cannot delete the document-object class"; return; }
            if (pClass->BaseClass::GetInheritCount())
            { error = "cannot delete a class with inheritances; remove them first"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            response["name"] = ToStd(pClass->GetName());
            pClass->Delete();
        });

    // set_class_serialize({name, value}) — toggles the serialize flag.
    // Calls Class::SetSerialize, which prunes inheritances on off->on
    // (keeps the first base leading to docObject, deletes the rest; adds
    // the docObject inherit if none) and removes the docObject inherit on
    // on->off. Rejects value=true when the model itself isn't serialize-on
    // (no DocumentObject to inherit from).
    CbCommandServer::Register("set_class_serialize",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            bool value = params["value"].get<bool>();

            Class* pClass = pDataModel->FindClass(CbString(name.c_str()));
            if (!pClass) { error = "no such class"; return; }

            if (value && !pDataModel->GetSerialize())
            { error = "cannot enable serialize in a non-serialize model"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            pClass->SetSerialize(value ? 1 : 0);

            response["name"]      = ToStd(pClass->GetName());
            response["serialize"] = (bool)pClass->GetSerialize();
        });

    // set_class_h_file({name, value}), set_class_note({name, value}),
    // set_class_dll_export({name, value}) — small attribute setters.
    CbCommandServer::Register("set_class_h_file",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();

            Class* pClass = pDataModel->FindClass(CbString(name.c_str()));
            if (!pClass) { error = "no such class"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            pClass->SetHFile(CbString(value.c_str()));
            response["name"]   = ToStd(pClass->GetName());
            response["h_file"] = ToStd(pClass->GetHFile());
        });

    CbCommandServer::Register("set_class_note",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();

            Class* pClass = pDataModel->FindClass(CbString(name.c_str()));
            if (!pClass) { error = "no such class"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            pClass->SetNote(CbString(value.c_str()));
            response["name"] = ToStd(pClass->GetName());
            response["note"] = ToStd(pClass->GetNote());
        });

    // add_inherit({class, base, virtual?}) — adds an outgoing inheritance
    // edge from `class` to `base`. Restricted to non-serialize classes;
    // for serialize-on classes the inheritance is auto-managed by
    // set_class_serialize (toggle off → modify → on if you need to change
    // a serialize class's base). Rejects self-inherit, cycles, duplicates.
    CbCommandServer::Register("add_inherit",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            std::string className = params.value("class", std::string());
            std::string baseName  = params.value("base",  std::string());
            if (className.empty() || baseName.empty())
            { error = "missing 'class' or 'base'"; return; }
            bool isVirtual = params.value("virtual", false);

            Class* pClass = pDataModel->FindClass(CbString(className.c_str()));
            if (!pClass) { error = "no such class"; return; }

            if (pClass->GetSerialize())
            { error = "serialize class inheritance is managed by set_class_serialize"; return; }

            BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                                CbString(baseName.c_str()));
            if (!pBC) { error = "no such base class"; return; }
            ExternClass* pBase = dynamic_cast<ExternClass*>(pBC);
            if (!pBase) { error = "base must be a Class or ExternClass"; return; }

            if (pBase == pClass)
            { error = "cannot inherit from self"; return; }

            // Cycle: would creating "class inherits base" close a cycle?
            // pBase->IsBaseClass(pClass) returns true iff pClass already
            // appears in pBase's ancestor chain.
            if (pBase->IsBaseClass(pClass))
            { error = "would create an inheritance cycle"; return; }

            // Duplicate: walk outgoing inherits, see if pBase is already there.
            ExternClass::InheritIterator iInherit(pClass);
            while (++iInherit)
            {
                if (iInherit->GetBaseClass() == pBase)
                { error = "class already inherits from this base"; return; }
            }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            Inherit* pInherit = new Inherit(pClass, pBase);
            if (isVirtual)
                pInherit->SetVirtual(1);
            pInherit->Add();

            response["class"]   = ToStd(pClass->GetName());
            response["base"]    = ToStd(pBase->GetName());
            response["virtual"] = (bool)pInherit->GetVirtual();
        });

    // remove_inherit({class, base}) — removes the inheritance edge from
    // `class` to `base`. Restricted to non-serialize classes (same rationale
    // as add_inherit). Errors if no such edge exists.
    CbCommandServer::Register("remove_inherit",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            std::string className = params.value("class", std::string());
            std::string baseName  = params.value("base",  std::string());
            if (className.empty() || baseName.empty())
            { error = "missing 'class' or 'base'"; return; }

            Class* pClass = pDataModel->FindClass(CbString(className.c_str()));
            if (!pClass) { error = "no such class"; return; }

            if (pClass->GetSerialize())
            { error = "serialize class inheritance is managed by set_class_serialize"; return; }

            BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                                CbString(baseName.c_str()));
            if (!pBC) { error = "no such base class"; return; }

            Inherit* pTarget = NULL;
            ExternClass::InheritIterator iInherit(pClass);
            while (++iInherit)
            {
                if (iInherit->GetBaseClass() == pBC)
                {
                    pTarget = iInherit;
                    break;
                }
            }
            if (!pTarget) { error = "class does not inherit from this base"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            response["class"] = ToStd(pClass->GetName());
            response["base"]  = ToStd(pBC->GetName());
            pTarget->Delete();
        });

    // ----- Relation commands ----------------------------------------------
    //
    // Lookup key: {class, from_name, to_name?}. `class` is the from-side.
    // `to_name` is optional; if omitted, the first match wins (and an
    // ambiguity error fires when more than one matches by from_name alone).
    //
    // Lookup helper. Mirrors Class::FindFromRelation but tolerant of an
    // omitted to_name (uses first-match-with-ambiguity-check semantics).
    auto resolveRelation =
        [](Class* pClass, const json& params, std::string& error) -> Relation*
    {
        std::string fromName = params.value("from_name", std::string());
        if (fromName.empty()) { error = "missing 'from_name'"; return NULL; }

        bool haveToName = params.contains("to_name");
        std::string toName = params.value("to_name", std::string());
        CbString cFromName(fromName.c_str());
        CbString cToName(toName.c_str());

        Relation* pHit = NULL;
        Class::FromRelationIterator iRel(pClass);
        while (++iRel)
        {
            if (iRel->GetFromName() != cFromName) continue;
            if (haveToName && iRel->GetToName() != cToName) continue;
            if (pHit && !haveToName)
            { error = "ambiguous: multiple relations match; specify 'to_name'"; return NULL; }
            pHit = iRel;
            if (haveToName) break;
        }
        if (!pHit) error = "no such relation on the class";
        return pHit;
    };

    // Translate kind string ↔ (static, multi, single) triple.
    auto kindToTriple =
        [](const std::string& kind, int& xStatic, int& multi, int& single,
           std::string& error) -> bool
    {
        if      (kind == "single")       { xStatic = 0; multi = 0; single = 1; }
        else if (kind == "multi")        { xStatic = 0; multi = 1; single = 0; }
        else if (kind == "static_multi") { xStatic = 1; multi = 1; single = 0; }
        else { error = "invalid 'kind' (use single/multi/static_multi)"; return false; }
        return true;
    };

    auto kindFromRelation = [](Relation* pR) -> const char*
    {
        if (pR->GetStatic()) return "static_multi";
        if (pR->GetMulti())  return "multi";
        return "single";
    };

    auto implFromRelation = [](Relation* pR) -> const char*
    {
        switch (pR->GetImplementation())
        {
        case 1: return pR->GetRelationMember()
                       && pR->GetRelationMember()->IsUniqueValueTree()
                           ? "unique_value_tree" : "value_tree";
        case 2: return "avl_tree";
        default: return "list";
        }
    };

    auto relationToJson = [&](Relation* pR) -> json
    {
        json j;
        j["from_class"]  = ToStd(pR->GetFromClass()->GetName());
        j["to_class"]    = ToStd(pR->GetToClass()->GetName());
        j["from_name"]   = ToStd(pR->GetFromName());
        j["to_name"]     = ToStd(pR->GetToName());
        j["kind"]        = kindFromRelation(pR);
        j["owned"]       = (bool)pR->GetOwned();
        j["critical"]    = (bool)pR->GetCritical();
        j["filter"]      = (bool)pR->GetFilter();
        j["implementation"] = implFromRelation(pR);
        if (pR->GetRelationMember() && pR->GetRelationMember()->GetMember())
            j["member"] = ToStd(pR->GetRelationMember()->GetMember()->GetVariableName());
        else
            j["member"] = nullptr;
        j["note"] = ToStd(pR->GetNote());
        return j;
    };

    // list_relations({class}) — outgoing relations of the class.
    CbCommandServer::Register("list_relations",
        [relationToJson](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            Class* pClass = pDataModel->FindClass(CbString(className.c_str()));
            if (!pClass) { error = "no such class"; return; }

            json arr = json::array();
            Class::FromRelationIterator iRel(pClass);
            while (++iRel)
                arr.push_back(relationToJson(iRel));
            response = arr;
        });

    // get_relation({class, from_name, to_name?}) — rich record.
    CbCommandServer::Register("get_relation",
        [resolveRelation, relationToJson]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            Class* pClass = pDataModel->FindClass(CbString(className.c_str()));
            if (!pClass) { error = "no such class"; return; }
            Relation* pRel = resolveRelation(pClass, params, error);
            if (!pRel) return;
            response = relationToJson(pRel);
        });

    // add_relation({from_class, to_class, from_name?, to_name?, kind?,
    //               owned?, critical?, filter?, implementation?, member?,
    //               unique?, note?})
    //
    // Defaults: from_name = from_class.GetBaseName(), to_name =
    // to_class.GetBaseName(), kind = "multi", owned = true, critical =
    // false, filter = false, implementation = "list".
    //
    // For implementation in {value_tree, avl_tree}: `member` is required
    // (must exist on `to_class`). `unique:true` promotes value_tree to
    // UniqueValueTree.
    //
    // Static-multi requires both classes to be non-template AND
    // non-serialize (matches the dialog's IDC_STATICMULTI gate).
    // Implementation flavours other than "list" only apply to multi
    // relations (not static_multi or single).
    CbCommandServer::Register("add_relation",
        [kindToTriple, relationToJson]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            std::string fromCls = params.value("from_class", std::string());
            std::string toCls   = params.value("to_class",   std::string());
            if (fromCls.empty() || toCls.empty())
            { error = "missing 'from_class' or 'to_class'"; return; }

            Class* pFrom = pDataModel->FindClass(CbString(fromCls.c_str()));
            Class* pTo   = pDataModel->FindClass(CbString(toCls.c_str()));
            if (!pFrom) { error = "from_class not found: " + fromCls; return; }
            if (!pTo)   { error = "to_class not found: "   + toCls;   return; }

            std::string kind = params.value("kind", std::string("multi"));
            int xStatic, multi, single;
            if (!kindToTriple(kind, xStatic, multi, single, error)) return;

            // Static-multi gate (matches RelationDialog OnCheckTemplate).
            if (xStatic)
            {
                if (!pFrom->GetTemplate().IsEmpty() ||
                    !pTo->GetTemplate().IsEmpty())
                { error = "static_multi requires non-template classes"; return; }
                if (pFrom->GetSerialize() || pTo->GetSerialize())
                { error = "static_multi requires non-serialize classes"; return; }
            }

            std::string fromName = params.value("from_name", std::string());
            std::string toName   = params.value("to_name",   std::string());
            if (fromName.empty()) fromName = ToStd(pFrom->GetBaseName());
            if (toName.empty())   toName   = ToStd(pTo->GetBaseName());

            // Reject duplicate (same from_name + to_name on the same from-class).
            if (pFrom->FindFromRelation(CbString(fromName.c_str()),
                                        CbString(toName.c_str())))
            { error = "a relation with that from_name + to_name already exists"; return; }

            bool owned    = params.value("owned",    true);
            bool critical = params.value("critical", false);
            bool filter   = params.value("filter",   false);

            // Implementation — only valid for multi (non-static).
            std::string impl = params.value("implementation", std::string("list"));
            if (impl != "list" && impl != "value_tree" && impl != "avl_tree")
            { error = "invalid 'implementation'"; return; }
            if (impl != "list" && (xStatic || single))
            { error = "tree implementations require kind=multi"; return; }

            Member* pMember = NULL;
            if (impl != "list")
            {
                std::string memberName = params.value("member", std::string());
                if (memberName.empty())
                { error = "tree implementations require 'member'"; return; }
                pMember = pTo->FindMember(CbString(memberName.c_str()));
                if (!pMember)
                { error = "member '" + memberName + "' not found on to_class"; return; }
            }
            bool unique = params.value("unique", false);
            if (unique && impl != "value_tree")
            { error = "'unique' only applies to value_tree"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();

            Relation* pRel = new Relation(
                pFrom, pTo,
                CbString(fromName.c_str()), CbString(toName.c_str()),
                xStatic, multi, single,
                owned ? 1 : 0, critical ? 1 : 0);
            pRel->SetFilter(filter ? 1 : 0);
            if (params.contains("note"))
                pRel->SetNote(CbString(params["note"].get<std::string>().c_str()));

            if (impl == "value_tree")
            {
                if (unique) (void)new UniqueValueTree(pRel, pMember);
                else        (void)new ValueTree(pRel, pMember);
            }
            else if (impl == "avl_tree")
            {
                (void)new AvlTree(pRel, pMember);
            }

            pRel->Add();

            // An owned relation contributes the from-side pointer to the
            // to-class's ConstructorIncludeMethod args (mirrors the dialog
            // path at RelationDialog.cpp:194). Without this, constructors
            // added on the to-class miss the parent-pointer arg.
            if (pRel->GetOwned())
                pTo->GetConstructorIncludeMethod()->UpdateArguments();

            response = relationToJson(pRel);
        });

    // delete_relation({class, from_name, to_name?})
    CbCommandServer::Register("delete_relation",
        [resolveRelation](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            Class* pClass = pDataModel->FindClass(CbString(className.c_str()));
            if (!pClass) { error = "no such class"; return; }
            Relation* pRel = resolveRelation(pClass, params, error);
            if (!pRel) return;

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            response["from_class"] = ToStd(pRel->GetFromClass()->GetName());
            response["to_class"]   = ToStd(pRel->GetToClass()->GetName());
            response["from_name"]  = ToStd(pRel->GetFromName());
            response["to_name"]    = ToStd(pRel->GetToName());
            pRel->Delete();
        });

    // Boolean / string setters share a small registration helper.
    auto registerRelationSetter =
        [resolveRelation, relationToJson]
        (const std::string& cmdName,
         std::function<void(Relation*, const json&, std::string&)> apply)
    {
        CbCommandServer::Register(cmdName,
            [resolveRelation, relationToJson, apply]
            (const json& params, json& response, std::string& error)
            {
                DataModel* pDataModel = GetActiveDataModel();
                if (!pDataModel) { error = "no active document"; return; }
                std::string className = params.value("class", std::string());
                if (className.empty()) { error = "missing 'class'"; return; }
                if (!params.contains("value"))
                { error = "missing 'value'"; return; }
                Class* pClass = pDataModel->FindClass(CbString(className.c_str()));
                if (!pClass) { error = "no such class"; return; }
                Relation* pRel = resolveRelation(pClass, params, error);
                if (!pRel) return;

                pDataModel->GetDataModelDoc()->MarkLastUndo();
                apply(pRel, params, error);
                if (!error.empty()) return;
                response = relationToJson(pRel);
            });
    };

    registerRelationSetter("set_relation_from_name",
        [](Relation* pR, const json& p, std::string&)
        {
            pR->SetFromName(CbString(p["value"].get<std::string>().c_str()));
        });
    registerRelationSetter("set_relation_to_name",
        [](Relation* pR, const json& p, std::string&)
        {
            pR->SetToName(CbString(p["value"].get<std::string>().c_str()));
        });
    registerRelationSetter("set_relation_note",
        [](Relation* pR, const json& p, std::string&)
        {
            pR->SetNote(CbString(p["value"].get<std::string>().c_str()));
        });
    registerRelationSetter("set_relation_owned",
        [](Relation* pR, const json& p, std::string&)
        {
            pR->SetOwned(p["value"].get<bool>() ? 1 : 0);
            // Toggling ownership changes whether the to-class's
            // ConstructorIncludeMethod takes the from-side pointer as an
            // arg. Mirrors RelationDialog::Update.
            Class* pToClass = dynamic_cast<Class*>(pR->GetToClass());
            if (pToClass && pToClass->GetConstructorIncludeMethod())
                pToClass->GetConstructorIncludeMethod()->UpdateArguments();
        });
    registerRelationSetter("set_relation_critical",
        [](Relation* pR, const json& p, std::string&)
        {
            pR->SetCritical(p["value"].get<bool>() ? 1 : 0);
        });
    registerRelationSetter("set_relation_filter",
        [](Relation* pR, const json& p, std::string& err)
        {
            if (pR->GetStatic())
            { err = "filter does not apply to static_multi relations"; return; }
            pR->SetFilter(p["value"].get<bool>() ? 1 : 0);
        });

    // set_relation_implementation({class, from_name, to_name?,
    //                              implementation, member?, unique?})
    //
    // implementation: "list" / "value_tree" / "avl_tree"
    // - "list" deletes any existing RelationMember
    // - "value_tree" / "avl_tree" require `member` (must exist on to_class)
    // - `unique:true` for value_tree promotes to UniqueValueTree
    //
    // Restricted to multi (non-static) relations.
    CbCommandServer::Register("set_relation_implementation",
        [resolveRelation, relationToJson]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            std::string impl = params.value("implementation", std::string());
            if (impl.empty()) { error = "missing 'implementation'"; return; }

            Class* pClass = pDataModel->FindClass(CbString(className.c_str()));
            if (!pClass) { error = "no such class"; return; }
            Relation* pRel = resolveRelation(pClass, params, error);
            if (!pRel) return;

            if (pRel->GetStatic() || !pRel->GetMulti())
            { error = "implementation only applies to multi relations"; return; }

            if (impl != "list" && impl != "value_tree" && impl != "avl_tree")
            { error = "invalid 'implementation'"; return; }

            Member* pMember = NULL;
            bool unique = params.value("unique", false);
            if (impl != "list")
            {
                std::string memberName = params.value("member", std::string());
                if (memberName.empty())
                { error = "tree implementations require 'member'"; return; }
                pMember = pRel->GetToClass()->FindMember(CbString(memberName.c_str()));
                if (!pMember)
                { error = "member '" + memberName + "' not found on to_class"; return; }
            }
            if (unique && impl != "value_tree")
            { error = "'unique' only applies to value_tree"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();

            // Delete the existing RelationMember (any subclass) and re-create.
            if (pRel->GetRelationMember())
                pRel->GetRelationMember()->Delete();

            if (impl == "value_tree")
            {
                if (unique) (void)new UniqueValueTree(pRel, pMember);
                else        (void)new ValueTree(pRel, pMember);
            }
            else if (impl == "avl_tree")
            {
                (void)new AvlTree(pRel, pMember);
            }

            response = relationToJson(pRel);
        });

    // ----- Class-diagram commands -----------------------------------------
    //
    // ClassDiagrams are owned by the DataModel. Each diagram has visibility
    // flags for which member/method categories to show. Classes are placed
    // on a diagram via ClassShape; auto-placement uses Grid::Place. UML
    // notation is set ON unconditionally (the legacy non-UML style isn't
    // exposed via the pipe).

    // Lookup a ClassDiagram by name on the active DataModel.
    auto resolveDiagram =
        [](DataModel* pDataModel, const std::string& name, std::string& error)
        -> ClassDiagram*
    {
        if (name.empty()) { error = "missing 'name'"; return NULL; }
        CbString cname(name.c_str());
        DataModelDoc::ClassDiagramIterator iCD(pDataModel->GetDataModelDoc());
        while (++iCD)
        {
            if (iCD->GetName() == cname) return iCD;
        }
        error = "no such class diagram";
        return NULL;
    };

    // Place a new ClassShape at the first non-overlapping grid position.
    // Mirrors SelectClassesDialog's placement loop.
    auto placeNewClassShape =
        [](ClassDiagram* pCD, BaseClass* pBC) -> ClassShape*
    {
        bool stop = false;
        int x = 0, y = 200;
        while (!stop && y < pCD->GetHeight())
        {
            x = 100;
            while (!stop && x < pCD->GetWidth())
            {
                stop = true;
                CbRect rect(x, -(y+250), x+250, -y);
                ClassDiagram::ClassDiagramShapeIterator
                    iShape(pCD, &ClassDiagramShape::IsClassShape);
                while (++iShape)
                {
                    // Overlap test (was CRect::IntersectRect; CbRect has no
                    // intersect helper -- normalized rects, strict overlap).
                    CbRect other = iShape->GetRect();
                    other.NormalizeRect();
                    if (rect.left < other.right && other.left < rect.right &&
                        rect.top < other.bottom && other.top < rect.bottom)
                    { stop = false; break; }
                }
                if (!stop) x += 500;
            }
            if (!stop) y += 600;
        }
        if (!stop) { x = y = 0; }
        CbPoint point(x, -y);
        return new ClassShape(pCD, pBC, point);
    };

    // Helper: read visibility flags from params and apply to the diagram.
    auto applyDiagramVisibility =
        [](ClassDiagram* pCD, const json& params)
    {
        if (params.contains("public_members"))
            pCD->SetPublicMembers(params["public_members"].get<bool>());
        if (params.contains("public_methods"))
            pCD->SetPublicMethods(params["public_methods"].get<bool>());
        if (params.contains("protected_members"))
            pCD->SetProtectedMembers(params["protected_members"].get<bool>());
        if (params.contains("protected_methods"))
            pCD->SetProtectedMethods(params["protected_methods"].get<bool>());
        if (params.contains("private_members"))
            pCD->SetPrivateMembers(params["private_members"].get<bool>());
        if (params.contains("private_methods"))
            pCD->SetPrivateMethods(params["private_methods"].get<bool>());
        if (params.contains("get_set_methods"))
            pCD->SetGetSetMethods(params["get_set_methods"].get<bool>());
    };

    auto diagramToJson = [](ClassDiagram* pCD) -> json
    {
        json j;
        j["name"] = ToStd(pCD->GetName());
        j["public_members"]    = (bool)pCD->GetPublicMembers();
        j["public_methods"]    = (bool)pCD->GetPublicMethods();
        j["protected_members"] = (bool)pCD->GetProtectedMembers();
        j["protected_methods"] = (bool)pCD->GetProtectedMethods();
        j["private_members"]   = (bool)pCD->GetPrivateMembers();
        j["private_methods"]   = (bool)pCD->GetPrivateMethods();
        j["get_set_methods"]   = (bool)pCD->GetGetSetMethods();

        json classes = json::array();
        ClassDiagram::ClassDiagramShapeIterator
            iShape(pCD, &ClassDiagramShape::IsClassShape);
        while (++iShape)
        {
            ClassShape* pCS = dynamic_cast<ClassShape*>(iShape.Get());
            if (pCS && pCS->GetBaseClass())
                classes.push_back(ToStd(pCS->GetBaseClass()->GetName()));
        }
        j["classes"] = classes;
        return j;
    };

    // add_class_diagram({name, classes?, auto_place?, public_members?,
    //                    public_methods?, protected_members?, protected_methods?,
    //                    private_members?, private_methods?, get_set_methods?})
    //
    // Creates a new ClassDiagram on the DataModel, optionally adds an
    // initial set of classes, optionally runs Grid::Place for layout.
    // Visibility flags default to public-only (matching the dialog default).
    CbCommandServer::Register("add_class_diagram",
        [applyDiagramVisibility, placeNewClassShape, diagramToJson]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            // Reject duplicate name (within the model).
            CbString cname(name.c_str());
            DataModelDoc::ClassDiagramIterator iCD(pDataModel->GetDataModelDoc());
            while (++iCD)
            {
                if (iCD->GetName() == cname)
                { error = "a class diagram with that name already exists"; return; }
            }

            // Pre-validate class list before mutating.
            if (params.contains("classes") && params["classes"].is_array())
            {
                for (size_t i = 0; i < params["classes"].size(); ++i)
                {
                    std::string cn = params["classes"][i].get<std::string>();
                    if (!pDataModel->GetDataModelDoc()->FindBaseClass(CbString(cn.c_str())))
                    { error = "class not found: " + cn; return; }
                }
            }

            // Resolve parent: DataModel root, a BaseClass, a ClassGroup, or
            // a MetaGroup. Same options as add_sequence_diagram — see the
            // group commands section for resolveDiagramParent semantics
            // (this is the equivalent inline since the helper is defined
            // later in the file).
            DataModelDoc& dmd = *pDataModel->GetDataModelDoc();
            Gti* pParent = pDataModel;
            if (params.contains("parent_class"))
            {
                std::string pc = params["parent_class"].get<std::string>();
                BaseClass* pBC = dmd.FindBaseClass(CbString(pc.c_str()));
                if (!pBC) { error = "parent_class not found"; return; }
                pParent = pBC;
            }
            else if (params.contains("parent_class_group_id"))
            {
                UINT id = (UINT)params["parent_class_group_id"].get<unsigned>();
                ClassGroup* pCG = dynamic_cast<ClassGroup*>(
                    dmd.FindDataModelDocObject(id));
                if (!pCG) { error = "no ClassGroup with that id"; return; }
                pParent = pCG;
            }
            else if (params.contains("parent_meta_group_id"))
            {
                UINT id = (UINT)params["parent_meta_group_id"].get<unsigned>();
                MetaGroup* pMG = dynamic_cast<MetaGroup*>(
                    dmd.FindDataModelDocObject(id));
                if (!pMG) { error = "no MetaGroup with that id"; return; }
                pParent = pMG;
            }

            dmd.MarkLastUndo();

            ClassDiagram* pCD = new ClassDiagram(pParent);
            pCD->SetName(cname);

            // Default visibility: public-only (matches dialog default).
            pCD->SetPublicMembers(true);
            pCD->SetPublicMethods(true);
            applyDiagramVisibility(pCD, params);

            pCD->Add();

            if (params.contains("classes") && params["classes"].is_array())
            {
                for (size_t i = 0; i < params["classes"].size(); ++i)
                {
                    std::string cn = params["classes"][i].get<std::string>();
                    BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                                    CbString(cn.c_str()));
                    placeNewClassShape(pCD, pBC);
                }
            }

            if (params.value("auto_place", false))
            {
                // Refresh cached rects — Grid::Place reads GetRect() which
                // only updates during Draw(), and no Draw has happened yet.
                ClassDiagram::ClassDiagramShapeIterator
                    iCS(pCD, &ClassDiagramShape::IsClassShape);
                while (++iCS)
                {
                    ClassShape* pCS = dynamic_cast<ClassShape*>(iCS.Get());
                    if (pCS) pCS->RecalculateRect();
                }
                Grid grid(pCD);
                grid.Place();
            }

            response = diagramToJson(pCD);
        });

    // list_class_diagrams() — names of every ClassDiagram on the model.
    CbCommandServer::Register("list_class_diagrams",
        [](const json& /*params*/, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            json arr = json::array();
            DataModelDoc::ClassDiagramIterator iCD(pDataModel->GetDataModelDoc());
            while (++iCD)
                arr.push_back(ToStd(iCD->GetName()));
            response = arr;
        });

    // get_class_diagram({name}) — full record incl. classes on the diagram.
    CbCommandServer::Register("get_class_diagram",
        [resolveDiagram, diagramToJson]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("name", std::string()), error);
            if (!pCD) return;
            response = diagramToJson(pCD);
        });

    // delete_class_diagram({name})
    CbCommandServer::Register("delete_class_diagram",
        [resolveDiagram](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("name", std::string()), error);
            if (!pCD) return;
            pDataModel->GetDataModelDoc()->MarkLastUndo();
            response["name"] = ToStd(pCD->GetName());
            pCD->Delete();
        });

    // add_class_to_diagram({diagram, class}) — adds a single class shape
    // to an existing diagram. Position is the first non-overlapping grid
    // slot; run auto_place_diagram afterward to clean up layout.
    CbCommandServer::Register("add_class_to_diagram",
        [resolveDiagram, placeNewClassShape]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("diagram", std::string()), error);
            if (!pCD) { if (error.empty()) error = "missing 'diagram'"; return; }

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                            CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return; }

            // Reject if class is already on the diagram.
            if (pBC->FindClassShape(pCD))
            { error = "class is already on this diagram"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            placeNewClassShape(pCD, pBC);
            response["diagram"] = ToStd(pCD->GetName());
            response["class"]   = cn;
        });

    // remove_class_from_diagram({diagram, class})
    CbCommandServer::Register("remove_class_from_diagram",
        [resolveDiagram](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("diagram", std::string()), error);
            if (!pCD) { if (error.empty()) error = "missing 'diagram'"; return; }

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                            CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return; }
            ClassShape* pCS = pBC->FindClassShape(pCD);
            if (!pCS) { error = "class is not on this diagram"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            response["diagram"] = ToStd(pCD->GetName());
            response["class"]   = cn;
            pCS->Delete();
        });

    // auto_place_diagram({name}) — runs Grid::Place on the diagram.
    CbCommandServer::Register("auto_place_diagram",
        [resolveDiagram](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("name", std::string()), error);
            if (!pCD) return;
            pDataModel->GetDataModelDoc()->MarkLastUndo();
            // Refresh cached rects — Grid::Place reads GetRect() which
            // only updates during Draw(). A diagram that has never been
            // opened in a view has stale (empty) rects.
            ClassDiagram::ClassDiagramShapeIterator
                iCS(pCD, &ClassDiagramShape::IsClassShape);
            while (++iCS)
            {
                ClassShape* pCS = dynamic_cast<ClassShape*>(iCS.Get());
                if (pCS) pCS->RecalculateRect();
            }
            Grid grid(pCD);
            grid.Place();
            response["name"] = ToStd(pCD->GetName());
        });

    // show_class_members({diagram, class, members:[name,...]})
    // show_class_methods({diagram, class, methods:[name,...]})
    //
    // Adds MemberShape / MethodShape children to the ClassShape on the
    // named diagram for each named feature. Members/methods that are
    // already shown are left alone (no duplicates). Methods are matched
    // by name — for overloads, every match becomes a shape; use a single
    // method via id if you need finer control (not yet exposed here).
    CbCommandServer::Register("show_class_members",
        [resolveDiagram](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("diagram", std::string()), error);
            if (!pCD) { if (error.empty()) error = "missing 'diagram'"; return; }

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                            CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return; }
            ClassShape* pCS = pBC->FindClassShape(pCD);
            if (!pCS) { error = "class is not on this diagram"; return; }

            if (!params.contains("members") || !params["members"].is_array())
            { error = "missing 'members' array"; return; }

            // Pre-validate.
            for (size_t i = 0; i < params["members"].size(); ++i)
            {
                std::string mn = params["members"][i].get<std::string>();
                if (!pBC->FindMember(CbString(mn.c_str())))
                { error = "member not found on class: " + mn; return; }
            }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            json added = json::array();
            for (size_t i = 0; i < params["members"].size(); ++i)
            {
                std::string mn = params["members"][i].get<std::string>();
                Member* pM = pBC->FindMember(CbString(mn.c_str()));

                // Skip if a shape already exists for this member on pCS.
                bool already = false;
                ClassShape::MemberShapeIterator iMS(pCS);
                while (++iMS)
                {
                    if (iMS->GetMember() == pM) { already = true; break; }
                }
                if (already) continue;

                (void)new MemberShape(pCS, pM);
                added.push_back(mn);
            }

            // Re-flow layout — adding shapes grows the class shape, which
            // can overlap neighbours. Grid::Place reads cached rects via
            // GetRect(), so recalc this shape's rect first (otherwise it'd
            // place using the stale pre-add size). Skip with
            // auto_place:false if you intend to batch more changes.
            if (params.value("auto_place", true) && !added.empty())
            {
                pCS->RecalculateRect();
                Grid grid(pCD);
                grid.Place();
            }

            response["diagram"] = ToStd(pCD->GetName());
            response["class"]   = cn;
            response["added"]   = added;
        });

    CbCommandServer::Register("show_class_methods",
        [resolveDiagram](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("diagram", std::string()), error);
            if (!pCD) { if (error.empty()) error = "missing 'diagram'"; return; }

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                            CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return; }
            ClassShape* pCS = pBC->FindClassShape(pCD);
            if (!pCS) { error = "class is not on this diagram"; return; }

            if (!params.contains("methods") || !params["methods"].is_array())
            { error = "missing 'methods' array"; return; }

            // Pre-validate every name resolves.
            for (size_t i = 0; i < params["methods"].size(); ++i)
            {
                std::string mn = params["methods"][i].get<std::string>();
                if (!pBC->FindMethodWithName(CbString(mn.c_str())))
                { error = "method not found on class: " + mn; return; }
            }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            json added = json::array();
            for (size_t i = 0; i < params["methods"].size(); ++i)
            {
                std::string mn = params["methods"][i].get<std::string>();
                Method* pM = pBC->FindMethodWithName(CbString(mn.c_str()));

                bool already = false;
                ClassShape::MethodShapeIterator iMS(pCS);
                while (++iMS)
                {
                    if (iMS->GetMethod() == pM) { already = true; break; }
                }
                if (already) continue;

                (void)new MethodShape(pCS, pM);
                added.push_back(mn);
            }

            if (params.value("auto_place", true) && !added.empty())
            {
                pCS->RecalculateRect();
                Grid grid(pCD);
                grid.Place();
            }

            response["diagram"] = ToStd(pCD->GetName());
            response["class"]   = cn;
            response["added"]   = added;
        });

    // show_class_features({diagram}) — bulk: walk every class shape on
    // the diagram and add MemberShape / MethodShape for every feature
    // matching the diagram's visibility flags. Skips features that
    // already have a shape (safe to call multiple times). The "do what
    // the diagram-flags suggest" shortcut.
    //
    // Filtering rules:
    //   members: include if access matches a diagram flag
    //     (public_members / protected_members / private_members)
    //   methods: include if (a) non-macro, (b) not Constructor/Destructor,
    //     (c) access matches a diagram flag, (d) if Get/Set member method
    //     then `get_set_methods` must be on
    CbCommandServer::Register("show_class_features",
        [resolveDiagram](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("diagram", std::string()), error);
            if (!pCD) { if (error.empty()) error = "missing 'diagram'"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();

            int memberShapesAdded = 0;
            int methodShapesAdded = 0;

            auto countShapes = [](ClassShape* pCS, int& m, int& f)
            {
                m = 0; f = 0;
                ClassShape::MemberShapeIterator iMS(pCS); while (++iMS) ++m;
                ClassShape::MethodShapeIterator iFS(pCS); while (++iFS) ++f;
            };

            ClassDiagram::ClassDiagramShapeIterator
                iShape(pCD, &ClassDiagramShape::IsClassShape);
            while (++iShape)
            {
                ClassShape* pCS = dynamic_cast<ClassShape*>(iShape.Get());
                if (!pCS) continue;

                int mBefore, fBefore, mAfter, fAfter;
                countShapes(pCS, mBefore, fBefore);
                pCS->PopulateFromDiagramFlags();
                countShapes(pCS, mAfter, fAfter);

                memberShapesAdded += (mAfter - mBefore);
                methodShapesAdded += (fAfter - fBefore);
            }

            if (params.value("auto_place", true) &&
                (memberShapesAdded > 0 || methodShapesAdded > 0))
            {
                // Refresh every ClassShape's cached rect first — Grid::Place
                // reads GetRect() which only updates during Draw().
                ClassDiagram::ClassDiagramShapeIterator
                    iCS(pCD, &ClassDiagramShape::IsClassShape);
                while (++iCS)
                {
                    ClassShape* pCS = dynamic_cast<ClassShape*>(iCS.Get());
                    if (pCS) pCS->RecalculateRect();
                }
                Grid grid(pCD);
                grid.Place();
            }

            response["diagram"] = ToStd(pCD->GetName());
            response["member_shapes_added"] = memberShapesAdded;
            response["method_shapes_added"] = methodShapesAdded;
        });

    // ----- Group commands -------------------------------------------------
    //
    // CB has two folder-like containers in the tree:
    //   - MetaGroup — top-level grouping (under DataModel only).
    //   - ClassGroup — holds classes / diagrams; can sit on DataModel
    //                  directly or be nested inside a MetaGroup.
    // Both inherit from Group and carry a single name. The diagram-add
    // commands accept either as a parent so diagrams can be placed
    // inside the folder structure the user has built up.
    //
    // Group lookup is **by id** (returned at creation time and listed
    // via list_groups). Names aren't guaranteed unique across the model
    // — two ClassGroups under different MetaGroups can share a name —
    // so the pipe stays unambiguous by always using id.

    // Lookup a MetaGroup by id on the active DataModel.
    auto resolveMetaGroupById =
        [](DataModelDoc& dmd, UINT id, std::string& error) -> MetaGroup*
    {
        if (id == 0) return NULL;
        DataModelDocObject* pObj = dmd.FindDataModelDocObject(id);
        MetaGroup* pMG = dynamic_cast<MetaGroup*>(pObj);
        if (!pMG) error = "no MetaGroup with that id";
        return pMG;
    };

    // Lookup a ClassGroup by id, scanning both DataModel-direct and
    // those nested inside MetaGroups.
    auto resolveClassGroupById =
        [](DataModelDoc& dmd, UINT id, std::string& error) -> ClassGroup*
    {
        if (id == 0) return NULL;
        DataModelDocObject* pObj = dmd.FindDataModelDocObject(id);
        ClassGroup* pCG = dynamic_cast<ClassGroup*>(pObj);
        if (!pCG) error = "no ClassGroup with that id";
        return pCG;
    };

    // Resolves the parent for an add_class_diagram / add_sequence_diagram
    // call. Order of precedence:
    //   parent_class       (BaseClass name)         — class-scoped diagram
    //   parent_class_group_id                       — under a folder
    //   parent_meta_group_id                        — under a meta-folder
    //   (none)                                      — DataModel root
    // Returns NULL only on error (sets `error`); otherwise the resolved
    // Gti is the DataModel root or one of the above.
    auto resolveDiagramParent =
        [resolveMetaGroupById, resolveClassGroupById]
        (DataModelDoc& dmd, const json& params, std::string& error) -> Gti*
    {
        DataModel* pDataModel = dmd.GetDataModel();
        if (params.contains("parent_class"))
        {
            std::string pc = params["parent_class"].get<std::string>();
            BaseClass* pBC = dmd.FindBaseClass(CbString(pc.c_str()));
            if (!pBC) { error = "parent_class not found"; return NULL; }
            return pBC;
        }
        if (params.contains("parent_class_group_id"))
        {
            UINT id = (UINT)params["parent_class_group_id"].get<unsigned>();
            ClassGroup* pCG = resolveClassGroupById(dmd, id, error);
            if (!pCG) return NULL;
            return pCG;
        }
        if (params.contains("parent_meta_group_id"))
        {
            UINT id = (UINT)params["parent_meta_group_id"].get<unsigned>();
            MetaGroup* pMG = resolveMetaGroupById(dmd, id, error);
            if (!pMG) return NULL;
            return pMG;
        }
        return pDataModel;
    };

    // add_meta_group({name}) — top-level folder under DataModel.
    CbCommandServer::Register("add_meta_group",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            DataModel* pDataModel = dmd.GetDataModel();
            if (!pDataModel) { error = "no DataModel on document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            dmd.MarkLastUndo();
            MetaGroup* pMG = new MetaGroup(pDataModel);
            pMG->SetName(CbString(name.c_str()));
            pMG->Add();

            response["id"]   = (unsigned)pMG->GetId();
            response["name"] = name;
            response["kind"] = "meta";
        });

    // add_class_group({name, parent_meta_group_id?})
    CbCommandServer::Register("add_class_group",
        [resolveMetaGroupById]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            DataModel* pDataModel = dmd.GetDataModel();
            if (!pDataModel) { error = "no DataModel on document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            dmd.MarkLastUndo();
            ClassGroup* pCG = NULL;
            if (params.contains("parent_meta_group_id"))
            {
                UINT id = (UINT)params["parent_meta_group_id"].get<unsigned>();
                MetaGroup* pMG = resolveMetaGroupById(dmd, id, error);
                if (!pMG) return;
                pCG = new ClassGroup(pMG);
            }
            else
            {
                pCG = new ClassGroup(pDataModel);
            }
            pCG->SetName(CbString(name.c_str()));
            pCG->Add();

            response["id"]   = (unsigned)pCG->GetId();
            response["name"] = name;
            response["kind"] = "class";
            if (params.contains("parent_meta_group_id"))
                response["parent_meta_group_id"] = params["parent_meta_group_id"];
        });

    // list_groups() — every MetaGroup + ClassGroup on the model.
    CbCommandServer::Register("list_groups",
        [](const json& /*params*/, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModel* pDataModel = pDoc->GetDataModelDoc().GetDataModel();
            if (!pDataModel) { error = "no DataModel on document"; return; }

            json arr = json::array();
            DataModel::MetaGroupIterator iMG(pDataModel);
            while (++iMG)
            {
                json m;
                m["id"]   = (unsigned)iMG->GetId();
                m["name"] = ToStd(iMG->GetName());
                m["kind"] = "meta";
                arr.push_back(m);

                MetaGroup::ClassGroupIterator iNested(iMG.Get());
                while (++iNested)
                {
                    json c;
                    c["id"]   = (unsigned)iNested->GetId();
                    c["name"] = ToStd(iNested->GetName());
                    c["kind"] = "class";
                    c["parent_meta_group_id"] = (unsigned)iMG->GetId();
                    arr.push_back(c);
                }
            }
            DataModel::ClassGroupIterator iCG(pDataModel);
            while (++iCG)
            {
                json c;
                c["id"]   = (unsigned)iCG->GetId();
                c["name"] = ToStd(iCG->GetName());
                c["kind"] = "class";
                arr.push_back(c);
            }
            response = arr;
        });

    // delete_group({id}) — works for MetaGroup, ClassGroup, or
    // MemberAndMethodGroup. The group must be empty; CB's delete check
    // refuses cascade on non-empty groups.
    CbCommandServer::Register("delete_group",
        [resolveClassGroupById, resolveMetaGroupById]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();

            UINT id = (UINT)params.value("id", 0u);
            if (id == 0) { error = "missing 'id'"; return; }
            DataModelDocObject* pObj = dmd.FindDataModelDocObject(id);
            if (!pObj) { error = "no object with that id"; return; }

            if (ClassGroup* pCG = dynamic_cast<ClassGroup*>(pObj))
            {
                dmd.MarkLastUndo();
                response["id"]   = (unsigned)pCG->GetId();
                response["kind"] = "class";
                pCG->Delete();
                return;
            }
            if (MetaGroup* pMG = dynamic_cast<MetaGroup*>(pObj))
            {
                dmd.MarkLastUndo();
                response["id"]   = (unsigned)pMG->GetId();
                response["kind"] = "meta";
                pMG->Delete();
                return;
            }
            if (MemberAndMethodGroup* pMMG =
                dynamic_cast<MemberAndMethodGroup*>(pObj))
            {
                dmd.MarkLastUndo();
                response["id"]   = (unsigned)pMMG->GetId();
                response["kind"] = "member_and_method";
                pMMG->Delete();
                return;
            }
            error = "object with that id is not a group";
        });

    // ----- In-class member/method groups ----------------------------------
    //
    // A `MemberAndMethodGroup` is an optional sub-folder inside a class.
    // Each member and method either sits directly under the class or
    // belongs to one group; the group is a non-owning index that the GUI
    // displays as a folder under the class. The model still owns members
    // and methods on the class itself — the group just tags them.

    // add_member_and_method_group({class, name})
    CbCommandServer::Register("add_member_and_method_group",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = dmd.FindBaseClass(CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            dmd.MarkLastUndo();
            MemberAndMethodGroup* pMMG = new MemberAndMethodGroup(pBC);
            pMMG->SetName(CbString(name.c_str()));
            pMMG->Add();
            response["id"]    = (unsigned)pMMG->GetId();
            response["name"]  = name;
            response["class"] = cn;
        });

    // set_member_group({class, member, group_id?})
    // Pass no group_id (or 0) to remove the member from its current group
    // and let it sit directly under the class again.
    CbCommandServer::Register("set_member_group",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = dmd.FindBaseClass(CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return; }

            std::string mn = params.value("member", std::string());
            if (mn.empty()) { error = "missing 'member'"; return; }
            Member* pMember = pBC->FindMember(CbString(mn.c_str()));
            if (!pMember) { error = "member not found"; return; }

            UINT gid = (UINT)params.value("group_id", 0u);
            MemberAndMethodGroup* pTarget = NULL;
            if (gid != 0)
            {
                pTarget = dynamic_cast<MemberAndMethodGroup*>(
                    dmd.FindDataModelDocObject(gid));
                if (!pTarget)
                { error = "group_id does not name a MemberAndMethodGroup"; return; }
                if (pTarget->GetBaseClass() != pBC)
                { error = "group belongs to a different class"; return; }
            }

            dmd.MarkLastUndo();
            pMember->SaveState();
            if (MemberAndMethodGroup* pCurrent = pMember->GetMemberAndMethodGroup())
            {
                if (pCurrent == pTarget) return;  // no-op
                pCurrent->RemoveMember(pMember);
            }
            if (pTarget)
                pTarget->AddMemberLast(pMember);

            response["class"]  = cn;
            response["member"] = mn;
            response["group_id"] = (unsigned)gid;
        });

    // set_method_group({class, method | method_id, group_id?})
    // Same as set_member_group but for methods.
    CbCommandServer::Register("set_method_group",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = dmd.FindBaseClass(CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return; }

            Method* pMethod = NULL;
            if (params.contains("method_id"))
                pMethod = pBC->FindMethodWithId(
                    (UINT)params["method_id"].get<unsigned>());
            else if (params.contains("method"))
                pMethod = pBC->FindMethodWithName(
                    CbString(params["method"].get<std::string>().c_str()));
            else { error = "missing 'method' or 'method_id'"; return; }
            if (!pMethod) { error = "method not found"; return; }

            UINT gid = (UINT)params.value("group_id", 0u);
            MemberAndMethodGroup* pTarget = NULL;
            if (gid != 0)
            {
                pTarget = dynamic_cast<MemberAndMethodGroup*>(
                    dmd.FindDataModelDocObject(gid));
                if (!pTarget)
                { error = "group_id does not name a MemberAndMethodGroup"; return; }
                if (pTarget->GetBaseClass() != pBC)
                { error = "group belongs to a different class"; return; }
            }

            dmd.MarkLastUndo();
            pMethod->SaveState();
            if (MemberAndMethodGroup* pCurrent = pMethod->GetMemberAndMethodGroup())
            {
                if (pCurrent == pTarget) return;
                pCurrent->RemoveMethod(pMethod);
            }
            if (pTarget)
                pTarget->AddMethodLast(pMethod);

            response["class"]   = cn;
            response["method"]  = ToStd(pMethod->GetName());
            response["group_id"] = (unsigned)gid;
        });

    // ----- Actor + SequenceDiagram commands -------------------------------
    //
    // Actors live under DataModelDoc::GetActors() (the "Actors" tree node).
    // SequenceDiagrams are owned by a Gti parent — typically the DataModel
    // root, but also possible under a BaseClass / ClassGroup / MetaGroup
    // (mirroring the class-diagram parent flexibility). The diagram owns a
    // RootActivationShape (created in its constructor) plus zero-or-more
    // LifeLineShapes (ActorLifeLineShape or ClassLifeLineShape). Each
    // LifeLineShape hosts a chain of ChildActivationShape; nested
    // activations are wired to a sender activation via a SignalShape.
    // ChildActivationShape -> Method is a passive relation set via
    // Method::AddChildActivationShapeLast(...).
    //
    // ID-based addressing: lifelines, activations and signals are all
    // DataModelDocObject subclasses, so they're addressable via the
    // global object id (the same id used by @CODE_NNNN markers in
    // generated source). Use DataModelDoc::FindDataModelDocObject(id).

    auto resolveSequenceDiagram =
        [](DataModelDoc* pDoc, const std::string& name, std::string& error)
        -> SequenceDiagram*
    {
        if (name.empty()) { error = "missing 'name'"; return NULL; }
        CbString cname(name.c_str());
        DataModelDoc::SequenceDiagramIterator iSD(pDoc);
        while (++iSD)
        {
            if (iSD->GetName() == cname) return iSD;
        }
        error = "no such sequence diagram";
        return NULL;
    };

    auto resolveActor =
        [](DataModelDoc* pDoc, const std::string& name, std::string& error)
        -> Actor*
    {
        if (name.empty()) { error = "missing 'name'"; return NULL; }
        CbString cname(name.c_str());
        DataModelDoc::ActorIterator iActor(pDoc);
        while (++iActor)
        {
            if (iActor->GetName() == cname) return iActor;
        }
        error = "no such actor";
        return NULL;
    };

    // Lookup a LifeLineShape by id under a specific diagram.
    auto resolveLifeline =
        [](SequenceDiagram* pSD, UINT id, std::string& error) -> LifeLineShape*
    {
        if (id == 0) { error = "missing 'lifeline'"; return NULL; }
        SequenceDiagram::LifeLineShapeIterator iLL(pSD);
        while (++iLL)
        {
            if (iLL->GetId() == id) return iLL;
        }
        error = "no such lifeline on this diagram";
        return NULL;
    };

    // Lookup a ChildActivationShape by id under a specific diagram.
    // Walks every ParentActivationShape's children — flat enumeration via
    // the diagram's SequenceDiagramShape iterator with the filter.
    auto resolveActivation =
        [](SequenceDiagram* pSD, UINT id, std::string& error) -> ChildActivationShape*
    {
        if (id == 0) { error = "missing 'activation'"; return NULL; }
        SequenceDiagram::SequenceDiagramShapeIterator iSh(pSD);
        while (++iSh)
        {
            ChildActivationShape* pCh = dynamic_cast<ChildActivationShape*>(iSh.Get());
            if (pCh && pCh->GetId() == id) return pCh;
        }
        error = "no such activation on this diagram";
        return NULL;
    };

    // Lookup a SignalShape by id under a specific diagram.
    auto resolveSignal =
        [](SequenceDiagram* pSD, UINT id, std::string& error) -> SignalShape*
    {
        if (id == 0) { error = "missing 'signal'"; return NULL; }
        SequenceDiagram::SequenceDiagramShapeIterator iSh(pSD);
        while (++iSh)
        {
            SignalShape* pSig = dynamic_cast<SignalShape*>(iSh.Get());
            if (pSig && pSig->GetId() == id) return pSig;
        }
        error = "no such signal on this diagram";
        return NULL;
    };

    // Map a SeqType enum to a JSON string (and back).
    auto seqTypeToString = [](SeqType t) -> std::string
    {
        switch (t)
        {
            case SEQ_NONE:    return "none";
            case SEQ_1:       return "1";
            case SEQ_1_1_1:   return "1.1.1";
            case SEQ_a:       return "a";
            case SEQ_a_a_a:   return "a.a.a";
            case SEQ_A:       return "A";
            case SEQ_A_A_A:   return "A.A.A";
        }
        return "none";
    };
    auto seqTypeFromString = [](const std::string& s) -> SeqType
    {
        if (s == "1")       return SEQ_1;
        if (s == "1.1.1")   return SEQ_1_1_1;
        if (s == "a")       return SEQ_a;
        if (s == "a.a.a")   return SEQ_a_a_a;
        if (s == "A")       return SEQ_A;
        if (s == "A.A.A")   return SEQ_A_A_A;
        return SEQ_NONE;
    };

    auto lifelineToJson = [](LifeLineShape* pLL) -> json
    {
        json j;
        j["id"]   = (unsigned)pLL->GetId();
        j["name"] = ToStd(pLL->GetName());
        if (ActorLifeLineShape* pA = dynamic_cast<ActorLifeLineShape*>(pLL))
        {
            j["kind"] = "actor";
            if (pA->GetActor())
                j["actor"] = ToStd(pA->GetActor()->GetName());
        }
        else if (ClassLifeLineShape* pC = dynamic_cast<ClassLifeLineShape*>(pLL))
        {
            j["kind"] = "class";
            if (pC->GetBaseClass())
                j["class"] = ToStd(pC->GetBaseClass()->GetName());
        }
        CbRect r = pLL->GetRect();
        j["x"] = (int)((r.left + r.right) / 2);
        return j;
    };

    auto activationToJson = [](ChildActivationShape* pAct) -> json
    {
        json j;
        j["id"] = (unsigned)pAct->GetId();
        if (pAct->GetLifeLineShape())
            j["lifeline_id"] = (unsigned)pAct->GetLifeLineShape()->GetId();
        if (Method* pM = pAct->GetMethod())
        {
            j["method"]       = ToStd(pM->GetName());
            j["method_id"]    = (unsigned)pM->GetId();
            if (pM->GetBaseClass())
                j["method_class"] = ToStd(pM->GetBaseClass()->GetName());
        }
        if (SignalShape* pSig = pAct->GetSender())
        {
            j["signal_id"] = (unsigned)pSig->GetId();
            if (pSig->GetSender())
                j["sender_activation_id"] = (unsigned)pSig->GetSender()->GetId();
        }
        if (pAct->GetCreation())    j["creation"]    = true;
        if (pAct->GetDestruction()) j["destruction"] = true;
        j["sequence_number"]    = pAct->GetSequenceNumber();
        j["sequence_subnumber"] = pAct->GetSequenceSubNumber();
        return j;
    };

    auto sequenceDiagramToJson =
        [lifelineToJson, activationToJson, seqTypeToString]
        (SequenceDiagram* pSD) -> json
    {
        json j;
        j["name"]      = ToStd(pSD->GetName());
        j["scale"]     = (int)pSD->GetScale();
        j["numbering"] = seqTypeToString(pSD->GetNumbering());
        j["arguments"]      = (bool)pSD->GetArguments();
        j["argument_names"] = (bool)pSD->GetArgumentNames();
        j["scope"]          = (bool)pSD->GetScope();
        j["caption"]   = ToStd(pSD->GetCaption());
        j["note"]      = ToStd(pSD->GetNote());

        json lifelines = json::array();
        SequenceDiagram::LifeLineShapeIterator iLL(pSD);
        while (++iLL)
            lifelines.push_back(lifelineToJson(iLL.Get()));
        j["lifelines"] = lifelines;

        json activations = json::array();
        SequenceDiagram::SequenceDiagramShapeIterator iSh(pSD);
        while (++iSh)
        {
            ChildActivationShape* pAct = dynamic_cast<ChildActivationShape*>(iSh.Get());
            if (pAct) activations.push_back(activationToJson(pAct));
        }
        j["activations"] = activations;
        return j;
    };

    // add_actor({name, note?})
    CbCommandServer::Register("add_actor",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            CbString cname(name.c_str());
            DataModelDoc::ActorIterator iA(&dmd);
            while (++iA)
            {
                if (iA->GetName() == cname)
                { error = "an actor with that name already exists"; return; }
            }

            dmd.MarkLastUndo();
            Actor* pActor = new Actor(&dmd);
            pActor->SetName(cname);
            if (params.contains("note"))
                pActor->SetNote(CbString(params["note"].get<std::string>().c_str()));
            pActor->Add();

            response["name"] = name;
            response["id"]   = (unsigned)pActor->GetId();
        });

    // list_actors()
    CbCommandServer::Register("list_actors",
        [](const json& /*params*/, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            json arr = json::array();
            DataModelDoc::ActorIterator iA(&dmd);
            while (++iA)
                arr.push_back(ToStd(iA->GetName()));
            response = arr;
        });

    // delete_actor({name}) — fails if the actor still has lifelines on
    // any diagram (deletion would cascade and orphan diagrams).
    CbCommandServer::Register("delete_actor",
        [resolveActor](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            Actor* pActor = resolveActor(&dmd,
                params.value("name", std::string()), error);
            if (!pActor) return;
            if (pActor->GetFirstActorLifeLineShape())
            { error = "actor still has lifelines on one or more diagrams"; return; }
            dmd.MarkLastUndo();
            response["name"] = ToStd(pActor->GetName());
            pActor->Delete();
        });

    // set_actor_note({name, value})
    CbCommandServer::Register("set_actor_note",
        [resolveActor](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            Actor* pActor = resolveActor(&dmd,
                params.value("name", std::string()), error);
            if (!pActor) return;
            std::string value = params.value("value", std::string());
            dmd.MarkLastUndo();
            pActor->SaveState();
            pActor->SetNote(CbString(value.c_str()));
            response["name"]  = ToStd(pActor->GetName());
            response["note"]  = value;
        });

    // add_sequence_diagram({name, parent_class?, scale?, numbering?,
    //                       arguments?, argument_names?, scope?, caption?})
    //
    // Parent defaults to the DataModel root (top of the tree). Pass
    // parent_class to nest the diagram under a BaseClass — same place
    // the GUI's "Add Sequence Diagram" puts it when invoked on a class.
    CbCommandServer::Register("add_sequence_diagram",
        [sequenceDiagramToJson, seqTypeFromString, resolveDiagramParent]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            DataModel* pDataModel = dmd.GetDataModel();
            if (!pDataModel) { error = "no DataModel on document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            CbString cname(name.c_str());
            DataModelDoc::SequenceDiagramIterator iSD(&dmd);
            while (++iSD)
            {
                if (iSD->GetName() == cname)
                { error = "a sequence diagram with that name already exists"; return; }
            }

            Gti* pParent = resolveDiagramParent(dmd, params, error);
            if (!pParent) return;

            dmd.MarkLastUndo();
            SequenceDiagram* pSD = new SequenceDiagram(pParent);
            pSD->SetName(cname);

            if (params.contains("scale"))
                pSD->SetScale((unsigned short)params["scale"].get<int>());
            if (params.contains("numbering"))
                pSD->SetNumbering(seqTypeFromString(
                    params["numbering"].get<std::string>()));
            if (params.contains("arguments"))
                pSD->SetArguments(params["arguments"].get<bool>());
            if (params.contains("argument_names"))
                pSD->SetArgumentNames(params["argument_names"].get<bool>());
            if (params.contains("scope"))
                pSD->SetScope(params["scope"].get<bool>());
            if (params.contains("caption"))
                pSD->SetCaption(CbString(params["caption"].get<std::string>().c_str()));
            if (params.contains("note"))
                pSD->SetNote(CbString(params["note"].get<std::string>().c_str()));

            pSD->Add();
            response = sequenceDiagramToJson(pSD);
        });

    // list_sequence_diagrams() — names of every SD on the doc.
    CbCommandServer::Register("list_sequence_diagrams",
        [](const json& /*params*/, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            json arr = json::array();
            DataModelDoc::SequenceDiagramIterator iSD(&pDoc->GetDataModelDoc());
            while (++iSD)
                arr.push_back(ToStd(iSD->GetName()));
            response = arr;
        });

    // get_sequence_diagram({name}) — full record incl. lifelines and
    // activations.
    CbCommandServer::Register("get_sequence_diagram",
        [resolveSequenceDiagram, sequenceDiagramToJson]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("name", std::string()), error);
            if (!pSD) return;
            response = sequenceDiagramToJson(pSD);
        });

    // delete_sequence_diagram({name}) — fails if any view is still open.
    CbCommandServer::Register("delete_sequence_diagram",
        [resolveSequenceDiagram]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("name", std::string()), error);
            if (!pSD) return;
            if (pSD->GetSequenceDiagramViewModelCount() > 0)
            { error = "diagram still has open views; close them first"; return; }
            dmd.MarkLastUndo();
            response["name"] = ToStd(pSD->GetName());
            pSD->Delete();
        });

    // Compute the next x position for a new lifeline (right of the
    // current last one, +20). Matches the GUI's add-class flow.
    auto nextLifelineX = [](SequenceDiagram* pSD) -> int
    {
        int lastRight = 80;
        if (pSD->GetLastLifeLineShape())
            lastRight = pSD->GetLastLifeLineShape()->GetRect().right;
        return lastRight + 20;
    };

    // add_actor_lifeline({diagram, actor, x?})
    CbCommandServer::Register("add_actor_lifeline",
        [resolveSequenceDiagram, resolveActor, nextLifelineX, lifelineToJson]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
            Actor* pActor = resolveActor(&dmd,
                params.value("actor", std::string()), error);
            if (!pActor) return;

            int x = params.contains("x") ? params["x"].get<int>()
                                         : nextLifelineX(pSD);
            CbPoint point(x, 0);
            Shape::Round(point);

            dmd.MarkLastUndo();
            ActorLifeLineShape* pLL = new ActorLifeLineShape(pSD, pActor, point);
            response = lifelineToJson(pLL);
        });

    // add_class_lifeline({diagram, class, x?})
    CbCommandServer::Register("add_class_lifeline",
        [resolveSequenceDiagram, nextLifelineX, lifelineToJson]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = dmd.FindBaseClass(CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return; }

            int x = params.contains("x") ? params["x"].get<int>()
                                         : nextLifelineX(pSD);
            CbPoint point(x, 0);
            Shape::Round(point);

            dmd.MarkLastUndo();
            ClassLifeLineShape* pLL = new ClassLifeLineShape(pSD, pBC, point);
            response = lifelineToJson(pLL);
        });

    // list_lifelines({diagram})
    CbCommandServer::Register("list_lifelines",
        [resolveSequenceDiagram, lifelineToJson]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
            json arr = json::array();
            SequenceDiagram::LifeLineShapeIterator iLL(pSD);
            while (++iLL)
                arr.push_back(lifelineToJson(iLL.Get()));
            response = arr;
        });

    // delete_lifeline({diagram, id})
    CbCommandServer::Register("delete_lifeline",
        [resolveSequenceDiagram, resolveLifeline]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
            UINT id = (UINT)params.value("id", 0u);
            LifeLineShape* pLL = resolveLifeline(pSD, id, error);
            if (!pLL) return;
            dmd.MarkLastUndo();
            response["id"] = (unsigned)pLL->GetId();
            pLL->Delete();
        });

    // Helper: bind a Method to a ChildActivationShape via Method's owned-
    // active relation. Removes any previous binding first.
    auto bindActivationMethod =
        [](ChildActivationShape* pAct, Method* pMethod)
    {
        if (pAct->GetMethod())
            pAct->GetMethod()->RemoveChildActivationShape(pAct);
        if (pMethod)
            pMethod->AddChildActivationShapeLast(pAct);
    };

    // Helper: resolve a method param ({class, method} OR {method_id}).
    // Returns NULL with empty error when no params present (= unbound).
    auto resolveMethodParam =
        [](DataModelDoc& dmd, const json& params, std::string& error) -> Method*
    {
        if (params.contains("method_id"))
        {
            UINT id = (UINT)params["method_id"].get<unsigned>();
            DataModelDocObject* pObj = dmd.FindDataModelDocObject(id);
            Method* pM = dynamic_cast<Method*>(pObj);
            if (!pM) { error = "method_id does not name a Method"; return NULL; }
            return pM;
        }
        if (params.contains("method"))
        {
            std::string cn = params.value("class", std::string());
            std::string mn = params["method"].get<std::string>();
            if (cn.empty()) { error = "method without 'class'"; return NULL; }
            BaseClass* pBC = dmd.FindBaseClass(CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return NULL; }
            Method* pM = pBC->FindMethodWithName(CbString(mn.c_str()));
            if (!pM) { error = "method not found: " + cn + "::" + mn; return NULL; }
            return pM;
        }
        return NULL;
    };

    // add_root_child_activation({diagram, lifeline,
    //                            method?, class?, method_id?,
    //                            creation?, destruction?})
    //
    // Adds a top-level activation directly under the diagram's root.
    // Use this for the activation that kicks the sequence off (no
    // incoming signal).
    CbCommandServer::Register("add_root_child_activation",
        [resolveSequenceDiagram, resolveLifeline, resolveMethodParam,
         bindActivationMethod, activationToJson]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
            UINT llId = (UINT)params.value("lifeline", 0u);
            LifeLineShape* pLL = resolveLifeline(pSD, llId, error);
            if (!pLL) return;

            Method* pMethod = resolveMethodParam(dmd, params, error);
            if (!error.empty()) return;

            dmd.MarkLastUndo();
            ChildActivationShape* pAct = new ChildActivationShape(pLL);
            bindActivationMethod(pAct, pMethod);
            if (params.value("creation",    false)) pAct->SetCreation(true);
            if (params.value("destruction", false)) pAct->SetDestruction(true);

            response = activationToJson(pAct);
        });

    // add_child_activation({sender_activation, lifeline,
    //                       method?, class?, method_id?,
    //                       creation?, destruction?,
    //                       signal_name?, signal_label?,
    //                       signal_async?, signal_enable_return?,
    //                       signal_return?})
    //
    // Adds a nested activation invoked by a signal from sender_activation.
    // The constructor wires both the activation and the SignalShape
    // automatically; the latter is then customised via the optional
    // signal_* fields.
    CbCommandServer::Register("add_child_activation",
        [resolveSequenceDiagram, resolveLifeline, resolveActivation,
         resolveMethodParam, bindActivationMethod, activationToJson]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }

            UINT senderId = (UINT)params.value("sender_activation", 0u);
            ChildActivationShape* pSender = resolveActivation(pSD, senderId, error);
            if (!pSender) return;

            UINT llId = (UINT)params.value("lifeline", 0u);
            LifeLineShape* pLL = resolveLifeline(pSD, llId, error);
            if (!pLL) return;

            Method* pMethod = resolveMethodParam(dmd, params, error);
            if (!error.empty()) return;

            dmd.MarkLastUndo();
            ChildActivationShape* pAct = new ChildActivationShape(pLL, pSender);
            bindActivationMethod(pAct, pMethod);
            if (params.value("creation",    false)) pAct->SetCreation(true);
            if (params.value("destruction", false)) pAct->SetDestruction(true);

            SignalShape* pSig = pAct->GetSender();
            if (pSig)
            {
                if (params.contains("signal_name"))
                    pSig->SetName(CbString(params["signal_name"].get<std::string>().c_str()));
                if (params.contains("signal_label"))
                    pSig->SetLabel(CbString(params["signal_label"].get<std::string>().c_str()));
                if (params.contains("signal_async"))
                    pSig->SetAsync(params["signal_async"].get<bool>());
                if (params.contains("signal_enable_return"))
                    pSig->SetEnableReturn(params["signal_enable_return"].get<bool>());
                if (params.contains("signal_return"))
                    pSig->SetReturn(CbString(params["signal_return"].get<std::string>().c_str()));
            }

            response = activationToJson(pAct);
        });

    // add_signal({diagram, sender_activation, receiver_activation,
    //              signal_name?, signal_label?, signal_async?,
    //              signal_enable_return?, signal_return?})
    //
    // Wires two existing activations together: re-parents the receiver
    // into the sender's subtree (preserving its own descendants) and
    // creates a SignalShape between them. Mirrors the GUI Ctrl+click
    // "Add Message" connect-flow.
    //
    // If the receiver already had a sender (it was nested under another
    // activation), the existing SignalShape is reused — it's just moved
    // over to the new sender. Otherwise a fresh SignalShape is created.
    //
    // Refuses to create a cycle (sender mustn't be in the receiver's
    // subtree) and is a no-op when the sender is already the receiver's
    // parent.
    CbCommandServer::Register("add_signal",
        [resolveSequenceDiagram, resolveActivation, activationToJson]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }

            UINT sId = (UINT)params.value("sender_activation",   0u);
            UINT rId = (UINT)params.value("receiver_activation", 0u);
            ChildActivationShape* pSender =
                resolveActivation(pSD, sId, error);
            if (!pSender) return;
            ChildActivationShape* pReceiver =
                resolveActivation(pSD, rId, error);
            if (!pReceiver) return;

            if (pSender == pReceiver)
            { error = "sender and receiver are the same activation"; return; }
            if (pReceiver->GetParentActivationShape() == pSender)
            { error = "receiver is already a direct child of sender"; return; }
            if (pSender->IsDirectOrIndirectChild(pReceiver))
            { error = "would create a cycle (sender is in receiver's subtree)"; return; }

            dmd.MarkLastUndo();
            pReceiver->SaveState(1);
            pSender->MoveChildActivationShapeLast(pReceiver);

            SignalShape* pSig = pReceiver->GetSender();
            if (pSig)
            {
                // Pre-existing signal — move it to the new sender.
                pSig->SaveState(1);
                pSender->MoveReceiverLast(pSig);
            }
            else
            {
                pSig = new SignalShape(pReceiver, pSender);
            }

            // Apply optional signal-* fields, same surface as add_child_activation.
            if (pSig)
            {
                if (params.contains("signal_name"))
                    pSig->SetName(CbString(params["signal_name"].get<std::string>().c_str()));
                if (params.contains("signal_label"))
                    pSig->SetLabel(CbString(params["signal_label"].get<std::string>().c_str()));
                if (params.contains("signal_async"))
                    pSig->SetAsync(params["signal_async"].get<bool>());
                if (params.contains("signal_enable_return"))
                    pSig->SetEnableReturn(params["signal_enable_return"].get<bool>());
                if (params.contains("signal_return"))
                    pSig->SetReturn(CbString(params["signal_return"].get<std::string>().c_str()));
            }

            response = activationToJson(pReceiver);
        });

    // set_activation_method({diagram, activation, method?, class?, method_id?})
    // Pass no method-fields to clear the binding.
    CbCommandServer::Register("set_activation_method",
        [resolveSequenceDiagram, resolveActivation, resolveMethodParam,
         bindActivationMethod, activationToJson]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
            UINT id = (UINT)params.value("activation", 0u);
            ChildActivationShape* pAct = resolveActivation(pSD, id, error);
            if (!pAct) return;

            Method* pMethod = resolveMethodParam(dmd, params, error);
            if (!error.empty()) return;

            dmd.MarkLastUndo();
            pAct->SaveState();
            bindActivationMethod(pAct, pMethod);
            response = activationToJson(pAct);
        });

    // delete_activation({diagram, activation}) — cascades to children.
    CbCommandServer::Register("delete_activation",
        [resolveSequenceDiagram, resolveActivation]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
            UINT id = (UINT)params.value("activation", 0u);
            ChildActivationShape* pAct = resolveActivation(pSD, id, error);
            if (!pAct) return;
            dmd.MarkLastUndo();
            response["id"] = (unsigned)pAct->GetId();
            pAct->Delete();
        });

    // ----- Signal setters -------------------------------------------------

    // set_signal_name / label / async / enable_return / return / scope /
    // arguments / argument_names / note — all addressed by signal id.
    auto registerSignalSetter =
        [resolveSequenceDiagram, resolveSignal](
            const std::string& cmd,
            std::function<void(SignalShape*, const json&)> apply)
    {
        CbCommandServer::Register(cmd,
            [resolveSequenceDiagram, resolveSignal, apply]
            (const json& params, json& response, std::string& error)
            {
                CClassBuilderDoc* pDoc = GetActiveDoc();
                if (!pDoc) { error = "no active document"; return; }
                DataModelDoc& dmd = pDoc->GetDataModelDoc();
                SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                    params.value("diagram", std::string()), error);
                if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
                UINT id = (UINT)params.value("signal", 0u);
                SignalShape* pSig = resolveSignal(pSD, id, error);
                if (!pSig) return;
                if (!params.contains("value"))
                { error = "missing 'value'"; return; }
                dmd.MarkLastUndo();
                pSig->SaveState();
                apply(pSig, params["value"]);
                response["id"] = (unsigned)pSig->GetId();
            });
    };

    registerSignalSetter("set_signal_name",
        [](SignalShape* p, const json& v)
        { p->SetName(CbString(v.get<std::string>().c_str())); });
    registerSignalSetter("set_signal_label",
        [](SignalShape* p, const json& v)
        { p->SetLabel(CbString(v.get<std::string>().c_str())); });
    registerSignalSetter("set_signal_return",
        [](SignalShape* p, const json& v)
        { p->SetReturn(CbString(v.get<std::string>().c_str())); });
    registerSignalSetter("set_signal_note",
        [](SignalShape* p, const json& v)
        { p->SetNote(CbString(v.get<std::string>().c_str())); });
    registerSignalSetter("set_signal_async",
        [](SignalShape* p, const json& v)
        { p->SetAsync(v.get<bool>()); });
    registerSignalSetter("set_signal_enable_return",
        [](SignalShape* p, const json& v)
        { p->SetEnableReturn(v.get<bool>()); });
    registerSignalSetter("set_signal_scope",
        [](SignalShape* p, const json& v)
        { p->SetScope(v.get<bool>()); });
    registerSignalSetter("set_signal_arguments",
        [](SignalShape* p, const json& v)
        { p->SetArguments(v.get<bool>()); });
    registerSignalSetter("set_signal_argument_names",
        [](SignalShape* p, const json& v)
        { p->SetArgumentNames(v.get<bool>()); });

    // ----- Layout commands ------------------------------------------------

    // optimize_placement({diagram}) — barycenter reorder + activation
    // offsets + horizontal packing + leftmost snap. See
    // SequenceDiagram::OptimizePlacement.
    CbCommandServer::Register("optimize_placement",
        [resolveSequenceDiagram]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
            pSD->OptimizePlacement();
            pSD->UpdateSequenceDiagramViews();
            dmd.MarkLastUndo();
            response["diagram"] = ToStd(pSD->GetName());
        });

    // space_lifelines({diagram}) — horizontal-only layout, preserving
    // the current lifeline order.
    CbCommandServer::Register("space_lifelines",
        [resolveSequenceDiagram]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
            pSD->SpaceLifeLines();
            pSD->UpdateSequenceDiagramViews();
            dmd.MarkLastUndo();
            response["diagram"] = ToStd(pSD->GetName());
        });

    // reset_activation_offsets({diagram}) — clear manual vertical tweaks
    // on every activation.
    CbCommandServer::Register("reset_activation_offsets",
        [resolveSequenceDiagram]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }
            pSD->ResetActivationOffsets();
            pSD->UpdateSequenceDiagramViews();
            dmd.MarkLastUndo();
            response["diagram"] = ToStd(pSD->GetName());
        });

    // ----- Note commands --------------------------------------------------
    //
    // A NoteShape (on a ClassDiagram) or SDNoteShape (on a SequenceDiagram)
    // is a free-floating text block with zero-or-more connection points.
    // A "connection" is just a NoteShapePoint with coordinates that fall
    // inside another shape's rect — when the shape moves, the diagram's
    // MoveNoteShapePoints(rect, offset) hook shifts every contained
    // NoteShapePoint by the same offset, so the connection line tracks
    // automatically. Nothing in the model "remembers" what a point is
    // connected to; it's purely positional.

    // add_class_diagram_note({diagram, text, x?, y?, font_height?})
    CbCommandServer::Register("add_class_diagram_note",
        [resolveDiagram]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("diagram", std::string()), error);
            if (!pCD) { if (error.empty()) error = "missing 'diagram'"; return; }

            std::string text = params.value("text", std::string());
            int x = params.value("x", 100);
            int y = params.value("y", 100);
            CbPoint point(x, -y);  // CD y-axis is inverted vs screen
            Shape::Round(point);

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            NoteShape* pNote = new NoteShape(pCD, point);
            pNote->SetNote(CbString(text.c_str()));
            if (params.contains("font_height"))
                pNote->SetFontHeight(params["font_height"].get<int>());

            response["id"] = (unsigned)pNote->GetId();
        });

    // connect_class_diagram_note({note, class}) — adds a NoteShapePoint at
    // the class shape's centre, so the connection line tracks the class
    // as it moves. The class must already be on the same diagram as the
    // note.
    CbCommandServer::Register("connect_class_diagram_note",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            DataModelDoc& dmd = *pDataModel->GetDataModelDoc();

            UINT noteId = (UINT)params.value("note", 0u);
            NoteShape* pNote = dynamic_cast<NoteShape*>(
                dmd.FindDataModelDocObject(noteId));
            if (!pNote) { error = "note id does not name a NoteShape"; return; }

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = dmd.FindBaseClass(CbString(cn.c_str()));
            if (!pBC) { error = "class not found: " + cn; return; }

            ClassDiagram* pCD = pNote->GetClassDiagram();
            ClassShape* pCS = pBC->FindClassShape(pCD);
            if (!pCS) { error = "class is not on the note's diagram"; return; }

            CbRect r = pCS->GetRect();
            CbPoint centre((r.left + r.right) / 2, (r.top + r.bottom) / 2);

            dmd.MarkLastUndo();
            NoteShapePoint* pPt = new NoteShapePoint(pNote, centre);
            response["id"]      = (unsigned)pPt->GetId();
            response["note_id"] = (unsigned)pNote->GetId();
            response["class"]   = cn;
        });

    // add_sequence_diagram_note({diagram, text, x?, y?, font_height?})
    CbCommandServer::Register("add_sequence_diagram_note",
        [resolveSequenceDiagram]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("diagram", std::string()), error);
            if (!pSD) { if (error.empty()) error = "missing 'diagram'"; return; }

            std::string text = params.value("text", std::string());
            int x = params.value("x", 100);
            int y = params.value("y", 100);
            CbPoint point(x, -y);
            Shape::Round(point);

            dmd.MarkLastUndo();
            SDNoteShape* pNote = new SDNoteShape(pSD, point);
            pNote->SetNote(CbString(text.c_str()));
            if (params.contains("font_height"))
                pNote->SetFontHeight(params["font_height"].get<int>());

            response["id"] = (unsigned)pNote->GetId();
        });

    // connect_sequence_diagram_note({note, lifeline}) — adds an
    // SDNoteShapePoint at the lifeline shape's centre.
    CbCommandServer::Register("connect_sequence_diagram_note",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();

            UINT noteId = (UINT)params.value("note", 0u);
            SDNoteShape* pNote = dynamic_cast<SDNoteShape*>(
                dmd.FindDataModelDocObject(noteId));
            if (!pNote) { error = "note id does not name an SDNoteShape"; return; }

            UINT llId = (UINT)params.value("lifeline", 0u);
            LifeLineShape* pLL = dynamic_cast<LifeLineShape*>(
                dmd.FindDataModelDocObject(llId));
            if (!pLL) { error = "lifeline id does not name a LifeLineShape"; return; }
            if (pLL->GetSequenceDiagram() != pNote->GetSequenceDiagram())
            { error = "lifeline is not on the note's diagram"; return; }

            CbRect r = pLL->GetRect();
            CbPoint centre((r.left + r.right) / 2, (r.top + r.bottom) / 2);

            dmd.MarkLastUndo();
            SDNoteShapePoint* pPt = new SDNoteShapePoint(pNote, centre);
            response["id"]      = (unsigned)pPt->GetId();
            response["note_id"] = (unsigned)pNote->GetId();
            response["lifeline_id"] = (unsigned)pLL->GetId();
        });

    // ----- Move commands (reparent existing objects) ----------------------
    //
    // Pipe-equivalent of GUI drag-and-drop between allowed parents.
    //
    // Diagrams (ClassDiagram / SequenceDiagram) reparent via Gti's tree
    // (AddChildLast). The same call the GUI uses in
    // SequenceDiagram::Drop / ClassDiagram::Drop — Gti's AddChild*
    // handles detaching from the old parent automatically.
    //
    // Classes reparent via the ClassGroup membership relation, which is
    // independent of class ownership (always DataModel). A class is
    // either in zero or one ClassGroup; we remove from the current group
    // (if any) then add to the target.
    //
    // ClassGroups reparent between DataModel root and a MetaGroup; this
    // uses the two passive relations on ClassGroup (one to each owner
    // class) — remove from the current owner, add to the new one.

    // move_class_diagram({name, parent_class?, parent_class_group_id?,
    //                     parent_meta_group_id?})
    CbCommandServer::Register("move_class_diagram",
        [resolveDiagram, resolveDiagramParent]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            DataModelDoc& dmd = *pDataModel->GetDataModelDoc();
            ClassDiagram* pCD = resolveDiagram(pDataModel,
                params.value("name", std::string()), error);
            if (!pCD) return;

            Gti* pNewParent = resolveDiagramParent(dmd, params, error);
            if (!pNewParent) return;

            dmd.MarkLastUndo();
            pCD->SaveState(1);
            pNewParent->AddChildLast(pCD);
            response["name"] = ToStd(pCD->GetName());
        });

    // move_sequence_diagram({name, parent_class?, parent_class_group_id?,
    //                        parent_meta_group_id?})
    CbCommandServer::Register("move_sequence_diagram",
        [resolveSequenceDiagram, resolveDiagramParent]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            SequenceDiagram* pSD = resolveSequenceDiagram(&dmd,
                params.value("name", std::string()), error);
            if (!pSD) return;

            Gti* pNewParent = resolveDiagramParent(dmd, params, error);
            if (!pNewParent) return;

            dmd.MarkLastUndo();
            pSD->SaveState(1);
            pNewParent->AddChildLast(pSD);
            response["name"] = ToStd(pSD->GetName());
        });

    // move_class({class, parent_class_group_id?})
    //   Sets the class's ClassGroup membership. Pass no group (or 0) to
    //   remove it from its current group, putting it back at the model
    //   root level. Class ownership (DataModel) is unchanged either way.
    CbCommandServer::Register("move_class",
        [resolveClassGroupById]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            DataModelDoc& dmd = *pDataModel->GetDataModelDoc();

            std::string cn = params.value("class", std::string());
            if (cn.empty()) { error = "missing 'class'"; return; }
            Class* pClass = pDataModel->FindClass(CbString(cn.c_str()));
            if (!pClass) { error = "class not found: " + cn; return; }

            UINT gid = (UINT)params.value("parent_class_group_id", 0u);
            ClassGroup* pTarget = NULL;
            if (gid != 0)
            {
                pTarget = resolveClassGroupById(dmd, gid, error);
                if (!pTarget) return;
            }

            ClassGroup* pCurrent = pClass->GetClassGroup();
            if (pCurrent == pTarget) { response["class"] = cn; return; }

            dmd.MarkLastUndo();
            pClass->SaveState(1);
            if (pCurrent) pCurrent->RemoveClass(pClass);
            if (pTarget)  pTarget->AddClassLast(pClass);

            response["class"] = cn;
            if (pTarget)
                response["parent_class_group_id"] = (unsigned)pTarget->GetId();
        });

    // move_class_group({id, parent_meta_group_id?})
    //   Moves a ClassGroup between DataModel root and a MetaGroup.
    CbCommandServer::Register("move_class_group",
        [resolveClassGroupById, resolveMetaGroupById]
        (const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            DataModelDoc& dmd = *pDataModel->GetDataModelDoc();

            UINT id = (UINT)params.value("id", 0u);
            ClassGroup* pCG = resolveClassGroupById(dmd, id, error);
            if (!pCG) { if (error.empty()) error = "missing 'id'"; return; }

            UINT mgId = (UINT)params.value("parent_meta_group_id", 0u);
            MetaGroup* pTarget = NULL;
            if (mgId != 0)
            {
                pTarget = resolveMetaGroupById(dmd, mgId, error);
                if (!pTarget) return;
            }

            dmd.MarkLastUndo();
            pCG->SaveState(1);
            // Detach from whichever owner currently holds it.
            if (pCG->GetMetaGroup())
                pCG->GetMetaGroup()->RemoveClassGroup(pCG);
            if (pCG->GetDataModel())
                pCG->GetDataModel()->RemoveClassGroup(pCG);
            // Reattach to the target — MetaGroup if provided, else root.
            if (pTarget)
                pTarget->AddClassGroupLast(pCG);
            else
                pDataModel->AddClassGroupLast(pCG);
            // Re-add to the Gti tree under the new parent.
            (pTarget ? (Gti*)pTarget : (Gti*)pDataModel)->AddChildLast(pCG);

            response["id"] = (unsigned)pCG->GetId();
            if (pTarget)
                response["parent_meta_group_id"] = (unsigned)pTarget->GetId();
        });

    // ----- Call-trace command ---------------------------------------------
    //
    // add_call_trace({name, start_class, start_method | start_method_id,
    //                 actor?, actor_id?,
    //                 max_depth?, parent_class?,
    //                 scale?, numbering?, arguments?, argument_names?,
    //                 scope?, caption?, note?})
    //
    // Bootstraps a SequenceDiagram from a starting method by text-scanning
    // its body for `Identifier(` call patterns, looking each name up in the
    // model, and recursing depth-first. Best-effort:
    //
    //  - Only unambiguous name matches (exactly one class has a method
    //    with that name) get bound; ambiguous / unknown names are skipped.
    //  - Visited-set on the current recursion path breaks direct/indirect
    //    cycles; the same method may still appear in sibling branches.
    //  - max_depth (default 3) bounds the recursion.
    //  - One lifeline is created per unique receiving class, on first
    //    encounter (left-to-right). optimize_placement is invoked at the
    //    end to give a clean layout.
    //
    // Limitations: no type analysis. Iterator-deref / pointer-deref calls
    // like `iAct->Compare(...)` resolve only when the method name happens
    // to be unique across the model. Stdlib / template / macro calls drop
    // out automatically (unknown name). Treat the output as a starting
    // sketch — expect to prune false matches and add missing ones via the
    // ordinary `add_child_activation` / `delete_activation` commands.
    CbCommandServer::Register("add_call_trace",
        [resolveSequenceDiagram, sequenceDiagramToJson, seqTypeFromString,
         bindActivationMethod, resolveMethodParam, nextLifelineX,
         resolveDiagramParent, resolveActor]
        (const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            DataModel* pDataModel = dmd.GetDataModel();
            if (!pDataModel) { error = "no DataModel on document"; return; }

            // --- Resolve start method --------------------------------------
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            json mp = params;
            mp["method"] = params.value("start_method", std::string());
            mp["class"]  = params.value("start_class",  std::string());
            if (params.contains("start_method_id"))
                mp["method_id"] = params["start_method_id"];
            Method* pStart = resolveMethodParam(dmd, mp, error);
            if (!pStart || !error.empty())
            {
                if (error.empty()) error = "missing 'start_method' / 'start_class'";
                return;
            }
            BaseClass* pStartClass = pStart->GetBaseClass();
            if (!pStartClass) { error = "start method has no owning class"; return; }

            // Reject duplicate diagram name.
            CbString cname(name.c_str());
            DataModelDoc::SequenceDiagramIterator iSD(&dmd);
            while (++iSD)
            {
                if (iSD->GetName() == cname)
                { error = "a sequence diagram with that name already exists"; return; }
            }

            int maxDepth = params.value("max_depth", 3);
            if (maxDepth < 1) maxDepth = 1;

            // --- User-supplied skip filters -------------------------------
            //
            // skip_methods : array of exact method-name strings to ignore
            //                during the call scan (e.g. trivial setters).
            // skip_pattern : single regex (ECMAScript syntax) matched
            //                against the method name; matching names are
            //                skipped. Useful for "all the offset
            //                getters/setters" with one rule.
            // Both filters apply BEFORE method lookup, so the names never
            // become activations (and they don't count toward
            // `ambiguous_skipped` — they're tallied separately as
            // `user_skipped`).
            std::set<std::string> skipSet;
            if (params.contains("skip_methods") && params["skip_methods"].is_array())
            {
                for (auto& v : params["skip_methods"])
                {
                    if (v.is_string()) skipSet.insert(v.get<std::string>());
                }
            }
            bool haveSkipRegex = false;
            std::regex skipRegex;
            if (params.contains("skip_pattern"))
            {
                try
                {
                    skipRegex = std::regex(params["skip_pattern"].get<std::string>());
                    haveSkipRegex = true;
                }
                catch (const std::regex_error& e)
                {
                    error = std::string("invalid 'skip_pattern' regex: ") + e.what();
                    return;
                }
            }

            // --- Pure-text call extraction --------------------------------
            //
            // Walk the source character-by-character. Identifier =
            // [A-Za-z_]\w*. A call is an identifier whose next non-space
            // char is '('. Strings and comments are skipped because they
            // can contain misleading text like "func(" inside a literal.
            // Cheap C/C++ keyword filter rejects control-flow and casts
            // that also look like calls (e.g. `if(`, `dynamic_cast(`).
            auto findCallsInBody = [](const CbString& body) -> std::vector<std::pair<std::string, bool>>
            {
                static const std::set<std::string> kSkip =
                {
                    "if","else","while","for","do","switch","case","default",
                    "return","break","continue","sizeof","new","delete","typeid",
                    "throw","try","catch","true","false","NULL","nullptr","this",
                    "static_cast","dynamic_cast","reinterpret_cast","const_cast",
                    "operator","auto","void","int","char","short","long","float",
                    "double","bool","unsigned","signed","class","struct","union",
                    "enum","typedef","template","typename","namespace","using",
                    "public","protected","private","virtual","explicit","inline",
                    "static","const","volatile","mutable","friend","extern",
                    "register","goto",
                    // Frequent MFC/CB types & macros that aren't call targets.
                    "CRect","CPoint","CSize","CString","CArray","CDC","CFont",
                    "BOOL","TRUE","FALSE","UINT","DWORD","WORD","BYTE",
                    "CbColorRef","RGB","HRESULT","LRESULT","WPARAM","LPARAM",
                    "assert","ASSERT","_T","_tcscmp","afx_msg",
                };

                // Output: (name, in_loop). When the same call name appears
                // multiple times (dedup'd), the entry is marked in_loop=true
                // if ANY occurrence is inside a loop body — conservative
                // because the sequence diagram only shows one activation per
                // unique name and we want the loop marker to surface even
                // if just one occurrence sits in a loop.
                std::vector<std::pair<std::string, bool>> out;
                std::map<std::string, size_t> nameToIdx;
                std::string text(ToStd(body));

                // Brace stack: for each `{` we push true if it's the body of
                // a loop (preceded by `for`/`while`/`do`), false otherwise
                // (function-call body, if/else, switch, etc.). Loop depth =
                // count of true entries currently on the stack.
                std::vector<bool> braceStack;
                bool pendingLoop = false;
                int loopDepth = 0;

                size_t n = text.size();
                size_t i = 0;
                while (i < n)
                {
                    char c = text[i];

                    // Skip line comment to end of line.
                    if (c == '/' && i + 1 < n && text[i+1] == '/')
                    {
                        while (i < n && text[i] != '\n') ++i;
                        continue;
                    }
                    // Skip block comment.
                    if (c == '/' && i + 1 < n && text[i+1] == '*')
                    {
                        i += 2;
                        while (i + 1 < n && !(text[i] == '*' && text[i+1] == '/'))
                            ++i;
                        if (i + 1 < n) i += 2;
                        continue;
                    }
                    // Skip string literal.
                    if (c == '"')
                    {
                        ++i;
                        while (i < n && text[i] != '"')
                        {
                            if (text[i] == '\\' && i + 1 < n) ++i;
                            ++i;
                        }
                        if (i < n) ++i;
                        continue;
                    }
                    // Skip char literal.
                    if (c == '\'')
                    {
                        ++i;
                        while (i < n && text[i] != '\'')
                        {
                            if (text[i] == '\\' && i + 1 < n) ++i;
                            ++i;
                        }
                        if (i < n) ++i;
                        continue;
                    }
                    // Brace tracking — the loop-detection mechanism.
                    if (c == '{')
                    {
                        braceStack.push_back(pendingLoop);
                        if (pendingLoop) ++loopDepth;
                        pendingLoop = false;
                        ++i;
                        continue;
                    }
                    if (c == '}')
                    {
                        if (!braceStack.empty())
                        {
                            if (braceStack.back()) --loopDepth;
                            braceStack.pop_back();
                        }
                        ++i;
                        continue;
                    }
                    // Identifier?
                    if (isalpha((unsigned char)c) || c == '_')
                    {
                        size_t start = i;
                        while (i < n &&
                               (isalnum((unsigned char)text[i]) || text[i] == '_'))
                            ++i;
                        std::string ident = text.substr(start, i - start);

                        // Loop-opening keywords arm `pendingLoop` so that
                        // the next `{` is recorded as a loop body. (The
                        // single-statement form `for(...) foo();` is not
                        // recognised — needs braces. Good enough for the
                        // CB codebase's usage.)
                        if (ident == "for" || ident == "while" || ident == "do")
                        {
                            pendingLoop = true;
                            continue;
                        }

                        // Look ahead for '(' through whitespace.
                        size_t j = i;
                        while (j < n && isspace((unsigned char)text[j])) ++j;
                        if (j < n && text[j] == '(' && kSkip.count(ident) == 0)
                        {
                            bool inLoop = (loopDepth > 0);
                            auto it = nameToIdx.find(ident);
                            if (it == nameToIdx.end())
                            {
                                nameToIdx[ident] = out.size();
                                out.push_back({ident, inLoop});
                            }
                            else if (inLoop && !out[it->second].second)
                            {
                                // Upgrade existing entry to in_loop=true so
                                // the "*" clause surfaces.
                                out[it->second].second = true;
                            }
                        }
                        continue;
                    }
                    ++i;
                }
                return out;
            };

            // --- Method lookup --------------------------------------------
            //
            // Returns the unique Method named `name` in the model, OR NULL
            // when 0 or >1 classes match. Looks across Class and ExternClass
            // (any BaseClass that owns methods).
            auto findUniqueMethod =
                [pDataModel](const std::string& mname) -> Method*
            {
                CbString cmname(mname.c_str());
                Method* hit = NULL;
                int count = 0;
                DataModel::ClassIterator iC(pDataModel);
                while (++iC)
                {
                    if (Method* pM = iC->FindMethodWithName(cmname))
                    {
                        hit = pM;
                        if (++count > 1) return NULL;
                    }
                }
                return hit;
            };

            // --- Lifeline cache --------------------------------------------
            std::map<UINT, LifeLineShape*> llByClass;
            auto getOrCreateLifeline =
                [&llByClass, nextLifelineX]
                (SequenceDiagram* pSD, BaseClass* pBC) -> LifeLineShape*
            {
                auto it = llByClass.find(pBC->GetId());
                if (it != llByClass.end()) return it->second;
                CbPoint p(nextLifelineX(pSD), 0);
                Shape::Round(p);
                LifeLineShape* pLL = new ClassLifeLineShape(pSD, pBC, p);
                llByClass[pBC->GetId()] = pLL;
                return pLL;
            };

            Gti* pParent = resolveDiagramParent(dmd, params, error);
            if (!pParent) return;

            dmd.MarkLastUndo();

            // --- Create the diagram + start activation --------------------
            SequenceDiagram* pSD = new SequenceDiagram(pParent);
            pSD->SetName(cname);
            if (params.contains("scale"))
                pSD->SetScale((unsigned short)params["scale"].get<int>());
            if (params.contains("numbering"))
                pSD->SetNumbering(seqTypeFromString(
                    params["numbering"].get<std::string>()));
            if (params.contains("arguments"))
                pSD->SetArguments(params["arguments"].get<bool>());
            if (params.contains("argument_names"))
                pSD->SetArgumentNames(params["argument_names"].get<bool>());
            if (params.contains("scope"))
                pSD->SetScope(params["scope"].get<bool>());
            if (params.contains("caption"))
                pSD->SetCaption(CbString(params["caption"].get<std::string>().c_str()));
            if (params.contains("note"))
                pSD->SetNote(CbString(params["note"].get<std::string>().c_str()));
            pSD->Add();

            // --- Optional actor wiring ------------------------------------
            //
            // When `actor` (name) or `actor_id` (model id) is provided, the
            // actor becomes the kick-off lifeline on the left, with a loose
            // root activation. The start method's activation hangs *below*
            // the actor activation, so a SignalShape from actor → start is
            // auto-created. The anchor preference in OptimizePlacement then
            // pins the actor to the leftmost position (it's the first
            // child of root).
            Actor* pActor = NULL;
            if (params.contains("actor_id"))
            {
                UINT aid = (UINT)params["actor_id"].get<unsigned>();
                pActor = dynamic_cast<Actor*>(dmd.FindDataModelDocObject(aid));
                if (!pActor)
                { error = "actor_id does not name an Actor"; return; }
            }
            else if (params.contains("actor"))
            {
                pActor = resolveActor(&dmd,
                    params["actor"].get<std::string>(), error);
                if (!pActor) return;
            }

            ChildActivationShape* pRoot = NULL;
            if (pActor)
            {
                // Actor lifeline goes leftmost (it's the first lifeline
                // added — getOrCreateLifeline's class cache is separate;
                // we add the actor LL manually to control ordering).
                CbPoint actorPt(nextLifelineX(pSD), 0);
                Shape::Round(actorPt);
                ActorLifeLineShape* pActorLL =
                    new ActorLifeLineShape(pSD, pActor, actorPt);

                // Loose root activation on the actor (no method binding —
                // the actor *is* the caller).
                ChildActivationShape* pActorAct = new ChildActivationShape(pActorLL);

                // Start activation on the start class, sender = actor act.
                // The two-arg constructor wires the SignalShape too.
                LifeLineShape* pStartLL = getOrCreateLifeline(pSD, pStartClass);
                pRoot = new ChildActivationShape(pStartLL, pActorAct);
                bindActivationMethod(pRoot, pStart);
            }
            else
            {
                // No actor — original behaviour: root activation directly
                // on the start class.
                LifeLineShape* pStartLL = getOrCreateLifeline(pSD, pStartClass);
                pRoot = new ChildActivationShape(pStartLL);
                bindActivationMethod(pRoot, pStart);
            }

            // --- Recursion -------------------------------------------------
            //
            // visited holds the method ids on the *current* path so a method
            // can still appear under a different parent branch. stats track
            // how the scan went (useful in the response so the caller can
            // sanity-check).
            std::set<UINT> visited;
            visited.insert(pStart->GetId());

            int activationsCreated = 1;
            int ambiguousSkipped   = 0;
            int unknownSkipped     = 0;
            int userSkipped        = 0;

            std::function<void(ChildActivationShape*, Method*, int)> recurse;
            recurse = [&](ChildActivationShape* pParentAct,
                          Method* pParentMethod, int depth)
            {
                if (depth >= maxDepth) return;
                std::vector<std::pair<std::string, bool>> calls =
                    findCallsInBody(pParentMethod->GetCode());

                for (size_t k = 0; k < calls.size(); ++k)
                {
                    const std::string& cname2 = calls[k].first;
                    bool inLoop               = calls[k].second;

                    // User-supplied filters: exact-name set or regex.
                    if (skipSet.count(cname2) ||
                        (haveSkipRegex && std::regex_search(cname2, skipRegex)))
                    { ++userSkipped; continue; }

                    Method* pCalled = findUniqueMethod(cname2);
                    if (!pCalled) { ++ambiguousSkipped; continue; }
                    if (visited.count(pCalled->GetId())) continue;
                    BaseClass* pCallClass = pCalled->GetBaseClass();
                    if (!pCallClass) { ++unknownSkipped; continue; }

                    LifeLineShape* pLL = getOrCreateLifeline(pSD, pCallClass);
                    ChildActivationShape* pAct =
                        new ChildActivationShape(pLL, pParentAct);
                    bindActivationMethod(pAct, pCalled);
                    // Mark the incoming signal as called-in-a-loop with the
                    // CB convention "*" clause, per the user pattern.
                    if (inLoop)
                    {
                        if (SignalShape* pSig = pAct->GetSender())
                            pSig->SetClause("*");
                    }
                    ++activationsCreated;

                    visited.insert(pCalled->GetId());
                    recurse(pAct, pCalled, depth + 1);
                    visited.erase(pCalled->GetId());
                }
            };
            recurse(pRoot, pStart, 1);

            // --- Layout + reply --------------------------------------------
            pSD->OptimizePlacement();
            pSD->UpdateSequenceDiagramViews();

            response = sequenceDiagramToJson(pSD);
            response["trace_stats"] = {
                { "activations_created", activationsCreated },
                { "ambiguous_skipped",   ambiguousSkipped   },
                { "user_skipped",        userSkipped        },
                { "unknown_class_skipped", unknownSkipped   },
                { "max_depth",           maxDepth           }
            };
        });

    // ----- Find-method commands -------------------------------------------
    //
    // FindMethod is a `FromRelationMethod` attached to the from-side of a
    // multi relation. It iterates the relation and returns the first
    // matching element. Each argument carries a `_path` (e.g. "->_id" or
    // "->GetParent()->_name") that becomes the comparison expression in
    // the generated body. InitCode() picks the impl-specific fast path
    // (avl/value-tree) when an argument's MemberArgument matches the
    // relation's tree-key member; otherwise it generates an iterate-loop.

    // add_find_method({class, from_name, to_name?, name?, access?,
    //                  reverse?, next?, args})
    //
    // `args` is required and non-empty. Each entry:
    //   {path, member?: {class, name}, nav_class?, arg_name?}
    // - exactly one of `member` or `nav_class` must be present
    // - `member` → MemberArgument; type and name auto-derived from the Member
    // - `nav_class` → plain Argument typed as `<nav_class>*`; default
    //   name is "p<nav_class>" unless `arg_name` overrides
    // - `path` is required; it's the comparison expression as the codegen
    //   will emit it (must start with "->")
    CbCommandServer::Register("add_find_method",
        [resolveRelation](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            Class* pClass = pDataModel->FindClass(CbString(className.c_str()));
            if (!pClass) { error = "no such class"; return; }
            Relation* pRel = resolveRelation(pClass, params, error);
            if (!pRel) return;

            if (!pRel->GetMulti())
            { error = "find methods only apply to multi relations"; return; }

            if (!params.contains("args") || !params["args"].is_array() ||
                params["args"].empty())
            { error = "'args' must be a non-empty array"; return; }

            bool reverse = params.value("reverse", false);
            bool nextFlag = params.value("next", false);

            // Pre-validate every arg before touching the model so we don't
            // leave a half-built FindMethod behind on bad input.
            const json& argsJ = params["args"];
            for (size_t i = 0; i < argsJ.size(); ++i)
            {
                const json& a = argsJ[i];
                if (!a.contains("path"))
                { error = "argument missing 'path'"; return; }
                bool hasMember = a.contains("member");
                bool hasNav    = a.contains("nav_class");
                if (hasMember == hasNav)
                { error = "each argument needs exactly one of 'member' or 'nav_class'"; return; }

                if (hasMember)
                {
                    std::string mc = a["member"].value("class", std::string());
                    std::string mn = a["member"].value("name",  std::string());
                    if (mc.empty() || mn.empty())
                    { error = "member needs both 'class' and 'name'"; return; }
                    BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                                            CbString(mc.c_str()));
                    if (!pBC || !pBC->FindMember(CbString(mn.c_str())))
                    { error = "member not found: " + mc + "." + mn; return; }
                }
                else
                {
                    std::string nc = a["nav_class"].get<std::string>();
                    BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                                            CbString(nc.c_str()));
                    if (!pBC)
                    { error = "nav_class not found: " + nc; return; }
                }
            }

            pDataModel->GetDataModelDoc()->MarkLastUndo();

            FindMethod* pFM = new FindMethod(pRel->GetFromRelation(), reverse);
            if (params.contains("name"))
                pFM->SetName(CbString(params["name"].get<std::string>().c_str()));
            if (params.contains("access"))
            {
                AccessType acc;
                if (!ParseAccess(params["access"].get<std::string>(), acc))
                { error = "invalid 'access' (use public/protected/private)"; return; }
                pFM->SetAccess(acc);
            }

            // Materialize each argument with its path.
            for (size_t i = 0; i < argsJ.size(); ++i)
            {
                const json& a = argsJ[i];
                Argument* pArg = NULL;
                if (a.contains("member"))
                {
                    std::string mc = a["member"]["class"].get<std::string>();
                    std::string mn = a["member"]["name"].get<std::string>();
                    BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                                            CbString(mc.c_str()));
                    Member* pMember = pBC->FindMember(CbString(mn.c_str()));
                    pArg = new MemberArgument(pFM, pMember);
                }
                else
                {
                    std::string nc = a["nav_class"].get<std::string>();
                    BaseClass* pBC = pDataModel->GetDataModelDoc()->FindBaseClass(
                                                            CbString(nc.c_str()));
                    pArg = new Argument(pFM, pBC);
                    pArg->SetPointer(1);
                    if (a.contains("arg_name"))
                        pArg->SetName(CbString(a["arg_name"].get<std::string>().c_str()));
                    else
                        pArg->SetName(CbString(("p" + nc).c_str()));
                }
                pArg->SetPath(CbString(a["path"].get<std::string>().c_str()));
            }

            // Optional "next" position arg (mirrors dialog's m_next handling).
            if (nextFlag)
            {
                CbString posName = (reverse ? "startBefore" : "startAfter") +
                                  pFM->GetType()->GetName();
                Argument* pPos = new Argument(pFM, pFM->GetType());
                pPos->SetPointer(1);
                pPos->SetName(posName);
                pPos->SetDefault("0");
                pPos->SetNote("Default argument to give the start position of the find.");
                pFM->SetNext(true);
            }

            pFM->Add();
            pFM->InitCode();
            pClass->NotifyAddMethod(pFM);

            response = ToJson(pFM);
        });

    // delete_find_method({class, from_name, to_name?, name|id})
    // Removes a FindMethod from a relation. Either `name` or `id` may
    // identify the method (id preferred — multiple Find methods on the
    // same relation can share name modifiers).
    CbCommandServer::Register("delete_find_method",
        [resolveRelation](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            Class* pClass = pDataModel->FindClass(CbString(className.c_str()));
            if (!pClass) { error = "no such class"; return; }
            Relation* pRel = resolveRelation(pClass, params, error);
            if (!pRel) return;

            // Walk the FromRelation's methods to find a FindMethod match.
            FindMethod* pHit = NULL;
            FromRelation::MethodIterator iMethod(pRel->GetFromRelation());
            while (++iMethod)
            {
                FindMethod* pFM = dynamic_cast<FindMethod*>(iMethod.Get());
                if (!pFM) continue;
                if (params.contains("id"))
                {
                    if (pFM->GetId() == (UINT)params["id"].get<unsigned>())
                    { pHit = pFM; break; }
                }
                else if (params.contains("name"))
                {
                    if (pFM->GetName() ==
                        CbString(params["name"].get<std::string>().c_str()))
                    { pHit = pFM; break; }
                }
            }
            if (!pHit) { error = "no matching find method on the relation"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            response["id"]   = (unsigned)pHit->GetId();
            response["name"] = ToStd(pHit->GetName());
            pHit->Delete();
        });

    // ----- Member commands -------------------------------------------------

    // get_member({class, name}) — rich record: type, access, flags,
    // initialization, note, getter/setter access ("none" if absent).
    CbCommandServer::Register("get_member",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className  = params.value("class", std::string());
            std::string memberName = params.value("name",  std::string());
            if (className.empty() || memberName.empty())
            { error = "missing 'class' or 'name'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { response = nullptr; return; }
            Member* pMember = pBC->FindMember(CbString(memberName.c_str()));
            response = pMember ? ToJsonFull(pMember) : json(nullptr);
        });

    // add_member({class, name, type, access?, static?, serialize?,
    //             initialization?, note?, getter?, setter?})
    //
    // `type` accepts modifiers (`Foo*`, `Foo&`, `Foo[]`, `Foo[N]`). Bare
    // type must already exist in the model's Type list.
    // `getter` / `setter` are access strings ("public"/"protected"/"private")
    // or omitted/"none" → no getter/setter created.
    CbCommandServer::Register("add_member",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string className = params.value("class", std::string());
            std::string name      = params.value("name",  std::string());
            std::string typeStr   = params.value("type",  std::string());
            if (className.empty() || name.empty() || typeStr.empty())
            { error = "missing 'class', 'name', or 'type'"; return; }

            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            BaseClass* pBC = dmd.FindBaseClass(CbString(className.c_str()));
            if (!pBC) { error = "class not found: " + className; return; }
            if (pBC->FindMember(CbString(name.c_str())))
            { error = "a member with that name already exists on the class"; return; }

            ParsedType pt = ParseTypeString(typeStr);
            Type* pType = dmd.FindType(CbString(pt.bareName.c_str()));
            if (!pType) { error = "type not found: " + pt.bareName; return; }

            // Access — defaults to private (typical for members).
            AccessType acc = PRIVATE;
            if (params.contains("access"))
            {
                std::string accStr = params["access"].get<std::string>();
                if (!ParseAccess(accStr, acc))
                { error = "invalid 'access' (use public/protected/private)"; return; }
            }

            // Optional getter/setter access — "none" or absent = skip.
            AccessType getAcc = NONE; bool wantGetter = false;
            AccessType setAcc = NONE; bool wantSetter = false;
            if (params.contains("getter"))
            {
                std::string s = params["getter"].get<std::string>();
                if (s != "none" && !ParseAccess(s, getAcc))
                { error = "invalid 'getter' (use none/public/protected/private)"; return; }
                wantGetter = (s != "none");
            }
            if (params.contains("setter"))
            {
                std::string s = params["setter"].get<std::string>();
                if (s != "none" && !ParseAccess(s, setAcc))
                { error = "invalid 'setter' (use none/public/protected/private)"; return; }
                wantSetter = (s != "none");
            }

            dmd.MarkLastUndo();

            Member* pMember = new Member(pBC, pType);
            ApplyTypeModifiers(pMember, pt);
            pMember->SetName(CbString(name.c_str()));
            pMember->SetAccess(acc);
            pMember->SetStatic   (params.value("static",    false) ? 1 : 0);
            pMember->SetSerialize(params.value("serialize", true)  ? 1 : 0);
            if (params.contains("initialization"))
                pMember->SetInitialization(
                    CbString(params["initialization"].get<std::string>().c_str()));
            if (params.contains("note"))
                pMember->SetNote(
                    CbString(params["note"].get<std::string>().c_str()));

            if (wantGetter)
            {
                GetMemberMethod* pGet = new GetMemberMethod(pMember);
                pGet->SetAccess(getAcc);
                pGet->SetStatic(pMember->GetStatic());
            }
            if (wantSetter)
            {
                SetMemberMethod* pSet = new SetMemberMethod(pMember);
                pSet->SetAccess(setAcc);
                pSet->SetStatic(pMember->GetStatic());
            }

            pMember->Add();

            response = ToJsonFull(pMember);
        });

    // delete_member({class, name}) — removes a member from a class.
    // Skips Member::OnDelete's confirmation dialog. Cascades to any
    // getter/setter via the relation's owned-active semantics.
    CbCommandServer::Register("delete_member",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className  = params.value("class", std::string());
            std::string memberName = params.value("name",  std::string());
            if (className.empty() || memberName.empty())
            { error = "missing 'class' or 'name'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Member* pMember = pBC->FindMember(CbString(memberName.c_str()));
            if (!pMember) { error = "no such member"; return; }

            pDoc->GetDataModelDoc().MarkLastUndo();
            response["class"] = ToStd(pBC->GetName());
            response["name"]  = ToStd(pMember->GetVariableName());
            pMember->Delete();
        });

    // set_member_name({class, name, value}) — renames a member.
    CbCommandServer::Register("set_member_name",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            std::string name      = params.value("name",  std::string());
            if (className.empty() || name.empty())
            { error = "missing 'class' or 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Member* pMember = pBC->FindMember(CbString(name.c_str()));
            if (!pMember) { error = "no such member"; return; }
            if (pBC->FindMember(CbString(value.c_str())))
            { error = "a member with that name already exists"; return; }

            pDoc->GetDataModelDoc().MarkLastUndo();
            pMember->SetName(CbString(value.c_str()));
            // Update() rebuilds the cached Gti item text/icon and fires
            // UpdateAllViews — the trees AND codegen read that cache. The
            // dialogs do this on OK; every label-affecting setter must too.
            pMember->Update();
            response = ToJsonFull(pMember);
        });

    // set_member_type({class, name, value}) — changes the member's type.
    // `value` accepts modifiers like `add_member`. Re-parents the member
    // onto the new Type via Type::MoveVariableLast (matches the dialog
    // path). Existing getter/setter are *not* regenerated here — call
    // set_member_getter/setter explicitly if signatures need to refresh.
    CbCommandServer::Register("set_member_type",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            std::string name      = params.value("name",  std::string());
            if (className.empty() || name.empty())
            { error = "missing 'class' or 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();

            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            BaseClass* pBC = dmd.FindBaseClass(CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Member* pMember = pBC->FindMember(CbString(name.c_str()));
            if (!pMember) { error = "no such member"; return; }

            ParsedType pt = ParseTypeString(value);
            Type* pType = dmd.FindType(CbString(pt.bareName.c_str()));
            if (!pType) { error = "type not found: " + pt.bareName; return; }

            dmd.MarkLastUndo();
            pType->MoveVariableLast(pMember);
            ApplyTypeModifiers(pMember, pt);
            pMember->Update();
            response = ToJsonFull(pMember);
        });

    // set_member_access({class, name, value}) — value: public/protected/private.
    CbCommandServer::Register("set_member_access",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            std::string name      = params.value("name",  std::string());
            if (className.empty() || name.empty())
            { error = "missing 'class' or 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }

            AccessType acc;
            if (!ParseAccess(params["value"].get<std::string>(), acc))
            { error = "invalid 'value' (use public/protected/private)"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Member* pMember = pBC->FindMember(CbString(name.c_str()));
            if (!pMember) { error = "no such member"; return; }

            pDoc->GetDataModelDoc().MarkLastUndo();
            pMember->SetAccess(acc);
            pMember->Update();
            response = ToJsonFull(pMember);
        });

    // set_member_static({class, name, value:bool})
    CbCommandServer::Register("set_member_static",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            std::string name      = params.value("name",  std::string());
            if (className.empty() || name.empty())
            { error = "missing 'class' or 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            bool value = params["value"].get<bool>();

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Member* pMember = pBC->FindMember(CbString(name.c_str()));
            if (!pMember) { error = "no such member"; return; }

            pDoc->GetDataModelDoc().MarkLastUndo();
            pMember->SetStatic(value ? 1 : 0);
            // Mirror the dialog: keep getter/setter static-flag in sync.
            if (pMember->GetGetMemberMethod())
            {
                pMember->GetGetMemberMethod()->SetStatic(value ? 1 : 0);
                pMember->GetGetMemberMethod()->Update();
            }
            if (pMember->GetSetMemberMethod())
            {
                pMember->GetSetMemberMethod()->SetStatic(value ? 1 : 0);
                pMember->GetSetMemberMethod()->Update();
            }
            pMember->Update();
            response = ToJsonFull(pMember);
        });

    // set_member_serialize({class, name, value:bool}) — controls whether
    // the member appears in the generated Serialize body.
    CbCommandServer::Register("set_member_serialize",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            std::string name      = params.value("name",  std::string());
            if (className.empty() || name.empty())
            { error = "missing 'class' or 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            bool value = params["value"].get<bool>();

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Member* pMember = pBC->FindMember(CbString(name.c_str()));
            if (!pMember) { error = "no such member"; return; }

            pDoc->GetDataModelDoc().MarkLastUndo();
            pMember->SetSerialize(value ? 1 : 0);
            response = ToJsonFull(pMember);
        });

    // set_member_initialization({class, name, value}) — sets the
    // C++ initializer expression (e.g. "0", "false", "\"\""). Empty string
    // clears it.
    CbCommandServer::Register("set_member_initialization",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            std::string name      = params.value("name",  std::string());
            if (className.empty() || name.empty())
            { error = "missing 'class' or 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Member* pMember = pBC->FindMember(CbString(name.c_str()));
            if (!pMember) { error = "no such member"; return; }

            pDoc->GetDataModelDoc().MarkLastUndo();
            pMember->SetInitialization(CbString(value.c_str()));
            response = ToJsonFull(pMember);
        });

    // reinit_constructor({class, init=true, code=false}) -- regenerate a class's
    // normal constructor(s) init list (Constructor::InitInit) and/or code body
    // (InitCode), the same actions the GUI ctor code editor exposes. The pipe
    // gap: add_member does NOT append new members to the already-built _init text,
    // so a member added via the pipe is left uninitialised until the init part is
    // regenerated -- this command automates that (the GUI "reinit init part"
    // step). Defaults to init-only (regenerating code can overwrite hand-written
    // ctor body, so it's opt-in).
    CbCommandServer::Register("reinit_constructor",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "class not found: " + className; return; }

            const bool doInit = params.value("init", true);
            const bool doCode = params.value("code", false);

            pDoc->GetDataModelDoc().MarkLastUndo();
            int n = 0;
            BaseClass::MethodIterator iCtor(pBC, &Method::IsNormalConstructor);
            while (++iCtor)
            {
                Constructor* pCtor = dynamic_cast<Constructor*>(iCtor.Get());
                if (!pCtor)
                    continue;
                if (doInit) pCtor->InitInit();
                if (doCode) pCtor->InitCode();
                n++;
            }
            response["class"]        = className;
            response["constructors"] = n;
            response["init"]         = doInit;
            response["code"]         = doCode;
        });

    // set_member_note({class, name, value})
    CbCommandServer::Register("set_member_note",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            std::string name      = params.value("name",  std::string());
            if (className.empty() || name.empty())
            { error = "missing 'class' or 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Member* pMember = pBC->FindMember(CbString(name.c_str()));
            if (!pMember) { error = "no such member"; return; }

            pDoc->GetDataModelDoc().MarkLastUndo();
            pMember->SetNote(CbString(value.c_str()));
            response = ToJsonFull(pMember);
        });

    // set_member_getter({class, name, access})
    // set_member_setter({class, name, access})
    //
    // `access` is "none" / "public" / "protected" / "private". "none"
    // deletes the existing GetMemberMethod / SetMemberMethod if any.
    // Any other value creates the method if absent and sets its access.
    // Mirrors CMemberDialog::Update's create/delete/set-access flow.
    auto registerGetterSetter =
        [](const std::string& cmdName, bool isGetter)
    {
        CbCommandServer::Register(cmdName,
            [isGetter](const json& params, json& response, std::string& error)
            {
                CClassBuilderDoc* pDoc = GetActiveDoc();
                if (!pDoc) { error = "no active document"; return; }
                std::string className = params.value("class", std::string());
                std::string name      = params.value("name",  std::string());
                if (className.empty() || name.empty())
                { error = "missing 'class' or 'name'"; return; }
                if (!params.contains("access"))
                { error = "missing 'access'"; return; }
                std::string accStr = params["access"].get<std::string>();

                bool wantNone = (accStr == "none");
                AccessType acc = NONE;
                if (!wantNone && !ParseAccess(accStr, acc))
                { error = "invalid 'access' (use none/public/protected/private)"; return; }

                BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                    CbString(className.c_str()));
                if (!pBC) { error = "no such class"; return; }
                Member* pMember = pBC->FindMember(CbString(name.c_str()));
                if (!pMember) { error = "no such member"; return; }

                pDoc->GetDataModelDoc().MarkLastUndo();

                if (wantNone)
                {
                    if (isGetter && pMember->GetGetMemberMethod())
                        pMember->GetGetMemberMethod()->Delete();
                    if (!isGetter && pMember->GetSetMemberMethod())
                        pMember->GetSetMemberMethod()->Delete();
                }
                else
                {
                    if (isGetter)
                    {
                        if (!pMember->GetGetMemberMethod())
                            (void)new GetMemberMethod(pMember);
                        pMember->GetGetMemberMethod()->SetAccess(acc);
                        pMember->GetGetMemberMethod()->SetStatic(pMember->GetStatic());
                    }
                    else
                    {
                        if (!pMember->GetSetMemberMethod())
                            (void)new SetMemberMethod(pMember);
                        pMember->GetSetMemberMethod()->SetAccess(acc);
                        pMember->GetSetMemberMethod()->SetStatic(pMember->GetStatic());
                    }
                }
                response = ToJsonFull(pMember);
            });
    };
    registerGetterSetter("set_member_getter", true);
    registerGetterSetter("set_member_setter", false);

    // set_class_member_prefix({name, value}) — Class-level member prefix
    // (e.g. "_" or "m_"). Member names stored in CB are *bare*; codegen
    // emits `prefix + bare_name`. The Class-level prefix overrides the
    // DataModel default for that class.
    CbCommandServer::Register("set_class_member_prefix",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();

            Class* pClass = pDataModel->FindClass(CbString(name.c_str()));
            if (!pClass) { error = "no such class"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            pClass->SetMemberPrefix(CbString(value.c_str()));
            response["name"]          = ToStd(pClass->GetName());
            response["member_prefix"] = ToStd(pClass->GetMemberPrefix());
        });

    CbCommandServer::Register("set_class_dll_export",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            bool value = params["value"].get<bool>();

            Class* pClass = pDataModel->FindClass(CbString(name.c_str()));
            if (!pClass) { error = "no such class"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            pClass->SetDllExport(value);
            response["name"]       = ToStd(pClass->GetName());
            response["dll_export"] = (bool)pClass->GetDllExport();
        });

    // find_method({class, name}) — full method record on the named class.
    CbCommandServer::Register("find_method",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string className  = params.value("class", std::string());
            std::string methodName = params.value("name",  std::string());
            if (className.empty() || methodName.empty())
            { error = "missing 'class' or 'name'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { response = nullptr; return; }

            Method* pMethod = pBC->FindMethodWithName(CbString(methodName.c_str()));
            response = pMethod ? ToJson(pMethod) : json(nullptr);
        });

    // find_method_by_id({class, id}) — id matches the //@CODE_NNNN tag the
    // generator embeds.
    CbCommandServer::Register("find_method_by_id",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string className = params.value("class", std::string());
            unsigned    id        = params.value("id",    0u);
            if (className.empty() || id == 0)
            { error = "missing 'class' or 'id'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { response = nullptr; return; }

            Method* pMethod = pBC->FindMethodWithId((UINT)id);
            response = pMethod ? ToJson(pMethod) : json(nullptr);
        });

    // find_member({class, name})
    CbCommandServer::Register("find_member",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string className  = params.value("class", std::string());
            std::string memberName = params.value("name",  std::string());
            if (className.empty() || memberName.empty())
            { error = "missing 'class' or 'name'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { response = nullptr; return; }

            Member* pMember = pBC->FindMember(CbString(memberName.c_str()));
            response = pMember ? ToJson(pMember) : json(nullptr);
        });

    // list_class_methods({class}) — full method records for every method on
    // the named class. Lets a client script tell overloads apart by argument
    // signature (FindMethodWithName only returns the first match).
    CbCommandServer::Register("list_class_methods",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { response = nullptr; return; }

            json arr = json::array();
            BaseClass::MethodIterator iMethod(pBC);
            while (++iMethod)
                arr.push_back(ToJson(iMethod));
            response = arr;
        });

    // list_methods_named({name}) — every class that has a method matching
    // `name`. The bulk-Serialize finder.
    CbCommandServer::Register("list_methods_named",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }

            CbString cname(name.c_str());
            json arr = json::array();
            DataModel::ClassIterator iClass(pDataModel);
            while (++iClass)
            {
                Method* pMethod = iClass->FindMethodWithName(cname);
                if (pMethod)
                {
                    json hit;
                    hit["class"]  = ToStd(iClass->GetName());
                    hit["method"] = ToJson(pMethod);
                    arr.push_back(hit);
                }
            }
            response = arr;
        });

    // find_methods_using_type({type}) — every method whose argument list
    // contains the named type. Walks: BaseClass IS-A Type → iterate Variable
    // children → filter IsArgument → cast to Argument → GetMethod →
    // GetBaseClass. Useful for migration audits (e.g. "find every method that
    // takes CArchive&").
    CbCommandServer::Register("find_methods_using_type",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string typeName = params.value("type", std::string());
            if (typeName.empty()) { error = "missing 'type'"; return; }

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(typeName.c_str()));
            if (!pBC) { error = "type not found: " + typeName; return; }

            json arr = json::array();
            Type::VariableIterator iVar(pBC);
            while (++iVar)
            {
                Variable* pVar = iVar.Get();
                if (!pVar->IsArgument()) continue;
                Argument* pArg = (Argument*)pVar;
                Method* pMethod = pArg->GetMethod();
                if (!pMethod) continue;
                BaseClass* pOwner = pMethod->GetBaseClass();
                if (!pOwner) continue;

                json hit;
                hit["class"]    = ToStd(pOwner->GetName());
                hit["method"]   = ToStd(pMethod->GetName());
                hit["id"]       = (unsigned)pMethod->GetId();
                hit["arg_name"] = ToStd(pArg->GetVariableName());
                hit["arg_type"] = ToStd(pArg->GetTypeName());
                arr.push_back(hit);
            }
            response = arr;
        });

    // -- Mutate commands ---------------------------------------------------

    // add_method({class, name, return_type, args[], access, virtual, static,
    //             const, pure, body}) — creates a new method on the class
    // and adds it through the same path the GUI uses, so undo / view refresh
    // / NotifyAddMethod side effects all run.
    CbCommandServer::Register("add_method",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string className  = params.value("class",       std::string());
            std::string methodName = params.value("name",        std::string());
            std::string returnType = params.value("return_type", std::string("void"));
            if (className.empty() || methodName.empty())
            { error = "missing 'class' or 'name'"; return; }

            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            BaseClass* pBC = dmd.FindBaseClass(CbString(className.c_str()));
            if (!pBC) { error = "class not found: " + className; return; }

            // Bare type for the constructor; modifiers applied after.
            ParsedType ptRet = ParseTypeString(returnType);
            Type* pRet = dmd.FindType(CbString(ptRet.bareName.c_str()));
            if (!pRet) { error = "return type not found: " + ptRet.bareName; return; }

            Method* pMethod = new Method(pBC, pRet);
            ApplyTypeModifiers(pMethod, ptRet);
            pMethod->SetName(CbString(methodName.c_str()));

            // Modifiers — defaults match the most common Serialize signature.
            std::string access = params.value("access", std::string("public"));
            AccessType  acc    = PUBLIC;
            if      (access == "protected") acc = PROTECTED;
            else if (access == "private")   acc = PRIVATE;
            pMethod->SetAccess(acc);

            pMethod->SetVirtual(params.value("virtual", false) ? 1 : 0);
            pMethod->SetStatic (params.value("static",  false) ? 1 : 0);
            pMethod->SetConst  (params.value("const",   false) ? 1 : 0);
            pMethod->SetPure   (params.value("pure",    false) ? 1 : 0);

            if (params.contains("body"))
                pMethod->SetCode(CbString(params["body"].get<std::string>().c_str()));

            // Arguments — created before Add so Method::Add picks them up
            // via its ArgumentIterator pass.
            if (params.contains("args") && params["args"].is_array())
            {
                for (size_t i = 0; i < params["args"].size(); ++i)
                {
                    const json& a = params["args"][i];
                    std::string aName = a.value("name", std::string());
                    std::string aType = a.value("type", std::string());
                    std::string aDef  = a.value("default", std::string());
                    if (aName.empty() || aType.empty())
                    { error = "argument missing 'name' or 'type'"; return; }

                    ParsedType pta = ParseTypeString(aType);
                    Type* pArgType = dmd.FindType(CbString(pta.bareName.c_str()));
                    if (!pArgType)
                    { error = "argument type not found: " + pta.bareName; return; }

                    Argument* pArg = new Argument(pMethod, pArgType);
                    ApplyTypeModifiers(pArg, pta);
                    pArg->SetName(CbString(aName.c_str()));
                    if (!aDef.empty())
                        pArg->SetDefault(CbString(aDef.c_str()));
                }
            }

            pMethod->Add();
            pBC->NotifyAddMethod(pMethod);

            response = ToJson(pMethod);
        });

    // set_method_body({class, name | id, body}) — replaces the //@CODE_NNNN
    // user-region content of an existing method.
    CbCommandServer::Register("set_method_body",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "class not found: " + className; return; }

            Method* pMethod = NULL;
            if (params.contains("id"))
                pMethod = pBC->FindMethodWithId(params.value("id", 0u));
            else if (params.contains("name"))
                pMethod = pBC->FindMethodWithName(
                    CbString(params["name"].get<std::string>().c_str()));
            else
            { error = "missing 'name' or 'id'"; return; }

            if (!pMethod) { error = "method not found"; return; }

            std::string body = params.value("body", std::string());
            pMethod->SetCode(CbString(body.c_str()));
            pMethod->Update();

            response = ToJson(pMethod);

            // Warn (don't block) when a constructor/destructor body is
            // replaced and the new body has lost the auto-stub include
            // call. CB relies on ConstructorInclude / DestructorInclude
            // for relation wiring & cascade-delete; user code goes after.
            if (pMethod->IsConstructor() &&
                body.find("ConstructorInclude(") == std::string::npos)
            {
                response["warning"] =
                    "constructor body has no ConstructorInclude(...) call — "
                    "relation wiring will be missing";
            }
            else if (pMethod->IsDestructor() &&
                     body.find("DestructorInclude(") == std::string::npos)
            {
                response["warning"] =
                    "destructor body has no DestructorInclude(...) call — "
                    "owned objects won't be cascade-deleted and parent links "
                    "won't be cleaned up";
            }
        });

    // delete_method({class, id | name}) — removes a method through the same
    // path the GUI uses (NotifyRemoveMethod, DataModelDocObject::Delete).
    // Prefer 'id' when overloads exist; 'name' takes the first match only.
    CbCommandServer::Register("delete_method",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "class not found: " + className; return; }

            Method* pMethod = NULL;
            if (params.contains("id"))
                pMethod = pBC->FindMethodWithId(params.value("id", 0u));
            else if (params.contains("name"))
                pMethod = pBC->FindMethodWithName(
                    CbString(params["name"].get<std::string>().c_str()));
            else
            { error = "missing 'name' or 'id'"; return; }

            if (!pMethod) { error = "method not found"; return; }

            // Method::Delete() detaches from parent + notifies; the object
            // becomes invalid afterwards, so capture the report fields first.
            json result;
            result["id"]    = (unsigned)pMethod->GetId();
            result["name"]  = ToStd(pMethod->GetName());
            result["class"] = ToStd(pBC->GetName());

            pMethod->Delete();
            response = result;
        });

    // add_argument({class, method, name, type, default?}) — appends a
    // single argument to an existing method.
    CbCommandServer::Register("add_argument",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string className  = params.value("class",  std::string());
            std::string methodName = params.value("method", std::string());
            std::string argName    = params.value("name",   std::string());
            std::string argType    = params.value("type",   std::string());
            std::string argDef     = params.value("default", std::string());
            if (className.empty() || methodName.empty() ||
                argName.empty()   || argType.empty())
            { error = "missing 'class','method','name', or 'type'"; return; }

            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            BaseClass* pBC = dmd.FindBaseClass(CbString(className.c_str()));
            if (!pBC) { error = "class not found: " + className; return; }

            Method* pMethod = pBC->FindMethodWithName(CbString(methodName.c_str()));
            if (!pMethod) { error = "method not found: " + methodName; return; }

            ParsedType pta = ParseTypeString(argType);
            Type* pArgType = dmd.FindType(CbString(pta.bareName.c_str()));
            if (!pArgType) { error = "argument type not found: " + pta.bareName; return; }

            Argument* pArg = new Argument(pMethod, pArgType);
            ApplyTypeModifiers(pArg, pta);
            pArg->SetName(CbString(argName.c_str()));
            if (!argDef.empty())
                pArg->SetDefault(CbString(argDef.c_str()));
            pArg->Add();
            pMethod->Update();

            response = ToJson(pMethod);
        });

    // ----- Method attribute setters ---------------------------------------

    // Helper macro: registers `cmd` as a method-attribute setter that calls
    // `setterCall` on the resolved method (taking one bool/int from `value`).
    // Usage: METHOD_BOOL_SETTER("set_method_virtual", SetVirtual)
#define METHOD_BOOL_SETTER(CMDNAME, SETTERFN)                                  \
    CbCommandServer::Register(CMDNAME,                                         \
        [](const json& params, json& response, std::string& error)             \
        {                                                                      \
            CClassBuilderDoc* pDoc = GetActiveDoc();                           \
            if (!pDoc) { error = "no active document"; return; }               \
            std::string className = params.value("class", std::string());      \
            if (className.empty()) { error = "missing 'class'"; return; }      \
            if (!params.contains("value")) { error = "missing 'value'"; return; }\
            bool value = params["value"].get<bool>();                          \
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(            \
                                                CbString(className.c_str()));   \
            if (!pBC) { error = "no such class"; return; }                     \
            Method* pMethod = ResolveMethod(pBC, params, error);               \
            if (!pMethod) return;                                              \
            pDoc->GetDataModelDoc().MarkLastUndo();                            \
            pMethod->SETTERFN(value ? 1 : 0);                                  \
            response = ToJson(pMethod);                                        \
        });

    METHOD_BOOL_SETTER("set_method_virtual",  SetVirtual);
    METHOD_BOOL_SETTER("set_method_static",   SetStatic);
    METHOD_BOOL_SETTER("set_method_const",    SetConst);
    METHOD_BOOL_SETTER("set_method_pure",     SetPure);
#undef METHOD_BOOL_SETTER

    // SetDllExport takes bool (not int) — bespoke handler.
    CbCommandServer::Register("set_method_dll_export",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            bool value = params["value"].get<bool>();
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Method* pMethod = ResolveMethod(pBC, params, error);
            if (!pMethod) return;
            pDoc->GetDataModelDoc().MarkLastUndo();
            pMethod->SetDllExport(value);
            response = ToJson(pMethod);
        });

    // set_method_access({class, id|name, value:"public"|"protected"|"private"})
    CbCommandServer::Register("set_method_access",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            AccessType acc;
            if (!ParseAccess(params["value"].get<std::string>(), acc))
            { error = "invalid 'value' (use public/protected/private)"; return; }
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Method* pMethod = ResolveMethod(pBC, params, error);
            if (!pMethod) return;
            pDoc->GetDataModelDoc().MarkLastUndo();
            pMethod->SetAccess(acc);
            pMethod->Update();
            response = ToJson(pMethod);
        });

    // set_method_name({class, id, value}) — rename. `id` is preferred over
    // `name` here since renaming by old-name is ambiguous with overloads.
    CbCommandServer::Register("set_method_name",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Method* pMethod = ResolveMethod(pBC, params, error);
            if (!pMethod) return;
            pDoc->GetDataModelDoc().MarkLastUndo();
            pMethod->SetName(CbString(value.c_str()));
            pMethod->Update();
            response = ToJson(pMethod);
        });

    // set_method_return_type({class, id|name, value}) — value accepts
    // modifiers (`Foo*`, `Foo&`, etc.). Re-parents onto the new Type.
    CbCommandServer::Register("set_method_return_type",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            BaseClass* pBC = dmd.FindBaseClass(CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Method* pMethod = ResolveMethod(pBC, params, error);
            if (!pMethod) return;
            ParsedType pt = ParseTypeString(value);
            Type* pType = dmd.FindType(CbString(pt.bareName.c_str()));
            if (!pType) { error = "type not found: " + pt.bareName; return; }
            dmd.MarkLastUndo();
            pType->MoveVariableLast(pMethod);
            ApplyTypeModifiers(pMethod, pt);
            pMethod->Update();
            response = ToJson(pMethod);
        });

    // add_constructor({class, init?, explicit?, args?, access?, body?})
    //
    // Mirrors BaseClass::OnAddConstructor: `new Constructor(class)` → set
    // name auto = class name, access = PUBLIC, type = empty (the
    // Constructor ctor handles all that). Then CreateArguments() runs the
    // CB default-arg setup, then any user-supplied `args` are appended in
    // addition (GUI flow appends after CreateArguments via the dialog).
    CbCommandServer::Register("add_constructor",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }

            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            BaseClass* pBC = dmd.FindBaseClass(CbString(className.c_str()));
            if (!pBC) { error = "class not found: " + className; return; }

            dmd.MarkLastUndo();

            Constructor* pCtor = new Constructor(pBC);
            // CreateArguments seeds the parent-pointer args from owned
            // (aggregation) relations; InitCode then writes the body stub
            // with ConstructorInclude(p<FromName>...) and "// Put in your
            // own code". Caller-supplied `args` are EXTRAS appended after
            // the auto-generated parent pointers; caller-supplied `body`
            // is appended after the auto stub (goes after ConstructorInclude).
            pCtor->CreateArguments();
            pCtor->InitCode();

            // Optional overrides — the Constructor ctor already set name
            // and access=PUBLIC; allow caller to override access.
            if (params.contains("access"))
            {
                AccessType acc;
                if (!ParseAccess(params["access"].get<std::string>(), acc))
                { error = "invalid 'access' (use public/protected/private)"; return; }
                pCtor->SetAccess(acc);
            }
            if (params.contains("explicit"))
                pCtor->SetExplicit(params["explicit"].get<bool>() ? 1 : 0);
            if (params.contains("init"))
                pCtor->SetInit(CbString(params["init"].get<std::string>().c_str()));
            // Append caller body to the auto stub (after ConstructorInclude).
            if (params.contains("body"))
            {
                CbString combined = pCtor->GetCode();
                combined += CbString(params["body"].get<std::string>().c_str());
                pCtor->SetCode(combined);
            }

            // Extra user args (in addition to those CreateArguments seeded).
            if (params.contains("args") && params["args"].is_array())
            {
                for (size_t i = 0; i < params["args"].size(); ++i)
                {
                    const json& a = params["args"][i];
                    std::string aName = a.value("name", std::string());
                    std::string aType = a.value("type", std::string());
                    std::string aDef  = a.value("default", std::string());
                    if (aName.empty() || aType.empty())
                    { error = "argument missing 'name' or 'type'"; return; }

                    ParsedType pta = ParseTypeString(aType);
                    Type* pArgType = dmd.FindType(CbString(pta.bareName.c_str()));
                    if (!pArgType)
                    { error = "argument type not found: " + pta.bareName; return; }

                    Argument* pArg = new Argument(pCtor, pArgType);
                    ApplyTypeModifiers(pArg, pta);
                    pArg->SetName(CbString(aName.c_str()));
                    if (!aDef.empty())
                        pArg->SetDefault(CbString(aDef.c_str()));
                }
            }

            pCtor->Add();
            pBC->NotifyAddMethod(pCtor);

            response = ToJson(pCtor);
        });

    // set_method_note({class, id|name, value}) — Method inherits SetNote
    // from Variable (the @NOTE_NNNN comment above the method, distinct
    // from the @CODE_NNNN body that set_method_body manages).
    CbCommandServer::Register("set_method_note",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            std::string value = params["value"].get<std::string>();
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }
            Method* pMethod = ResolveMethod(pBC, params, error);
            if (!pMethod) return;
            pDoc->GetDataModelDoc().MarkLastUndo();
            pMethod->SetNote(CbString(value.c_str()));
            response = ToJson(pMethod);
        });

    // set_class_name({name, value}) — renames a Class. Rejects collisions.
    CbCommandServer::Register("set_class_name",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string name  = params.value("name",  std::string());
            std::string value = params.value("value", std::string());
            if (name.empty() || value.empty())
            { error = "missing 'name' or 'value'"; return; }

            Class* pClass = pDataModel->FindClass(CbString(name.c_str()));
            if (!pClass) { error = "no such class"; return; }
            if (name == value)
            { response["name"] = name; return; }
            if (pDataModel->GetDataModelDoc()->FindBaseClass(CbString(value.c_str())))
            { error = "a class with that name already exists"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            pClass->SetName(CbString(value.c_str()));
            response["name"] = ToStd(pClass->GetName());
        });

    // ----- Argument-level commands ----------------------------------------

    // delete_argument({class, method (name) | method_id, arg|arg_index})
    CbCommandServer::Register("delete_argument",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }

            // Method lookup uses `method` (name) or `method_id`. Translate
            // those into the {name|id} keys ResolveMethod expects.
            json mp;
            if (params.contains("method_id")) mp["id"]   = params["method_id"];
            else                              mp["name"] = params.value("method", std::string());
            Method* pMethod = ResolveMethod(pBC, mp, error);
            if (!pMethod) return;

            Argument* pArg = ResolveArgument(pMethod, params, error);
            if (!pArg) return;

            pDoc->GetDataModelDoc().MarkLastUndo();
            response["method_id"] = (unsigned)pMethod->GetId();
            response["arg"]       = ToStd(pArg->GetVariableName());
            pArg->Delete();
            pMethod->Update();
        });

    // set_argument_name / set_argument_type / set_argument_default
    // — share the same lookup envelope.
    auto registerArgSetter =
        [](const std::string& cmdName,
           std::function<void(Argument*, const std::string&, DataModelDoc&,
                              std::string&)> apply)
    {
        CbCommandServer::Register(cmdName,
            [apply](const json& params, json& response, std::string& error)
            {
                CClassBuilderDoc* pDoc = GetActiveDoc();
                if (!pDoc) { error = "no active document"; return; }
                std::string className = params.value("class", std::string());
                if (className.empty()) { error = "missing 'class'"; return; }
                if (!params.contains("value")) { error = "missing 'value'"; return; }
                std::string value = params["value"].get<std::string>();
                DataModelDoc& dmd = pDoc->GetDataModelDoc();
                BaseClass* pBC = dmd.FindBaseClass(CbString(className.c_str()));
                if (!pBC) { error = "no such class"; return; }

                json mp;
                if (params.contains("method_id")) mp["id"]   = params["method_id"];
                else                              mp["name"] = params.value("method", std::string());
                Method* pMethod = ResolveMethod(pBC, mp, error);
                if (!pMethod) return;

                Argument* pArg = ResolveArgument(pMethod, params, error);
                if (!pArg) return;

                dmd.MarkLastUndo();
                apply(pArg, value, dmd, error);
                if (!error.empty()) return;
                // The argument has its own tree node; Argument::Update()
                // refreshes it and cascades to GetMethod()->Update().
                pArg->Update();
                response = ToJson(pMethod);
            });
    };

    registerArgSetter("set_argument_name",
        [](Argument* pArg, const std::string& value, DataModelDoc&, std::string&)
        {
            pArg->SetName(CbString(value.c_str()));
        });

    registerArgSetter("set_argument_type",
        [](Argument* pArg, const std::string& value, DataModelDoc& dmd, std::string& error)
        {
            ParsedType pt = ParseTypeString(value);
            Type* pType = dmd.FindType(CbString(pt.bareName.c_str()));
            if (!pType) { error = "type not found: " + pt.bareName; return; }
            pType->MoveVariableLast(pArg);
            ApplyTypeModifiers(pArg, pt);
        });

    // move_argument({class, method|method_id, arg|arg_index, position, target?})
    //
    // Reorders an argument within its method. `position` is one of:
    //   "first"  — move to head; `target` ignored
    //   "last"   — move to tail;  `target` ignored
    //   "before" — move directly before `target`; `target` required
    //   "after"  — move directly after `target`;  `target` required
    //
    // `target` accepts the same envelope as `arg` (name or by `target_index`).
    CbCommandServer::Register("move_argument",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string className = params.value("class", std::string());
            if (className.empty()) { error = "missing 'class'"; return; }
            std::string position = params.value("position", std::string());
            if (position.empty()) { error = "missing 'position'"; return; }
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(className.c_str()));
            if (!pBC) { error = "no such class"; return; }

            json mp;
            if (params.contains("method_id")) mp["id"]   = params["method_id"];
            else                              mp["name"] = params.value("method", std::string());
            Method* pMethod = ResolveMethod(pBC, mp, error);
            if (!pMethod) return;

            Argument* pArg = ResolveArgument(pMethod, params, error);
            if (!pArg) return;

            // Resolve `target` if needed (before/after). Reuse ResolveArgument
            // by remapping target → arg / target_index → arg_index.
            Argument* pTarget = NULL;
            if (position == "before" || position == "after")
            {
                json tp;
                if (params.contains("target_index")) tp["arg_index"] = params["target_index"];
                else                                  tp["arg"]       = params.value("target", std::string());
                pTarget = ResolveArgument(pMethod, tp, error);
                if (!pTarget) return;
                if (pTarget == pArg)
                { error = "target must be a different argument"; return; }
            }

            pDoc->GetDataModelDoc().MarkLastUndo();
            if      (position == "first")  pMethod->MoveArgumentFirst(pArg);
            else if (position == "last")   pMethod->MoveArgumentLast(pArg);
            else if (position == "before") pMethod->MoveArgumentBefore(pArg, pTarget);
            else if (position == "after")  pMethod->MoveArgumentAfter(pArg, pTarget);
            else { error = "invalid 'position' (use first/last/before/after)"; return; }
            pMethod->Update();

            response = ToJson(pMethod);
        });

    registerArgSetter("set_argument_default",
        [](Argument* pArg, const std::string& value, DataModelDoc&, std::string&)
        {
            pArg->SetDefault(CbString(value.c_str()));
        });

    // ----- Extern class commands -----------------------------------------

    // add_extern_class({name, suppress_forward?})
    CbCommandServer::Register("add_extern_class",
        [](const json& params, json& response, std::string& error)
        {
            DataModel* pDataModel = GetActiveDataModel();
            if (!pDataModel) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            CbString cname(name.c_str());
            if (pDataModel->GetDataModelDoc()->FindBaseClass(cname))
            { error = "a class or extern class with that name already exists"; return; }

            pDataModel->GetDataModelDoc()->MarkLastUndo();
            ExternClass* pEC = new ExternClass(pDataModel->GetDataModelDoc());
            pEC->SetName(cname);
            if (params.contains("suppress_forward"))
                pEC->SetSuppressForwardDeclaration(
                    params["suppress_forward"].get<bool>());
            pEC->Add();

            response["name"]             = ToStd(pEC->GetName());
            response["kind"]             = "ExternClass";
            response["suppress_forward"] = (bool)pEC->GetSuppressForwardDeclaration();
        });

    // delete_extern_class({name}) — rejects if any class still inherits
    // from it (incoming inherit count > 0). Skips GUI confirmation.
    CbCommandServer::Register("delete_extern_class",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(name.c_str()));
            if (!pBC) { error = "no such class"; return; }
            ExternClass* pEC = dynamic_cast<ExternClass*>(pBC);
            if (!pEC || pEC->IsClass())
            { error = "not an extern class (use delete_class for Class)"; return; }
            // Any incoming inherits → reject.
            if (pEC->BaseClass::GetInheritCount())
            { error = "extern class is still inherited by other classes"; return; }

            pDoc->GetDataModelDoc().MarkLastUndo();
            response["name"] = ToStd(pEC->GetName());
            pEC->Delete();
        });

    // list_extern_classes() — names of all ExternClass objects (excludes Class).
    CbCommandServer::Register("list_extern_classes",
        [](const json& /*params*/, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            json arr = json::array();
            DataModelDoc::BaseClassIterator iBC(&pDoc->GetDataModelDoc());
            while (++iBC)
            {
                if (iBC->IsExternClass() && !iBC->IsClass())
                    arr.push_back(ToStd(iBC->GetName()));
            }
            response = arr;
        });

    // set_extern_class_suppress_forward({name, value:bool})
    CbCommandServer::Register("set_extern_class_suppress_forward",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            if (!params.contains("value")) { error = "missing 'value'"; return; }
            bool value = params["value"].get<bool>();

            BaseClass* pBC = pDoc->GetDataModelDoc().FindBaseClass(
                                                CbString(name.c_str()));
            if (!pBC) { error = "no such class"; return; }

            pDoc->GetDataModelDoc().MarkLastUndo();
            pBC->SetSuppressForwardDeclaration(value);
            response["name"]             = ToStd(pBC->GetName());
            response["suppress_forward"] = (bool)pBC->GetSuppressForwardDeclaration();
        });

    // ----- Type commands -------------------------------------------------

    // add_type({name}) — creates a new OtherType. Rejects duplicates
    // (a Type, ExternClass, or Class with that name already exists).
    CbCommandServer::Register("add_type",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            CbString cname(name.c_str());
            if (dmd.FindType(cname))
            { error = "a type with that name already exists"; return; }

            dmd.MarkLastUndo();
            OtherType* pOT = new OtherType(&dmd);
            pOT->SetName(cname);
            pOT->Add();

            response["name"] = ToStd(pOT->GetName());
            response["kind"] = "OtherType";
        });

    // list_types() — every Type in the model, tagged with kind. Includes
    // OtherType, ExternClass, and Class (any of which can be referenced
    // as a member or argument type).
    CbCommandServer::Register("list_types",
        [](const json& /*params*/, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            json arr = json::array();
            DataModelDoc::TypeIterator iType(&pDoc->GetDataModelDoc());
            while (++iType)
            {
                json e;
                e["name"] = ToStd(iType->GetName());
                const char* kind = "OtherType";
                BaseClass* pBC = dynamic_cast<BaseClass*>(iType.operator->());
                if (pBC)
                {
                    if      (pBC->IsClass())       kind = "Class";
                    else if (pBC->IsExternClass()) kind = "ExternClass";
                    else                            kind = "BaseClass";
                }
                e["kind"] = kind;
                arr.push_back(e);
            }
            response = arr;
        });

    // delete_type({name}) — only OtherType. Rejects if any Variable
    // references the type, mirroring OtherType::OnDelete's guard. Also
    // refuses the built-in non-deletable types ("void", "int", "").
    CbCommandServer::Register("delete_type",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            Type* pType = dmd.FindType(CbString(name.c_str()));
            if (!pType) { error = "no such type"; return; }
            OtherType* pOT = dynamic_cast<OtherType*>(pType);
            if (!pOT) { error = "type is a Class or ExternClass; use delete_class / delete_extern_class"; return; }
            if (pOT->GetVariableCount())
            { error = "type is referenced by a Variable; remove or retype the references first"; return; }
            if (name == "void" || name == "int" || name.empty())
            { error = "cannot delete built-in type"; return; }

            dmd.MarkLastUndo();
            response["name"] = name;
            pOT->Delete();
        });

    // set_type_declaration({name, declaration}) — sets an OtherType's declaration
    // text (the typedef / enum / struct body the codegen emits into the master
    // include between //@START_DECLARATION_<id> markers). Mirrors TypeDialog::accept.
    // Closes the add_type gap: add_type makes the empty slot, this fills it, so a
    // typedef can be created entirely over the pipe with no GUI round-trip.
    CbCommandServer::Register("set_type_declaration",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            std::string name = params.value("name", std::string());
            if (name.empty()) { error = "missing 'name'"; return; }
            if (!params.contains("declaration")) { error = "missing 'declaration'"; return; }
            std::string decl = params["declaration"].get<std::string>();

            DataModelDoc& dmd = pDoc->GetDataModelDoc();
            Type* pType = dmd.FindType(CbString(name.c_str()));
            if (!pType) { error = "no such type"; return; }
            OtherType* pOT = dynamic_cast<OtherType*>(pType);
            if (!pOT) { error = "type is a Class or ExternClass, not an OtherType"; return; }

            dmd.MarkLastUndo();
            pOT->SetDeclaration(CbString(decl.c_str()));
            response["name"]        = ToStd(pOT->GetName());
            response["declaration"] = ToStd(pOT->GetDeclaration());
        });

    // -- Source generation / read-back ------------------------------------

    // write_source({modified_only?}) — regenerate .h/.cpp from the model, the
    // headless equivalent of File > Save Source. Default modified_only:true
    // (only classes whose model changed since the last save), matching the
    // dialog's "Save modifications"; modified_only:false rewrites every file.
    // Mirrors the menu handler: _chdir to the project dir first (codegen uses
    // paths relative to it).
    CbCommandServer::Register("write_source",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModel* pDataModel = pDoc->GetDataModelDoc().GetDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            bool modifiedOnly = params.value("modified_only", true);
            _chdir(pDoc->GetDataModelDoc().GetPath());

            PipeSourceLog log;
            if (modifiedOnly) pDataModel->SaveModifiedFiles(&log);
            else              pDataModel->SaveAllFiles(&log);

            // NOTE: Save{Modified,All}Files themselves end with
            // OnSaveDocument(GetPathName()) -- the model (.cbz) is ALWAYS
            // saved and the modified flag cleared by the calls above. The
            // save_cbz param predates that realisation; it is kept for
            // compatibility but is a redundant second save.
            if (params.value("save_cbz", false))
                response["cbz_saved"] = pDoc->DoSave() ? true : false;

            response["modified_only"] = modifiedOnly;
            response["warnings"]      = log.warnings;
            response["errors"]        = log.errors;
            if (!log.warningMsgs.empty()) response["warning_messages"] = log.warningMsgs;
            if (!log.errorMsgs.empty())   response["error_messages"]   = log.errorMsgs;
        });

    // read_source({all?}) — import on-disk hand-edits back into the model
    // (//@CODE bodies, @START_USER regions), the headless equivalent of
    // File > Read Source. Default all:false (only files modified on disk since
    // the last save); all:true re-reads every file unconditionally. Marks one
    // undo step, like the menu handler.
    CbCommandServer::Register("read_source",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }
            DataModel* pDataModel = pDoc->GetDataModelDoc().GetDataModel();
            if (!pDataModel) { error = "no active document"; return; }

            bool all = params.value("all", false);
            _chdir(pDoc->GetDataModelDoc().GetPath());

            PipeParseLog log;
            pDataModel->ReadAllFiles(&log, all);
            pDoc->GetDataModelDoc().MarkLastUndo();

            response["all"]      = all;
            response["warnings"] = log.warnings;
            response["errors"]   = log.errors;
            if (!log.warningMsgs.empty()) response["warning_messages"] = log.warningMsgs;
            if (!log.errorMsgs.empty())   response["error_messages"]   = log.errorMsgs;
        });

    // -- CBZ round-trip commands -------------------------------------------

    // save_cb({path}) — writes the active document to `path` via the
    // CbArchive code path (uncompressed for now; Zstd to follow). Does NOT
    // touch the document's modified flag or pathname — purely an export.
    CbCommandServer::Register("save_cb",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document"; return; }

            std::string path = params.value("path", std::string());
            if (path.empty()) { error = "missing 'path'"; return; }

            std::ofstream os(path.c_str(), std::ios::binary | std::ios::trunc);
            if (!os) { error = "could not open '" + path + "' for writing"; return; }

            try
            {
                CbZstdOutBuf zbuf(os);
                std::ostream zos(&zbuf);
                CbArchive ar(zos);
                pDoc->GetDataModelDoc().Serialize(ar);
                response["path"]            = path;
                response["bytes_logical"]   = ar.GetTotalLength();
            }
            catch (...)
            {
                error = "exception during CbArchive serialize";
            }
            // Compressed file size only known after the streambuf finalised on dtor.
            os.close();
            std::ifstream sized(path.c_str(),
                std::ios::binary | std::ios::ate);
            if (sized)
            {
                response["bytes_on_disk"] = (long long)sized.tellg();
            }
        });

    // load_cb({path}) — replaces the active document's contents from `path`
    // via the CbArchive code path. Loads into the active doc rather than
    // creating a new one, because the new-doc path triggers the DataModel
    // wizard dialog and blocks the pipe. Caller should ensure they're OK with
    // the active doc being overwritten (e.g. close + reopen the .cbd to
    // restore).
    CbCommandServer::Register("load_cb",
        [](const json& params, json& response, std::string& error)
        {
            CClassBuilderDoc* pDoc = GetActiveDoc();
            if (!pDoc) { error = "no active document to load into"; return; }

            std::string path = params.value("path", std::string());
            if (path.empty()) { error = "missing 'path'"; return; }

            std::ifstream is(path.c_str(), std::ios::binary);
            if (!is) { error = "could not open '" + path + "' for reading"; return; }

            // Hold the archive in scope outside the try so the catch handlers
            // can read its diagnostic state.
            pDoc->DeleteContents();
            CbZstdInBuf zbuf(is);
            std::istream zis(&zbuf);
            CbArchive ar(zis);
            try
            {
                pDoc->GetDataModelDoc().Serialize(ar);
                pDoc->SetPathName(CbString(path.c_str()), 0 /*addToMru*/);
                pDoc->SetModifiedFlag(0);
                pDoc->NotifyStructureChanged();
                response["title"]          = ToStd(pDoc->GetTitle());
                response["path"]           = ToStd(pDoc->GetPathName());
                response["bytes_logical"]  = ar.GetTotalLength();
                response["objects_loaded"] = ar._objectsLoaded;
            }
            catch (int code)
            {
                std::ostringstream oss;
                oss << "CbArchive::Read short read (code " << code << ")"
                    << " | last class loaded: " << ToStd(ar._lastLoadedClass)
                    << " | objects so far: " << ar._objectsLoaded
                    << " | byte offset: " << ar.GetTotalLength();
                error = oss.str();
            }
            catch (std::exception& e)
            {
                std::ostringstream oss;
                oss << "std::exception: " << e.what()
                    << " | last class loaded: " << ToStd(ar._lastLoadedClass)
                    << " | objects so far: " << ar._objectsLoaded
                    << " | byte offset: " << ar.GetTotalLength();
                error = oss.str();
            }
            catch (...)
            {
                std::ostringstream oss;
                oss << "unknown exception during CbArchive deserialize"
                    << " | last class loaded: " << ToStd(ar._lastLoadedClass)
                    << " | objects so far: " << ar._objectsLoaded
                    << " | byte offset: " << ar.GetTotalLength();
                error = oss.str();
            }
        });
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void CbCommandServer::Register(const std::string& name, CbCommandFn fn)
{
    g_dispatcher[name] = std::move(fn);
}

namespace
{
    CbCommandServer::PendingNewModelParams* g_pPendingNewModelParams = NULL;
}

CbCommandServer::PendingNewModelParams* CbCommandServer::GetPendingNewModelParams()
{
    return g_pPendingNewModelParams;
}

void CbCommandServer::SetPendingNewModelParams(PendingNewModelParams* p)
{
    g_pPendingNewModelParams = p;
}

void CbCommandServer::EnsureRegistered()
{
    if (g_dispatcher.empty())
        RegisterBuiltinCommands();
}

std::string CbCommandServer::ProcessRequestLine(const std::string& jsonLine)
{
    // Parse one request line, run the command on THIS (GUI) thread, serialise the
    // reply. The transport (qt/QtCommandServer) calls this from a readyRead slot,
    // so the handler touches the model on the GUI thread -- exactly like a menu
    // action, no marshaling needed.
    CbCommandRequest req;
    req.ok = false;

    try
    {
        json j = json::parse(jsonLine);
        req.command = j.value("cmd", std::string());
        if (j.contains("params"))
            req.params = j["params"];
    }
    catch (const std::exception& e)
    {
        json resp = {
            {"ok", false},
            {"error", std::string("invalid JSON: ") + e.what()}
        };
        return resp.dump();
    }

    DispatchOnUiThread(&req);   // fills req.ok / req.response / req.error

    json resp;
    resp["ok"] = req.ok;
    if (!req.error.empty())
        resp["error"] = req.error;
    if (!req.response.is_null())
        resp["result"] = req.response;
    return resp.dump();
}

void CbCommandServer::DispatchOnUiThread(CbCommandRequest* pRequest)
{
    std::map<std::string, CbCommandFn>::iterator it =
        g_dispatcher.find(pRequest->command);
    if (it == g_dispatcher.end())
    {
        pRequest->ok    = false;
        pRequest->error = "unknown command: " + pRequest->command;
        return;
    }

    try
    {
        it->second(pRequest->params, pRequest->response, pRequest->error);
        pRequest->ok = pRequest->error.empty();
    }
    catch (const std::exception& e)
    {
        pRequest->ok    = false;
        pRequest->error = std::string("exception: ") + e.what();
    }
    catch (...)
    {
        pRequest->ok    = false;
        pRequest->error = "unknown exception";
    }
}
