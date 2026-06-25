// qt/QtUserSectionsDialog.h -- MFC-callable entry point for the Qt
// "edit user sections" dialog.
//
// Qt-free and MFC-free: the MFC call sites include only this header. It edits
// a Class's six //@START_USER regions (HUser1..3 / CppUser1..3).
#pragma once

class Class;

// Shows the modal dialog. Applies the edited user sections to the model on OK.
void Qt_ShowUserSectionsDialog(Class* pClass, void* ownerHwnd);
