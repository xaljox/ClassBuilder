// qt/QtSelectClassesDialog.h -- bridge into the Qt Select Classes dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly.
#pragma once

class ClassDiagram;

// Shows the modal "Select classes of class diagram" dialog over the MFC owner
// window: a checkbox tree of the model's classes; on OK the diagram's class
// shapes are added / removed to match the ticked set. Returns true if OK was
// pressed. `ownerHwnd` is the MFC owner HWND as void*.
bool Qt_ShowSelectClassesDialog(ClassDiagram* pClassDiagram, void* ownerHwnd);
