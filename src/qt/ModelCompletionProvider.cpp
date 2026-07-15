// qt/ModelCompletionProvider.cpp -- see ModelCompletionProvider.h.

#include "ModelCompletionProvider.h"
#include "QtModelText.h"             // toQ / toCb

#include <QHash>
#include <QSet>

#include <algorithm>

#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

bool isIdentChar(QChar c) { return c.isLetterOrNumber() || c == '_'; }

// The model's name-to-class lookups, rebuilt per completion request (the
// model is small and may have changed since the last keystroke).
struct TypeMaps
{
    QHash<QString, BaseClass*> classes;    // class / extern-class name
    QHash<QString, BaseClass*> iterators;  // <ToName>Iterator -> TARGET class
};

TypeMaps buildTypeMaps(Method* pMethod)
{
    TypeMaps maps;

    DataModel::ClassIterator iClass(
        pMethod->GetDataModelDoc()->GetDataModel());
    while (++iClass)
    {
        maps.classes.insert(toQ(iClass->GetName()), iClass.Get());
        Class::FromRelationIterator iRelation(iClass, &Relation::GetMulti);
        while (++iRelation)
            maps.iterators.insert(toQ(iRelation->GetToName()) + "Iterator",
                                  iRelation->GetToClass());
    }

    DataModelDoc::TypeIterator iType(pMethod->GetDataModelDoc(),
                                     &Type::IsExternClass);
    while (++iType)
        maps.classes.insert(toQ(iType->GetName()),
                            (ExternClass*)iType.Get());

    return maps;
}

// `TypeName [*&] varName` declarations scanned out of the text: every
// identifier token that is a known class or iterator type marks the next
// identifier token (across whitespace / '*' / '&') as a variable of it.
// "Matrix::RowIterator row" works because "::" re-evaluates the next token.
QHash<QString, QString> declaredVariables(const QString& text,
                                          const TypeMaps& maps)
{
    QHash<QString, QString> vars;      // variable name -> type name
    const int len = text.length();
    QString pendingType;
    int i = 0;
    while (i < len)
    {
        const QChar c = text[i];
        if (isIdentChar(c))
        {
            int j = i;
            while (j < len && isIdentChar(text[j]))
                ++j;
            const QString token = text.mid(i, j - i);
            if (!pendingType.isEmpty())
            {
                vars.insert(token, pendingType);
                pendingType.clear();
            }
            if (maps.classes.contains(token) ||
                maps.iterators.contains(token))
                pendingType = token;
            i = j;
        }
        else if (c == ' ' || c == '\t' || c == '*' || c == '&')
        {
            ++i;                       // allowed between type and variable
        }
        else if (c == ':' && i + 1 < len && text[i + 1] == ':')
        {
            i += 2;                    // qualified: next token re-evaluates
        }
        else
        {
            pendingType.clear();
            ++i;
        }
    }
    return vars;
}

CodeCompletionItem methodItem(Method* pMethod)
{
    CodeCompletionItem item;
    item.display   = toQ(pMethod->GetName()) + "()";
    item.insert    = item.display;
    item.caretBack = pMethod->GetArgumentCount() > 0 ? 1 : 0;
    return item;
}

CodeCompletionItem wordItem(const QString& word)
{
    CodeCompletionItem item;
    item.display = word;
    item.insert  = word;
    return item;
}

// All methods of `pClass` -- inherited ones included (macro methods come out
// of MethodIterator already). publicOnly for access through a variable;
// everything for the own class / this.
void collectMethods(BaseClass* pClass, bool publicOnly, QSet<QString>& seen,
                    QList<CodeCompletionItem>& out, int depth = 0)
{
    if (!pClass || depth > 8)
        return;

    BaseClass::MethodIterator iMethod(
        pClass, publicOnly ? &Method::IsPublicMethod : 0);
    while (++iMethod)
    {
        const QString name = toQ(iMethod->GetName());
        if (name.isEmpty() || seen.contains(name))
            continue;                  // overloads collapse to one entry
        seen.insert(name);
        out.append(methodItem(iMethod.Get()));
    }

    if (Class* pAsClass = dynamic_cast<Class*>(pClass))
    {
        Class::InheritIterator iInherit(pAsClass);
        while (++iInherit)
            collectMethods(iInherit->GetBaseClass(), publicOnly, seen, out,
                           depth + 1);
    }
}

void sortItems(QList<CodeCompletionItem>& items)
{
    std::sort(items.begin(), items.end(),
        [](const CodeCompletionItem& a, const CodeCompletionItem& b)
        { return a.display.compare(b.display, Qt::CaseInsensitive) < 0; });
}

} // namespace

ModelCompletionProvider::ModelCompletionProvider(Method* pMethod)
    : _pMethod(pMethod)
{
}

QList<CodeCompletionItem> ModelCompletionProvider::completions(
    const QString& textToCursor, int& prefixLen)
{
    QList<CodeCompletionItem> items;

    // --- Parse the tail: [base][. -> ::][prefix-being-typed] -------------
    const QString& text = textToCursor;
    const int end = text.length();
    int p = end;
    while (p > 0 && isIdentChar(text[p - 1]))
        --p;
    prefixLen = end - p;

    QString access;
    int basePos = p;
    if (p >= 1 && text[p - 1] == '.')
    {
        access = ".";
        basePos = p - 1;
    }
    else if (p >= 2 && text[p - 2] == '-' && text[p - 1] == '>')
    {
        access = "->";
        basePos = p - 2;
    }
    else if (p >= 2 && text[p - 2] == ':' && text[p - 1] == ':')
    {
        access = "::";
        basePos = p - 2;
    }

    QString base;
    if (!access.isEmpty())
    {
        int b = basePos;
        while (b > 0 && isIdentChar(text[b - 1]))
            --b;
        base = text.mid(b, basePos - b);
        if (base.isEmpty() || base[0].isDigit())
            return items;              // "3." float, ")->" chain -- no idea
    }

    const TypeMaps maps = buildTypeMaps(_pMethod);
    QSet<QString> seen;

    // --- var. / var-> : the resolved type's methods ----------------------
    if (access == "." || access == "->")
    {
        BaseClass* pClass = nullptr;
        bool publicOnly = true;

        if (base == "this")
        {
            pClass = _pMethod->GetBaseClass();
            publicOnly = false;
        }

        if (!pClass)                   // an argument of this method?
        {
            Method::ArgumentIterator iArgument(_pMethod);
            while (!pClass && ++iArgument)
            {
                if (toQ(iArgument->GetName()) == base)
                    pClass = maps.classes.value(
                        toQ(iArgument->GetType()->GetName()));
            }
        }

        if (!pClass)                   // a variable declared in the text?
        {
            const QString typeName =
                declaredVariables(text, maps).value(base);
            if (!typeName.isEmpty())
            {
                pClass = maps.classes.value(typeName);
                if (!pClass)           // iterator: deref to the target class
                    pClass = maps.iterators.value(typeName);
            }
        }

        if (!pClass)                   // a member of the owning class?
        {
            BaseClass::MemberIterator iMember(_pMethod->GetBaseClass());
            while (!pClass && ++iMember)
            {
                if (toQ(iMember->GetName()) == base)
                    pClass = maps.classes.value(
                        toQ(iMember->GetType()->GetName()));
            }
        }

        collectMethods(pClass, publicOnly, seen, items);
        sortItems(items);
        return items;
    }

    // --- Class:: : static methods + the class's iterator types -----------
    if (access == "::")
    {
        BaseClass* pClass = maps.classes.value(base);
        if (pClass)
        {
            BaseClass::MethodIterator iMethod(pClass);
            while (++iMethod)
            {
                const QString name = toQ(iMethod->GetName());
                if (iMethod->GetStatic() && !seen.contains(name))
                {
                    seen.insert(name);
                    items.append(methodItem(iMethod.Get()));
                }
            }
            if (Class* pAsClass = dynamic_cast<Class*>(pClass))
            {
                Class::FromRelationIterator iRelation(pAsClass,
                                                      &Relation::GetMulti);
                while (++iRelation)
                    items.append(wordItem(
                        toQ(iRelation->GetToName()) + "Iterator"));
            }
        }
        sortItems(items);
        return items;
    }

    // --- Plain identifier: everything reachable from here ----------------
    Method::ArgumentIterator iArgument(_pMethod);
    while (++iArgument)
    {
        const QString name = toQ(iArgument->GetName());
        if (!name.isEmpty() && !seen.contains(name))
        {
            seen.insert(name);
            items.append(wordItem(name));
        }
    }

    const QHash<QString, QString> vars = declaredVariables(text, maps);
    for (auto it = vars.constBegin(); it != vars.constEnd(); ++it)
    {
        if (!seen.contains(it.key()))
        {
            seen.insert(it.key());
            items.append(wordItem(it.key()));
        }
    }

    BaseClass::MemberIterator iMember(_pMethod->GetBaseClass());
    while (++iMember)
    {
        const QString name = toQ(iMember->GetName());
        if (!name.isEmpty() && !seen.contains(name))
        {
            seen.insert(name);
            items.append(wordItem(name));
        }
    }

    collectMethods(_pMethod->GetBaseClass(), false, seen, items);

    for (auto it = maps.classes.constBegin();
         it != maps.classes.constEnd(); ++it)
        if (!seen.contains(it.key()))
        {
            seen.insert(it.key());
            items.append(wordItem(it.key()));
        }
    for (auto it = maps.iterators.constBegin();
         it != maps.iterators.constEnd(); ++it)
        if (!seen.contains(it.key()))
        {
            seen.insert(it.key());
            items.append(wordItem(it.key()));
        }

    if (!seen.contains("this"))
        items.append(wordItem("this"));

    sortItems(items);
    return items;
}
