// qt/QtClassDiagramView.h -- bridge into the Qt ClassDiagram view.
//
// MFC-free, Qt-free. Included by the MFC CClassDiagramView to launch the
// Qt-rendered counterpart side-by-side (M1: non-modal, read-only, live-mirrored
// through the ClassDiagramViewModel). Mirrors qt/QtSequenceDiagramView.h.
// Interaction (selection, drag, dialogs) lands in M2+.
#pragma once

class ClassDiagram;

// Show the Qt rendering of `pClassDiagram` as a non-modal top-level window.
// `ownerHwnd` is the MFC owner HWND as void*. Returns immediately.
void Qt_ShowClassDiagramView(ClassDiagram* pClassDiagram, void* ownerHwnd);
