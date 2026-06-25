// qt/QtNoteShapeDialog.h -- bridge into the Qt Note properties dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. One dialog serves both
// the class-diagram note (NoteShape) and the sequence-diagram note
// (SDNoteShape) -- hence the two overloads.
#pragma once

class NoteShape;
class SDNoteShape;

// Show the modal Note properties dialog over the MFC owner window. The dialog
// applies accepted edits to the shape itself; `modelChangedOut` reports
// whether anything changed (so the caller can refresh the views). `ownerHwnd`
// is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowNoteShapeDialog(NoteShape* pNoteShape,
                            bool& modelChangedOut, void* ownerHwnd);
bool Qt_ShowNoteShapeDialog(SDNoteShape* pSDNoteShape,
                            bool& modelChangedOut, void* ownerHwnd);
