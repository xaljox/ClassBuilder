// qt/QtCodeEditor.h -- MFC-free bridge for the modeless code editors.
//
// The Method / Constructor code editors are modeless (non-blocking) Qt windows.
// Each registers itself on its Method via Method::SetOpenDialog() so reopening
// the same method refocuses the existing window instead of spawning another.
// When the method object is destroyed, the model calls Qt_CloseCodeEditor() with
// the stored dialog so the editor can't outlive (and dereference) a dead method.
//
// Header is Qt-include-free so the model code (Method.cpp) can include it;
// QDialog is only forward-declared (same as the model's own headers).
#pragma once

class QDialog;
class CbString;
class Method;

// Force-close the open code editor returned by Method::GetOpenDialog().
// Detaches it from the model first, so no save prompt runs and the dying
// method is never touched again. No-op on null.
void Qt_CloseCodeEditor(QDialog* pDialog);

// The model rewrote this method's stored code with a whole-identifier
// replacement (argument / member / type rename -- Method::ReplaceInCode).
// Mirror that replacement into the open editor's text so the window tracks
// the model: unsaved edits get the rename applied too, and an unedited
// editor stays byte-identical to the stored code (no phantom save prompt
// on close). No-op on null.
void Qt_ReplaceInOpenCodeEditor(QDialog* pDialog, const CbString& oldString,
                                const CbString& newString);

// Undo/redo restored this method's state behind the open editor
// (Method/Constructor::OnUndoRedoChanged). pOldState is the pre-restore
// state object. If the editor text matches the pre-restore code (no unsaved
// edits) it silently reloads the restored code; edited text is left alone --
// the close prompt then reports a real difference. No-op on null.
void Qt_UndoRedoOpenCodeEditor(QDialog* pDialog, Method* pOldState);
