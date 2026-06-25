// qt/QtCommentHeaderDialog.h -- bridge into the Qt comment-header editor.
//
// Qt-free and MFC-free, so the MFC DLL code (DataModelDialog.cpp) can include
// it. This is the editor behind the DataModel dialog's *.h / *.cpp buttons:
// it edits the template comment header prepended to generated source files.
#pragma once

#include <string>

// "Update All Headers" clears every class's per-file comment header -- a
// model operation the Qt code must not know about. The DLL supplies it as a
// plain function pointer plus opaque user data (no MFC, no Qt types).
typedef void (*Qt_UpdateAllHeadersFn)(void* userData);

// Shows the modal comment-header editor.
//   comment         in/out; raw bytes as stored in the model (system 8-bit
//                   encoding, CRLF line endings). On a true return it holds
//                   the edited text in that same encoding/convention.
//   windowTitle     window caption, e.g. "Template comment header for *.cpp".
//   updateAllPrompt confirmation text shown before "Update All Headers" runs.
//   updateAll       callback invoked (after the user confirms) by the
//                   "Update All Headers" button; may be null to omit it.
//   userData        passed verbatim to updateAll.
//   ownerHwnd       MFC owner window (HWND as void*); the dialog is made
//                   truly modal over it.
// Returns true if OK was pressed (comment may have changed), false on Cancel.
bool Qt_ShowCommentHeaderDialog(std::string& comment,
                                const char* windowTitle,
                                const char* updateAllPrompt,
                                Qt_UpdateAllHeadersFn updateAll,
                                void* userData,
                                void* ownerHwnd);
