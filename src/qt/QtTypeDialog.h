// qt/QtTypeDialog.h -- bridge into the Qt Type (OtherType) properties dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live OtherType*).
#pragma once

class OtherType;

// Shows the modal Type properties dialog over the MFC owner window. The dialog
// applies accepted edits to `pOtherType` itself; `modelChangedOut` reports
// whether anything changed (so the caller can run OtherType::Update).
// `ownerHwnd` is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowTypeDialog(OtherType* pOtherType, bool& modelChangedOut,
                       void* ownerHwnd);
