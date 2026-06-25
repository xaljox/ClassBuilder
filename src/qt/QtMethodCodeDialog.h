// qt/QtMethodCodeDialog.h -- bridge into the Qt Method Code editor.
//
// Qt-free and MFC-free, so the MFC code can include it.
#pragma once

class Method;

// Shows the modal code editor for a method body. The MFC dialog was modeless;
// this Qt port is modal (the host is still MFC). Drives the model directly --
// Save writes the body back. `ownerHwnd` is the MFC owner HWND as void*.
void Qt_ShowMethodCodeDialog(Method* pMethod, void* ownerHwnd);
