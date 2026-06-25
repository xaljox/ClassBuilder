// qt/QtConstructorDialog.h -- bridge into the Qt Constructor properties dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live Constructor*).
#pragma once

class Constructor;

// Shows the modal Constructor properties dialog over the MFC owner window. The
// dialog applies accepted edits to `pConstructor` itself; `modelChangedOut`
// reports whether anything changed (so the caller can run Constructor::Update).
// `ownerHwnd` is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowConstructorDialog(Constructor* pConstructor, bool& modelChangedOut,
                              void* ownerHwnd);
