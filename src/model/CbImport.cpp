// model/CbImport.cpp -- see CbImport.h. The cross-project Import / Clone engine.
//
// Copying reuses the model's own paste copy-constructors -- Member / Method /
// Constructor (BaseClass*, Type*, source) and Inherit(ExternClass*, BaseClass*,
// source) -- which resolve every type through the TARGET's FindOrDuplicateType.
// The plan pre-creates every dependency in its chosen form first, so that call
// only ever FINDS: its own fallback (duplicating a class as an ExternClass that
// copies the whole public interface, recursively) is exactly the unbounded
// cascade the stub design avoids.
#include "StdAfx.h"
#include "CbImport.h"
#include "ClassBuilderInclude.h"   // model classes + iterators
#include "CbViewLock.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <string>
#include <vector>

namespace {

std::string ToStd(const CbString& s) { return std::string((const char*)s); }

bool EqualsNoCase(const std::string& a, const std::string& b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i]))
            return false;
    return true;
}

// A method a clone copies verbatim: hand-written, i.e. none of the kinds that
// regenerate from the class (ctor/dtor/getter/setter/find/relation macro/fixed).
bool IsUserMethod(Method* m)
{
    return !m->IsConstructor() && !m->IsDestructor() &&
           !m->IsGetMemberMethod() && !m->IsSetMemberMethod() &&
           !m->IsFindMethod() && !m->IsMacroMethod() && !m->IsFixedMethod();
}

// The serialize runtime root; a plain import never inherits it.
bool IsSerializeRoot(const std::string& name) { return name == "CbObject"; }

// "" is the constructors' return type, "..." the internal varargs type.
bool IsSkippableTypeName(const std::string& n) { return n.empty() || n == "..."; }

int KindOf(Type* t)
{
    if (t->IsClass())       return CB_IMPORT_CLASS;
    if (t->IsExternClass()) return CB_IMPORT_EXTERNCLASS;
    if (t->IsOtherType())   return CB_IMPORT_OTHERTYPE;
    return CB_IMPORT_NONE;
}

void AddUnique(std::vector<std::string>& v, const std::string& s)
{
    if (!IsSkippableTypeName(s) && std::find(v.begin(), v.end(), s) == v.end())
        v.push_back(s);
}

void AddMethodTypes(Method* m, std::vector<std::string>& types)
{
    if (m->GetType())
        AddUnique(types, ToStd(m->GetType()->GetName()));
    Method::ArgumentIterator iArg(m);
    while (++iArg)
        if (iArg->GetType())
            AddUnique(types, ToStd(iArg->GetType()->GetName()));
    if (ExceptionSpecification* pEs = m->GetExceptionSpecification())
    {
        ExceptionSpecification::ExceptionSpecificationTypeIterator iEt(pEs);
        while (++iEt)
            if (iEt->GetType())
                AddUnique(types, ToStd(iEt->GetType()->GetName()));
    }
}

// What a Class import pulls on. Mirrors exactly what the copy step copies, so
// every type the copy constructors resolve is planned (and pre-created).
struct ClassDeps
{
    std::vector<std::string> bases;     // optional, min ExternClass
    std::vector<std::string> types;     // mandatory, min OtherType
    std::vector<std::string> partners;  // relation partners (Class keeps the relation)
};

ClassDeps CollectDeps(Class* pClass)
{
    ClassDeps d;
    ExternClass::InheritIterator iInh(pClass);
    while (++iInh)
    {
        BaseClass* pBase = iInh->GetBaseClass();
        if (pBase && !IsSerializeRoot(ToStd(pBase->GetName())))
            AddUnique(d.bases, ToStd(pBase->GetName()));
    }
    BaseClass::MemberIterator iMem(pClass);
    while (++iMem)
        if (iMem->GetType())
            AddUnique(d.types, ToStd(iMem->GetType()->GetName()));
    BaseClass::MethodIterator iMeth(pClass);
    while (++iMeth)
    {
        Method* m = iMeth;
        if (IsUserMethod(m) || m->IsNormalConstructor())
            AddMethodTypes(m, d.types);
    }
    Class::FromRelationIterator iFrom(pClass);
    while (++iFrom)
        if (iFrom->GetToClass())
            AddUnique(d.partners, ToStd(iFrom->GetToClass()->GetName()));
    Class::ToRelationIterator iTo(pClass);
    while (++iTo)
        if (iTo->GetFromClass())
            AddUnique(d.partners, ToStd(iTo->GetFromClass()->GetName()));
    return d;
}

void AddRequiredBy(CbImportRow& r, const std::string& why)
{
    if (r.requiredBy.find(why) != std::string::npos)
        return;
    if (!r.requiredBy.empty())
        r.requiredBy += "; ";
    r.requiredBy += why;
}

// "ConstructorInclude(pA, pB)" -- the call a normal constructor body makes,
// built from the class's ConstructorIncludeMethod args (as UpdateArguments does).
std::string IncludeCall(Class* c)
{
    std::string s = "ConstructorInclude(";
    if (Method* m = c->GetConstructorIncludeMethod())
    {
        bool first = true;
        Method::ArgumentIterator iArg(m);
        while (++iArg)
        {
            if (!first) s += ", ";
            first = false;
            s += ToStd(iArg->GetName());
        }
    }
    return s + ")";
}

void CopyBaseClassShape(BaseClass* pTo, BaseClass* pFrom)
{
    pTo->SetTemplate(pFrom->GetTemplateDeclaration(), pFrom->GetTemplate());
    pTo->SetStruct(pFrom->GetStruct());
    pTo->SetSuppressForwardDeclaration(pFrom->GetSuppressForwardDeclaration());
}

// The source's file names when free in the target, else the ClassDialog's own
// derivation (name minus the target's class prefix, C-symbol safe), numbered
// until unique.
void SetClassFiles(DataModel* pTgtModel, Class* pClass, Class* pSrc)
{
    const std::string master = ToStd(pTgtModel->GetHFile());
    auto taken = [&](const std::string& f) -> bool {
        if (f.empty() || EqualsNoCase(f, master))
            return true;
        DataModel::ClassIterator iC(pTgtModel);
        while (++iC)
        {
            Class* c = iC;
            if (c == pClass)
                continue;
            if (EqualsNoCase(f, ToStd(c->GetHFile())) || EqualsNoCase(f, ToStd(c->GetCppFile())))
                return true;
        }
        return false;
    };

    std::string h   = ToStd(pSrc->GetHFile());
    std::string cpp = ToStd(pSrc->GetCppFile());
    if (taken(h) || taken(cpp))
    {
        std::string base = ToStd(pClass->GetName());
        const std::string prefix = ToStd(pTgtModel->GetClassPrefix());
        if (!prefix.empty() && base.compare(0, prefix.size(), prefix) == 0)
            base = base.substr(prefix.size());
        CbString cb(base.c_str());
        pClass->GetDataModelDoc()->ConvertNonCSymbols(cb);
        base = ToStd(cb);

        h = base + ".h"; cpp = base + ".cpp";
        for (int n = 2; taken(h) || taken(cpp); ++n)
        {
            h   = base + std::to_string(n) + ".h";
            cpp = base + std::to_string(n) + ".cpp";
        }
    }
    pClass->SetHFile(CbString(h.c_str()));
    pClass->SetCppFile(CbString(cpp.c_str()));
}

} // namespace

const char* Cb_ImportFormName(int form)
{
    switch (form)
    {
    case CB_IMPORT_CLASS:       return "class";
    case CB_IMPORT_EXTERNCLASS: return "extern_class";
    case CB_IMPORT_OTHERTYPE:   return "other_type";
    default:                    return "none";
    }
}

int Cb_ImportFormFromName(const std::string& name)
{
    if (name == "class")        return CB_IMPORT_CLASS;
    if (name == "extern_class") return CB_IMPORT_EXTERNCLASS;
    if (name == "other_type")   return CB_IMPORT_OTHERTYPE;
    return CB_IMPORT_NONE;
}

void Cb_ImportCandidates(DataModelDoc* pSrc, DataModelDoc* pTgt, CbImportPlan& plan)
{
    plan = CbImportPlan();
    if (!pSrc || !pTgt || pSrc == pTgt)
        return;

    DataModelDoc::TypeIterator iType(pSrc);
    while (++iType)
    {
        if (!iType->GetAdded())
            continue;
        const std::string n = ToStd(iType->GetName());
        if (IsSkippableTypeName(n))
            continue;
        if (pTgt->FindType(CbString(n.c_str())))   // present in the target -> reused
            continue;
        const int k = KindOf(iType);
        if (k == CB_IMPORT_NONE)
            continue;
        CbImportRow r;
        r.name = n;
        r.sourceKind = k;
        plan.rows.push_back(r);
    }
    std::sort(plan.rows.begin(), plan.rows.end(),
        [](const CbImportRow& a, const CbImportRow& b) {
            if (a.sourceKind != b.sourceKind) return a.sourceKind > b.sourceKind;
            return a.name < b.name;
        });
    Cb_ImportResolve(pSrc, pTgt, plan);
}

void Cb_ImportResolve(DataModelDoc* pSrc, DataModelDoc* pTgt, CbImportPlan& plan)
{
    plan.notes.clear();
    plan.classes = plan.externClasses = plan.otherTypes = 0;
    plan.relationsKept = plan.relationsDropped = 0;
    if (!pSrc || !pTgt)
        return;

    std::vector<CbImportRow>& rows = plan.rows;
    std::map<std::string, size_t> idx;
    for (size_t i = 0; i < rows.size(); ++i)
    {
        idx[rows[i].name] = i;
        rows[i].requiredForm = rows[i].suggestedForm = CB_IMPORT_NONE;
        rows[i].mandatory = false;
        rows[i].requiredBy.clear();
    }
    std::vector<int> reqMand(rows.size(), CB_IMPORT_NONE);
    std::vector<int> reqOpt (rows.size(), CB_IMPORT_NONE);

    // Only Class picks create requirements, and a requirement never reaches
    // Class, so one pass over the explicit Class picks settles everything.
    for (size_t i = 0; i < rows.size(); ++i)
    {
        if (rows[i].explicitForm != CB_IMPORT_CLASS)
            continue;
        const std::string me = rows[i].name;
        Class* pClass = dynamic_cast<Class*>(pSrc->FindBaseClass(CbString(me.c_str())));
        if (!pClass)
            continue;
        const ClassDeps d = CollectDeps(pClass);

        for (const std::string& b : d.bases)
        {
            auto it = idx.find(b);
            if (b == me || it == idx.end()) continue;
            CbImportRow& dep = rows[it->second];
            reqOpt[it->second] = std::max(reqOpt[it->second], (int)CB_IMPORT_EXTERNCLASS);
            AddRequiredBy(dep, "base of " + me + (dep.optOut ? " (declined)" : ""));
            if (dep.sourceKind == CB_IMPORT_CLASS)
                dep.suggestedForm = std::max(dep.suggestedForm, (int)CB_IMPORT_CLASS);
        }
        for (const std::string& t : d.types)
        {
            auto it = idx.find(t);
            if (t == me || it == idx.end()) continue;
            CbImportRow& dep = rows[it->second];
            reqMand[it->second] = CB_IMPORT_OTHERTYPE;
            AddRequiredBy(dep, "type in " + me);
            if (dep.sourceKind >= CB_IMPORT_EXTERNCLASS)
                dep.suggestedForm = std::max(dep.suggestedForm, (int)CB_IMPORT_EXTERNCLASS);
        }
        for (const std::string& p : d.partners)
        {
            auto it = idx.find(p);
            if (p == me || it == idx.end()) continue;
            CbImportRow& dep = rows[it->second];
            if (dep.sourceKind != CB_IMPORT_CLASS) continue;
            dep.suggestedForm = std::max(dep.suggestedForm, (int)CB_IMPORT_CLASS);
            AddRequiredBy(dep, "relation with " + me);
        }
    }

    for (size_t i = 0; i < rows.size(); ++i)
    {
        CbImportRow& r = rows[i];
        r.mandatory    = reqMand[i] != CB_IMPORT_NONE;
        r.requiredForm = std::max(reqMand[i], r.optOut ? (int)CB_IMPORT_NONE : reqOpt[i]);
        int eff = r.explicitForm != CB_IMPORT_NONE ? std::max(r.explicitForm, reqMand[i])
                                                   : r.requiredForm;
        eff = std::min(eff, r.sourceKind);           // never richer than the source
        r.effectiveForm = eff;
        if (r.suggestedForm <= eff || r.suggestedForm > r.sourceKind)
            r.suggestedForm = CB_IMPORT_NONE;
        if      (eff == CB_IMPORT_CLASS)       ++plan.classes;
        else if (eff == CB_IMPORT_EXTERNCLASS) ++plan.externClasses;
        else if (eff == CB_IMPORT_OTHERTYPE)   ++plan.otherTypes;
    }

    // Notes: which relations / bases survive. A relation is recreated only when
    // BOTH ends are newly imported Classes -- the import never edits a class the
    // target already had (an owned relation would change its constructors).
    auto isNewClass = [&](const std::string& n) {
        auto it = idx.find(n);
        return it != idx.end() && rows[it->second].effectiveForm == CB_IMPORT_CLASS;
    };
    auto why = [&](const std::string& n) -> std::string {
        return idx.count(n) ? n + " is not imported as a class"
                            : n + " already exists in the target";
    };
    for (const CbImportRow& r : rows)
    {
        if (r.effectiveForm != CB_IMPORT_CLASS)
            continue;
        Class* pClass = dynamic_cast<Class*>(pSrc->FindBaseClass(CbString(r.name.c_str())));
        if (!pClass)
            continue;

        Class::FromRelationIterator iFrom(pClass);
        while (++iFrom)
        {
            const std::string to = ToStd(iFrom->GetToClass()->GetName());
            if (to == r.name || isNewClass(to))
                ++plan.relationsKept;
            else
            {
                ++plan.relationsDropped;
                plan.notes.push_back("Relation " + r.name + " -> " + to + " dropped (" + why(to) + ")");
            }
        }
        Class::ToRelationIterator iTo(pClass);
        while (++iTo)
        {
            const std::string from = ToStd(iTo->GetFromClass()->GetName());
            if (from == r.name || isNewClass(from))
                continue;                           // counted from the owning side
            ++plan.relationsDropped;
            plan.notes.push_back("Relation " + from + " -> " + r.name + " dropped (" + why(from) + ")");
        }

        ExternClass::InheritIterator iInh(pClass);
        while (++iInh)
        {
            const std::string b = ToStd(iInh->GetBaseClass()->GetName());
            if (IsSerializeRoot(b))
                continue;
            auto it = idx.find(b);
            if (it != idx.end())
            {
                if (rows[it->second].effectiveForm < CB_IMPORT_EXTERNCLASS)
                    plan.notes.push_back(r.name + " no longer inherits " + b + " (" + b +
                                         " not imported as a class or extern class)");
            }
            else
            {
                Type* t = pTgt->FindType(CbString(b.c_str()));
                if (!t || !t->IsExternClass())
                    plan.notes.push_back(r.name + " no longer inherits " + b +
                                         " (the target has it as a non-class type)");
            }
        }
    }
}

bool Cb_ImportExecute(DataModelDoc* pSrc, DataModelDoc* pTgt, const CbImportPlan& plan,
                      CbImportResult& out, std::string& error)
{
    out = CbImportResult();
    if (!pSrc || !pTgt) { error = "no source or target project"; return false; }
    if (pSrc == pTgt)   { error = "the source and target are the same project"; return false; }
    DataModel* pTgtModel = pTgt->GetDataModel();
    if (!pTgtModel)     { error = "the target project has no data model"; return false; }

    bool any = false;
    for (const CbImportRow& r : plan.rows)
        if (r.effectiveForm != CB_IMPORT_NONE) { any = true; break; }
    if (!any) { error = "nothing is selected to import"; return false; }

    CbViewLock lock(pTgt);                          // one coalesced refresh
    pTgt->MarkLastUndo();                           // the whole import = one undo step

    // 1) Stubs first, so every later FindOrDuplicateType only FINDS.
    for (const CbImportRow& r : plan.rows)
    {
        if (r.effectiveForm != CB_IMPORT_OTHERTYPE && r.effectiveForm != CB_IMPORT_EXTERNCLASS)
            continue;
        const CbString name(r.name.c_str());
        if (pTgt->FindType(name))
        {
            out.notes.push_back(r.name + " already exists in the target; reused");
            continue;
        }
        Type* srcT = pSrc->FindType(name);
        if (r.effectiveForm == CB_IMPORT_OTHERTYPE)
        {
            OtherType* pOT = 0;
            if (OtherType* srcOT = dynamic_cast<OtherType*>(srcT))
                pOT = new OtherType(pTgt, srcOT);   // carries the declaration
            else
            {
                pOT = new OtherType(pTgt);
                pOT->SetName(name);
            }
            pOT->Add();
            ++out.otherTypes;
        }
        else
        {
            // Explicit, non-recursive stub: ExternClass(doc, src) would copy the
            // whole public interface and duplicate its types recursively.
            ExternClass* pEC = new ExternClass(pTgt);
            pEC->SetName(name);
            if (BaseClass* srcBC = dynamic_cast<BaseClass*>(srcT))
                CopyBaseClassShape(pEC, srcBC);
            pEC->Add();
            ++out.externClasses;
        }
    }

    // 2) Every class shell, before any content, so members / bases / relations
    //    can refer to classes imported in the same batch.
    std::map<std::string, Class*> newClasses;
    std::vector<std::pair<Class*, Class*>> pairs;    // (source, target)
    for (const CbImportRow& r : plan.rows)
    {
        if (r.effectiveForm != CB_IMPORT_CLASS)
            continue;
        const CbString name(r.name.c_str());
        Class* srcC = dynamic_cast<Class*>(pSrc->FindBaseClass(name));
        if (!srcC)
            continue;
        if (pTgt->FindType(name))
        {
            out.notes.push_back(r.name + " already exists in the target; not cloned");
            continue;
        }

        Class* pC = new Class(pTgtModel);
        pC->SetOrder(pTgtModel->GetClassCount() - 1);
        if (pC->GetSerialize())                     // serialize never travels
            pC->SetSerialize(0);
        pC->SetName(name);
        CopyBaseClassShape(pC, srcC);
        pC->SetMemberPrefix(srcC->GetMemberPrefix());
        pC->SetNote(srcC->GetNote());
        pC->SetDllExport(srcC->GetDllExport());
        pC->SetHUser1(srcC->GetHUser1());
        pC->SetHUser2(srcC->GetHUser2());
        pC->SetHUser3(srcC->GetHUser3());
        pC->SetCppUser1(srcC->GetCppUser1());
        pC->SetCppUser2(srcC->GetCppUser2());
        pC->SetCppUser3(srcC->GetCppUser3());
        SetClassFiles(pTgtModel, pC, srcC);
        pC->Add();

        newClasses[r.name] = pC;
        pairs.push_back(std::make_pair(srcC, pC));
        ++out.classes;
    }

    // Is base `name` kept for a class import? Only when it resolves to a class
    // in the target (a declined or OtherType-only base lapses).
    std::map<std::string, int> effective;
    for (const CbImportRow& r : plan.rows)
        effective[r.name] = r.effectiveForm;

    // 3) Bases + members (+ getter/setter) for every class -- members first, as
    //    a tree relation (step 4) needs its key member on the to-class.
    for (const auto& pr : pairs)
    {
        Class* src = pr.first;
        Class* tgt = pr.second;

        ExternClass::InheritIterator iInh(src);
        while (++iInh)
        {
            const std::string b = ToStd(iInh->GetBaseClass()->GetName());
            if (IsSerializeRoot(b))
                continue;
            auto it = effective.find(b);
            if (it != effective.end() && it->second < CB_IMPORT_EXTERNCLASS)
                continue;                           // declined / OtherType -> lapses
            ExternClass* pBase = dynamic_cast<ExternClass*>(pTgt->FindBaseClass(CbString(b.c_str())));
            if (!pBase || pBase == tgt || pBase->IsBaseClass(tgt))
                continue;
            Inherit* pInherit = new Inherit(tgt, pBase, iInh);
            pInherit->Add();
            ++out.inherits;
        }

        BaseClass::MemberIterator iMem(src);
        while (++iMem)
        {
            Member* srcM = iMem;
            Member* pM = new Member(tgt, pTgt->FindOrDuplicateType(srcM->GetType()), srcM);
            if (GetMemberMethod* g = srcM->GetGetMemberMethod())
            {
                GetMemberMethod* ng = new GetMemberMethod(pM);
                ng->SetAccess(g->GetAccess());
                ng->SetStatic(pM->GetStatic());
            }
            if (SetMemberMethod* s = srcM->GetSetMemberMethod())
            {
                SetMemberMethod* ns = new SetMemberMethod(pM);
                ns->SetAccess(s->GetAccess());
                ns->SetStatic(pM->GetStatic());
            }
            pM->Add();
            tgt->NotifyAddMember(pM);
            ++out.members;
        }
    }

    // 4) Relations, only between classes newly imported in this batch -- BEFORE
    //    the constructors: an owned relation's UpdateArguments mirrors the new
    //    parent pointer onto every existing normal constructor, and the copied
    //    constructors already carry it from the source (it would duplicate).
    for (const auto& pr : pairs)
    {
        Class* src = pr.first;
        Class* tgt = pr.second;
        Class::FromRelationIterator iRel(src);
        while (++iRel)
        {
            Relation* r = iRel;
            auto it = newClasses.find(ToStd(r->GetToClass()->GetName()));
            if (it == newClasses.end())
                continue;                           // dropped -- listed in the plan notes
            Class* pTo = it->second;

            const int xStatic = r->GetStatic() ? 1 : 0;
            const int multi   = r->GetMulti()  ? 1 : 0;
            const int single  = multi ? 0 : 1;
            Relation* pR = new Relation(tgt, pTo, r->GetFromName(), r->GetToName(),
                                        xStatic, multi, single,
                                        r->GetOwned() ? 1 : 0, r->GetCritical() ? 1 : 0);
            pR->SetFilter(r->GetFilter() ? 1 : 0);
            pR->SetNote(r->GetNote());

            const int impl = r->GetImplementation();
            if (impl == 1 || impl == 2)
            {
                Member* key = 0;
                if (r->GetRelationMember() && r->GetRelationMember()->GetMember())
                    key = pTo->FindMember(r->GetRelationMember()->GetMember()->GetVariableName());
                if (!key)
                    out.notes.push_back("Relation " + ToStd(tgt->GetName()) + " -> " +
                                        ToStd(pTo->GetName()) + " imported as a list (key member not found)");
                else if (impl == 2)
                    (void)new AvlTree(pR, key);
                else if (r->GetRelationMember()->IsUniqueValueTree())
                    (void)new UniqueValueTree(pR, key);
                else
                    (void)new ValueTree(pR, key);
            }

            pR->Add();
            if (pR->GetOwned())                     // to-class gains the parent-pointer arg
                pTo->GetConstructorIncludeMethod()->UpdateArguments();
            ++out.relations;
        }
    }

    // 5) User methods, constructors, destructor body.
    for (const auto& pr : pairs)
    {
        Class* src = pr.first;
        Class* tgt = pr.second;

        BaseClass::MethodIterator iMeth(src);
        while (++iMeth)
        {
            Method* m = iMeth;
            if (IsUserMethod(m))
            {
                Method* pNM = new Method(tgt, pTgt->FindOrDuplicateType(m->GetType()), m);
                if (pNM->GetName() != m->GetName())    // paste renames virtual overloads
                    pNM->SetName(m->GetName());
                pNM->Add();
                tgt->NotifyAddMethod(pNM);
                ++out.methods;
            }
            else if (m->IsNormalConstructor())
            {
                Constructor* sc = dynamic_cast<Constructor*>(m);
                if (!sc)
                    continue;
                Constructor* nc = new Constructor(tgt, sc);
                nc->SetExplicit(sc->GetExplicit());
                nc->Add();
                tgt->NotifyAddMethod(nc);
                ++out.constructors;
            }
        }

        Method* srcD = 0;
        Method* tgtD = 0;
        BaseClass::MethodIterator iSD(src);
        while (++iSD) if (iSD->IsDestructor()) { srcD = iSD; break; }
        BaseClass::MethodIterator iTD(tgt);
        while (++iTD) if (iTD->IsDestructor()) { tgtD = iTD; break; }
        if (srcD && tgtD)
        {
            tgtD->SetCode(srcD->GetCode());
            tgtD->SetAccess(srcD->GetAccess());
            tgtD->SetVirtual(srcD->GetVirtual());
            tgtD->Update();
        }
    }

    // 6) A copied constructor body still calls ConstructorInclude(...) with the
    //    SOURCE's parent pointers. Re-sync it to the target's (a relation that
    //    did not come along drops its pointer), as UpdateArguments itself does.
    //    The constructor keeps the argument itself: user code may use it.
    for (const auto& pr : pairs)
    {
        const std::string srcCall = IncludeCall(pr.first);
        const std::string tgtCall = IncludeCall(pr.second);
        if (srcCall == tgtCall)
            continue;
        BaseClass::MethodIterator iCtor(pr.second, &Method::IsNormalConstructor);
        while (++iCtor)
            iCtor->ReplaceInCode(CbString(srcCall.c_str()), CbString(tgtCall.c_str()));
    }

    // 7) Anything a copy constructor still had to duplicate (a fallback the plan
    //    should have prevented) goes into the tree; never the internal "...".
    DataModelDoc::TypeIterator iType(pTgt);
    while (++iType)
        if (!iType->GetAdded() && ToStd(iType->GetName()) != "...")
            iType->Add();

    out.notes.insert(out.notes.end(), plan.notes.begin(), plan.notes.end());
    pTgt->SetModifiedFlag(1);
    return true;
}
