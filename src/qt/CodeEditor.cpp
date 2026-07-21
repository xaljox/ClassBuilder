// qt/CodeEditor.cpp -- a plain-text C++ code editor widget.
//
// Ported from the MFC CCodeEdit (ClassBuilder/CodeEdit.cpp). The core is the
// indent predictor: GetIndent walks the text up to the cursor line by line,
// and GetNextLineIndent predicts the indent of the line that follows each.

#include "CodeEditor.h"
#include "CppHighlighter.h"
#include "QtSoftSelection.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QColor>
#include <QCompleter>
#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QList>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTextBlock>
#include <QTimer>
#include <QTextCursor>
#include <QTextEdit>
#include <QToolTip>
#include <QWheelEvent>

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

bool isIdentChar(QChar c) { return c.isLetterOrNumber() || c == '_'; }

// Start indices of every whole-identifier occurrence of `word` in `text` --
// the same boundary rule as the model's FindStringInStr (no C-symbol char on
// either side, so "row" does not hit inside "rowCount").
QList<int> identifierOccurrences(const QString& text, const QString& word)
{
    QList<int> hits;
    const int wordLen = word.length();
    int from = 0;
    int i;
    while ((i = text.indexOf(word, from)) != -1)
    {
        const int after = i + wordLen;
        if ((i == 0 || !isIdentChar(text[i - 1])) &&
            (after >= text.length() || !isIdentChar(text[after])))
            hits.append(i);
        from = i + 1;
    }
    return hits;
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
    // macOS renders points smaller than Windows; bump like the UI font
    // (CB_UI_FONT_PT 11 -> 15) so code doesn't read smaller than the menu
    // strip above it. Menlo is the native mac monospace (Consolas is absent
    // -- naming it directly also avoids the font-alias fallback cost).
#ifdef __APPLE__
    QFont f("Menlo");
    f.setPointSize(14);
#else
    QFont f("Consolas");
    f.setPointSize(11);
#endif
    f.setStyleHint(QFont::Monospace);
    f.setFixedPitch(true);
    f.setWeight(QFont::Medium);
    return f;
}

CodeEditor::CodeEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    _basePt = codeFont().pointSize();
    // Adopt the shared zoom level so a newly opened editor already matches the
    // others (e.g. reopening a dialog after zooming elsewhere).
    _zoomPt = (s_zoomPt > 0) ? s_zoomPt : _basePt;
    liveEditors().append(this);
    applyEditorFont(_zoomPt);        // font + tab stops (overrides app QSS)

    // Force crisp black-on-white -- do not inherit a washed-out palette
    // from the app-wide style.
    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::white);
    pal.setColor(QPalette::Text, Qt::black);
    setPalette(pal);
    // Pin the text-selection colour to the live theme accent (see below).
    applyThemeAccent();

    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabChangesFocus(false);

    _highlighter = new CppHighlighter(document());

    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::updateExtraSelections);
    connect(this, &QPlainTextEdit::selectionChanged,
            this, &CodeEditor::updateExtraSelections);
    // A visible parameter hint follows the caret: re-resolved on every move
    // (typing, click, backspace out of the call), never shown by this path.
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, [this] { updateParameterHint(false); });

    // Method-not-found diagnostics: recompute the wave-underline ranges a
    // short while after typing stops (resolving every call on each keystroke
    // would be wasteful, and mid-word the text is transiently invalid).
    _diagnosticTimer = new QTimer(this);
    _diagnosticTimer->setSingleShot(true);
    _diagnosticTimer->setInterval(400);
    connect(_diagnosticTimer, &QTimer::timeout,
            this, &CodeEditor::updateDiagnostics);
    connect(this, &QPlainTextEdit::textChanged,
            this, [this] { _diagnosticTimer->start(); });

    // Keep the footer band above the horizontal scrollbar: a line longer than
    // the viewport makes that scrollbar appear (or disappear) without resizing
    // the editor, so the bands must be re-laid-out on that range change too --
    // resizeEvent alone doesn't fire for it.
    connect(horizontalScrollBar(), &QScrollBar::rangeChanged,
            this, [this](int, int) { layoutBands(); });

    updateExtraSelections();
}

void CodeEditor::applyThemeAccent()
{
    // Pin the TEXT SELECTION to the live theme accent (QPalette::Active/Highlight)
    // + its contrasting text, so a selection in the editor follows the accent on
    // every platform -- the same colour the tree, diagram and popups key off.
    // Read the ACTIVE group explicitly (Inactive Highlight is the grey
    // unfocused-selection colour); without pinning it, the gtk3 platform style
    // can paint the text-edit selection in its OWN selection colour, not the
    // accent. Base/Text (white/black) are set once in the constructor and left
    // untouched here -- only the accent-derived roles need re-deriving.
    // The accent AS CHOSEN, like every filled selection in CB: a text selection
    // is a row-sized fill, so it does not need the deepening the tree's small
    // glyphs get (Qt_ChromeAccent) -- and its HighlightedText is derived at the
    // chokepoint, so a light accent gets black text rather than white.
    const QPalette appPal = QApplication::palette();
    QPalette pal = palette();
    pal.setColor(QPalette::Highlight,
                 appPal.color(QPalette::Active, QPalette::Highlight));
    pal.setColor(QPalette::HighlightedText,
                 appPal.color(QPalette::Active, QPalette::HighlightedText));
    setPalette(pal);
}

void CodeEditor::reapplyThemeAccent()
{
    // The desktop accent changed while CB is open: re-pin the selection colour.
    // The completion popup is a separate widget re-derived by the app watcher
    // via its cbSoftSelection marker, so it is not touched here.
    applyThemeAccent();
    viewport()->update();
}

void CodeEditor::setModelTypes(const QSet<QString>& names)
{
    _highlighter->setModelTypes(names);
}

void CodeEditor::setArgumentNames(const QSet<QString>& names)
{
    _highlighter->setArgumentNames(names);
    if (names != _argumentNames)
    {
        _argumentNames = names;
        renderHeader();          // arguments are italic in the band too
    }
}

void CodeEditor::setHighlightWord(const QString& word)
{
    if (word == _highlightWord)
        return;
    _highlightWord = word;
    updateExtraSelections();
}

int CodeEditor::identifierCount(const QString& text, const QString& word)
{
    return word.isEmpty() ? 0 : identifierOccurrences(text, word).size();
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
    return computeIndentAt(textCursor().position());
}

// The predictor for a line starting at `pos`: walk the text before it line
// by line, tracking the reference indent (the MFC GetNextLineIndent walk).
int CodeEditor::computeIndentAt(int pos) const
{
    QString code = toPlainText().left(pos);

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

namespace {
// The line's leading whitespace (chars before `col`, which is the position
// of a just-typed brace) -- true when the brace is the line's first content.
bool onlyWhitespaceBefore(const QString& block, int col)
{
    for (int i = 0; i < col; ++i)
        if (!block[i].isSpace())
            return false;
    return true;
}
}

// Replace the leading whitespace (the `count` chars at the line start) with
// `target` spaces.
static void reindentLineTo(QTextCursor cur, int count, int target)
{
    cur.beginEditBlock();
    QTextCursor line = cur;
    line.movePosition(QTextCursor::StartOfBlock);
    line.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, count);
    line.insertText(QString(target, ' '));
    cur.endEditBlock();
}

// Typing '}' on an otherwise-blank line: re-indent that line so the brace
// lines up one step out from the block body (the caret sits just AFTER the
// typed brace -- inspect the chars before it, not before the caret).
void CodeEditor::reindentClosingBrace()
{
    const QTextCursor cur = textCursor();
    const QString block = cur.block().text();
    const int col = cur.positionInBlock();
    if (col < 1 || block[col - 1] != '}' ||
        !onlyWhitespaceBefore(block, col - 1))
        return;

    int target = computeIndentAt(cur.block().position()) - _indentSize;
    if (target < 0)
        target = 0;
    reindentLineTo(cur, col - 1, target);
}

// Typing '{' on an otherwise-blank line: Allman style puts the brace at the
// indent of the construct it belongs to -- the previous non-blank line
// (if/for/function header, or the statement before a free block). Only a
// nested block directly under another '{' keeps the predicted (+step) indent.
void CodeEditor::reindentOpenBrace()
{
    const QTextCursor cur = textCursor();
    const QString block = cur.block().text();
    const int col = cur.positionInBlock();
    if (col < 1 || block[col - 1] != '{' ||
        !onlyWhitespaceBefore(block, col - 1))
        return;

    QTextBlock prev = cur.block().previous();
    while (prev.isValid() && prev.text().trimmed().isEmpty())
        prev = prev.previous();

    int target;
    QString prevText = prev.isValid() ? prev.text() : QString();
    while (!prevText.isEmpty() && prevText.back().isSpace())
        prevText.chop(1);
    if (!prev.isValid() || prevText.endsWith('{'))
        target = computeIndentAt(cur.block().position());
    else
        target = leadingIndent(prevText);

    reindentLineTo(cur, col - 1, target);
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

// Re-indent the selected lines (or all lines) with the same rules that
// drive typing: the predictor for ordinary lines, one step out for a line
// starting with '}', the previous non-blank line's indent for one starting
// with '{' (unless nested directly under another '{'), column 0 for
// preprocessor lines. Lines inside block comments are left untouched.
// Top-down, one line at a time, so each line's correction feeds the
// prediction of the next. One undo step.
void CodeEditor::reformatCode()
{
    QTextCursor cur = textCursor();
    int firstBlock = 0;
    int lastBlock  = document()->blockCount() - 1;
    if (cur.hasSelection())
    {
        QTextCursor it(document());
        it.setPosition(cur.selectionStart());
        firstBlock = it.blockNumber();
        it.setPosition(cur.selectionEnd());
        lastBlock = it.blockNumber();
    }

    cur.beginEditBlock();
    for (int b = firstBlock; b <= lastBlock; ++b)
    {
        const QTextBlock block = document()->findBlockByNumber(b);
        const QString text = block.text();

        int lead = 0;
        while (lead < text.length() && text[lead].isSpace())
            ++lead;
        const QString content = text.mid(lead);

        // Inside an unterminated block comment: leave the line alone.
        QString head = toPlainText().left(block.position());
        if (stripComment(head))
            continue;

        int target;
        if (content.isEmpty())
            target = 0;                          // blank line: no trailing indent
        else if (content[0] == '#')
            target = 0;                          // preprocessor at column 0
        else if (content[0] == '}')
        {
            target = computeIndentAt(block.position()) - _indentSize;
            if (target < 0)
                target = 0;
        }
        else if (content[0] == '{')
        {
            QTextBlock prev = block.previous();
            while (prev.isValid() && prev.text().trimmed().isEmpty())
                prev = prev.previous();
            QString prevText = prev.isValid() ? prev.text() : QString();
            while (!prevText.isEmpty() && prevText.back().isSpace())
                prevText.chop(1);
            target = (!prev.isValid() || prevText.endsWith('{'))
                ? computeIndentAt(block.position())
                : leadingIndent(prevText);
        }
        else
        {
            target = computeIndentAt(block.position());
        }

        if (leadingIndent(text) != target || text.left(lead).contains('\t'))
        {
            QTextCursor line(block);
            line.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                              lead);
            line.insertText(QString(target, ' '));
        }
    }
    cur.endEditBlock();
}

// Move the selected lines (or the current line) as a block: the adjacent
// line on the other side is swapped over the block, and the selection is
// restored onto the moved lines.
void CodeEditor::moveSelectedLines(bool up)
{
    if (isReadOnly())
        return;

    QTextCursor cur = textCursor();
    const int selStart = cur.selectionStart();
    const int selEnd   = cur.selectionEnd();

    QTextBlock first = document()->findBlock(selStart);
    QTextBlock last  = document()->findBlock(selEnd);
    // A selection ending at column 0 doesn't include that line.
    if (selEnd > selStart && selEnd == last.position())
        last = last.previous();

    const QTextBlock swap = up ? first.previous() : last.next();
    if (!swap.isValid())
        return;

    // Selection offsets inside the moved block, to restore afterwards.
    const int startOfs = selStart - first.position();
    const int endOfs   = selEnd   - first.position();

    QString movedText;
    for (QTextBlock b = first; ; b = b.next())
    {
        if (b != first)
            movedText += '\n';
        movedText += b.text();
        if (b == last)
            break;
    }
    const QString swapText = swap.text();

    // Capture all positions BEFORE editing -- the blocks invalidate.
    const int swapPos    = swap.position();
    const int firstPos   = first.position();
    const int lastEnd    = last.position() + last.text().length();
    const int swapEnd    = swapPos + swapText.length();
    const int newBase    = up ? swapPos
                              : firstPos + swapText.length() + 1;

    QTextCursor region(document());
    region.beginEditBlock();
    if (up)
    {
        region.setPosition(swapPos);
        region.setPosition(lastEnd, QTextCursor::KeepAnchor);
        region.insertText(movedText + '\n' + swapText);
    }
    else
    {
        region.setPosition(firstPos);
        region.setPosition(swapEnd, QTextCursor::KeepAnchor);
        region.insertText(swapText + '\n' + movedText);
    }
    region.endEditBlock();

    QTextCursor sel(document());
    sel.setPosition(newBase + startOfs);
    sel.setPosition(newBase + endOfs, QTextCursor::KeepAnchor);
    setTextCursor(sel);
}

// Toggle `//` on the selected lines (or the current line). Uncomments when
// every non-blank line is already commented, else comments; blank lines are
// left alone. One undo step, comment inserted at each line's indent.
void CodeEditor::toggleLineComment()
{
    if (isReadOnly())
        return;

    QTextCursor cur = textCursor();
    QTextBlock first = document()->findBlock(cur.selectionStart());
    QTextBlock last  = document()->findBlock(cur.selectionEnd());
    if (cur.selectionEnd() > cur.selectionStart() &&
        cur.selectionEnd() == last.position())
        last = last.previous();

    bool allCommented = true;
    for (QTextBlock b = first; b.isValid(); b = b.next())
    {
        const QString trimmed = b.text().trimmed();
        if (!trimmed.isEmpty() && !trimmed.startsWith("//"))
            allCommented = false;
        if (b == last)
            break;
    }

    cur.beginEditBlock();
    for (QTextBlock b = first; b.isValid(); b = b.next())
    {
        const QString t = b.text();
        int lead = 0;
        while (lead < t.length() && (t[lead] == ' ' || t[lead] == '\t'))
            ++lead;

        if (allCommented)
        {
            if (t.mid(lead).startsWith("//"))
            {
                const int n = (lead + 2 < t.length() && t[lead + 2] == ' ')
                    ? 3 : 2;
                QTextCursor e(document());
                e.setPosition(b.position() + lead);
                e.setPosition(b.position() + lead + n,
                              QTextCursor::KeepAnchor);
                e.removeSelectedText();
            }
        }
        else if (!t.trimmed().isEmpty())
        {
            QTextCursor e(document());
            e.setPosition(b.position() + lead);
            e.insertText("// ");
        }
        if (b == last)
            break;
    }
    cur.endEditBlock();
}

// Wrap the selection (or the current line) in `/* ... */`; unwrap when it
// already is one. One undo step.
void CodeEditor::toggleBlockComment()
{
    if (isReadOnly())
        return;

    QTextCursor cur = textCursor();
    if (!cur.hasSelection())
    {
        cur.movePosition(QTextCursor::StartOfBlock);
        cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    }

    QString sel = cur.selectedText();
    const QString trimmed = sel.trimmed();
    cur.beginEditBlock();
    if (trimmed.startsWith("/*") && trimmed.endsWith("*/") &&
        trimmed.length() >= 4)
    {
        // Unwrap: drop the /* */ and one padding space on each side.
        int b = sel.indexOf("/*");
        int e = sel.lastIndexOf("*/");
        QString inner = sel.mid(b + 2, e - (b + 2));
        if (inner.startsWith(' ')) inner.remove(0, 1);
        if (inner.endsWith(' '))   inner.chop(1);
        cur.insertText(sel.left(b) + inner + sel.mid(e + 2));
    }
    else
    {
        cur.insertText("/* " + sel + " */");
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
        // next line, drop the caret back onto the empty body line, and give
        // it one indent step so typing starts at body level, not at the
        // brace's column.
        const int caret = textCursor().position();
        insertSnippet("\n}");
        QTextCursor cur = textCursor();
        cur.setPosition(caret);
        setTextCursor(cur);
        insertPlainText(QString(_indentSize, ' '));
    }
}

// Claim the Ctrl+zoom keys at the ShortcutOverride stage so they reach our
// keyPressEvent instead of firing a window-scoped shortcut. The main window
// owns a "Zoom Out" action on Ctrl+- (for diagrams); without this it would
// swallow Ctrl+- while the editor has focus, and the editor would never see
// it (JV 2026-07-16 -- Ctrl+- did not even reach keyPressEvent).
bool CodeEditor::event(QEvent* e)
{
    if (e->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);
        if (ke->modifiers() & Qt::ControlModifier)
        {
            switch (ke->key())
            {
            case Qt::Key_Minus: case Qt::Key_Underscore:
            case Qt::Key_Plus:  case Qt::Key_Equal:
            case Qt::Key_0:
                e->accept();      // deliver as a normal key press to us
                return true;
            default:
                break;
            }
        }
    }
    return QPlainTextEdit::event(e);
}

void CodeEditor::keyPressEvent(QKeyEvent* event)
{
    if (completionKeyPressEvent(event))
        return;

    // Esc dismisses a visible parameter hint (the completion popup, when
    // open, already took its Esc above).
    if (event->key() == Qt::Key_Escape && _paramHint && _paramHint->isVisible())
    {
        hideParameterHint();
        event->accept();
        return;
    }

    // Ctrl+Shift+Space summons the parameter hint on demand -- clicking
    // into an existing call deliberately shows nothing by itself.
    if (event->key() == Qt::Key_Space &&
        event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
    {
        updateParameterHint(true);
        event->accept();
        return;
    }

    // Font zoom -- mirrors the diagram canvas (Ctrl + 0 / + / = / -). Both the
    // key code AND the typed text are checked, so the +/- shift asymmetry
    // across keyboard layouts cannot leave a gesture unmatched.
    if (event->modifiers() & Qt::ControlModifier)
    {
        const int key = event->key();
        const QString t = event->text();
        if (key == Qt::Key_0 || t == "0")
        {
            applySharedZoom(_basePt);
            event->accept();
            return;
        }
        if (key == Qt::Key_Plus || key == Qt::Key_Equal ||
            t == "+" || t == "=")
        {
            applySharedZoom(_zoomPt + 1);
            event->accept();
            return;
        }
        if (key == Qt::Key_Minus || key == Qt::Key_Underscore ||
            t == "-" || t == "_")
        {
            applySharedZoom(_zoomPt - 1);
            event->accept();
            return;
        }
    }

    // Auto-close pairs, type-over, empty-pair backspace, and `{|}` Enter
    // expansion -- handled before the normal keys.
    if (autoCloseKeyPressEvent(event))
    {
        if (event->text().contains('(') || event->text().contains(','))
            updateParameterHint(true);
        return;
    }

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

    default:
        break;
    }

    QPlainTextEdit::keyPressEvent(event);

    // Brace re-indent by the typed CHARACTER, not the key code -- '{'/'}'
    // arrive via different keys per layout (Shift+], AltGr, ...).
    if (event->text() == "{")
        reindentOpenBrace();
    else if (event->text() == "}")
        reindentClosingBrace();

    maybeTriggerCompletion(event);

    // '(' / ',' opens (or re-anchors) the parameter hint; every other caret
    // move only updates an already-visible one (the ctor's connection).
    if (event->text().contains('(') || event->text().contains(','))
        updateParameterHint(true);
}

// Auto-close pairs. Openers: `(` `[` `{` `"` `'`. See the header.
bool CodeEditor::autoCloseKeyPressEvent(QKeyEvent* event)
{
    if (isReadOnly())
        return false;

    static const QString openers = "([{\"'";
    static const QString closers = ")]}\"'";

    const QString text = toPlainText();
    QTextCursor cur = textCursor();
    const int pos = cur.position();
    const QChar next = pos < text.length() ? text[pos] : QChar();
    const QChar prev = pos > 0 ? text[pos - 1] : QChar();

    // `{|}` on Enter -> expand into an indented three-line block, caret on the
    // middle line.
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        event->modifiers() == Qt::NoModifier &&
        prev == '{' && next == '}')
    {
        const int lineIndent = leadingIndent(cur.block().text());
        const QString mid = "\n" + QString(lineIndent + _indentSize, ' ');
        const QString end = "\n" + QString(lineIndent, ' ');
        cur.beginEditBlock();
        cur.insertText(mid + end);
        cur.setPosition(pos + mid.length());     // end of the indented line
        cur.endEditBlock();
        setTextCursor(cur);
        return true;
    }

    // Backspace on an empty pair (`(|)`, `"|"`, ...) deletes both halves --
    // but only when the LINE is balanced for that bracket kind. Unbalanced,
    // the closer belongs to an OUTER construct: in `if (pRow->IsLast(|)` the
    // `)` is the if's (IsLast's own was just backspaced away), and eating it
    // silently breaks the if; deleting only the opener restores the balance
    // instead (JV 2026-07-18, the "eaten if-closer" repro).
    if (event->key() == Qt::Key_Backspace &&
        event->modifiers() == Qt::NoModifier && !cur.hasSelection())
    {
        const int oi = openers.indexOf(prev);
        if (oi >= 0 && next == closers[oi])
        {
            bool eatBoth = true;              // quotes: no reliable count
            if (prev == '(' || prev == '[' || prev == '{')
            {
                int balance = 0;
                const QString line = cur.block().text();
                for (QChar ch : line)
                {
                    if (ch == prev)
                        ++balance;
                    else if (ch == next)
                        --balance;
                }
                eatBoth = (balance == 0);
            }
            if (eatBoth)
            {
                cur.beginEditBlock();
                cur.deleteChar();                // the closer
                cur.deletePreviousChar();        // the opener
                cur.endEditBlock();
                return true;
            }
        }
        return false;
    }

    const QString typed = event->text();
    if (typed.size() != 1)
        return false;
    const QChar c = typed[0];

    // Type-over: a closer typed where it already sits just steps over it --
    // but only when the line up to the caret still has an unmatched opener
    // for it. With everything balanced, the closer under the caret belongs
    // to an OUTER construct (the `if (`'s, say) and stepping over it would
    // silently swallow the keystroke -- insert normally instead.
    if (!cur.hasSelection() && next == c &&
        (c == ')' || c == ']' || c == '}' || c == '"' || c == '\''))
    {
        bool unmatched = true;              // quotes: no reliable count, keep
        if (c == ')' || c == ']' || c == '}')
        {
            const QChar open = c == ')' ? '(' : c == ']' ? '[' : '{';
            int balance = 0;
            const QString line = cur.block().text();
            for (int i = 0; i < cur.positionInBlock(); ++i)
            {
                if (line[i] == open)
                    ++balance;
                else if (line[i] == c)
                    --balance;
            }
            unmatched = balance > 0;
        }
        if (unmatched)
        {
            cur.movePosition(QTextCursor::Right);
            setTextCursor(cur);
            return true;
        }
        return false;                       // a NEW closer: really insert it
    }

    const int oi = openers.indexOf(c);
    if (oi < 0)
        return false;
    const QChar close = closers[oi];

    // Surround a selection with the pair.
    if (cur.hasSelection())
    {
        const int s = cur.selectionStart();
        const QString sel = cur.selectedText();
        cur.insertText(c + sel + close);
        cur.setPosition(s + 1);
        cur.setPosition(s + 1 + sel.length(), QTextCursor::KeepAnchor);
        setTextCursor(cur);
        return true;
    }

    // Insert the pair only when the caret is not glued to a word (so typing
    // `(` before `foo` does not orphan a `)`); a quote also needs a
    // non-word char before it (an apostrophe inside `don't` stays literal).
    const bool nextOk = next.isNull() || next.isSpace() ||
                        QString(")]},;").contains(next);
    const bool quoteOk = !(c == '"' || c == '\'') ||
                         !(isIdentChar(prev));
    if (nextOk && quoteOk)
    {
        cur.insertText(QString(c) + close);
        cur.movePosition(QTextCursor::Left);
        setTextCursor(cur);
        if (c == '{')
            reindentOpenBrace();     // keep the Allman alignment of a lone {
        return true;
    }
    return false;
}

// Shared zoom state: one level for every open code editor (see header). -1
// until the first editor is constructed / the user first zooms.
int CodeEditor::s_zoomPt = -1;

QList<CodeEditor*>& CodeEditor::liveEditors()
{
    static QList<CodeEditor*> editors;
    return editors;
}

CodeEditor::~CodeEditor()
{
    liveEditors().removeAll(this);
}

// Store the new zoom level and apply it to EVERY live editor, so all panes of
// one logical edit (constructor init + body, the User Sections editors, ...)
// scale together instead of independently. Bounds match applyEditorFont().
void CodeEditor::applySharedZoom(int pt)
{
    s_zoomPt = qBound(6, pt, 32);
    for (CodeEditor* ed : liveEditors())
        ed->applyEditorFont(s_zoomPt);
}

// Set the editor's font size. The app-wide stylesheet carries a bare
// `QWidget { font-size }` rule that OVERRIDES setFont() -- so plain setFont /
// zoomIn does nothing. A widget-local stylesheet wins over the inherited one,
// so the size is set that way (the family still comes from setFont).
void CodeEditor::applyEditorFont(int pt)
{
    _zoomPt = qBound(6, pt, 32);
    QFont f = codeFont();
    f.setPointSize(_zoomPt);
    setFont(f);
    setStyleSheet(QString("font-size:%1pt;").arg(_zoomPt));
    QFontMetricsF fm(f);
    setTabStopDistance(fm.horizontalAdvance(' ') * _indentSize);
    // The marker bands (signature header, //@CODE footer) must zoom with the
    // code: their own font drives their height, and the editor's font-size
    // stylesheet above would otherwise cascade into them, enlarging the
    // rendered text past the (un-grown) band height and clipping the letters.
    refreshBands();
}

// Ctrl+wheel zooms the editor font in small steps.
void CodeEditor::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        const int delta = event->angleDelta().y();
        if (delta != 0)
            applySharedZoom(_zoomPt + (delta > 0 ? 1 : -1));
        event->accept();
        return;
    }
    QPlainTextEdit::wheelEvent(event);
}

// Cmd+Click (macOS) / Ctrl+Click: go to definition of the clicked
// identifier -- the mouse path to the same command as F12 (function keys on
// macOS are frequently swallowed by system shortcuts; this path is the
// VS Code / Xcode convention and always reaches the editor).
void CodeEditor::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton &&
        (event->modifiers() & Qt::ControlModifier))
    {
        setTextCursor(cursorForPosition(event->pos()));
        emit definitionRequested();
        return;
    }
    QPlainTextEdit::mousePressEvent(event);
}

// Parameter hint. A tooltip-styled label floated above the caret's line;
// the provider resolves the innermost open call from the text up to the
// caret. allowShow: only '(' / ',' may POP the hint -- plain caret moves
// merely re-resolve (or hide) a hint that is already up.
void CodeEditor::updateParameterHint(bool allowShow)
{
    if (!_provider)
        return;
    if (!allowShow && (!_paramHint || !_paramHint->isVisible()))
        return;

    const QString hint =
        _provider->parameterHint(toPlainText().left(textCursor().position()));
    if (hint.isEmpty())
    {
        hideParameterHint();
        return;
    }

    if (!_paramHint)
    {
        _paramHint = new QLabel(this, Qt::ToolTip | Qt::FramelessWindowHint);
        _paramHint->setAttribute(Qt::WA_ShowWithoutActivating, true);
        _paramHint->setTextFormat(Qt::RichText);
        // The classic info-yellow, like the app's tooltips (the QToolTip
        // stylesheet rule does not reach a plain QLabel).
        _paramHint->setStyleSheet(
            "QLabel { background-color: #FFFFE1; color: black;"
            " border: 1px solid #767676; padding: 2px 6px; }");
    }
    _paramHint->setText(hint);
    _paramHint->adjustSize();

    QPoint pos = viewport()->mapToGlobal(cursorRect().topLeft());
    pos.ry() -= _paramHint->height() + 6;    // above the line, out of the way
    _paramHint->move(pos);
    _paramHint->show();
}

void CodeEditor::hideParameterHint()
{
    if (_paramHint)
        _paramHint->hide();
}

// Hover documentation: the provider supplies a rich-text tooltip for the
// identifier under the mouse. cursorForPosition clamps to the nearest
// character, so hovering the empty space right of a line would still hit its
// last word -- the distance guard filters that out.
bool CodeEditor::viewportEvent(QEvent* event)
{
    if (event->type() == QEvent::ToolTip && _provider)
    {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        const QTextCursor cursor = cursorForPosition(helpEvent->pos());
        if (qAbs(helpEvent->pos().x() - cursorRect(cursor).center().x())
                > fontMetrics().averageCharWidth() * 2)
        {
            QToolTip::hideText();
        }
        else
        {
            const QString tip = _provider->hoverText(toPlainText(),
                                                     cursor.position());
            if (tip.isEmpty())
                QToolTip::hideText();
            else
                QToolTip::showText(helpEvent->globalPos(), tip, viewport());
        }
        return true;
    }
    return QPlainTextEdit::viewportEvent(event);
}

// --- Completion --------------------------------------------------------------
//
// The canonical QCompleter-on-a-text-edit pattern: the completer's popup owns
// the navigation keys while visible; the editor decides after each ordinary
// keystroke whether to (re)query the provider and show, refilter or hide the
// popup. All knowledge of WHAT to offer lives in the provider.

namespace {
// The extra model role the popup delegate reads: the muted right-aligned
// detail text (a method's return type, a variable's type, "class").
const int kDetailRole = Qt::UserRole + 4;

// The direct-pick shortcut of a VISIBLE popup row: Cmd+1..9 on macOS,
// Ctrl+1..9 elsewhere, accepting that row without arrowing down + Enter
// (see completionKeyPressEvent). Nine slots -- Ctrl/Cmd+0 stays the
// editor's font-zoom reset, and typing narrows the list anyway. Empty for
// rows beyond the ninth.
QString rowShortcutLabel(int row)
{
    if (row < 0 || row >= 9)
        return QString();
#ifdef __APPLE__
    return QChar(0x2318) + QString::number(row + 1);        // ⌘1
#else
    return "Ctrl+" + QString::number(row + 1);
#endif
}

// Popup row painter: the model icon (decoration role) and the display name on
// the left, the detail plus the row's direct-pick shortcut right-aligned in
// muted colours. The display name is elided BEFORE those columns, so they
// never overlap; the full-width selection background is the style's, so a
// selected row reads normally.
class CompletionItemDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        const QString detail = index.data(kDetailRole).toString();
        if (!detail.isEmpty())          // reserve the detail column's width
            size.rwidth() +=
                option.fontMetrics.horizontalAdvance(detail) + 24;
        const QString hint = rowShortcutLabel(index.row());
        if (!hint.isEmpty())            // and the shortcut column's
            size.rwidth() +=
                option.fontMetrics.horizontalAdvance(hint) + 12;
        // Compact rows -- the windows11 style pads items touch-friendly tall;
        // match the who-calls-me popup so the two read consistently.
        size.setHeight(option.fontMetrics.height() + 2);
        return size;
    }

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        // The base delegate draws the row (background, selection, icon, and
        // the display name) with the correct per-platform / per-state text
        // colour. We ONLY add the right-hand columns -- drawing the name
        // ourselves double-struck it (the base re-inits from the model) and
        // mis-coloured the selected row; letting the base own the name fixes
        // both (JV 2026-07-16). sizeHint() reserves their width, so the
        // name never runs under them.
        QStyledItemDelegate::paint(painter, option, index);

        const QString detail = index.data(kDetailRole).toString();
        const QString hint = rowShortcutLabel(index.row());
        if (detail.isEmpty() && hint.isEmpty())
            return;

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QWidget* w = opt.widget;
        QStyle* style = w ? w->style() : QApplication::style();
        const QRect textRect =
            style->subElementRect(QStyle::SE_ItemViewItemText, &opt, w);

        // The popup's selection is the tree-matching soft accent tint
        // (Qt_ApplySoftSelection), so the right-hand columns always sit on a
        // light background: the normal Text colour, muted with alpha, reads
        // in every state on every platform. (Before the tint, a selected row
        // needed HighlightedText to survive GNOME's saturated blue -- JV
        // 2026-07-18, Linux; the light tint removes that case.)
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
            ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;
        const QColor textColor = opt.palette.color(cg, QPalette::Text);

        painter->save();
        painter->setFont(opt.font);
        int right = -6;
        if (!hint.isEmpty())            // shortcut at the far right edge
        {
            QColor c = textColor;
            c.setAlpha(120);
            painter->setPen(c);
            painter->drawText(textRect.adjusted(0, 0, right, 0),
                              Qt::AlignRight | Qt::AlignVCenter, hint);
            right -= opt.fontMetrics.horizontalAdvance(hint) + 12;
        }
        if (!detail.isEmpty())
        {
            QColor c = textColor;
            c.setAlpha(150);
            painter->setPen(c);
            painter->drawText(textRect.adjusted(0, 0, right, 0),
                              Qt::AlignRight | Qt::AlignVCenter, detail);
        }
        painter->restore();
    }
};
} // namespace

void CodeEditor::setCompletionProvider(CodeCompletionProvider* provider)
{
    _provider = provider;
    if (!_completer && provider)
    {
        _completionModel = new QStandardItemModel(this);
        _completer = new QCompleter(_completionModel, this);
        _completer->setWidget(this);
        _completer->setCompletionMode(QCompleter::PopupCompletion);
        _completer->setCaseSensitivity(Qt::CaseInsensitive);
        _completer->popup()->setFont(codeFont());
        _completer->popup()->setIconSize(QSize(16, 16));
        _completer->popup()->setItemDelegate(
            new CompletionItemDelegate(_completer->popup()));
        // Selection look: the tree's soft accent tint instead of the
        // platform default (saturated accent on macOS/GNOME, grey inactive
        // fill on Windows) -- see QtSoftSelection.h.
        Qt_ApplySoftSelection(_completer->popup());
        connect(_completer,
                QOverload<const QModelIndex&>::of(&QCompleter::activated),
                this, &CodeEditor::insertCompletion);
    }
    // The provider drives the diagnostics -- resolve the initial text now.
    if (provider && _diagnosticTimer)
        _diagnosticTimer->start();
}

void CodeEditor::updateDiagnostics()
{
    _diagnosticRanges = _provider ? _provider->unresolvedCalls(toPlainText())
                                  : QVector<QPair<int, int>>();
    updateExtraSelections();
}

int CodeEditor::typedPrefixLength() const
{
    const QString text = toPlainText();
    const int pos = textCursor().position();
    int p = pos;
    while (p > 0 && isIdentChar(text[p - 1]))
        --p;
    return pos - p;
}

bool CodeEditor::completionKeyPressEvent(QKeyEvent* event)
{
    if (!_provider)
        return false;

    // While the popup is visible those keys belong to it (the completer's
    // own event filter drives the popup; we just must not act on them).
    if (_completer->popup()->isVisible())
    {
        // Cmd+1..9 (mac; ControlModifier = the Cmd key there) / Ctrl+1..9:
        // accept the Nth visible row directly -- the rows show the shortcut
        // (see rowShortcutLabel). KeypadModifier masked out: the numeric pad
        // digits carry it on macOS and must work the same.
        const Qt::KeyboardModifiers mods =
            event->modifiers() & ~Qt::KeypadModifier;
        if (mods == Qt::ControlModifier &&
            event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9)
        {
            const QModelIndex idx = _completer->completionModel()->index(
                event->key() - Qt::Key_1, 0);
            if (idx.isValid())
            {
                _completer->popup()->hide();
                insertCompletion(idx);
            }
            return true;               // ours even when the row is empty
        }
        switch (event->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Tab:
        case Qt::Key_Escape:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            event->ignore();
            return true;
        default:
            break;
        }
    }

    // Ctrl+Space forces the popup in any context. Qt swaps Ctrl and Cmd on
    // macOS (ControlModifier = the Cmd key there), so accept MetaModifier
    // too: that is the PHYSICAL Ctrl key on the Mac -- the same binding VS
    // Code and Xcode use, and Cmd+Space itself is taken by Spotlight.
    // WITH Shift it is not ours: Ctrl+Shift+Space is the parameter hint
    // (handled in keyPressEvent after this returns false).
    if (event->key() == Qt::Key_Space &&
        (event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) &&
        !(event->modifiers() & Qt::ShiftModifier))
    {
        triggerCompletion();
        return true;
    }
    return false;
}

// After an ordinary keystroke: identifier chars keep the popup filtering (or
// open it once 2+ chars are typed); '.', '->' and '::' open it for member /
// scope access; anything else closes it.
void CodeEditor::maybeTriggerCompletion(QKeyEvent* event)
{
    if (!_provider)
        return;

    const bool visible = _completer->popup()->isVisible();

    // A bare modifier going down (Cmd on its way to Cmd+3, Shift for a
    // capital) is not typing -- it must not dismiss the popup.
    switch (event->key())
    {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_AltGr:
    case Qt::Key_CapsLock:
        return;
    default:
        break;
    }

    if (event->key() == Qt::Key_Backspace)
    {
        if (visible)
        {
            if (typedPrefixLength() > 0)
                triggerCompletion();
            else
                _completer->popup()->hide();
        }
        return;
    }

    const QString typed = event->text();
    if (typed.isEmpty())              // cursor movement etc.
    {
        if (visible)
            _completer->popup()->hide();
        return;
    }

    const QChar c = typed[0];
    const QString text = toPlainText();
    const int pos = textCursor().position();

    bool trigger = false;
    if (isIdentChar(c))
    {
        // A prefix that starts with '_' pops the popup at once: member names
        // (and the constructor init pane's members) are all '_'-prefixed, so
        // one '_' is a strong signal -- matches "typing _ offers members".
        const int prefixLen = typedPrefixLength();
        const bool underscore = prefixLen >= 1 && text[pos - prefixLen] == '_';
        trigger = visible || prefixLen >= 2 || underscore;
    }
    else if (c == '.')
        trigger = true;
    else if (c == '>')
        trigger = (pos >= 2 && text[pos - 2] == '-');
    else if (c == ':')
        trigger = (pos >= 2 && text[pos - 2] == ':');

    if (trigger)
        triggerCompletion();
    else if (visible)
        _completer->popup()->hide();
}

void CodeEditor::triggerCompletion()
{
    int prefixLen = 0;
    const QList<CodeCompletionItem> items = _provider->completions(
        toPlainText().left(textCursor().position()), prefixLen);
    if (items.isEmpty())
    {
        _completer->popup()->hide();
        return;
    }

    _completionModel->clear();
    for (const CodeCompletionItem& item : items)
    {
        QStandardItem* row = new QStandardItem(item.display);
        if (!item.icon.isNull())
            row->setIcon(item.icon);
        row->setData(item.insert, Qt::UserRole);
        row->setData(item.caretBack, Qt::UserRole + 1);
        row->setData(item.selectBack, Qt::UserRole + 2);
        row->setData(item.selectLen, Qt::UserRole + 3);
        row->setData(item.detail, Qt::UserRole + 4);
        _completionModel->appendRow(row);
    }

    const QString text = toPlainText();
    const int pos = textCursor().position();
    _completer->setCompletionPrefix(text.mid(pos - prefixLen, prefixLen));
    if (_completer->completionCount() == 0)
    {
        _completer->popup()->hide();
        return;
    }
    _completer->popup()->setCurrentIndex(
        _completer->completionModel()->index(0, 0));

    QRect rect = cursorRect();
    rect.setWidth(_completer->popup()->sizeHintForColumn(0) +
                  _completer->popup()->verticalScrollBar()->sizeHint().width());
    _completer->complete(rect);
}

// A popup row was accepted: replace the typed prefix with the item's insert
// text and step the caret back into "()" when asked to. A multi-line insert
// (the iterator loop skeleton) goes through the wizard-snippet path, which
// indents continuation lines and leaves the caret inside a trailing {} block.
void CodeEditor::insertCompletion(const QModelIndex& index)
{
    QString insert        = index.data(Qt::UserRole).toString();
    int caretBack         = index.data(Qt::UserRole + 1).toInt();
    const int selectBack  = index.data(Qt::UserRole + 2).toInt();
    int selectLen         = index.data(Qt::UserRole + 3).toInt();
    const int prefixLen   = typedPrefixLength();

    const QString text = toPlainText();
    const int pos = textCursor().position();

    // Replace the WHOLE identifier under the caret, not just the typed
    // prefix -- completing in the middle of an existing name must not leave
    // its tail behind.
    int suffixLen = 0;
    while (pos + suffixLen < text.length() && isIdentChar(text[pos + suffixLen]))
        ++suffixLen;

    // Overwriting a name that is already a call: keep the existing argument
    // list -- drop OUR whole "(...)" (with or without placeholder args, and
    // the caret/selection tricks inside it), so picking GetCell(int, int)
    // over DoIets4 in "DoIets4(a, b)" yields "GetCell(a, b)".
    const int paren = insert.indexOf('(');
    if (paren != -1 && pos + suffixLen < text.length() &&
        text[pos + suffixLen] == '(')
    {
        insert.truncate(paren);
        caretBack = 0;
        selectLen = 0;
    }

    QTextCursor cur = textCursor();
    cur.setPosition(pos - prefixLen);
    cur.setPosition(pos + suffixLen, QTextCursor::KeepAnchor);

    if (insert.contains('\n'))
    {
        cur.removeSelectedText();
        setTextCursor(cur);
        insertWizardSnippet(insert);
        return;
    }

    cur.insertText(insert);
    if (selectLen > 0)
    {
        // Select the first inserted argument name, ready to be overtyped.
        const int end = cur.position();
        cur.setPosition(end - selectBack - selectLen);
        cur.setPosition(end - selectBack, QTextCursor::KeepAnchor);
    }
    else
    {
        cur.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor,
                         caretBack);
    }
    setTextCursor(cur);

    // A completed method arrives with its "(args)" already inserted -- no
    // '(' is ever TYPED on this path, so pop the parameter hint here.
    updateParameterHint(true);
}

// The identifier the caret is in or touching. An explicit selection is used
// only when it is exactly an identifier (a double-clicked word); a partial or
// multi-word selection means "no identifier".
QString CodeEditor::identifierUnderCursor() const
{
    const QTextCursor cur = textCursor();
    if (cur.hasSelection())
    {
        const QString sel = cur.selectedText();
        if (sel.isEmpty() || sel[0].isDigit())
            return QString();
        for (QChar c : sel)
            if (!isIdentChar(c))
                return QString();
        return sel;
    }

    const QString block = cur.block().text();
    int left  = cur.positionInBlock();
    int right = left;
    while (left > 0 && isIdentChar(block[left - 1]))
        --left;
    while (right < block.length() && isIdentChar(block[right]))
        ++right;
    if (left == right || block[left].isDigit())
        return QString();
    return block.mid(left, right - left);
}

// Replace every whole-identifier occurrence, back to front (keeps the earlier
// offsets valid), as one editor-undo step.
int CodeEditor::renameIdentifier(const QString& oldName, const QString& newName)
{
    if (oldName.isEmpty() || oldName == newName)
        return 0;
    QString text = toPlainText();
    const QList<int> hits = identifierOccurrences(text, oldName);
    if (hits.isEmpty())
        return 0;

    for (int i = hits.size() - 1; i >= 0; --i)
        text.replace(hits[i], oldName.length(), newName);

    QTextCursor cur = textCursor();
    const int pos = cur.position();
    cur.select(QTextCursor::Document);
    cur.insertText(text);
    cur.setPosition(qMin(pos, cur.position()));
    setTextCursor(cur);
    updateExtraSelections();
    return hits.size();
}

// --- Current line + brace matching -----------------------------------------
//
// Both are drawn as QPlainTextEdit "extra selections": a full-width tint on
// the caret's line, plus -- when the caret sits next to a brace -- a box on
// that brace and its partner. Recomputed on every cursor move.

// Walk out from the brace at `pos` to its match, tracking nesting depth.
// `forward` means the brace is the char at `pos` and we scan right for its
// closer; otherwise it is the char before `pos` and we scan left. Skips
// nothing (a brace inside a string counts) -- good enough for editing feedback.
// Bounds-safe char accessor (QString::at asserts; QString has no value()).
static QChar charAt(const QString& s, int i)
{
    return (i >= 0 && i < s.length()) ? s.at(i) : QChar();
}

int CodeEditor::matchingBrace(int pos, bool forward) const
{
    const QString text = toPlainText();
    const QChar open  = forward ? charAt(text, pos) : charAt(text, pos - 1);
    const QChar close = (open == '{') ? '}'
                      : (open == '(') ? ')'
                      : (open == '[') ? ']' : QChar();
    const QChar openC = (open == '}') ? '{'
                      : (open == ')') ? '('
                      : (open == ']') ? '[' : QChar();

    if (forward && !close.isNull())
    {
        int depth = 0;
        for (int i = pos; i < text.length(); ++i)
        {
            if (text[i] == open)  ++depth;
            if (text[i] == close) { if (--depth == 0) return i; }
        }
    }
    else if (!forward && !openC.isNull())
    {
        int depth = 0;
        for (int i = pos - 1; i >= 0; --i)
        {
            if (text[i] == open)  ++depth;
            if (text[i] == openC) { if (--depth == 0) return i; }
        }
    }
    return -1;
}

void CodeEditor::updateExtraSelections()
{
    QList<QTextEdit::ExtraSelection> selections;
    const QString text = toPlainText();
    const bool focused = hasFocus();

    // Current-line tint -- a faint blue-grey band the full editor width.
    // Caret decorations only in the focused editor: dialogs with two editors
    // (ConstructorCodeDialog: init list + body) would otherwise tint a
    // "current" line in both at once.
    if (focused && !isReadOnly())
    {
        // Current-line tint: a faint wash of the LIVE accent over the editor's
        // white base -- so the active line tracks the SAME accent as the tree
        // selection, its hover, and the completion popups, instead of a
        // hardcoded light blue (JV 2026-07-18). A low mix (~12%) keeps it
        // subtle, below the tree's hover (10%) / selection (28%) but always in
        // the theme's colour.
        const QColor acc = QApplication::palette().color(
            QPalette::Active, QPalette::Highlight);
        auto mix = [](int base, int a) { return (base * 88 + a * 12) / 100; };
        QTextEdit::ExtraSelection line;
        line.format.setBackground(QColor(mix(255, acc.red()),
                                         mix(255, acc.green()),
                                         mix(255, acc.blue())));
        line.format.setProperty(QTextFormat::FullWidthSelection, true);
        line.cursor = textCursor();
        line.cursor.clearSelection();
        selections.append(line);
    }

    // All occurrences of the dialog-coordinated highlight word -- soft
    // yellow, shown regardless of focus: it previews the set of places an
    // F2 rename would touch, across both constructor editors.
    if (!_highlightWord.isEmpty())
    {
        QTextCharFormat fmt;
        fmt.setBackground(QColor(255, 237, 153));
        for (int p : identifierOccurrences(text, _highlightWord))
        {
            QTextEdit::ExtraSelection sel;
            sel.format = fmt;
            sel.cursor = textCursor();
            sel.cursor.setPosition(p);
            sel.cursor.setPosition(p + _highlightWord.length(),
                                   QTextCursor::KeepAnchor);
            selections.append(sel);
        }
    }

    // Method-not-found wave underlines (red), shown regardless of focus so
    // the cue is always visible -- hovering one then explains it. The ranges
    // are refreshed (debounced) by updateDiagnostics.
    if (!_diagnosticRanges.isEmpty())
    {
        QTextCharFormat fmt;
        fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        fmt.setUnderlineColor(QColor(0xC0, 0x00, 0x00));
        const int textLen = text.length();
        for (const QPair<int, int>& range : _diagnosticRanges)
        {
            if (range.first < 0 || range.first + range.second > textLen)
                continue;                       // stale after a fast edit
            QTextEdit::ExtraSelection sel;
            sel.format = fmt;
            sel.cursor = textCursor();
            sel.cursor.setPosition(range.first);
            sel.cursor.setPosition(range.first + range.second,
                                   QTextCursor::KeepAnchor);
            selections.append(sel);
        }
    }

    if (!focused)
    {
        setExtraSelections(selections);
        return;
    }

    // Brace match -- look for an opening/closing brace touching the caret,
    // preferring the char just before it (where you land after typing one).
    const int pos = textCursor().position();
    int braceAt = -1, matchAt = -1;

    auto isBrace = [](QChar c) {
        return c == '{' || c == '}' || c == '(' || c == ')' ||
               c == '[' || c == ']';
    };
    auto isOpen = [](QChar c) { return c == '{' || c == '(' || c == '['; };

    if (pos > 0 && isBrace(charAt(text, pos - 1)))
    {
        braceAt = pos - 1;
        matchAt = matchingBrace(pos, isOpen(charAt(text, pos - 1)));
    }
    else if (pos < text.length() && isBrace(charAt(text, pos)))
    {
        braceAt = pos;
        matchAt = matchingBrace(pos, isOpen(charAt(text, pos)));
    }

    if (braceAt != -1 && matchAt != -1)
    {
        QTextCharFormat fmt;
        fmt.setBackground(QColor(179, 229, 179));   // soft green
        fmt.setFontWeight(QFont::Bold);
        for (int p : {braceAt, matchAt})
        {
            QTextEdit::ExtraSelection sel;
            sel.format = fmt;
            sel.cursor = textCursor();
            sel.cursor.setPosition(p);
            sel.cursor.setPosition(p + 1, QTextCursor::KeepAnchor);
            selections.append(sel);
        }
    }

    setExtraSelections(selections);

    // Report the identifier at the caret so the owning dialog can spread the
    // occurrence highlight across its editors and the signature strip. Only
    // the focused editor reports (we return early above when unfocused), so
    // clicking between editors hands the highlight over cleanly.
    const QString word = identifierUnderCursor();
    if (word != _lastEmittedWord)
    {
        _lastEmittedWord = word;
        emit identifierUnderCursorChanged(word);
    }
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
    // Bands render through bandHtml (escaped + optional highlight spans) --
    // never let auto-detection treat template args like CbArray<int> as tags.
    l->setTextFormat(Qt::RichText);
    return l;
}

// The band text as HTML: escaped, newlines as <br/>. Identifier tokens equal
// to `word` get the same soft yellow as the editors' occurrence highlight;
// tokens in `italics` (the argument names) render italic like they do in the
// code. Both can combine on the same token.
QString bandHtml(const QString& plain, const QString& word,
                 const QSet<QString>& italics)
{
    QString html;
    const int len = plain.length();
    int from = 0;
    int i = 0;
    while (i < len)
    {
        if (!isIdentChar(plain[i]))
        {
            ++i;
            continue;
        }
        int j = i;
        while (j < len && isIdentChar(plain[j]))
            ++j;
        const QString token = plain.mid(i, j - i);
        const bool highlight = (token == word);
        const bool italic    = italics.contains(token);
        if (highlight || italic)
        {
            html += plain.mid(from, i - from).toHtmlEscaped();
            QString style;
            if (highlight)
                style += "background-color:#ffed99;";
            if (italic)
                style += "font-style:italic;";
            html += "<span style=\"" + style + "\">" +
                    token.toHtmlEscaped() + "</span>";
            from = j;
        }
        i = j;
    }
    html += plain.mid(from).toHtmlEscaped();
    html.replace("\n", "<br/>");
    return html;
}

// A band counts as active (reserves space, gets laid out) when it exists and
// carries text. NOTE: do not test QWidget::isVisible() here -- during the
// dialog ctor the editor's ancestors are not shown yet, so isVisible() is
// false and the margins would never be reserved.
bool bandActive(QLabel* b) { return b && !b->text().isEmpty(); }
}

void CodeEditor::renderHeader()
{
    if (_header)
        _header->setText(bandHtml(_headerPlain, _headerWord, _argumentNames));
}

void CodeEditor::setHeaderText(const QString& text)
{
    if (!_header)
    {
        _header = makeBand(this);
        // Ctrl+Click on the signature strip = "who calls me": the strip IS
        // the definition, and go-to-definition ON the definition means
        // show-the-references (the identifier Ctrl+Click's mirror image).
        _header->installEventFilter(this);
    }
    _headerPlain = text;
    renderHeader();
    _header->setVisible(!text.isEmpty());
    refreshBands();   // apply the current zoom font, then reserve/lay out
}

bool CodeEditor::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == _header && event->type() == QEvent::MouseButtonPress)
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton &&
            (mouseEvent->modifiers() & Qt::ControlModifier))
        {
            emit whoCallsMeRequested();
            return true;
        }
    }
    return QPlainTextEdit::eventFilter(obj, event);
}

QRect CodeEditor::headerGlobalRect() const
{
    if (!_header || !_header->isVisible())
        return QRect();
    return QRect(_header->mapToGlobal(QPoint(0, 0)), _header->size());
}

void CodeEditor::setHeaderHighlightWord(const QString& word)
{
    if (word == _headerWord)
        return;
    _headerWord = word;
    renderHeader();
}

void CodeEditor::setFooterText(const QString& text)
{
    if (!_footer)
        _footer = makeBand(this);
    _footer->setText(bandHtml(text, QString(), QSet<QString>()));
    _footer->setVisible(!text.isEmpty());
    refreshBands();   // apply the current zoom font, then reserve/lay out
}

// Apply the current zoom font to the bands so their height AND rendered text
// both track the editor zoom. Setting font-size in each band's OWN stylesheet
// makes it win over the editor's cascading font-size rule (applyEditorFont),
// and setFont at the same size keeps sizeHint()/height in step with the render
// -- without this the cascade grew the text but not the band, clipping it.
void CodeEditor::refreshBands()
{
    QFont f = codeFont();
    f.setPointSize(_zoomPt);
    const QString css =
        QString("background:#e8e8e8; padding:1px 3px; font-size:%1pt;")
            .arg(_zoomPt);
    for (QLabel* b : {_header, _footer})
    {
        if (!b)
            continue;
        b->setFont(f);
        b->setStyleSheet(css);
    }
    updateBandMargins();   // re-reserve height for the new font + reposition
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

// Span each band across the full editor width, pinned to the top / bottom of
// the VIEWPORT (not contentsRect). Anchoring on the viewport is what keeps the
// footer above a horizontal scrollbar: when the scrollbar appears the viewport
// shrinks from the bottom, so viewport().bottom() rises and the footer sits in
// the reserved margin just above the scrollbar instead of being overlapped by
// it. Width still spans the full frame so the grey band runs edge to edge.
void CodeEditor::layoutBands()
{
    const QRect cr = contentsRect();
    const QRect vp = viewport()->geometry();
    if (bandActive(_header))
    {
        const int h = _header->sizeHint().height();
        _header->setGeometry(cr.left(), vp.top() - h, cr.width(), h);
        _header->raise();
    }
    if (bandActive(_footer))
    {
        const int h = _footer->sizeHint().height();
        _footer->setGeometry(cr.left(), vp.bottom() + 1, cr.width(), h);
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

void CodeEditor::focusInEvent(QFocusEvent* event)
{
    QPlainTextEdit::focusInEvent(event);
    updateExtraSelections();
}

void CodeEditor::focusOutEvent(QFocusEvent* event)
{
    QPlainTextEdit::focusOutEvent(event);
    updateExtraSelections();
    hideParameterHint();
}
