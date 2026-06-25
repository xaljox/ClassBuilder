// qt/QtArgumentDialog.h -- bridge into the Qt Argument dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly.
#pragma once

class Argument;

// Shows the modal Argument attributes dialog over the MFC owner window.
// Returns true if OK was pressed; on OK `changed` is set when a field
// actually changed -- the caller then calls Argument::Update. `ownerHwnd`
// is the MFC owner HWND as void*.
bool Qt_ShowArgumentDialog(Argument* pArgument, bool& changed,
                           void* ownerHwnd);
