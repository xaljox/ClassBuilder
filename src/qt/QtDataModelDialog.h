// qt/QtDataModelDialog.h -- bridge into the Qt DataModel properties dialog.
//
// Qt-free and MFC-free, so the MFC code (ClassBuilderDoc / DataModel) can
// include it. Unlike the Search dialog, this one carries no value struct: the
// Qt dialog is handed the live DataModel* and reads / writes it directly --
// the model is now MFC-free, so the Qt translation unit can include the model
// headers (see ClassBuilderInclude.h with FORWARD_ONLY).
#pragma once

#include <string>

class DataModel;

// Outcome of the modal DataModel dialog.
struct DataModelDialogResult
{
    bool        accepted     = false;  // OK pressed
    bool        modelChanged = false;  // the dialog wrote changes into the model
    std::string className;             // the document-class name (edited only
                                       // for a brand-new, unnamed model)
};

// Shows the modal DataModel properties dialog over the MFC owner window.
// `classNameIn` seeds the Document Class Name field (the current doc class).
// `ownerHwnd` is the MFC owner HWND, passed as void* to keep this header
// MFC-free. The dialog applies all accepted edits to `pDataModel` itself.
DataModelDialogResult Qt_ShowDataModelDialog(DataModel*         pDataModel,
                                             const std::string& classNameIn,
                                             void*              ownerHwnd);
