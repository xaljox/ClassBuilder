// qt/QtInheritTreeDialog.h -- bridge into the Qt inheritance-tree dialogs.
//
// Qt-free and MFC-free, so the MFC code can include it. Two read-only tree
// windows: the base-class chain of a class, and the classes derived from it.
#pragma once

class BaseClass;
class ExternClass;

// "Base classes of class X" -- the inheritance chain upward.
void Qt_ShowInheritsFromDialog(ExternClass* pExternClass, void* ownerHwnd);

// "Classes derived from class X" -- the derivation tree downward.
void Qt_ShowInheritedByDialog(BaseClass* pBaseClass, void* ownerHwnd);
