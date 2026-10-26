// model/CbTypeMerge.cpp -- see CbTypeMerge.h. The shared merge operation behind
// the merge_types command and the Merge Types dialog.
#include "StdAfx.h"
#include "CbTypeMerge.h"
#include "ClassBuilderInclude.h"   // model classes + iterators

#include <string>

namespace {
    std::string ToStd(const CbString& s) { return std::string((const char*)s); }

    // Whole-word (C-identifier boundary) helpers over std::string; a portable
    // mirror of the model's FindStringInStr (__iscsym), so "int" never matches
    // inside "int61".
    bool IsIdent(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == '_';
    }
    int CountWholeWord(const std::string& hay, const std::string& word)
    {
        if (word.empty()) return 0;
        int n = 0; size_t pos = 0;
        while ((pos = hay.find(word, pos)) != std::string::npos)
        {
            size_t end = pos + word.size();
            bool lOk = (pos == 0)          || !IsIdent(hay[pos - 1]);
            bool rOk = (end >= hay.size()) || !IsIdent(hay[end]);
            if (lOk && rOk) ++n;
            pos = end;
        }
        return n;
    }
    int ReplaceWholeWord(std::string& hay, const std::string& word, const std::string& to)
    {
        if (word.empty()) return 0;
        int n = 0; size_t pos = 0;
        while ((pos = hay.find(word, pos)) != std::string::npos)
        {
            size_t end = pos + word.size();
            bool lOk = (pos == 0)          || !IsIdent(hay[pos - 1]);
            bool rOk = (end >= hay.size()) || !IsIdent(hay[end]);
            if (lOk && rOk) { hay.replace(pos, word.size(), to); pos += to.size(); ++n; }
            else pos = end;
        }
        return n;
    }
}

bool Cb_MergeOtherType(DataModelDoc* pDoc,
                       const std::string& from,
                       const std::string& to,
                       bool replaceInCode,
                       CbMergeResult& out,
                       std::string& error)
{
    out = CbMergeResult();
    if (!pDoc) { error = "no active document"; return false; }
    if (from.empty() || to.empty()) { error = "missing 'from' or 'to'"; return false; }
    if (from == to) { error = "'from' and 'to' are the same type"; return false; }

    Type* pFromT = pDoc->FindType(CbString(from.c_str()));
    if (!pFromT) { error = "no such type: " + from; return false; }
    Type* pToT = pDoc->FindType(CbString(to.c_str()));
    if (!pToT) { error = "no such type: " + to; return false; }

    // v1: OtherType -> OtherType only (merging a Class/ExternClass is the Import
    // resolve-by-name case, a separate feature).
    OtherType* pFrom = dynamic_cast<OtherType*>(pFromT);
    OtherType* pTo   = dynamic_cast<OtherType*>(pToT);
    if (!pFrom) { error = "'" + from + "' is not an OtherType (merge is OtherType->OtherType)"; return false; }
    if (!pTo)   { error = "'" + to + "' is not an OtherType (merge is OtherType->OtherType)"; return false; }
    if (from == "void" || from == "int")
    { error = "cannot merge away built-in type '" + from + "'"; return false; }

    DataModel* pDataModel = pDoc->GetDataModel();
    if (!pDataModel) { error = "no active document"; return false; }

    pDoc->MarkLastUndo();

    // 1) Re-point every variable (member / argument / method-return -- a Method is
    //    a Variable) from `from` onto `to`. Drain via GetFirstVariable so mutating
    //    the owned list can't invalidate a live iterator.
    while (Variable* v = pFrom->GetFirstVariable())
    {
        pTo->MoveVariableLast(v);
        v->Update();
        ++out.variablesMoved;
    }

    // 2) Code-text migration (counts pre-measured for the report).
    if (replaceInCode)
    {
        DataModel::ClassIterator iClass(pDataModel);
        while (++iClass)
        {
            BaseClass::MethodIterator iM(iClass);
            while (++iM) out.bodies += CountWholeWord(ToStd(iM->GetCode()), from);
            out.userBlocks += CountWholeWord(ToStd(iClass->GetHUser1()),   from);
            out.userBlocks += CountWholeWord(ToStd(iClass->GetHUser2()),   from);
            out.userBlocks += CountWholeWord(ToStd(iClass->GetHUser3()),   from);
            out.userBlocks += CountWholeWord(ToStd(iClass->GetCppUser1()), from);
            out.userBlocks += CountWholeWord(ToStd(iClass->GetCppUser2()), from);
            out.userBlocks += CountWholeWord(ToStd(iClass->GetCppUser3()), from);
        }
        out.userBlocks += CountWholeWord(ToStd(pDataModel->GetHUser1()), from);
        out.userBlocks += CountWholeWord(ToStd(pDataModel->GetHUser2()), from);

        // The ReplaceInX cascade (what Type::SetName runs): bodies+notes,
        // constructor init-lists, OtherType declarations, user blocks, templates.
        CbString fromCb(from.c_str()), toCb(to.c_str());
        DataModelDoc::DataModelDocObjectIterator iObj(pDoc);
        while (++iObj)
            iObj->ReplaceInX(fromCb, toCb);

        // Per-member initialization -- the one surface ReplaceInX misses
        // (Variable::ReplaceInX only touches _template).
        DataModel::ClassIterator iClass2(pDataModel);
        while (++iClass2)
        {
            BaseClass::MemberIterator iMem(iClass2);
            while (++iMem)
            {
                std::string s = ToStd(iMem->GetInitialization());
                int n = ReplaceWholeWord(s, from, to);
                if (n)
                {
                    iMem->SetInitialization(CbString(s.c_str()));
                    iMem->Update();
                    out.inits += n;
                }
            }
        }
    }

    // 3) `from` is now unreferenced -> delete it.
    pFrom->Delete();

    pDoc->SetModifiedFlag(1);
    return true;
}
