// qt/QtSequenceDiagramView.h -- bridge into the Qt SequenceDiagram view.
//
// MFC-free, Qt-free. Included by the MFC SD view to launch the Qt-rendered
// counterpart side-by-side (M1: modal, read-only, no MDI plumbing). The
// modal restriction is transitional -- once the Qt view grows interactive
// features (M2+) the embedded path under MFC MDI replaces this.
#pragma once

class SequenceDiagram;

// Show the Qt rendering of `pSD` modally. `ownerHwnd` is the MFC owner
// HWND as void*. Returns when the user closes the Qt window.
void Qt_ShowSequenceDiagramView(SequenceDiagram* pSD, void* ownerHwnd);

// Export `pSD` to a standalone .svg (vector, selection-free, page extent).
// Reuses the diagram's open canvas when one exists, else opens the view first
// (same path as a tree double-click). Returns false when the Qt Svg module is
// unavailable or the write failed. Pipe-API backend (export_diagram_svg);
// `path` is 8-bit local encoding.
bool Qt_ExportSequenceDiagramSvg(SequenceDiagram* pSD, const char* path);
