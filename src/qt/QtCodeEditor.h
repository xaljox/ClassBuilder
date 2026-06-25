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

// Force-close the open code editor returned by Method::GetOpenDialog().
// Detaches it from the model first, so no save prompt runs and the dying
// method is never touched again. No-op on null.
void Qt_CloseCodeEditor(QDialog* pDialog);
