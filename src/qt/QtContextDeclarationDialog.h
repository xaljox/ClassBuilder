// qt/QtContextDeclarationDialog.h -- bridge into the Qt Context Declarations
// dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly; the caller wraps it in MarkLastUndo / RollBack so
// Cancel undoes the session.
#pragma once

class DataModel;

// Shows the modal Context Declarations dialog over the MFC owner window.
// Returns true if OK was pressed. `ownerHwnd` is the MFC owner HWND as void*.
bool Qt_ShowContextDeclarationDialog(DataModel* pDataModel, void* ownerHwnd);
