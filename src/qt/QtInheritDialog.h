// qt/QtInheritDialog.h -- bridge into the Qt Inherit dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly.
#pragma once

class Inherit;

// Shows the modal Inherit attributes dialog over the MFC owner window.
// Returns true if OK was pressed; on OK `changed` is set when a field
// actually changed -- the caller then calls Inherit::Update. `ownerHwnd`
// is the MFC owner HWND as void*.
bool Qt_ShowInheritDialog(Inherit* pInherit, bool& changed, void* ownerHwnd);
