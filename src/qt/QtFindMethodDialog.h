// qt/QtFindMethodDialog.h -- bridge into the Qt Find Method dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly.
#pragma once

class FindMethod;

// Shows the modal Find Method dialog over the MFC owner window: edits the
// attributes and argument map of `pFindMethod`. Returns true if OK was
// pressed. On OK, `fieldsChanged` is set when an attribute (name / note /
// access / ...) actually changed -- the caller then calls FindMethod::Update.
// `ownerHwnd` is the MFC owner HWND as void*.
bool Qt_ShowFindMethodDialog(FindMethod* pFindMethod, void* ownerHwnd,
                             bool& fieldsChanged);
