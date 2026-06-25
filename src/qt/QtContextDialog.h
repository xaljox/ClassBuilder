// qt/QtContextDialog.h -- bridge into the Qt "Assign Context" dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live Gti*).
#pragma once

class Gti;

// Shows the modal "Assign Context" dialog over the MFC owner window. The
// dialog APPLIES each Add/Remove live (CreateContext / Context::Delete) -- the
// caller is responsible for the MarkLastUndo / RollBack wrapper so Cancel
// undoes the sequence. `ownerHwnd` is the MFC owner HWND as void*. Returns
// true if OK was pressed, false on Cancel.
bool Qt_ShowContextDialog(Gti* pGti, void* ownerHwnd);
