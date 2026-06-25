// qt/QtReadSourceDialog.h -- bridge into the Qt Read Source dialog.
//
// Qt-free and MFC-free, so the MFC code can include it.
#pragma once

class DataModel;

// Shows the modal "Read source files" dialog over the MFC owner window. The
// dialog reads the on-disk sources back into the model (it auto-reads
// modifications on open). `ownerHwnd` is the MFC owner HWND as void*.
void Qt_ShowReadSourceDialog(DataModel* pDataModel, void* ownerHwnd);
