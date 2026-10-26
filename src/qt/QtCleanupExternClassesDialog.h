// qt/QtCleanupExternClassesDialog.h -- Qt-free bridge into the "Clean Up Extern
// Classes" action. Finds the ExternClasses that NOTHING uses (no class inherits
// from them, no member/argument is typed as them) and offers to delete them --
// the GUI counterpart of the list_unused_extern_classes command.
//
// Qt-free signature so the MFC-free model (ClassBuilderDoc) can call it.
#pragma once

class DataModelDoc;

// Modal over ownerHwnd. Lists the unused ExternClasses (default-checked), deletes
// the checked ones as ONE undo step, then -- when a deletion freed FURTHER unused
// externs (an extern used only by a now-deleted extern) -- offers to run again.
// Shows an info box, and does nothing else, when none are unused.
void Qt_ShowCleanupExternClassesDialog(DataModelDoc* pDoc, void* ownerHwnd);

// Same, for unused OtherTypes (types no member/argument uses; the protected
// built-ins like void/int are excluded by the model's own delete gate). No
// re-run: other types reference nothing, so a deletion never frees more.
void Qt_ShowCleanupOtherTypesDialog(DataModelDoc* pDoc, void* ownerHwnd);
