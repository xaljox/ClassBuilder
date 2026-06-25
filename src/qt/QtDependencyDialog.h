// qt/QtDependencyDialog.h -- bridge into the Qt Dependency properties dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live DependencyShape*).
#pragma once

class DependencyShape;
class ClassDiagram;
class ClassShape;

// Shows the modal Dependency properties dialog over the MFC owner window. The
// dialog applies accepted edits to `pDependencyShape` itself; `modelChangedOut`
// reports whether anything changed (so the caller can refresh the views).
// `ownerHwnd` is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowDependencyDialog(DependencyShape* pDependencyShape,
                             bool& modelChangedOut, void* ownerHwnd);

// "Create" entry point: the dialog gathers the spec (no shape exists during it);
// the shape is created in `pClassDiagram` ONLY after OK and the spec applied.
// `initFrom` / `initTo` pre-seed the combos (either may be null). Returns the
// newly-created shape, or nullptr if cancelled.
DependencyShape* Qt_CreateDependencyDialog(
    ClassDiagram* pClassDiagram, ClassShape* initFrom, ClassShape* initTo,
    void* ownerHwnd);
