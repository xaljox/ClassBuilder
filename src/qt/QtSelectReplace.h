// qt/QtSelectReplace.h -- bridge into the Qt "select & replace" dialog.
//
// MFC-free and Qt-free, so the MFC model code (Method.cpp) can include it.
// Ported from the MFC SelectReplace: when a method is renamed and its old
// name is referenced in other method bodies, this dialog lists every affected
// method (grouped by class, with checkboxes) so the user can pick which
// bodies get the rename rippled through. Double-clicking a method opens its
// code editor.
#pragma once

#include "CbString.h"

class DataModel;

// Shows the modal select-&-replace dialog. `oldString` / `newString` are the
// strings to ripple (the old and new method name). On OK, the checked
// methods have `oldString` replaced with `newString` in their code.
// `ownerHwnd` is the MFC owner HWND passed as void*.
void Qt_ShowSelectReplace(DataModel* pDataModel,
                          const CbString& oldString,
                          const CbString& newString,
                          void* ownerHwnd);
