// qt/QtActorDialog.h -- bridge into the Qt Actor properties dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly (it is handed the live Actor*).
#pragma once

class Actor;

// Shows the modal Actor properties dialog over the MFC owner window. The
// dialog applies accepted edits to `pActor` itself; `modelChangedOut` reports
// whether anything changed (so the caller can run Actor::Update). `ownerHwnd`
// is the MFC owner HWND as void*. Returns true if OK was pressed.
bool Qt_ShowActorDialog(Actor* pActor, bool& modelChangedOut, void* ownerHwnd);
