// qt/QtClassShapeDialog.h -- bridge into the Qt Class Shape dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live ClassShape*).
#pragma once

class ClassShape;

// Shows the modal Class Shape dialog over the MFC owner window: two checkbox
// lists (members / methods) picking which of the class's members and methods
// the diagram shape displays. Returns true if the user pressed OK.
// `ownerHwnd` is the MFC owner HWND as void*.
bool Qt_ShowClassShapeDialog(ClassShape* pClassShape, void* ownerHwnd);
