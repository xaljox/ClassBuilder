// qt/QtSelectMembersAndMethods.h -- bridge into the Qt Select Members and
// Methods dialog.
//
// Qt-free and MFC-free, so the MFC code can include it.
#pragma once

class ClassDiagram;

// Callback invoked after an in-dialog Apply so the host can refresh its views
// (the modal Qt dialog cannot reach the MFC document framework itself). The
// MFC caller passes a free function plus an opaque context (its CDocument*).
typedef void (*SelectMembersApplyCallback)(void* context);

// Shows the modal "select members/methods" dialog for a class diagram: a
// checkbox tree of each class shape's members and methods, controlling which
// appear on the shape. Applies on OK / Apply; Cancel rolls back. The caller
// should refresh its views afterwards regardless of outcome. `ownerHwnd` is
// the MFC owner HWND as void*. `onApply` (may be null) is called after each
// Apply so the host repaints while the dialog stays open.
void Qt_ShowSelectMembersAndMethodsDialog(ClassDiagram* pClassDiagram,
                                          void* ownerHwnd,
                                          SelectMembersApplyCallback onApply,
                                          void* applyContext);
