// qt/QtUserCodeDialog.h -- bridge into the Qt User Code dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly and applies on OK.
#pragma once

class Class;

// Shows the modal User Section editor for one of a class's six user-code
// sections (header / cpp x section 1..3). Applies on OK; nothing returned.
// `start` / `end` optionally pre-select a character range in the code.
// `ownerHwnd` is the MFC owner HWND as void*.
void Qt_ShowUserCodeDialog(Class* pClass, int section, bool header,
                           void* ownerHwnd, int start = 0, int end = 0);
