// qt/QtDestructorDialog.h -- bridge into the Qt Destructor properties dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live Destructor*).
#pragma once

class Destructor;

// Shows the modal Destructor properties dialog over the MFC owner window. The
// dialog applies accepted edits to `pDestructor` itself; `modelChangedOut`
// reports whether anything changed (so the caller can run Destructor::Update).
// `ownerHwnd` is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowDestructorDialog(Destructor* pDestructor, bool& modelChangedOut,
                             void* ownerHwnd);
