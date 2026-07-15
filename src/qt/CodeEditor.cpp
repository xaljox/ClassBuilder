// qt/CodeEditor.cpp -- a plain-text C++ code editor widget.
//
// Ported from the MFC CCodeEdit (ClassBuilder/CodeEdit.cpp). The core is the
// indent predictor: GetIndent walks the text up to the cursor line by line,
// and GetNextLineIndent predicts the indent of the line that follows each.

#include "CodeEditor.h"
#include "CppHighlighter.h"

#include <QAbstractItemView>
#include <QColor>
#include <QCompleter>
#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QList>
#include <QPalette>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QToolTip>

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

    _highlighter = new CppHighlighter(document());

    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::updateExtraSelections);
    connect(this, &QPlainTextEdit::selectionChanged,
            this, &CodeEditor::updateExtraSelections);
    updateExtraSelections();
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

void CodeEditor::keyPressEvent(QKeyEvent* event)
{
    if (completionKeyPressEvent(event))
        return;

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
        connect(_completer,
                QOverload<const QModelIndex&>::of(&QCompleter::activated),
                this, &CodeEditor::insertCompletion);
    }
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
    if (event->key() == Qt::Key_Space &&
        (event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)))
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
        trigger = visible || typedPrefixLength() >= 2;
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
        row->setData(item.insert, Qt::UserRole);
        row->setData(item.caretBack, Qt::UserRole + 1);
        row->setData(item.selectBack, Qt::UserRole + 2);
        row->setData(item.selectLen, Qt::UserRole + 3);
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
        QTextEdit::ExtraSelection line;
        line.format.setBackground(QColor(232, 242, 254));
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
        _header = makeBand(this);
    _headerPlain = text;
    renderHeader();
    _header->setVisible(!text.isEmpty());
    updateBandMargins();
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

void CodeEditor::focusInEvent(QFocusEvent* event)
{
    QPlainTextEdit::focusInEvent(event);
    updateExtraSelections();
}

void CodeEditor::focusOutEvent(QFocusEvent* event)
{
    QPlainTextEdit::focusOutEvent(event);
    updateExtraSelections();
}
