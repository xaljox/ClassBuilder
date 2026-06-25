// qt/QtGroupDialog.h -- bridge into the Qt Group properties dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live Group*).
#pragma once

class Group;

// Shows the modal Group properties dialog over the MFC owner window. The
// dialog applies accepted edits to `pGroup` itself; `modelChangedOut` reports
// whether anything changed (so the caller can run Group::Update). `ownerHwnd`
// is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowGroupDialog(Group* pGroup, bool& modelChangedOut, void* ownerHwnd);
