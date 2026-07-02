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

// Export `pClassDiagram` to a standalone .svg (vector, selection-free, page
// extent). Reuses the diagram's open canvas when one exists, else opens the
// view first (same path as a tree double-click). Returns false when the Qt
// Svg module is unavailable or the write failed. Pipe-API backend
// (export_diagram_svg); `path` is 8-bit local encoding.
bool Qt_ExportClassDiagramSvg(ClassDiagram* pClassDiagram, const char* path);
