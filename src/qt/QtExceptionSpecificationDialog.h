// qt/QtExceptionSpecificationDialog.h -- bridge into the Qt Exception
// Specification dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly. Each edit mutates the model immediately; the caller
// wraps the call in MarkLastUndo / RollBack so Cancel undoes the sequence.
#pragma once

class Method;

// Shows the modal Exception Specification dialog over the MFC owner window:
// edits the throw clause of `pMethod`. Returns true if OK was pressed, false
// on Cancel (the caller rolls the model back on false). `ownerHwnd` is the
// MFC owner HWND as void*.
bool Qt_ShowExceptionSpecificationDialog(Method* pMethod, void* ownerHwnd);
