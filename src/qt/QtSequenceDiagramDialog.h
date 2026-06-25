// qt/QtSequenceDiagramDialog.h -- bridge into the Qt SequenceDiagram dialog.
//
// Qt-free and MFC-free, so the MFC code can include it.
#pragma once

class SequenceDiagram;

// Shows the modal SequenceDiagram properties dialog over the MFC owner window.
// The dialog applies accepted edits to `pSequenceDiagram` itself.
//   modelChangedOut -- anything changed (caller runs SequenceDiagram::Update).
//   sizeChangedOut  -- the page width/height changed; the caller must re-zoom
//                      the sequence-diagram views (a GUI step the Qt dialog
//                      cannot do -- SequenceDiagramView is MFC-only).
// `ownerHwnd` is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowSequenceDiagramDialog(SequenceDiagram* pSequenceDiagram,
                                  bool& modelChangedOut, bool& sizeChangedOut,
                                  void* ownerHwnd);
