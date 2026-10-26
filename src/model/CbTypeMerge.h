// model/CbTypeMerge.h -- the "merge one type into another" model operation,
// shared by the merge_types command (CbCommandServer) and the Merge Types GUI
// dialog (qt/MergeTypesDialog). Qt-free so the MFC-free model can call it.
//
// Folds OtherType `from` into the existing OtherType `to`: re-points every
// member / argument / method-return typed `from` onto `to` (modifiers preserved),
// optionally whole-word-migrates `from`->`to` in all code text, then deletes
// `from`. One undo step (MarkLastUndo) + marks the model modified.
#pragma once

#include <string>

class DataModelDoc;

struct CbMergeResult
{
    int variablesMoved = 0;   // members + arguments + method returns re-pointed
    int bodies         = 0;   // whole-word replacements in method bodies
    int userBlocks     = 0;   // ... in //@START_USER* blocks (classes + master)
    int inits          = 0;   // ... in per-member initializations
};

// Returns true on success. On a validation failure (from/to missing, same,
// not an OtherType, a built-in as `from`, or `to` already... — see the .cpp)
// returns false and sets `error`. `replaceInCode` defaults on at the call sites
// because `from` is deleted, so a leftover literal `from` would not compile.
bool Cb_MergeOtherType(DataModelDoc* pDoc,
                       const std::string& from,
                       const std::string& to,
                       bool replaceInCode,
                       CbMergeResult& out,
                       std::string& error);
