// qt/QtIsClassMethodsDialog.h -- bridge into the Qt IsClass Methods dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live Class* / MemberAndMethodGroup*).
#pragma once

class Class;
class MemberAndMethodGroup;

// Shows the modal IsClass Methods dialog over the MFC owner window: lists the
// `Is<Base>()` methods the class could gain from its inherited base classes
// and, on OK, adds an IsClassMethod for each ticked entry. `pGroup` (optional)
// is the member-and-method group the new methods are appended to.
// `ownerHwnd` is the MFC owner HWND as void*.
void Qt_ShowIsClassMethodsDialog(Class* pClass, MemberAndMethodGroup* pGroup,
                                 void* ownerHwnd);
