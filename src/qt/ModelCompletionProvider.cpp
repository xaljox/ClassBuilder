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

// A relation-generated iterator type: what it iterates (deref target) and
// who owns the relation (the iterator's constructor argument).
struct IteratorInfo
{
    BaseClass* target = nullptr;   // row-> offers THIS class's methods
    Class*     from   = nullptr;   // RowIterator iRow(<a Matrix*>)
    QString    toName;             // "Row" -- for the loop variable name
};

// The model's name-to-class lookups, rebuilt per completion request (the
// model is small and may have changed since the last keystroke).
struct TypeMaps
{
    QHash<QString, BaseClass*>   classes;    // class / extern-class name
    QHash<QString, IteratorInfo> iterators;  // <ToName>Iterator
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
        {
            IteratorInfo info;
            info.target = iRelation->GetToClass();
            info.from   = iClass.Get();
            info.toName = toQ(iRelation->GetToName());
            maps.iterators.insert(info.toName + "Iterator", info);
        }
    }

    DataModelDoc::TypeIterator iType(pMethod->GetDataModelDoc(),
                                     &Type::IsExternClass);
    while (++iType)
        maps.classes.insert(toQ(iType->GetName()),
                            (ExternClass*)iType.Get());

    return maps;
}

// True when the end of `text` sits inside a comment or a string / char
// literal -- no completion popup there.
bool inCommentOrString(const QString& text)
{
    const int len = text.length();
    bool lineComment = false, blockComment = false, inQuote = false;
    QChar quote;
    for (int i = 0; i < len; ++i)
    {
        const QChar c = text[i];
        if (lineComment)
        {
            if (c == '\n')
                lineComment = false;
        }
        else if (blockComment)
        {
            if (c == '*' && i + 1 < len && text[i + 1] == '/')
            {
                blockComment = false;
                ++i;
            }
        }
        else if (inQuote)
        {
            if (c == '\\')
                ++i;
            else if (c == quote)
                inQuote = false;
        }
        else if (c == '/' && i + 1 < len && text[i + 1] == '/')
        {
            lineComment = true;
            ++i;
        }
        else if (c == '/' && i + 1 < len && text[i + 1] == '*')
        {
            blockComment = true;
            ++i;
        }
        else if (c == '"' || c == '\'')
        {
            inQuote = true;
            quote = c;
        }
    }
    return lineComment || blockComment || inQuote;
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

// A method candidate: the popup shows the argument TYPES (what goes in),
// the insertion carries the argument NAMES (placeholders to overtype); the
// first name comes pre-selected via selectBack/selectLen.
CodeCompletionItem methodItem(Method* pMethod)
{
    const QString name = toQ(pMethod->GetName());
    QString displayArgs, insertArgs, firstName;

    Method::ArgumentIterator iArgument(pMethod);
    while (++iArgument)
    {
        if (!displayArgs.isEmpty())
        {
            displayArgs += ", ";
            insertArgs  += ", ";
        }
        const QString typeName = toQ(iArgument->GetTypeName()).trimmed();
        QString argName = toQ(iArgument->GetName());
        if (argName.isEmpty())
            argName = typeName;        // unnamed argument: type as placeholder
        displayArgs += typeName;
        insertArgs  += argName;
        if (firstName.isEmpty())
            firstName = argName;
    }

    CodeCompletionItem item;
    item.display = name + "(" + displayArgs + ")";
    item.insert  = name + "(" + insertArgs + ")";
    if (!firstName.isEmpty())
    {
        item.selectLen  = firstName.length();
        item.selectBack = item.insert.length()
                          - (name.length() + 1) - firstName.length();
    }
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
        if (iMethod->GetName().IsEmpty())
            continue;
        const CodeCompletionItem item = methodItem(iMethod.Get());
        if (seen.contains(item.display))
            continue;   // keyed by full signature: overloads stay distinct,
                        // overrides of the same signature collapse
        seen.insert(item.display);
        out.append(item);
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

// Find `name` as a method of `pClass` or one of its bases.
Method* findMethodInClass(BaseClass* pClass, const QString& name,
                          int depth = 0)
{
    if (!pClass || depth > 8)
        return nullptr;
    if (Method* pMethod = pClass->FindMethodWithName(toCb(name)))
        return pMethod;
    if (Class* pAsClass = dynamic_cast<Class*>(pClass))
    {
        Class::InheritIterator iInherit(pAsClass);
        while (++iInherit)
            if (Method* pMethod = findMethodInClass(iInherit->GetBaseClass(),
                                                    name, depth + 1))
                return pMethod;
    }
    return nullptr;
}

// The model class of the expression ENDING at `endPos` (exclusive): a bare
// identifier (this / argument / declared variable / member) or a call chain
// ending in ')' -- then the called method's return type, with the receiver
// resolved recursively, so `GetRow(i)->` and `_pDoc->GetModel()->` work.
BaseClass* resolveExpressionType(Method* pMethod, const QString& text,
                                 int endPos, const TypeMaps& maps,
                                 int depth = 0)
{
    if (depth > 8 || endPos <= 0)
        return nullptr;

    if (text[endPos - 1] == ')')
    {
        int open = 0;
        int i = endPos - 1;
        for (; i >= 0; --i)
        {
            if (text[i] == ')')
                ++open;
            else if (text[i] == '(' && --open == 0)
                break;
        }
        if (i < 0)
            return nullptr;

        int nameEnd = i;
        while (nameEnd > 0 && text[nameEnd - 1].isSpace())
            --nameEnd;
        int nameStart = nameEnd;
        while (nameStart > 0 && isIdentChar(text[nameStart - 1]))
            --nameStart;
        if (nameStart == nameEnd)
            return nullptr;
        const QString methodName = text.mid(nameStart, nameEnd - nameStart);

        BaseClass* pReceiver = nullptr;
        if (nameStart >= 1 && text[nameStart - 1] == '.')
            pReceiver = resolveExpressionType(pMethod, text, nameStart - 1,
                                              maps, depth + 1);
        else if (nameStart >= 2 && text[nameStart - 2] == '-' &&
                 text[nameStart - 1] == '>')
            pReceiver = resolveExpressionType(pMethod, text, nameStart - 2,
                                              maps, depth + 1);
        else if (nameStart >= 2 && text[nameStart - 2] == ':' &&
                 text[nameStart - 1] == ':')
        {
            int s = nameStart - 2;
            const int b = s;
            while (s > 0 && isIdentChar(text[s - 1]))
                --s;
            pReceiver = maps.classes.value(text.mid(s, b - s));
        }
        else
        {
            pReceiver = pMethod->GetBaseClass();   // own-class call
        }

        Method* pCalled = findMethodInClass(pReceiver, methodName);
        return pCalled
            ? maps.classes.value(toQ(pCalled->GetType()->GetName()))
            : nullptr;
    }

    int start = endPos;
    while (start > 0 && isIdentChar(text[start - 1]))
        --start;
    if (start == endPos)
        return nullptr;
    const QString name = text.mid(start, endPos - start);
    if (name[0].isDigit())
        return nullptr;

    if (name == "this")
        return pMethod->GetBaseClass();

    Method::ArgumentIterator iArgument(pMethod);
    while (++iArgument)
        if (toQ(iArgument->GetName()) == name)
            return maps.classes.value(toQ(iArgument->GetType()->GetName()));

    const QString typeName = declaredVariables(text, maps).value(name);
    if (!typeName.isEmpty())
    {
        if (BaseClass* pClass = maps.classes.value(typeName))
            return pClass;
        return maps.iterators.value(typeName).target;  // iterator deref
    }

    BaseClass::MemberIterator iMember(pMethod->GetBaseClass());
    while (++iMember)
        if (toQ(iMember->GetPrefixedName()) == name)
            return maps.classes.value(toQ(iMember->GetType()->GetName()));

    return nullptr;
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
    if (access == "::")
    {
        int b = basePos;
        while (b > 0 && isIdentChar(text[b - 1]))
            --b;
        base = text.mid(b, basePos - b);
        if (base.isEmpty() || base[0].isDigit())
            return items;
    }

    // No popup inside comments and string / char literals.
    if (inCommentOrString(text.left(basePos)))
        return items;

    const TypeMaps maps = buildTypeMaps(_pMethod);
    QSet<QString> seen;

    // --- expr. / expr-> : the resolved type's methods ---------------------
    // The expression may be a variable OR a call chain (GetRow(i)->).
    if (access == "." || access == "->")
    {
        const bool isThis = (basePos >= 4 &&
                             text.mid(basePos - 4, 4) == "this" &&
                             (basePos == 4 || !isIdentChar(text[basePos - 5])));
        BaseClass* pClass =
            resolveExpressionType(_pMethod, text, basePos, maps);
        collectMethods(pClass, !isThis, seen, items);
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
                if (!iMethod->GetStatic())
                    continue;
                const CodeCompletionItem item = methodItem(iMethod.Get());
                if (!seen.contains(item.display))
                {
                    seen.insert(item.display);
                    items.append(item);
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
        const QString name = toQ(iMember->GetPrefixedName());
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

    // Iterator types, each with a second "loop" item: the full while-loop
    // skeleton, its constructor argument pre-filled with something of the
    // relation's from-class that is in scope (this, an argument, a declared
    // variable, or a member) -- the Iterator wizard's knowledge, inline.
    // (IsBaseClass(BaseClass*) -- the inheritance test -- lives on
    // ExternClass; BaseClass::IsBaseClass() is the Gti type predicate.)
    for (auto it = maps.iterators.constBegin();
         it != maps.iterators.constEnd(); ++it)
    {
        if (!seen.contains(it.key()))
        {
            seen.insert(it.key());
            items.append(wordItem(it.key()));
        }

        const IteratorInfo& info = it.value();
        QString receiver;
        BaseClass* pOwn = _pMethod->GetBaseClass();
        ExternClass* pOwnExt = dynamic_cast<ExternClass*>(pOwn);
        if (pOwn == info.from ||
            (pOwnExt && info.from && pOwnExt->IsBaseClass(info.from)))
            receiver = "this";
        if (receiver.isEmpty())
        {
            const QString fromName = info.from
                ? toQ(info.from->GetName()) : QString();
            Method::ArgumentIterator iArgument(_pMethod);
            while (receiver.isEmpty() && ++iArgument)
                if (toQ(iArgument->GetType()->GetName()) == fromName)
                    receiver = toQ(iArgument->GetName());
            for (auto v = vars.constBegin();
                 receiver.isEmpty() && v != vars.constEnd(); ++v)
                if (v.value() == fromName)
                    receiver = v.key();
            BaseClass::MemberIterator iFromMember(pOwn);
            while (receiver.isEmpty() && ++iFromMember)
                if (toQ(iFromMember->GetType()->GetName()) == fromName)
                    receiver = toQ(iFromMember->GetPrefixedName());
        }

        const QString loopVar = "i" + info.toName;
        CodeCompletionItem loop;
        loop.display = it.key() + " loop";
        loop.insert  = QString("%1 %2(%3);\nwhile (++%2)\n{\n}")
                           .arg(it.key(), loopVar, receiver);
        items.append(loop);
    }

    if (!seen.contains("this"))
        items.append(wordItem("this"));

    sortItems(items);
    return items;
}
