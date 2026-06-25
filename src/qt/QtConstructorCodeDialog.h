// qt/QtConstructorCodeDialog.h -- bridge into the Qt constructor code editor.
//
// MFC-free and Qt-free, so the MFC model code (Constructor.cpp) can include
// it. Ported from the modeless MFC CConstructorCodeDialog; this Qt port is
// MODAL (the MFC host still owns the message loop -- the modeless flip waits
// for the full-Qt stage).
#pragma once

class Constructor;

// Shows the modal constructor code editor on `pConstructor`. The dialog drives
// the model directly (Save writes the init list + body back). `ownerHwnd` is
// the MFC owner window (an HWND passed as void* to keep this header MFC-free).
void Qt_ShowConstructorCodeDialog(Constructor* pConstructor, void* ownerHwnd);
