// qt/QtLifeLineDialog.h -- bridge into the Qt Life Line dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The Qt dialog drives
// the model directly. Two entry points, mirroring the two MFC constructors:
//   * edit  -- edit an existing life-line shape;
//   * new   -- pick an actor / class for a life line about to be created.
#pragma once

#include "CbString.h"

class LifeLineShape;
class DataModelDoc;
class Actor;
class BaseClass;

// Edit an existing life-line shape. Applies on OK; nothing returned.
void Qt_ShowLifeLineDialog(LifeLineShape* pLifeLineShape, void* ownerHwnd);

// Pick the actor / class for a NEW life line. Returns true on OK; exactly one
// of `pActor` / `pBaseClass` is set, `templ` carries the template reference
// when a (templated) class was chosen, `showActivations` carries the
// show-activations choice (defaults on), and `name` / `note` carry the
// typed-in name and note -- all to apply to the created shape.
bool Qt_ShowLifeLineDialogNew(DataModelDoc* pDataModelDoc, void* ownerHwnd,
                              Actor*& pActor, BaseClass*& pBaseClass,
                              CbString& templ, bool& showActivations,
                              CbString& name, CbString& note);
