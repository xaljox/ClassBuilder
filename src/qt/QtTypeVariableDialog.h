// qt/QtTypeVariableDialog.h -- bridge into the Qt "Type Variable" dialog.
//
// Qt-free and MFC-free, so the MFC code-edit dialogs can include it.
#pragma once

#include <string>

class DataModelDoc;

// Shows the modal "Type Variable Wizard" over the MFC owner window: the user
// picks a type/name and pointer/reference, and on OK `insertCodeOut` receives
// the variable-declaration snippet to splice into the code being edited.
// `ownerHwnd` is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowTypeVariableDialog(DataModelDoc* pDataModelDoc,
                               std::string&  insertCodeOut,
                               void*         ownerHwnd);
