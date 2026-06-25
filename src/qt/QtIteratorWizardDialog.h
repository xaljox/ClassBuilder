// qt/QtIteratorWizardDialog.h -- bridge into the Qt Iterator Wizard dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly; it produces a snippet of C++ to splice into the editor.
#pragma once

#include "CbString.h"

class Method;

// Shows the modal Iterator Wizard over the MFC owner window. `pMethod` is the
// method being edited, `code` its current (stripped) body text -- scanned for
// usable variable declarations. On OK, `insertCode` receives the generated
// iterator snippet and the function returns true. `ownerHwnd` is the MFC
// owner HWND as void*.
bool Qt_ShowIteratorWizardDialog(Method* pMethod, const char* code,
                                 CbString& insertCode, void* ownerHwnd);
