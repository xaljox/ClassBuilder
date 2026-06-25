// qt/QtMethodDialog.h -- bridge into the Qt Method dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly.
#pragma once

class Method;

// Shows the modal Method attributes dialog over the MFC owner window.
// Returns true if OK was pressed; on OK `changed` is set when a field
// actually changed -- the caller then calls Method::Update. `ownerHwnd` is
// the MFC owner HWND as void*.
bool Qt_ShowMethodDialog(Method* pMethod, bool& changed, void* ownerHwnd);
