// qt/QtClassDialog.h -- bridge into the Qt Class dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly.
#pragma once

class Class;

// Shows the modal Class attributes dialog over the MFC owner window. Returns
// true if OK was pressed; on OK `changed` is set when a field actually
// changed -- the caller then calls Class::Update. `ownerHwnd` is the MFC
// owner HWND as void*.
bool Qt_ShowClassDialog(Class* pClass, bool& changed, void* ownerHwnd);
