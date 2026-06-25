// qt/QtExternClassDialog.h -- bridge into the Qt Extern Class dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly.
#pragma once

class ExternClass;

// Shows the modal Extern Class attributes dialog over the MFC owner window.
// Returns true if OK was pressed; on OK `changed` is set when a field
// actually changed -- the caller then calls ExternClass::Update. `ownerHwnd`
// is the MFC owner HWND as void*.
bool Qt_ShowExternClassDialog(ExternClass* pExternClass, bool& changed,
                              void* ownerHwnd);
