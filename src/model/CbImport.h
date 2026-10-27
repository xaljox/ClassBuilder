// model/CbImport.h -- the cross-project Import / Clone model operation, shared by
// the import_plan / import_execute commands and the Import / Clone GUI dialog.
// Qt-free so the MFC-free model can call it.
//
// One unified feature (Clone + Import collapse): every entity of the SOURCE
// project whose name is absent in the TARGET project is a candidate row, and the
// user picks the FORM it is imported as:
//   Class        full plain clone -- members, user methods, constructors,
//                destructor body, bases, relations between imported classes
//   ExternClass  a non-recursive stub (name / template / struct / forward decl)
//   OtherType    a stub type (an OtherType source carries its declaration)
// Names present in the target are reused, never duplicated. Serialize never
// travels: classes always import plain; add serialize in the target afterwards.
//
// Dependencies of a Class import become requirements on other rows:
//   member / argument / return / exception type  -> MANDATORY, min OtherType
//   base class                                   -> optional,  min ExternClass
//                                                   (decline it = the base lapses)
//   relation partner                             -> suggestion only (Class keeps
//                                                   the relation, else it drops)
// Stubs are non-recursive, so they bound the cascade; only a Class pick recurses.
#pragma once

#include <string>
#include <vector>

class DataModelDoc;

enum CbImportForm
{
    CB_IMPORT_NONE        = 0,
    CB_IMPORT_OTHERTYPE   = 1,
    CB_IMPORT_EXTERNCLASS = 2,
    CB_IMPORT_CLASS       = 3
};

struct CbImportRow
{
    std::string name;
    int  sourceKind    = CB_IMPORT_NONE;  // richest form available (the source's own kind)

    // User input -- set by the dialog / command, kept across Cb_ImportResolve.
    int  explicitForm  = CB_IMPORT_NONE;  // the user's own pick
    bool optOut        = false;           // user declined an optional (base) requirement

    // Derived by Cb_ImportResolve.
    int  requiredForm  = CB_IMPORT_NONE;  // minimum form other imports need
    bool mandatory     = false;           // ... from a type use -> cannot be dropped
    int  suggestedForm = CB_IMPORT_NONE;  // richer form worth upgrading to (grey)
    std::string requiredBy;               // "type in Row; base of Cell"
    int  effectiveForm = CB_IMPORT_NONE;  // what will actually be imported
};

struct CbImportPlan
{
    std::vector<CbImportRow> rows;        // candidates, sorted Class / Extern / Other, then name
    std::vector<std::string> notes;       // dropped relations, lapsed bases
    int classes = 0, externClasses = 0, otherTypes = 0;   // effective counts
    int relationsKept = 0, relationsDropped = 0;
};

// Fill plan.rows with the candidates: every source Class / ExternClass / OtherType
// (in the tree) whose name the target lacks. Clears any previous picks.
void Cb_ImportCandidates(DataModelDoc* pSrc, DataModelDoc* pTgt, CbImportPlan& plan);

// Recompute the derived fields of every row (and the counts / notes) from the
// rows' explicitForm + optOut. Cheap; call after every user change.
void Cb_ImportResolve(DataModelDoc* pSrc, DataModelDoc* pTgt, CbImportPlan& plan);

struct CbImportResult
{
    int classes = 0, externClasses = 0, otherTypes = 0;
    int members = 0, methods = 0, constructors = 0, inherits = 0, relations = 0;
    std::vector<std::string> notes;
};

// Execute a resolved plan into the target as ONE undo step. Returns false and
// sets `error` only when nothing could be done (bad documents / nothing picked).
bool Cb_ImportExecute(DataModelDoc* pSrc, DataModelDoc* pTgt, const CbImportPlan& plan,
                      CbImportResult& out, std::string& error);

// Form <-> text, shared by the commands ("class" / "extern_class" / "other_type").
const char* Cb_ImportFormName(int form);
int         Cb_ImportFormFromName(const std::string& name);
