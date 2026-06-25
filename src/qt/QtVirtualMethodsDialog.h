// qt/QtVirtualMethodsDialog.h -- bridge into the Qt Virtual Methods dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly. Two modes, one per MFC constructor:
//   * extern-class mode -- pick inherited virtual methods to override onto an
//     ExternClass (optionally appended to a MemberAndMethodGroup);
//   * method mode -- pick which base classes to override one Method into.
#pragma once

class ExternClass;
class MemberAndMethodGroup;
class Method;

// Extern-class mode: "Virtual Methods for class <X>".
void Qt_ShowVirtualMethodsDialog(ExternClass* pExternClass,
                                 MemberAndMethodGroup* pGroup, void* ownerHwnd);

// Method mode: "Override virtual method '<X>::<Y>'".
void Qt_ShowVirtualMethodsDialogForMethod(Method* pMethod, void* ownerHwnd);
