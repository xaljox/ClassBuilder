// model/CbUpdate.h -- Update Extern Classes: refresh the target project's
// ExternClasses from the same-named Classes of another open project.
//
// Qt-free; shared by the update_plan / update_execute commands and the
// Project > Update Extern Classes dialog. Matching is BY NAME (no origin link):
// a target ExternClass `Foo` is updated from the source's Class `Foo`.
//
// The extern takes the source class's shape: EVERY member with its getter /
// setter (as the Member dialog has them), and the non-private methods minus
// the relation / macro ones (as CB's own Class -> Extern conversion keeps
// them), with empty bodies and init lists.
// Differences are applied IN PLACE -- a changed member or method keeps its
// object, so sequence-diagram messages and class-diagram shapes on it survive.
// A method whose arguments changed but whose name is unique on both sides is
// such a change, not a remove + add. Types the new interface uses that the
// target lacks come along: an OtherType copy (with its declaration) for a
// source OtherType, a bare ExternClass stub for a source class.
#pragma once

#include <string>
#include <vector>

class DataModelDoc;

// One difference between a target extern and its source class.
struct CbUpdateChange
{
    std::string what;     // "member" | "method" | "base" | "class" | "type"
    char        op;       // '+' added, '-' removed, '~' changed
    std::string before;   // the extern's current text ('-' and '~')
    std::string after;    // the source's text ('+' and '~')
    int         sdUses;   // '-' / '~' method: sequence-diagram messages on it
};

struct CbUpdateRow
{
    std::string                 name;
    std::vector<CbUpdateChange> changes;  // empty = up to date
    std::vector<std::string>    notes;    // what an update cannot carry over
};

struct CbUpdatePlan
{
    std::vector<CbUpdateRow> rows;        // every target extern with a same-named source Class
};

// Every target ExternClass that has a same-named Class in pSrc, with the
// differences an update would apply. Sorted by name.
void Cb_UpdatePlan(DataModelDoc* pSrc, DataModelDoc* pTgt, CbUpdatePlan& plan);

struct CbUpdateResult
{
    int externs = 0;                      // externs that changed
    int added = 0, removed = 0, changed = 0;
    int types = 0;                        // new types created in the target
    std::vector<std::string> notes;
};

// Update the named externs (one undo step). Names without a source Class or
// without differences are skipped.
bool Cb_UpdateExecute(DataModelDoc* pSrc, DataModelDoc* pTgt,
                      const std::vector<std::string>& names,
                      CbUpdateResult& out, std::string& error);
