// qt/QtVariableMethodDialog.h -- bridge into the Qt Variable->Method dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly; it produces a member/method access path for the editor.
#pragma once

#include "CbString.h"

class Method;

// Shows the modal Variable->Method() Wizard over the MFC owner window.
// `pMethod` is the method being edited, `code` its current (stripped) body
// text -- scanned for usable variable declarations. On OK, `insertCode`
// receives the chosen access path (e.g. "this->GetChild()->Update()") and the
// function returns true. `ownerHwnd` is the MFC owner HWND as void*.
bool Qt_ShowVariableMethodDialog(Method* pMethod, const char* code,
                                 CbString& insertCode, void* ownerHwnd);
