// qt/QtClassShapeOrderDialog.h -- bridge into the Qt Class Shape Order dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live ClassShape*).
#pragma once

class ClassShape;

// Shows the modal Class Shape Order dialog over the MFC owner window: two
// single-select lists (members / methods) with Move Up / Move Down buttons
// reordering how the diagram shape lists them. Returns true if OK was pressed.
// `ownerHwnd` is the MFC owner HWND as void*.
bool Qt_ShowClassShapeOrderDialog(ClassShape* pClassShape, void* ownerHwnd);
