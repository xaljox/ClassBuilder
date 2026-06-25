// qt/QtProjectSettingsDialog.h -- bridge into the Qt Project Settings dialog.
//
// Qt-free and MFC-free, so the MFC code (ClassBuilderDoc) can include it.
#pragma once

class DataModelDoc;

// Shows the modal Project Settings dialog over the MFC owner window. The
// dialog applies accepted edits to `pDataModelDoc` itself (and marks the
// document dirty via DataModelDoc::SetModified). `ownerHwnd` is the MFC owner
// HWND as void*.
void Qt_ShowProjectSettingsDialog(DataModelDoc* pDataModelDoc, void* ownerHwnd);
