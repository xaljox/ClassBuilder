// qt/QtRelationDiagramOnlyDialog.h -- bridge into the Qt "Relation
// ClassDiagram Only" dialog.
//
// Qt-free and MFC-free, so the MFC code can include it.
#pragma once

class RelationDiagramOnlyShape;
class ClassDiagram;
class ClassShape;

// Shows the modal "Relation ClassDiagram Only" properties dialog over the MFC
// owner window. The dialog applies accepted edits to the shape itself;
// `modelChangedOut` reports whether anything changed (caller refreshes views).
// `ownerHwnd` is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowRelationDiagramOnlyDialog(RelationDiagramOnlyShape* pShape,
                                      bool& modelChangedOut, void* ownerHwnd);

// "Create" entry point: the dialog gathers the spec (no shape exists during it);
// the shape is created in `pClassDiagram` ONLY after OK and the spec applied.
// `initFrom` / `initTo` pre-seed the combos (either may be null). Returns the
// newly-created shape, or nullptr if cancelled.
RelationDiagramOnlyShape* Qt_CreateRelationDiagramOnlyDialog(
    ClassDiagram* pClassDiagram, ClassShape* initFrom, ClassShape* initTo,
    void* ownerHwnd);
