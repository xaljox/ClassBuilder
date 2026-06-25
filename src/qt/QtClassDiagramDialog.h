// qt/QtClassDiagramDialog.h -- MFC-callable entry point for the Qt
// ClassDiagram properties dialog.
//
// Qt-free and MFC-free: the MFC call site includes only this header.
#pragma once

class ClassDiagram;

// Shows the modal dialog. On OK, applies the edits to the model and reports
// via the out-params:
//   modelChangedOut -- a field changed; the caller should call Update().
//   sizeChangedOut  -- the page width/height changed; the caller re-zooms the
//                      (MFC-only) ClassDiagramView objects.
// Returns false if the user cancelled.
bool Qt_ShowClassDiagramDialog(ClassDiagram* pClassDiagram,
                               bool& modelChangedOut, bool& sizeChangedOut,
                               void* ownerHwnd);
