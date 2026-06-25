// qt/QtSignalDialog.h -- bridge into the Qt Signal (Message) dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly and applies on OK.
#pragma once

class SignalShape;

// Shows the modal Message dialog for a sequence-diagram signal over the MFC
// owner window. Applies on OK; returns true if OK was pressed. `ownerHwnd`
// is the MFC owner HWND as void*.
bool Qt_ShowSignalDialog(SignalShape* pSignalShape, void* ownerHwnd);
