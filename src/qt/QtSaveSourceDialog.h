// qt/QtSaveSourceDialog.h -- bridge into the Qt Save Source dialog.
//
// Qt-free and MFC-free, so the MFC code can include it.
#pragma once

class DataModel;

// Shows the modal "Save source files" dialog over the MFC owner window.
// The dialog drives the model's code generation directly. `ownerHwnd` is the
// MFC owner HWND as void*.
void Qt_ShowSaveSourceDialog(DataModel* pDataModel, void* ownerHwnd);
