// qt/QtMemberMethodsDialog.h -- bridge into the Qt Member Methods dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live Member*).
#pragma once

class Member;

// Shows the modal Member Methods dialog over the MFC owner window: lists the
// public methods of the member's (class) type and, on OK, wraps each selected
// one onto the member. `ownerHwnd` is the MFC owner HWND as void*.
void Qt_ShowMemberMethodsDialog(Member* pMember, void* ownerHwnd);
