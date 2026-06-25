// qt/QtAddSerializeDialog.h -- bridge into the Qt "Add Serialize" dialog.
//
// Qt-free and MFC-free, so the MFC code (ClassBuilderDoc) can include it.
#pragma once

#include <string>

class DataModelDoc;

// Shows the modal "Add Serialization to project" dialog over the MFC owner
// window. On OK, `classNameOut` receives the validated document-class name
// (the caller then runs DataModel::AddSerialize on it). `ownerHwnd` is the
// MFC owner HWND, passed as void* to keep this header MFC-free. Returns true
// if OK was pressed.
bool Qt_ShowAddSerializeDialog(DataModelDoc*  pDataModelDoc,
                               std::string&   classNameOut,
                               void*          ownerHwnd);
