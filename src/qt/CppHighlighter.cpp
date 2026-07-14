// qt/CppHighlighter.cpp -- see CppHighlighter.h.

#include "CppHighlighter.h"

#include <QColor>
#include <QSet>
#include <QString>

namespace {

// C++ keywords (control flow, storage, operators-as-words). Built-in *types*
// live in the second set so they can take a distinct colour.
const QSet<QString>& keywords()
{
    static const QSet<QString> k = {
        "alignas", "alignof", "and", "and_eq", "asm", "break", "case", "catch",
        "class", "co_await", "co_return", "co_yield", "compl", "concept",
        "const", "consteval", "constexpr", "constinit", "const_cast",
        "continue", "decltype", "default", "delete", "do", "dynamic_cast",
        "else", "enum", "explicit", "export", "extern", "for", "friend", "goto",
        "if", "inline", "mutable", "namespace", "new", "noexcept", "not",
        "not_eq", "operator", "or", "or_eq", "private", "protected", "public",
        "register", "reinterpret_cast", "requires", "return", "sizeof",
        "static", "static_assert", "static_cast", "struct", "switch",
        "template", "this", "thread_local", "throw", "try", "typedef", "typeid",
        "typename", "union", "using", "virtual", "volatile", "while", "xor",
        "xor_eq", "override", "final",
    };
    return k;
}

// Built-in types + the common literals (true/false/nullptr read as types here).
const QSet<QString>& types()
{
    static const QSet<QString> t = {
        "auto", "bool", "char", "char8_t", "char16_t", "char32_t", "double",
        "float", "int", "long", "short", "signed", "unsigned", "void",
        "wchar_t", "true", "false", "nullptr", "size_t", "int8_t", "int16_t",
        "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
        // MFC/CB spellings that pepper the model's bodies.
        "BOOL", "TRUE", "FALSE", "NULL", "BYTE", "WORD", "DWORD", "UINT",
        "LONG", "CString", "POSITION",
    };
    return t;
}

bool isIdentStart(QChar c) { return c.isLetter() || c == '_'; }
bool isIdentChar(QChar c)  { return c.isLetterOrNumber() || c == '_'; }

} // namespace

CppHighlighter::CppHighlighter(QTextDocument* document)
    : QSyntaxHighlighter(document)
{
    // A palette tuned for black-on-white (the CodeEditor forces that base).
    _keyword.setForeground(QColor(0, 0, 255));       // blue
    _keyword.setFontWeight(QFont::Bold);
    _type.setForeground(QColor(43, 145, 175));       // teal
    _comment.setForeground(QColor(0, 128, 0));       // green
    _comment.setFontItalic(true);
    _string.setForeground(QColor(163, 21, 21));      // dark red
    _number.setForeground(QColor(128, 0, 128));      // purple
    _preprocessor.setForeground(QColor(128, 128, 128)); // grey
}

// Block state: 1 == this block ended inside an unterminated /* block comment.
void CppHighlighter::highlightBlock(const QString& text)
{
    int i = 0;

    // Continue a block comment carried in from the previous line.
    if (previousBlockState() == 1)
    {
        i = scanBlockComment(text, 0);
        // scanBlockComment sets the state; if it consumed the whole line the
        // state stays 1 and there is nothing else on this line.
        if (currentBlockState() == 1)
            return;
    }
    else
    {
        setCurrentBlockState(0);
    }

    // A '#' as the first non-space character makes the line a preprocessor
    // directive -- colour it whole (comments after it are a rare edge we skip).
    for (int j = i; j < text.length(); ++j)
    {
        if (text[j].isSpace())
            continue;
        if (text[j] == '#')
            i = scanPreprocessor(text, j);
        break;
    }

    while (i < text.length())
    {
        const QChar c = text[i];

        if (c == '/' && i + 1 < text.length() && text[i + 1] == '/')
            i = scanLineComment(text, i);
        else if (c == '/' && i + 1 < text.length() && text[i + 1] == '*')
            i = scanBlockComment(text, i);
        else if (c == '"' || c == '\'')
            i = scanStringLiteral(text, i);
        else if (c.isDigit())
            i = scanNumber(text, i);
        else if (isIdentStart(c))
            i = scanIdentifier(text, i);
        else
            ++i;
    }
}

int CppHighlighter::scanLineComment(const QString& text, int i)
{
    setFormat(i, text.length() - i, _comment);
    return text.length();
}

// Colours from `i` to the closing "*/" or end of line. Sets the block state to
// 1 (still open) or 0 (closed on this line).
int CppHighlighter::scanBlockComment(const QString& text, int i)
{
    const int end = text.indexOf("*/", i);
    if (end == -1)
    {
        setFormat(i, text.length() - i, _comment);
        setCurrentBlockState(1);
        return text.length();
    }
    setFormat(i, end + 2 - i, _comment);
    setCurrentBlockState(0);
    return end + 2;
}

// A "..." or '...' literal with backslash escapes. Unterminated (runs to end of
// line) is coloured as-is -- C++ forbids it, but the user is mid-typing.
int CppHighlighter::scanStringLiteral(const QString& text, int i)
{
    const QChar quote = text[i];
    int j = i + 1;
    while (j < text.length())
    {
        if (text[j] == '\\' && j + 1 < text.length())
        {
            j += 2;
            continue;
        }
        if (text[j] == quote)
        {
            ++j;
            break;
        }
        ++j;
    }
    setFormat(i, j - i, _string);
    return j;
}

int CppHighlighter::scanNumber(const QString& text, int i)
{
    int j = i;
    // Hex / binary / float / suffix chars -- a loose sweep, good enough to
    // tint the token without a full numeric grammar.
    while (j < text.length() &&
           (text[j].isLetterOrNumber() || text[j] == '.'))
        ++j;
    setFormat(i, j - i, _number);
    return j;
}

int CppHighlighter::scanIdentifier(const QString& text, int i)
{
    int j = i;
    while (j < text.length() && isIdentChar(text[j]))
        ++j;
    const QString word = text.mid(i, j - i);
    if (keywords().contains(word))
        setFormat(i, j - i, _keyword);
    else if (types().contains(word))
        setFormat(i, j - i, _type);
    return j;
}

int CppHighlighter::scanPreprocessor(const QString& text, int i)
{
    setFormat(i, text.length() - i, _preprocessor);
    return text.length();
}
