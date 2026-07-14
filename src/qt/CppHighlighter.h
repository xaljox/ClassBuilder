// qt/CppHighlighter.h -- C++ syntax highlighting for the CodeEditor.
//
// A QSyntaxHighlighter attached to a CodeEditor's document (the editor owns
// one; see CodeEditor's ctor). It colours keywords, built-in types, string and
// character literals, numbers, preprocessor directives and comments -- both
// // line comments and multi-line block comments, the latter carried across
// blocks with the highlighter's block state.
//
// It is a scanner, not a list of regexes: highlightBlock walks the line
// character by character. That is what makes "http://x" stay a string and
// '"' stay a character literal -- the regex-per-rule shape from the Qt
// "syntax highlighter" example gets both of those wrong.
//
// There is no semantic colouring here (no "this identifier is a class in the
// model"). The editor holds one method body, and the model knowledge is
// better spent on completion than on tinting names.
#pragma once

#include <QSet>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

class CppHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit CppHighlighter(QTextDocument* document);

    // Names known from the data model. Model types take the same teal as the
    // built-in types (one consistent "known type" signal, no extra colour);
    // the method's own arguments render italic (emphasis without noise).
    void setModelTypes(const QSet<QString>& names);
    void setArgumentNames(const QSet<QString>& names);

protected:
    void highlightBlock(const QString& text) override;

private:
    // Scan helpers -- each takes the line and the index of its first character,
    // formats the token, and returns the index just past it.
    int scanLineComment(const QString& text, int i);
    int scanBlockComment(const QString& text, int i);   // sets the block state
    int scanStringLiteral(const QString& text, int i);  // "..." and '...'
    int scanNumber(const QString& text, int i);
    int scanIdentifier(const QString& text, int i);
    int scanPreprocessor(const QString& text, int i);

    QTextCharFormat _keyword;
    QTextCharFormat _type;
    QTextCharFormat _comment;
    QTextCharFormat _string;
    QTextCharFormat _number;
    QTextCharFormat _preprocessor;
    QTextCharFormat _argument;

    QSet<QString> _modelTypes;      // model class/type names -> _type teal
    QSet<QString> _argumentNames;   // this method's arguments -> italic
};
