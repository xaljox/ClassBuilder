// qt/QtSimilarLinesDialog.h -- bridge into the Qt Insert Similar Lines dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly; it produces a block of generated code for the editor.
#pragma once

#include "CbString.h"

class BaseClass;

// Shows the modal "Insert similar lines" dialog over the MFC owner window:
// expands a line template over the (checked) members of a class. On OK,
// `code` receives the generated block and the function returns true.
// `ownerHwnd` is the MFC owner HWND as void*.
bool Qt_ShowSimilarLinesDialog(BaseClass* pBaseClass, CbString& code,
                               void* ownerHwnd);
