// qt/QtUpdateExternClassesDialog.h -- Qt-free bridge into the "Update Extern
// Classes" action. Refresh this project's extern classes from the same-named
// classes of another OPEN project -- the GUI over the CbUpdate engine
// (model/CbUpdate.h), shared with the update_plan / update_execute commands.
//
// Qt-free signature so the MFC-free model (ClassBuilderDoc) can call it.
#pragma once

class DataModelDoc;

// Modal over ownerHwnd. Target = pTarget (the active project); source = another
// open project (a dropdown when there is more than one). Lists every extern class
// with a same-named class in the source and what an update changes; the checked
// ones are updated as ONE undo step. Shows an info box (and does nothing else)
// when no other project is open or no extern class matches a source class.
void Qt_ShowUpdateExternClassesDialog(DataModelDoc* pTarget, void* ownerHwnd);
