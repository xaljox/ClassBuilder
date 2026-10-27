// model/CbUpdate.cpp -- see CbUpdate.h. The Update Extern Classes engine.
//
// One diff routine (Diff) serves both the plan and the execute step: execute
// re-diffs each extern against the live model, so it always acts on current
// pointers. Copies go through the model's cross-project copy-constructors
// (Member / Method / Constructor / Argument / Inherit), resolving types through
// the target's FindOrDuplicateType after the missing types were pre-created,
// so that call only ever FINDS (its fallback would duplicate a class as an
// extern carrying its whole interface, recursively).
#include "StdAfx.h"
#include "CbUpdate.h"
#include "ClassBuilderInclude.h"   // model classes + iterators
#include "CbViewLock.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <string>
#include <vector>

namespace {

std::string ToStd(const CbString& s) { return std::string((const char*)s); }

std::string Trim(std::string s)
{
    while (!s.empty() && std::isspace((unsigned char)s.back()))
        s.pop_back();
    size_t b = 0;
    while (b < s.size() && std::isspace((unsigned char)s[b]))
        ++b;
    return s.substr(b);
}

bool LessNoCase(const std::string& a, const std::string& b)
{
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
        [](char x, char y) { return std::tolower((unsigned char)x) < std::tolower((unsigned char)y); });
}

// "" is the constructors' return type, "..." the internal varargs type.
bool IsSkippableTypeName(const std::string& n) { return n.empty() || n == "..."; }

// The serialize runtime root; an extern never needs it as a base.
bool IsSerializeRoot(const std::string& name) { return name == "CbObject"; }

const char* AccessWord(AccessType a)
{
    switch (a)
    {
    case PUBLIC:    return "public";
    case PROTECTED: return "protected";
    case PRIVATE:   return "private";
    default:        return "";
    }
}

std::string TypeText(Variable* v) { return Trim(ToStd(v->GetTypeName())); }

// --- texts: what the dialog shows, and what "changed" compares --------------

// A member's own type text: what its getter / setter signatures derive from.
std::string MemberTypeText(Member* m)
{
    return std::string(m->GetStatic() ? "static " : "") + TypeText(m);
}

std::string MemberText(Member* m)
{
    std::string s = std::string(AccessWord(m->GetAccess())) + " ";
    if (m->GetStatic())  s += "static ";
    if (m->GetMutable()) s += "mutable ";
    s += TypeText(m) + " " + ToStd(m->GetName());
    if (m->GetArray())    s += "[" + ToStd(m->GetArraySizeStr()) + "]";
    if (m->GetBitField()) s += " : " + std::to_string(m->GetBitFieldSize());
    // The getter / setter belong to the member (Member dialog), not to the
    // method list.
    std::string acc;
    if (GetMemberMethod* g = m->GetGetMemberMethod())
        acc += std::string("get ") + AccessWord(g->GetAccess());
    if (SetMemberMethod* st = m->GetSetMemberMethod())
        acc += std::string(acc.empty() ? "" : ", ") + "set " + AccessWord(st->GetAccess());
    if (!acc.empty())
        s += "  [" + acc + "]";
    return s;
}

std::string ArgsText(Method* m)
{
    std::string s;
    Method::ArgumentIterator iArg(m);
    while (++iArg)
    {
        if (!s.empty()) s += ", ";
        s += TypeText(iArg);
        const std::string name = ToStd(iArg->GetName());
        if (!name.empty())
            s += " " + name;
        if (!iArg->GetDefault().IsEmpty())
            s += " = " + ToStd(iArg->GetDefault());
    }
    return s;
}

std::string ThrowText(Method* m)
{
    ExceptionSpecification* es = m->GetExceptionSpecification();
    return es ? ToStd(es->GetThrowString()) : std::string();
}

std::string MethodText(Method* m)
{
    std::string s = std::string(AccessWord(m->GetAccess())) + " ";
    if (m->GetStatic())  s += "static ";
    if (m->GetVirtual()) s += "virtual ";
    if (Constructor* c = dynamic_cast<Constructor*>(m))
        if (c->GetExplicit()) s += "explicit ";
    const std::string ret = TypeText(m);
    if (!ret.empty())
        s += ret + " ";
    s += ToStd(m->GetName()) + "(" + ArgsText(m) + ")";
    if (m->GetConst() && !m->GetStatic()) s += " const";
    s += ThrowText(m);
    if (m->GetPure())        s += " = 0";
    else if (m->GetDelete()) s += " = delete";
    return s;
}

// Overload identity: name + parameter types + const.
std::string MethodKey(Method* m)
{
    std::string s = ToStd(m->GetName()) + "(";
    bool first = true;
    Method::ArgumentIterator iArg(m);
    while (++iArg)
    {
        if (!first) s += ",";
        first = false;
        s += TypeText(iArg);
    }
    s += ")";
    if (m->GetConst() && !m->GetStatic()) s += " const";
    return s;
}

std::string InheritText(Inherit* i)
{
    std::string s = AccessWord(i->GetAccess());
    if (i->GetVirtual()) s += " virtual";
    return s + " " + ToStd(i->GetBaseClass()->GetName());
}

std::string ShapeText(BaseClass* c)
{
    std::string s = Trim(ToStd(c->GetTemplateDeclaration()));
    if (!s.empty()) s += " ";
    s += c->GetStruct() ? "struct " : "class ";
    s += ToStd(c->GetName()) + ToStd(c->GetTemplate());
    if (!c->GetMemberPrefix().IsEmpty())
        s += "  (member prefix " + ToStd(c->GetMemberPrefix()) + ")";
    return s;
}

// The part of a source class an extern mirrors: EVERY member (with its getter /
// setter), and the non-private methods minus the relation / macro ones (as
// ExternClass(Class*) keeps them). Getters / setters travel with their member.
bool InInterface(Member*) { return true; }
bool IsAccessor(Method* m) { return m->IsGetMemberMethod() || m->IsSetMemberMethod(); }
bool InInterface(Method* m)
{
    return m->GetAccess() != PRIVATE && !m->IsFromRelationMethod() && !m->IsMacroMethod() &&
           !IsAccessor(m);
}

// --- the diff -----------------------------------------------------------------

struct MemberOp { Member* src; Member* tgt; };     // src only = add, tgt only = remove
struct MethodOp { Method* src; Method* tgt; bool viaMember; };  // viaMember: goes with its removed member
struct BaseOp   { Inherit* src; Inherit* tgt; };
struct TypeNeed { std::string name; Type* src; };

struct Ops
{
    bool shape = false;
    std::vector<BaseOp>   bases;
    std::vector<MemberOp> members;
    std::vector<MethodOp> methods;
    std::vector<TypeNeed> types;
    std::vector<std::string> notes;

    bool Empty() const { return !shape && bases.empty() && members.empty() && methods.empty(); }
};

void NeedType(Ops& ops, DataModelDoc* pTgt, Type* srcT)
{
    if (!srcT)
        return;
    const std::string n = ToStd(srcT->GetName());
    if (IsSkippableTypeName(n) || pTgt->FindType(CbString(n.c_str())))
        return;
    for (const TypeNeed& t : ops.types)
        if (t.name == n)
            return;
    ops.types.push_back({ n, srcT });
}

void NeedMethodTypes(Ops& ops, DataModelDoc* pTgt, Method* m)
{
    NeedType(ops, pTgt, m->GetType());
    Method::ArgumentIterator iArg(m);
    while (++iArg)
        NeedType(ops, pTgt, iArg->GetType());
    if (ExceptionSpecification* es = m->GetExceptionSpecification())
    {
        ExceptionSpecification::ExceptionSpecificationTypeIterator iEt(es);
        while (++iEt)
            NeedType(ops, pTgt, iEt->GetType());
    }
}

bool HasInterfaceMembers(BaseClass* c);

// The member prefix only matters when there are members to prefix.
bool SameShape(Class* src, BaseClass* tgt)
{
    return src->GetStruct() == tgt->GetStruct() &&
           src->GetTemplateDeclaration() == tgt->GetTemplateDeclaration() &&
           src->GetTemplate() == tgt->GetTemplate() &&
           (!HasInterfaceMembers(src) || src->GetMemberPrefix() == tgt->GetMemberPrefix());
}

bool HasInterfaceMembers(BaseClass* c)
{
    BaseClass::MemberIterator iMem(c);
    while (++iMem)
        if (InInterface(iMem))
            return true;
    return false;
}

Ops Diff(Class* src, ExternClass* tgt, DataModelDoc* pTgt)
{
    Ops ops;
    ops.shape = !SameShape(src, tgt);

    // Bases, by base name.
    std::vector<Inherit*> tgtInh;
    {
        ExternClass::InheritIterator i(tgt);
        while (++i) tgtInh.push_back(i);
    }
    std::set<Inherit*> tgtInhSeen;
    ExternClass::InheritIterator iSrcInh(src);
    while (++iSrcInh)
    {
        Inherit* si = iSrcInh;
        const std::string b = ToStd(si->GetBaseClass()->GetName());
        if (IsSerializeRoot(b))
            continue;
        Inherit* ti = 0;
        for (Inherit* t : tgtInh)
            if (ToStd(t->GetBaseClass()->GetName()) == b) { ti = t; break; }
        if (ti)
        {
            tgtInhSeen.insert(ti);
            if (InheritText(si) != InheritText(ti))
                ops.bases.push_back({ si, ti });
            continue;
        }
        Type* there = pTgt->FindType(CbString(b.c_str()));
        if (there && !dynamic_cast<ExternClass*>(there))
        {
            ops.notes.push_back("base " + b + " is not a class in this project; the inheritance is not added");
            continue;
        }
        NeedType(ops, pTgt, si->GetBaseClass());
        ops.bases.push_back({ si, 0 });
    }
    for (Inherit* t : tgtInh)
        if (!tgtInhSeen.count(t))
            ops.bases.push_back({ 0, t });

    // Members, by name.
    std::set<Member*> removedMembers;
    std::set<Member*> tgtMemSeen;
    BaseClass::MemberIterator iSrcMem(src);
    while (++iSrcMem)
    {
        Member* sm = iSrcMem;
        if (!InInterface(sm))
            continue;
        Member* tm = tgt->FindMember(sm->GetName());
        if (tm)
        {
            tgtMemSeen.insert(tm);
            if (MemberText(sm) != MemberText(tm))
            {
                ops.members.push_back({ sm, tm });
                NeedType(ops, pTgt, sm->GetType());
            }
            continue;
        }
        ops.members.push_back({ sm, 0 });
        NeedType(ops, pTgt, sm->GetType());
    }
    {
        BaseClass::MemberIterator iTgtMem(tgt);
        while (++iTgtMem)
            if (!tgtMemSeen.count(iTgtMem))
            {
                ops.members.push_back({ 0, iTgtMem });
                removedMembers.insert(iTgtMem);
            }
    }

    // Methods: exact overload match first, then the leftovers by a name that is
    // unique on both sides (a changed signature), the rest added / removed.
    std::vector<Method*> srcM, tgtM;
    {
        BaseClass::MethodIterator i(src);
        while (++i) if (InInterface(i)) srcM.push_back(i);
    }
    {
        BaseClass::MethodIterator i(tgt);
        while (++i)
        {
            Method* m = i;
            if (IsAccessor(m))
                continue;                       // diffed with its member
            // A member method (wrap) of a member that goes away goes with it.
            if (MemberMethod* mm = dynamic_cast<MemberMethod*>(m))
                if (removedMembers.count(mm->GetMember()))
                {
                    ops.methods.push_back({ 0, m, true });
                    continue;
                }
            tgtM.push_back(m);
        }
    }
    std::vector<bool> srcDone(srcM.size(), false), tgtDone(tgtM.size(), false);
    for (size_t s = 0; s < srcM.size(); ++s)
        for (size_t t = 0; t < tgtM.size(); ++t)
            if (!tgtDone[t] && MethodKey(srcM[s]) == MethodKey(tgtM[t]))
            {
                srcDone[s] = tgtDone[t] = true;
                if (MethodText(srcM[s]) != MethodText(tgtM[t]))
                    ops.methods.push_back({ srcM[s], tgtM[t], false });
                break;
            }
    auto countName = [](const std::vector<Method*>& v, const std::vector<bool>& done, const CbString& n) {
        int c = 0;
        for (size_t i = 0; i < v.size(); ++i)
            if (!done[i] && v[i]->GetName() == n) ++c;
        return c;
    };
    for (size_t s = 0; s < srcM.size(); ++s)
    {
        if (srcDone[s])
            continue;
        const CbString n = srcM[s]->GetName();
        if (countName(srcM, srcDone, n) != 1 || countName(tgtM, tgtDone, n) != 1)
            continue;
        for (size_t t = 0; t < tgtM.size(); ++t)
            if (!tgtDone[t] && tgtM[t]->GetName() == n)
            {
                srcDone[s] = tgtDone[t] = true;
                ops.methods.push_back({ srcM[s], tgtM[t], false });
                break;
            }
    }
    for (size_t s = 0; s < srcM.size(); ++s)
        if (!srcDone[s])
            ops.methods.push_back({ srcM[s], 0, false });
    for (size_t t = 0; t < tgtM.size(); ++t)
        if (!tgtDone[t] && !tgtM[t]->IsDestructor())   // an extern always keeps its destructor
            ops.methods.push_back({ 0, tgtM[t], false });

    for (const MethodOp& op : ops.methods)
        if (op.src)
            NeedMethodTypes(ops, pTgt, op.src);

    return ops;
}

const char* TypeFormText(Type* t)
{
    return t->IsOtherType() ? "other type" : "extern class";
}

ExternClass* FindExtern(DataModelDoc* pDoc, const std::string& name)
{
    BaseClass* bc = pDoc->FindBaseClass(CbString(name.c_str()));
    if (!bc || bc->IsClass())
        return 0;
    return dynamic_cast<ExternClass*>(bc);
}

Class* FindSourceClass(DataModelDoc* pDoc, const std::string& name)
{
    BaseClass* bc = pDoc->FindBaseClass(CbString(name.c_str()));
    return bc && bc->IsClass() ? dynamic_cast<Class*>(bc) : 0;
}

// --- execute helpers ----------------------------------------------------------

void CopyShape(BaseClass* pTo, BaseClass* pFrom)
{
    pTo->SetTemplate(pFrom->GetTemplateDeclaration(), pFrom->GetTemplate());
    pTo->SetStruct(pFrom->GetStruct());
    pTo->SetMemberPrefix(pFrom->GetMemberPrefix());
}

// A missing type, created the way Import creates a stub.
void CreateType(DataModelDoc* pTgt, Type* srcT)
{
    const CbString name = srcT->GetName();
    if (pTgt->FindType(name))
        return;
    if (OtherType* srcOT = dynamic_cast<OtherType*>(srcT))
    {
        OtherType* pOT = new OtherType(pTgt, srcOT);    // carries the declaration
        pOT->Add();
        return;
    }
    ExternClass* pEC = new ExternClass(pTgt);           // bare: no recursive interface
    pEC->SetName(name);
    if (BaseClass* srcBC = dynamic_cast<BaseClass*>(srcT))
    {
        pEC->SetTemplate(srcBC->GetTemplateDeclaration(), srcBC->GetTemplate());
        pEC->SetStruct(srcBC->GetStruct());
        pEC->SetSuppressForwardDeclaration(srcBC->GetSuppressForwardDeclaration());
        pEC->SetMemberPrefix(srcBC->GetMemberPrefix());
    }
    pEC->Add();
}

// An extern method is a declaration only: no body, nothing to emit.
void MakeDeclarationOnly(Method* m)
{
    m->SetCode("");
    m->SetDeclare(false);
    m->SetImplement(false);
    if (Constructor* c = dynamic_cast<Constructor*>(m))
        c->SetInit("");
}

void UpdateMethodInPlace(DataModelDoc* pTgt, Method* tm, Method* sm)
{
    tm->SaveState();
    Type* t = pTgt->FindOrDuplicateType(sm->GetType());
    if (tm->GetType() != t)
        t->MoveVariableLast(tm);
    tm->Method::CopyValuesFrom(*sm);
    if (Constructor* tc = dynamic_cast<Constructor*>(tm))
        if (Constructor* sc = dynamic_cast<Constructor*>(sm))
            tc->SetExplicit(sc->GetExplicit());
    MakeDeclarationOnly(tm);

    if (ArgsText(tm) != ArgsText(sm) || MethodKey(tm) != MethodKey(sm))
    {
        std::vector<Argument*> old;
        Method::ArgumentIterator iOld(tm);
        while (++iOld) old.push_back(iOld);
        for (Argument* a : old)
            a->Delete();
        Method::ArgumentIterator iArg(sm);
        while (++iArg)
        {
            Argument* a = new Argument(tm, pTgt->FindOrDuplicateType(iArg->GetType()), iArg);
            a->Add();
        }
    }

    if (ThrowText(tm) != ThrowText(sm))
    {
        // As the exception-specification dialog: drop the thrown types, then the
        // clause; rebuild from the source as the copy-constructor does.
        if (ExceptionSpecification* es = tm->GetExceptionSpecification())
        {
            std::vector<ExceptionSpecificationType*> old;
            ExceptionSpecification::ExceptionSpecificationTypeIterator iEt(es);
            while (++iEt) old.push_back(iEt);
            for (ExceptionSpecificationType* e : old)
                e->Delete();
            es->Delete();
        }
        if (ExceptionSpecification* ses = sm->GetExceptionSpecification())
        {
            ExceptionSpecification* es = new ExceptionSpecification(tm);
            ExceptionSpecification::ExceptionSpecificationTypeIterator iEt(ses);
            while (++iEt)
                (void)new ExceptionSpecificationType(es, pTgt->FindOrDuplicateType(iEt->GetType()));
        }
    }
    tm->Update();
}

Method* AddMethodCopy(DataModelDoc* pTgt, ExternClass* tgt, Method* sm)
{
    Method* pNM = 0;
    if (Constructor* sc = dynamic_cast<Constructor*>(sm))
    {
        Constructor* nc = new Constructor(tgt, sc);
        nc->SetExplicit(sc->GetExplicit());
        pNM = nc;
    }
    else if (Destructor* sd = dynamic_cast<Destructor*>(sm))
        pNM = new Destructor(tgt, sd);
    else
    {
        pNM = new Method(tgt, pTgt->FindOrDuplicateType(sm->GetType()), sm);
        if (pNM->GetName() != sm->GetName())         // the copy renames virtual overloads
            pNM->SetName(sm->GetName());
    }
    MakeDeclarationOnly(pNM);
    pNM->Add();
    tgt->NotifyAddMethod(pNM);
    return pNM;
}

// Give member tm the getter / setter sm has, with sm's access. As the Member
// dialog: a retyped member gets fresh accessors (their signatures derive from
// the member). New accessors are added by the caller's Member::Add / Update.
void SyncAccessors(Member* tm, Member* sm, bool rebuild)
{
    GetMemberMethod* sg = sm->GetGetMemberMethod();
    GetMemberMethod* tg = tm->GetGetMemberMethod();
    if (tg && (!sg || rebuild)) { tg->Delete(); tg = 0; }
    if (sg && !tg)
        tg = new GetMemberMethod(tm);               // static / const from the member
    else if (tg)
    {
        tg->SaveState();
        tg->SetStatic(tm->GetStatic());
        tg->SetConst(!tm->GetStatic());
    }
    if (tg)
        tg->SetAccess(sg->GetAccess());

    SetMemberMethod* ss = sm->GetSetMemberMethod();
    SetMemberMethod* ts = tm->GetSetMemberMethod();
    if (ts && (!ss || rebuild)) { ts->Delete(); ts = 0; }
    if (ss && !ts)
        ts = new SetMemberMethod(tm);
    else if (ts)
        ts->SaveState();
    if (ts)
    {
        ts->SetAccess(ss->GetAccess());
        ts->SetStatic(tm->GetStatic());
    }
}

} // namespace

void Cb_UpdatePlan(DataModelDoc* pSrc, DataModelDoc* pTgt, CbUpdatePlan& plan)
{
    plan = CbUpdatePlan();
    if (!pSrc || !pTgt || pSrc == pTgt)
        return;

    DataModelDoc::TypeIterator iType(pTgt);
    while (++iType)
    {
        Type* t = iType;
        if (!t->IsExternClass() || t->IsClass())
            continue;
        const std::string name = ToStd(t->GetName());
        Class* src = FindSourceClass(pSrc, name);
        ExternClass* tgt = dynamic_cast<ExternClass*>(t);
        if (!src || !tgt)
            continue;

        Ops ops = Diff(src, tgt, pTgt);
        CbUpdateRow row;
        row.name = name;
        row.notes = ops.notes;
        if (ops.shape)
            row.changes.push_back({ "class", '~', ShapeText(tgt), ShapeText(src), 0 });
        for (const TypeNeed& tn : ops.types)
            row.changes.push_back({ "type", '+', std::string(),
                                    tn.name + " (new " + TypeFormText(tn.src) + ")", 0 });
        for (const BaseOp& b : ops.bases)
        {
            if (b.src && b.tgt)  row.changes.push_back({ "base", '~', InheritText(b.tgt), InheritText(b.src), 0 });
            else if (b.src)      row.changes.push_back({ "base", '+', std::string(), InheritText(b.src), 0 });
            else                 row.changes.push_back({ "base", '-', InheritText(b.tgt), std::string(), 0 });
        }
        for (const MemberOp& m : ops.members)
        {
            if (m.src && m.tgt)  row.changes.push_back({ "member", '~', MemberText(m.tgt), MemberText(m.src), 0 });
            else if (m.src)      row.changes.push_back({ "member", '+', std::string(), MemberText(m.src), 0 });
            else                 row.changes.push_back({ "member", '-', MemberText(m.tgt), std::string(), 0 });
        }
        for (const MethodOp& m : ops.methods)
        {
            if (m.src && m.tgt)  row.changes.push_back({ "method", '~', MethodText(m.tgt), MethodText(m.src), 0 });
            else if (m.src)      row.changes.push_back({ "method", '+', std::string(), MethodText(m.src), 0 });
            else                 row.changes.push_back({ "method", '-', MethodText(m.tgt), std::string(),
                                                         m.tgt->GetChildActivationShapeCount() });
        }
        plan.rows.push_back(row);
    }
    std::sort(plan.rows.begin(), plan.rows.end(),
              [](const CbUpdateRow& a, const CbUpdateRow& b) { return LessNoCase(a.name, b.name); });
}

bool Cb_UpdateExecute(DataModelDoc* pSrc, DataModelDoc* pTgt,
                      const std::vector<std::string>& names,
                      CbUpdateResult& out, std::string& error)
{
    out = CbUpdateResult();
    if (!pSrc || !pTgt) { error = "no source or target project"; return false; }
    if (pSrc == pTgt)   { error = "the source and target are the same project"; return false; }
    if (names.empty())  { error = "no extern class selected to update"; return false; }

    CbViewLock lock(pTgt);                          // one coalesced refresh
    pTgt->MarkLastUndo();                           // the whole update = one undo step

    for (const std::string& name : names)
    {
        ExternClass* tgt = FindExtern(pTgt, name);
        Class* src = FindSourceClass(pSrc, name);
        if (!tgt || !src)
        {
            out.notes.push_back(name + ": no extern class in this project with a same-named "
                                       "class in the source; skipped");
            continue;
        }
        Ops ops = Diff(src, tgt, pTgt);
        for (const std::string& n : ops.notes)
            out.notes.push_back(name + ": " + n);
        if (ops.Empty())
            continue;
        ++out.externs;

        // 1) Missing types, so every later FindOrDuplicateType only finds.
        for (const TypeNeed& tn : ops.types)
            if (!pTgt->FindType(CbString(tn.name.c_str())))
            {
                CreateType(pTgt, tn.src);
                ++out.types;
            }

        // 2) Class shape.
        if (ops.shape)
        {
            tgt->SaveState();
            CopyShape(tgt, src);
            tgt->Update();
            ++out.changed;
        }

        // 3) Bases.
        for (const BaseOp& b : ops.bases)
        {
            if (b.src && b.tgt)
            {
                b.tgt->SaveState();
                b.tgt->SetAccess(b.src->GetAccess());
                b.tgt->SetVirtual(b.src->GetVirtual());
                b.tgt->Update();
                ++out.changed;
            }
            else if (b.src)
            {
                ExternClass* pBase = dynamic_cast<ExternClass*>(
                    pTgt->FindBaseClass(b.src->GetBaseClass()->GetName()));
                if (!pBase || pBase == tgt || pBase->IsBaseClass(tgt))
                {
                    out.notes.push_back(name + ": base " + ToStd(b.src->GetBaseClass()->GetName()) +
                                        " not added (it would make an inheritance cycle)");
                    continue;
                }
                Inherit* pI = new Inherit(tgt, pBase, b.src);
                pI->Add();
                ++out.added;
            }
            else
            {
                b.tgt->Delete();
                ++out.removed;
            }
        }

        // 4) Removals: methods first, then members (a removed member takes its
        //    getter / setter along -- those are not deleted twice).
        for (const MethodOp& m : ops.methods)
        {
            if (m.src)
                continue;
            const int uses = m.tgt->GetChildActivationShapeCount();
            if (uses > 0)
                out.notes.push_back(name + ": removed " + MethodText(m.tgt) + " -- it had " +
                                    std::to_string(uses) + " sequence-diagram message(s)");
            if (!m.viaMember)
                m.tgt->Delete();
            ++out.removed;
        }
        for (const MemberOp& m : ops.members)
            if (!m.src)
            {
                m.tgt->Delete();
                ++out.removed;
            }

        // 5) Members: changed in place, then added.
        for (const MemberOp& m : ops.members)
        {
            if (!m.src)
                continue;
            Type* t = pTgt->FindOrDuplicateType(m.src->GetType());
            if (m.tgt)
            {
                const bool retyped = MemberTypeText(m.tgt) != MemberTypeText(m.src);
                m.tgt->SaveState();
                if (m.tgt->GetType() != t)
                    t->MoveVariableLast(m.tgt);
                m.tgt->CopyValuesFrom(*m.src);
                SyncAccessors(m.tgt, m.src, retyped);
                m.tgt->Update();                    // adds new accessors, refreshes the rest
                ++out.changed;
            }
            else
            {
                Member* pM = new Member(tgt, t, m.src);
                SyncAccessors(pM, m.src, false);
                pM->Add();
                tgt->NotifyAddMember(pM);
                ++out.added;
            }
        }

        // 6) Methods: changed in place, then added.
        for (const MethodOp& m : ops.methods)
        {
            if (!m.src)
                continue;
            if (m.tgt)
            {
                UpdateMethodInPlace(pTgt, m.tgt, m.src);
                ++out.changed;
            }
            else
            {
                (void)AddMethodCopy(pTgt, tgt, m.src);
                ++out.added;
            }
        }
    }

    // Anything a copy constructor still had to duplicate goes into the tree;
    // never the internal "...".
    DataModelDoc::TypeIterator iType(pTgt);
    while (++iType)
        if (!iType->GetAdded() && ToStd(iType->GetName()) != "...")
            iType->Add();

    pTgt->SetModifiedFlag(1);
    return true;
}
