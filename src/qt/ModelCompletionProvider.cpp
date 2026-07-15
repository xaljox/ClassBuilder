// qt/ModelCompletionProvider.cpp -- see ModelCompletionProvider.h.

#include "ModelCompletionProvider.h"
#include "QtModelText.h"             // toQ / toCb

#include <QHash>
#include <QSet>
#include <QStringList>

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
    QHash<QString, IteratorInfo> iterators;  // <ToName>Iterator (same-named
                                             // iterators of different classes
                                             // collapse -- deref target only)
    QList<IteratorInfo>          relationIterators;  // one per multi-relation
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
            maps.relationIterators.append(info);
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

// Something of the relation's from-class that is in scope, for the loop
// item's constructor argument: `this` when the edited class is (or derives
// from) the from-class, else a suitable argument, declared variable, or
// member -- the Iterator wizard's knowledge, inline. Empty when nothing
// qualifies. (IsBaseClass(BaseClass*) -- the inheritance test -- lives on
// ExternClass; BaseClass::IsBaseClass() is the Gti type predicate.)
QString iteratorReceiver(Method* pMethod, const IteratorInfo& info,
                         const QHash<QString, QString>& vars)
{
    BaseClass* pOwn = pMethod->GetBaseClass();
    ExternClass* pOwnExt = dynamic_cast<ExternClass*>(pOwn);
    if (pOwn == info.from ||
        (pOwnExt && info.from && pOwnExt->IsBaseClass(info.from)))
        return "this";

    const QString fromName = info.from
        ? toQ(info.from->GetName()) : QString();
    Method::ArgumentIterator iArgument(pMethod);
    while (++iArgument)
        if (toQ(iArgument->GetType()->GetName()) == fromName)
            return toQ(iArgument->GetName());
    for (auto v = vars.constBegin(); v != vars.constEnd(); ++v)
        if (v.value() == fromName)
            return v.key();
    BaseClass::MemberIterator iMember(pOwn);
    while (++iMember)
        if (toQ(iMember->GetType()->GetName()) == fromName)
            return toQ(iMember->GetPrefixedName());
    return QString();
}

// The "<Name>Iterator loop" item for `typeName` (scope-qualified where
// needed): the full while-loop skeleton, the constructor argument
// pre-filled with `receiver`.
CodeCompletionItem loopItem(const QString& typeName, const QString& toName,
                            const QString& receiver)
{
    const QString loopVar = "i" + toName;
    CodeCompletionItem item;
    item.display = typeName + " loop";
    item.insert  = QString("%1 %2(%3);\nwhile (++%2)\n{\n}")
                       .arg(typeName, loopVar, receiver);
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
        // Not callable through an expression: constructors, destructors,
        // and = delete'd methods.
        if (iMethod->IsConstructor() || iMethod->IsDestructor() ||
            iMethod->GetDelete())
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

// --- Hover documentation -----------------------------------------------------

// Signature in the editor's own code font (the rich-text default fixed font
// -- Courier New on Windows -- renders too thin); the model's @NOTE text
// (when there is one) below a rule, line breaks preserved.
QString hoverHtml(const QString& signature, const CbString& note)
{
    const QFont codeFont = CodeEditor::codeFont();
    QString html = QString("<pre style='margin:0;"
                           " font-family:\"%1\"; font-size:%2pt'>")
                       .arg(codeFont.family())
                       .arg(codeFont.pointSize())
                   + signature.toHtmlEscaped() + "</pre>";
    QString noteText = toQ(note).trimmed();
    if (!noteText.isEmpty())
    {
        noteText.replace("\r\n", "\n");

        // Long notes are capped -- the tooltip is a reminder, not the docs.
        const int maxLines = 12;
        const int maxChars = 700;
        bool truncated = false;
        QStringList lines = noteText.split('\n');
        if (lines.size() > maxLines)
        {
            lines = lines.mid(0, maxLines);
            truncated = true;
        }
        noteText = lines.join('\n');
        if (noteText.length() > maxChars)
        {
            noteText.truncate(maxChars);
            const int lastSpace = noteText.lastIndexOf(' ');
            if (lastSpace > maxChars - 60)
                noteText.truncate(lastSpace);
            truncated = true;
        }

        noteText = noteText.toHtmlEscaped();
        noteText.replace('\n', "<br/>");
        if (truncated)
            noteText += "&hellip;";

        // A point below the signature: readable, but clearly secondary.
        html += QString("<hr/><span style='font-size:%1pt'>")
                    .arg(codeFont.pointSize() - 1)
                + noteText + "</span>";
    }
    return html;
}

// GetInterfaceCpp gives `Type Class::Name(args) const`; a constructor
// carries a trailing " //@INIT_n" marker that means nothing here.
QString methodSignature(Method* pMethod)
{
    QString signature = toQ(pMethod->GetInterfaceCpp());
    const int marker = signature.indexOf(" //@INIT_");
    if (marker >= 0)
        signature.truncate(marker);
    return signature;
}

// Number of arguments in the call whose '(' sits at `parenPos`: top-level
// commas counted, nested parens and string/char literals respected. -1 when
// the list is not (yet) closed -- arity matching is skipped then.
int callArgumentCount(const QString& text, int parenPos)
{
    const int len = text.length();
    int depth = 0, count = 0;
    bool any = false, inQuote = false;
    QChar quote;
    for (int i = parenPos; i < len; ++i)
    {
        const QChar c = text[i];
        if (inQuote)
        {
            if (c == '\\')
                ++i;
            else if (c == quote)
                inQuote = false;
        }
        else if (c == '"' || c == '\'')
        {
            inQuote = true;
            quote = c;
            any = true;
        }
        else if (c == '(')
        {
            ++depth;
        }
        else if (c == ')')
        {
            if (--depth == 0)
                return any ? count + 1 : 0;
        }
        else if (c == ',' && depth == 1)
        {
            ++count;
            any = true;
        }
        else if (!c.isSpace())
        {
            any = true;
        }
    }
    return -1;
}

// Every method named `name` on `pClass` and its bases -- the overload set
// (the derived-most class comes first).
void findMethodsInClass(BaseClass* pClass, const QString& name,
                        QList<Method*>& out, int depth = 0)
{
    if (!pClass || depth > 8)
        return;
    BaseClass::MethodIterator iMethod(pClass);
    while (++iMethod)
        if (toQ(iMethod->GetName()) == name)
            out.append(iMethod.Get());
    if (Class* pAsClass = dynamic_cast<Class*>(pClass))
    {
        Class::InheritIterator iInherit(pAsClass);
        while (++iInherit)
            findMethodsInClass(iInherit->GetBaseClass(), name, out,
                               depth + 1);
    }
}

// The overload matching the call behind the identifier ending at `end`:
// the candidate whose arity spans the call's argument count (arguments
// with a default value are optional). A single candidate always matches;
// null when it cannot be decided (no closed argument list, no fit).
Method* matchOverload(const QList<Method*>& candidates, const QString& text,
                      int end)
{
    if (candidates.size() == 1)
        return candidates.first();

    const int len = text.length();
    int k = end;
    while (k < len && text[k].isSpace())
        ++k;
    if (k >= len || text[k] != '(')
        return nullptr;
    const int count = callArgumentCount(text, k);
    if (count < 0)
        return nullptr;

    for (Method* pCandidate : candidates)
    {
        int required = 0, total = 0;
        Method::ArgumentIterator iArgument(pCandidate);
        while (++iArgument)
        {
            ++total;
            if (iArgument->GetDefault().IsEmpty())
                ++required;
        }
        if (count >= required && count <= total)
            return pCandidate;
    }
    return nullptr;
}

// The parameter-hint line: the signature rebuilt from its parts (the flat
// GetInterfaceCpp string cannot mark the active argument), the active
// argument bold, default values included, in the editor's code font.
QString parameterHintHtml(Method* pMethod, int activeArg)
{
    const QFont codeFont = CodeEditor::codeFont();
    QString html = QString("<pre style='margin:0;"
                           " font-family:\"%1\"; font-size:%2pt'>")
                       .arg(codeFont.family())
                       .arg(codeFont.pointSize());
    html += (toQ(pMethod->GetTypeName()) +
             toQ(pMethod->GetBaseClass()->GetName()) + "::" +
             toQ(pMethod->GetName()) + "(").toHtmlEscaped();

    int index = 0;
    Method::ArgumentIterator iArgument(pMethod);
    while (++iArgument)
    {
        if (index)
            html += ", ";
        QString piece = toQ(iArgument->GetTypeName()
                            + iArgument->GetVariableName());
        if (!iArgument->GetDefault().IsEmpty())
            piece += " = " + toQ(iArgument->GetDefault());
        piece = piece.toHtmlEscaped();
        html += (index == activeArg) ? "<b>" + piece + "</b>" : piece;
        ++index;
    }

    html += ")";
    if (pMethod->GetConst())
        html += " const";
    html += "</pre>";
    return html;
}

// All candidates' signatures, one per line -- the undecided-overload hover.
QString joinedSignatures(const QList<Method*>& candidates)
{
    QString signatures;
    for (Method* pCandidate : candidates)
    {
        if (!signatures.isEmpty())
            signatures += '\n';
        signatures += methodSignature(pCandidate);
    }
    return signatures;
}

} // namespace

ModelCompletionProvider::ModelCompletionProvider(Method* pMethod)
    : _pMethod(pMethod)
{
}

QString ModelCompletionProvider::parameterHint(const QString& textToCursor)
{
    const QString& text = textToCursor;
    if (inCommentOrString(text))
        return QString();

    // The innermost UNCLOSED '(' before the caret, and how many top-level
    // commas the caret has passed inside it (= the active argument index).
    int depth = 0, open = -1, commas = 0;
    for (int i = text.length() - 1; i >= 0; --i)
    {
        const QChar c = text[i];
        if (c == ')')
        {
            ++depth;
        }
        else if (c == '(')
        {
            if (depth == 0)
            {
                open = i;
                break;
            }
            --depth;
        }
        else if (c == ',' && depth == 0)
        {
            ++commas;
        }
        else if ((c == ';' || c == '{' || c == '}') && depth == 0)
        {
            return QString();          // statement boundary: no open call
        }
    }
    if (open < 0)
        return QString();
    const int activeArg = commas;

    // The called name just before the '('.
    int nameEnd = open;
    while (nameEnd > 0 && text[nameEnd - 1].isSpace())
        --nameEnd;
    int nameStart = nameEnd;
    while (nameStart > 0 && isIdentChar(text[nameStart - 1]))
        --nameStart;
    if (nameStart == nameEnd)
        return QString();
    const QString name = text.mid(nameStart, nameEnd - nameStart);
    if (name[0].isDigit())
        return QString();
    // Control-flow parens are not calls.
    static const QSet<QString> keywords = { "if", "while", "for", "switch",
                                            "return", "sizeof", "catch" };
    if (keywords.contains(name))
        return QString();

    const TypeMaps maps = buildTypeMaps(_pMethod);

    // The receiver, resolved exactly like definitionAtCursor.
    BaseClass* pReceiver = nullptr;
    if (nameStart >= 1 && text[nameStart - 1] == '.')
        pReceiver = resolveExpressionType(_pMethod, text, nameStart - 1, maps);
    else if (nameStart >= 2 && text[nameStart - 2] == '-' &&
             text[nameStart - 1] == '>')
        pReceiver = resolveExpressionType(_pMethod, text, nameStart - 2, maps);
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
        pReceiver = _pMethod->GetBaseClass();
    }

    QList<Method*> named;
    findMethodsInClass(pReceiver, name, named);
    if (named.isEmpty())
    {
        // A class name -- `new Row(`, a declaration: its constructors (they
        // carry the class's name, so the same lookup applies).
        if (BaseClass* pClass = maps.classes.value(name))
            findMethodsInClass(pClass, name, named);
    }
    if (named.isEmpty())
        return QString();

    // The overload that still has room for the argument being typed wins;
    // with none (too many arguments already), keep the first as anchor.
    Method* pBest = named.first();
    for (Method* pCandidate : named)
    {
        int total = 0;
        Method::ArgumentIterator iArgument(pCandidate);
        while (++iArgument)
            ++total;
        if (activeArg < total)
        {
            pBest = pCandidate;
            break;
        }
    }
    return parameterHintHtml(pBest, activeArg);
}

QString ModelCompletionProvider::hoverText(const QString& text, int pos)
{
    const int len = text.length();
    int start = pos, end = pos;
    while (start > 0 && isIdentChar(text[start - 1]))
        --start;
    while (end < len && isIdentChar(text[end]))
        ++end;
    if (start == end || text[start].isDigit())
        return QString();
    const QString name = text.mid(start, end - start);

    const TypeMaps maps = buildTypeMaps(_pMethod);

    // The receiver, resolved exactly like definitionAtCursor.
    bool qualified = true;
    BaseClass* pReceiver = nullptr;
    if (start >= 1 && text[start - 1] == '.')
        pReceiver = resolveExpressionType(_pMethod, text, start - 1, maps);
    else if (start >= 2 && text[start - 2] == '-' && text[start - 1] == '>')
        pReceiver = resolveExpressionType(_pMethod, text, start - 2, maps);
    else if (start >= 2 && text[start - 2] == ':' && text[start - 1] == ':')
    {
        int s = start - 2;
        const int b = s;
        while (s > 0 && isIdentChar(text[s - 1]))
            --s;
        pReceiver = maps.classes.value(text.mid(s, b - s));
    }
    else
    {
        qualified = false;
        pReceiver = _pMethod->GetBaseClass();
    }

    QList<Method*> named;
    findMethodsInClass(pReceiver, name, named);
    if (!named.isEmpty())
    {
        if (Method* pMatch = matchOverload(named, text, end))
            return hoverHtml(methodSignature(pMatch), pMatch->GetNote());
        return hoverHtml(joinedSignatures(named), CbString());
    }

    if (!qualified)
    {
        Method::ArgumentIterator iArgument(_pMethod);
        while (++iArgument)
            if (toQ(iArgument->GetName()) == name)
                return hoverHtml(
                    toQ(iArgument->GetTypeName()
                        + iArgument->GetVariableName()),
                    iArgument->GetNote());

        BaseClass::MemberIterator iMember(_pMethod->GetBaseClass());
        while (++iMember)
            if (toQ(iMember->GetPrefixedName()) == name)
                return hoverHtml(
                    toQ(iMember->GetTypeName() + iMember->GetPrefixedName()),
                    iMember->GetNote());

        if (BaseClass* pClass = maps.classes.value(name))
        {
            // After `new` the name means a constructor call -- show the
            // constructor signature(s), not the class.
            int q = start;
            while (q > 0 && (text[q - 1] == ' ' || text[q - 1] == '\t'))
                --q;
            if (q >= 3 && text.mid(q - 3, 3) == "new" &&
                (q == 3 || !isIdentChar(text[q - 4])))
            {
                QList<Method*> constructors;
                BaseClass::MethodIterator iMethod(pClass);
                while (++iMethod)
                    if (iMethod->IsConstructor() && !iMethod->GetDelete())
                        constructors.append(iMethod.Get());
                if (!constructors.isEmpty())
                {
                    if (Method* pMatch = matchOverload(constructors, text,
                                                       end))
                        return hoverHtml(methodSignature(pMatch),
                                         pMatch->GetNote());
                    return hoverHtml(joinedSignatures(constructors),
                                     CbString());
                }
            }
            return hoverHtml("class " + toQ(pClass->GetName()),
                             pClass->GetNote());
        }
    }
    return QString();
}

Gti* ModelCompletionProvider::definitionAtCursor(const QString& text, int pos)
{
    const int len = text.length();
    int start = pos, end = pos;
    while (start > 0 && isIdentChar(text[start - 1]))
        --start;
    while (end < len && isIdentChar(text[end]))
        ++end;
    if (start == end || text[start].isDigit())
        return nullptr;
    const QString name = text.mid(start, end - start);

    const TypeMaps maps = buildTypeMaps(_pMethod);

    BaseClass* pReceiver = nullptr;
    if (start >= 1 && text[start - 1] == '.')
        pReceiver = resolveExpressionType(_pMethod, text, start - 1, maps);
    else if (start >= 2 && text[start - 2] == '-' && text[start - 1] == '>')
        pReceiver = resolveExpressionType(_pMethod, text, start - 2, maps);
    else if (start >= 2 && text[start - 2] == ':' && text[start - 1] == ':')
    {
        int s = start - 2;
        const int b = s;
        while (s > 0 && isIdentChar(text[s - 1]))
            --s;
        pReceiver = maps.classes.value(text.mid(s, b - s));
    }
    else
    {
        pReceiver = _pMethod->GetBaseClass();
    }

    if (Method* pFound = findMethodInClass(pReceiver, name))
        return pFound;

    // Not a method of the receiver -- a CLASS name (`new Row(this, r)`, a
    // declaration, a cast)? Its constructor when it has one (that is what
    // `new` calls), else the class itself.
    if (BaseClass* pClass = maps.classes.value(name))
    {
        if (Method* pCtor = findMethodInClass(pClass, name))
            return pCtor;
        return pClass;
    }
    return nullptr;
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
                // The insertion lands behind the already-typed "Class::",
                // so the snippet's type name stays unqualified.
                const QHash<QString, QString> vars =
                    declaredVariables(text, maps);
                Class::FromRelationIterator iRelation(pAsClass,
                                                      &Relation::GetMulti);
                while (++iRelation)
                {
                    IteratorInfo info;
                    info.target = iRelation->GetToClass();
                    info.from   = pAsClass;
                    info.toName = toQ(iRelation->GetToName());
                    const QString typeName = info.toName + "Iterator";
                    items.append(wordItem(typeName));
                    items.append(loopItem(typeName, info.toName,
                        iteratorReceiver(_pMethod, info, vars)));
                }
            }
        }
        sortItems(items);
        return items;
    }

    // --- new <Class> : constructors, argument placeholders included ------
    // A bare class name would compile-complete too, but then the arguments
    // are lost -- `new` calls a constructor, so offer those.
    int q = p;
    while (q > 0 && (text[q - 1] == ' ' || text[q - 1] == '\t'))
        --q;
    if (q >= 3 && text.mid(q - 3, 3) == "new" &&
        (q == 3 || !isIdentChar(text[q - 4])))
    {
        for (auto it = maps.classes.constBegin();
             it != maps.classes.constEnd(); ++it)
        {
            bool hasConstructor = false;
            BaseClass::MethodIterator iMethod(it.value());
            while (++iMethod)
            {
                if (!iMethod->IsConstructor() || iMethod->GetDelete())
                    continue;
                hasConstructor = true;
                const CodeCompletionItem item = methodItem(iMethod.Get());
                if (!seen.contains(item.display))
                {
                    seen.insert(item.display);
                    items.append(item);
                }
            }
            if (!hasConstructor && !seen.contains(it.key()))
            {
                seen.insert(it.key());     // no modeled constructor: the
                items.append(wordItem(it.key()));  // bare class name then
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

    // Iterator types, one per relation, each with a second "loop" item (the
    // full while-loop skeleton, receiver pre-filled). A relation of another
    // class is offered scope-qualified -- inside a Matrix method, Row->Cell
    // is Row::CellIterator; only the own class's (and its bases') relations
    // come unqualified.
    for (const IteratorInfo& info : maps.relationIterators)
    {
        BaseClass* pOwn = _pMethod->GetBaseClass();
        ExternClass* pOwnExt = dynamic_cast<ExternClass*>(pOwn);
        const bool ownRelation = pOwn == info.from ||
            (pOwnExt && info.from && pOwnExt->IsBaseClass(info.from));
        const QString typeName = (ownRelation ? QString()
            : toQ(info.from->GetName()) + "::") + info.toName + "Iterator";
        if (!seen.contains(typeName))
        {
            seen.insert(typeName);
            items.append(wordItem(typeName));
        }
        items.append(loopItem(typeName, info.toName,
                              iteratorReceiver(_pMethod, info, vars)));
    }

    if (!seen.contains("this"))
        items.append(wordItem("this"));

    sortItems(items);
    return items;
}
