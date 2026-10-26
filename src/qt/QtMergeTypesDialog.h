// qt/QtMergeTypesDialog.h -- Qt-free bridge into the "Merge Types" action.
// Fold one OtherType into another existing one: every use of the first is
// retyped to the second, then the first is deleted. The GUI counterpart of the
// merge_types command; both drive the same model op (Cb_MergeOtherType).
//
// Qt-free signature so the MFC-free model (ClassBuilderDoc) can call it.
#pragma once

class DataModelDoc;

// Modal over ownerHwnd. Pick a type to merge away and the type to merge it into;
// OK re-points every member/argument/return to the second type, migrates the
// name in code, and deletes the first -- all as ONE undo step. Shows an info box
// (and does nothing else) when the model has fewer than two other types.
void Qt_ShowMergeTypesDialog(DataModelDoc* pDoc, void* ownerHwnd);
