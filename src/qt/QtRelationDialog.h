// qt/QtRelationDialog.h -- bridge into the Qt Relation dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly.
#pragma once

class Relation;

// Shows the modal Relation attributes dialog over the MFC owner window.
// Returns true if OK was pressed; on OK `changed` is set when a field
// actually changed -- the caller then calls Relation::Update. `ownerHwnd`
// is the MFC owner HWND as void*.
bool Qt_ShowRelationDialog(Relation* pRelation, bool& changed,
                           void* ownerHwnd);
