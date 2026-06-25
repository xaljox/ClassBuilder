// qt/QtMemberDialog.h -- bridge into the Qt Member dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly.
#pragma once

class Member;

// Shows the modal Member attributes dialog over the MFC owner window.
// Returns true if OK was pressed; on OK `changed` is set when a field
// actually changed -- the caller then calls Member::Update. `ownerHwnd` is
// the MFC owner HWND as void*.
bool Qt_ShowMemberDialog(Member* pMember, bool& changed, void* ownerHwnd);
