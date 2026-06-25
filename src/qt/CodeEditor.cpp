// qt/CodeEditor.cpp -- a plain-text C++ code editor widget.
//
// Ported from the MFC CCodeEdit (ClassBuilder/CodeEdit.cpp). The core is the
// indent predictor: GetIndent walks the text up to the cursor line by line,
// and GetNextLineIndent predicts the indent of the line that follows each.

#include "CodeEditor.h"

#include <QFont>
#include <QKeyEvent>
#include <QLabel>
#include <QPalette>
#include <QResizeEvent>
#include <QTextBlock>
#include <QTextCursor>

namespace {

// Strip C++ comments from `code` in place. Returns true when `code` ends
// inside an unterminated block comment -- the MFC CCodeEdit::StripComment.
// Works on '\n'-separated text (QPlainTextEdit's plain text).
bool stripComment(QString& code)
{
    int index;
    while ((index = code.indexOf("//")) != -1)
    {
        QString head = code.left(index);
        code = code.mid(index + 2);
        const int nl = code.indexOf('\n');
        if (nl != -1)
            head += '\n' + code.mid(nl + 1);
        code = head;
    }
    while ((index = code.indexOf("/*")) != -1)
    {
        QString head = code.left(index);
        code = code.mid(index + 2);
        const int end = code.indexOf("*/");
        if (end == -1)
            return true;                 // still inside a block comment
        head += code.mid(end + 2);
        code = head;
    }
    return false;
}

// Remove balanced `{...}` blocks, innermost-last -- the MFC StripBracketCode.
void stripBracketCode(QString& code)
{
    int closeIndex;
    while ((closeIndex = code.lastIndexOf('}')) != -1)
    {
        int open = 0;
        int openIndex = closeIndex;
        for (; openIndex >= 0; --openIndex)
        {
            const QChar c = code[openIndex];
            if (c != '{') --open;
            if (c != '}') ++open;
            if (open == 0)
                break;
        }
        if (openIndex == -1)
            break;
        code = code.left(openIndex) + code.mid(closeIndex + 1);
    }
}

// Count the leading-whitespace indent of `line`, tabs rounded to 8 columns.
int leadingIndent(const QString& line)
{
    int indent = 0;
    for (int i = 0; i < line.length() && line[i].isSpace(); ++i)
        indent = (line[i] == '\t') ? ((indent + 8) / 8) * 8 : indent + 1;
    return indent;
}

// Carries the predictor state across the line-by-line walk -- the statics in
// the MFC GetNextLineIndent (refIndent / indent / prevLastChar).
struct IndentState
{
    int   refIndent;
    int   indent;
    QChar prevLastChar = '{';
};

// Predict the indent of the line that follows `line` -- the MFC
// GetNextLineIndent. `line` is expected right-trimmed.
int nextLineIndent(IndentState& s, const QString& line, int step)
{
    const int len = line.length();
    if (len == 0)
        return s.indent;

    int i = 0;
    int lineIndent = 0;
    for (; i < len && line[i].isSpace(); ++i)
        lineIndent = (line[i] == '\t') ? ((lineIndent + 8) / 8) * 8
                                       : lineIndent + 1;

    // A line starting with '#' (preprocessor) does not affect the indent.
    if (i < len && line[i] == '#')
        return s.indent;

    const bool prevBreak = (s.prevLastChar == '{' || s.prevLastChar == '}' ||
                            s.prevLastChar == ';');
    if (prevBreak)
        s.refIndent = lineIndent;

    const QChar last = line[len - 1];
    if (last == '}')
        s.refIndent = s.indent = lineIndent;
    else if (last == '{')
        s.indent = s.refIndent + step;
    else if (last == ';')
        s.indent = s.refIndent;
    else if (last == ':')
        s.indent = s.refIndent + step;
    else
        s.indent = prevBreak ? s.refIndent + step : lineIndent;

    if (last != ':')
        s.prevLastChar = last;

    return s.indent;
}

} // namespace

// The shared editor font. A Medium weight -- a normal-weight monospace
// antialiased reads as thin and washed-out grey; Medium gives crisp black
// strokes. Reused by callers that put a marker label next to the editor so
// the label matches the code exactly.
QFont CodeEditor::codeFont()
{
    QFont f("Consolas");
    f.setStyleHint(QFont::Monospace);
    f.setFixedPitch(true);
    f.setPointSize(11);
    f.setWeight(QFont::Medium);
    return f;
}

CodeEditor::CodeEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setFont(codeFont());

    // Force crisp black-on-white -- do not inherit a washed-out palette
    // from the app-wide style.
    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::white);
    pal.setColor(QPalette::Text, Qt::black);
    setPalette(pal);

    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabChangesFocus(false);

    QFontMetricsF fm(font());
    setTabStopDistance(fm.horizontalAdvance(' ') * _indentSize);
}

void CodeEditor::setIndentSize(int spaces)
{
    if (spaces > 0)
    {
        _indentSize = spaces;
        QFontMetricsF fm(font());
        setTabStopDistance(fm.horizontalAdvance(' ') * _indentSize);
    }
}

// The MFC CCodeEdit::GetIndent. Returns the indent (in spaces) for a new
// line inserted at the cursor.
int CodeEditor::computeIndent() const
{
    QString code = toPlainText().left(textCursor().position());

    // Inside an unterminated block comment -- just keep the current line's
    // own indent (the MFC GetLineIndent fallback).
    QString stripped = code;
    if (stripComment(stripped))
    {
        const int nl = code.lastIndexOf('\n');
        return leadingIndent(code.mid(nl + 1));
    }
    code = stripped;

    IndentState s;
    s.refIndent = _indentSize;
    s.indent    = _indentSize;

    int index;
    while ((index = code.indexOf('\n')) != -1)
    {
        QString line = code.left(index);
        while (!line.isEmpty() && line.back().isSpace())
            line.chop(1);
        nextLineIndent(s, line, _indentSize);
        code = code.mid(index + 1);
    }
    while (!code.isEmpty() && code.back().isSpace())
        code.chop(1);
    return nextLineIndent(s, code, _indentSize);
}

void CodeEditor::insertNewlineWithIndent()
{
    QTextCursor cur = textCursor();
    cur.beginEditBlock();

    // Drop any whitespace immediately after the cursor on the current line,
    // so a re-indented line is not left with trailing spaces.
    while (!cur.atBlockEnd())
    {
        cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        const QString ch = cur.selectedText();
        if (ch == " " || ch == "\t")
            cur.removeSelectedText();
        else
        {
            cur.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
            break;
        }
        cur = textCursor();
    }

    const int indent = computeIndent();
    cur.insertText('\n' + QString(indent, ' '));
    cur.endEditBlock();
    setTextCursor(cur);
}

// Typing '}' on an otherwise-blank line: re-indent that line so the brace
// lines up one step out from the block body.
void CodeEditor::reindentClosingBrace()
{
    QTextCursor cur = textCursor();
    const QString block = cur.block().text();
    const int col = cur.positionInBlock();

    // Only when everything before the cursor on this line is whitespace.
    for (int i = 0; i < col; ++i)
        if (!block[i].isSpace())
            return;

    int target = computeIndent() - _indentSize;
    if (target < 0)
        target = 0;

    cur.beginEditBlock();
    QTextCursor line = cur;
    line.movePosition(QTextCursor::StartOfBlock);
    line.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, col);
    line.insertText(QString(target, ' '));
    cur.endEditBlock();
}

// Tab / Shift+Tab over a multi-line selection: indent or unindent each line.
void CodeEditor::indentSelection(bool unindent)
{
    QTextCursor cur = textCursor();
    const int selStart = cur.selectionStart();
    const int selEnd   = cur.selectionEnd();

    QTextCursor it(document());
    it.setPosition(selStart);
    const int firstBlock = it.blockNumber();
    it.setPosition(selEnd);
    const int lastBlock = it.blockNumber();

    cur.beginEditBlock();
    for (int b = firstBlock; b <= lastBlock; ++b)
    {
        QTextBlock block = document()->findBlockByNumber(b);
        QTextCursor line(block);
        if (unindent)
        {
            int n = 0;
            const QString text = block.text();
            while (n < _indentSize && n < text.length() &&
                   (text[n] == ' ' || text[n] == '\t'))
                ++n;
            line.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, n);
            line.removeSelectedText();
        }
        else
        {
            line.insertText(QString(_indentSize, ' '));
        }
    }
    cur.endEditBlock();
}

// The editor text with comments and braced blocks stripped -- the MFC
// CCodeEdit::GetStrippedCode.
QString CodeEditor::strippedCode() const
{
    QString code = toPlainText();
    QString stripped = code;
    stripComment(stripped);          // returns true if inside a block comment
    stripBracketCode(stripped);
    return stripped;
}

// Insert a multi-line snippet; continuation lines get the current line's
// leading indent (the MFC fed snippets through the per-line auto-indent).
void CodeEditor::insertSnippet(const QString& text)
{
    QTextCursor cur = textCursor();
    const QString block = cur.block().text();
    int n = 0;
    while (n < block.length() && (block[n] == ' ' || block[n] == '\t'))
        ++n;
    const QString indent = block.left(n);

    // Normalise line breaks -- the Insert wizards emit bare CR, others LF.
    QString out = text;
    out.replace("\r\n", "\n");
    out.replace('\r', '\n');
    out.replace("\n", '\n' + indent);

    cur.beginEditBlock();
    cur.insertText(out);
    cur.endEditBlock();
}

// Insert wizard-generated code. If it ends with an empty `{}` block, open the
// block up and leave the caret on a fresh indented line inside it.
void CodeEditor::insertWizardSnippet(const QString& text)
{
    QString src = text;
    src.replace("\r\n", "\n");
    src.replace('\r', '\n');

    const bool openBlock = src.endsWith("{\n}");
    if (openBlock)
        src.chop(1);                 // drop trailing '}', reinserted below

    insertSnippet(src);

    if (openBlock)
    {
        // Caret now sits just after "{\n<indent>" -- reinsert "}" on the
        // next line and drop the caret back onto the empty body line.
        const int caret = textCursor().position();
        insertSnippet("\n}");
        QTextCursor cur = textCursor();
        cur.setPosition(caret);
        setTextCursor(cur);
    }
}

void CodeEditor::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() == Qt::NoModifier)
        {
            insertNewlineWithIndent();
            return;
        }
        break;

    case Qt::Key_Tab:
        if (textCursor().hasSelection())
        {
            indentSelection(false);
            return;
        }
        // No selection: insert spaces to the next indent stop.
        {
            const int col = textCursor().positionInBlock();
            insertPlainText(QString(_indentSize - (col % _indentSize), ' '));
        }
        return;

    case Qt::Key_Backtab:
        indentSelection(true);
        return;

    case Qt::Key_BraceRight:                 // '}'
        QPlainTextEdit::keyPressEvent(event);
        reindentClosingBrace();
        return;

    default:
        break;
    }

    QPlainTextEdit::keyPressEvent(event);
}

// --- Marker bands ----------------------------------------------------------
//
// A header / footer band is a QLabel child of the editor, placed inside the
// frame in space reserved by setViewportMargins. The label never scrolls (it
// is not in the viewport); the code scrolls between the two bands.

namespace {
QLabel* makeBand(QWidget* parent)
{
    QLabel* l = new QLabel(parent);
    l->setFont(CodeEditor::codeFont());
    l->setAutoFillBackground(true);
    l->setStyleSheet("background:#e8e8e8; padding:1px 3px;");
    l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    l->setTextInteractionFlags(Qt::NoTextInteraction);
    return l;
}

// A band counts as active (reserves space, gets laid out) when it exists and
// carries text. NOTE: do not test QWidget::isVisible() here -- during the
// dialog ctor the editor's ancestors are not shown yet, so isVisible() is
// false and the margins would never be reserved.
bool bandActive(QLabel* b) { return b && !b->text().isEmpty(); }
}

void CodeEditor::setHeaderText(const QString& text)
{
    if (!_header)
        _header = makeBand(this);
    _header->setText(text);
    _header->setVisible(!text.isEmpty());
    updateBandMargins();
}

void CodeEditor::setFooterText(const QString& text)
{
    if (!_footer)
        _footer = makeBand(this);
    _footer->setText(text);
    _footer->setVisible(!text.isEmpty());
    updateBandMargins();
}

// Reserve viewport space for whichever bands are active, then position them.
void CodeEditor::updateBandMargins()
{
    const int top    = bandActive(_header)
        ? _header->sizeHint().height() : 0;
    const int bottom = bandActive(_footer)
        ? _footer->sizeHint().height() : 0;
    setViewportMargins(0, top, 0, bottom);
    layoutBands();
}

// Span each band across the full editor width, pinned top / bottom, above
// the viewport.
void CodeEditor::layoutBands()
{
    const QRect cr = contentsRect();
    if (bandActive(_header))
    {
        _header->setGeometry(cr.left(), cr.top(), cr.width(),
                             _header->sizeHint().height());
        _header->raise();
    }
    if (bandActive(_footer))
    {
        const int h = _footer->sizeHint().height();
        _footer->setGeometry(cr.left(), cr.bottom() - h + 1,
                             cr.width(), h);
        _footer->raise();
    }
}

void CodeEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);
    layoutBands();
}

void CodeEditor::showEvent(QShowEvent* event)
{
    QPlainTextEdit::showEvent(event);
    layoutBands();
}
