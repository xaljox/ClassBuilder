// qt/CodeEditor.h -- a plain-text C++ code editor widget.
//
// QPlainTextEdit subclass: a monospace editor with C++ auto-indentation,
// ported from the MFC CCodeEdit control (archive/rtf is unrelated -- the
// MFC source is ClassBuilder/CodeEdit.{cpp,h}). It is the reusable editing
// widget for the code-edit dialogs (UserCodeDialog / MethodCodeDialog /
// ConstructorCodeDialog).
//
// Deliberately NOT carried over from CCodeEdit:
//   * the hand-rolled 64-entry undo ring buffer -- QPlainTextEdit's document
//     undo (unlimited, with typing coalescing) replaces it;
//   * the context menu / "Insert Iterator" / Alt+F8 hooks -- those are
//     dialog concerns, wired in when the dialogs are ported.
//
// Syntax highlighting can be added later via a QSyntaxHighlighter on this
// widget's document -- no widget change and no extra dependency needed.
#pragma once

#include <QPlainTextEdit>
#include <QFont>
#include <QSet>

class QCompleter;
class QFocusEvent;
class QKeyEvent;
class QModelIndex;
class QResizeEvent;
class QShowEvent;
class QLabel;
class QStandardItemModel;
class CppHighlighter;

// One completion candidate. `display` is shown in the popup (a method shows
// its argument TYPES), `insert` replaces the typed prefix (a method inserts
// its argument NAMES), and `caretBack` steps the caret back after insertion.
// When selectLen > 0, the range ending `selectBack` chars before the end of
// the insert is selected instead -- the first argument name, ready to be
// overtyped.
struct CodeCompletionItem
{
    QString display;
    QString insert;
    int     caretBack  = 0;
    int     selectBack = 0;
    int     selectLen  = 0;
};

// Supplies completion candidates for the caret context; the model-aware
// implementation lives dialog-side (ModelCompletionProvider) so the editor
// itself stays model-free.
class CodeCompletionProvider
{
public:
    virtual ~CodeCompletionProvider() {}

    // Candidates for the caret context. `textToCursor` is the editor text up
    // to the caret; `prefixLen` returns how many trailing characters are the
    // already-typed part of the word being completed (the popup filters on
    // them and the insertion replaces them). Empty list = no popup.
    virtual QList<CodeCompletionItem> completions(const QString& textToCursor,
                                                  int& prefixLen) = 0;

    // Rich-text tooltip for the identifier at `pos` in `text` (the FULL
    // editor text); empty = no tooltip. Drives hover documentation.
    virtual QString hoverText(const QString& text, int pos)
    {
        Q_UNUSED(text)
        Q_UNUSED(pos)
        return QString();
    }
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget* parent = nullptr);

    // Indent step in spaces (the MFC DataModel::GetIndentSize). Default 4.
    void setIndentSize(int spaces);
    int  indentSize() const { return _indentSize; }

    // Fixed marker bands drawn *inside* the editor frame -- a header pinned
    // to the top, a footer pinned to the bottom. The code scrolls in the
    // region between them; the bands never scroll and are not editable.
    // Empty text hides the band. Text may be multi-line.
    void setHeaderText(const QString& text);
    void setFooterText(const QString& text);

    // The shared monospace editor font -- so a marker label placed next to
    // an editor can match the code exactly.
    static QFont codeFont();

    // The editor text with C++ comments and braced blocks removed -- what the
    // Iterator / Variable-Method wizards analyse (the MFC GetStrippedCode).
    QString strippedCode() const;

    // Insert a multi-line snippet at the cursor; continuation lines are given
    // the current line's leading indent so the block lands aligned. Line
    // breaks may be LF, CRLF or bare CR (the wizards emit bare CR).
    void insertSnippet(const QString& text);

    // Insert wizard-generated code at the cursor. Like insertSnippet, but if
    // the snippet ends with an empty `{}` block the caret is left on a fresh
    // line inside it (the logical spot to start typing the body).
    void insertWizardSnippet(const QString& text);

    // The identifier the caret is in or touching (or the exact selected
    // identifier); empty if none. Drives the occurrence highlight and the
    // dialogs' Rename action.
    QString identifierUnderCursor() const;

    // Replace every whole-identifier occurrence of oldName with newName, as
    // one editor-undo step, keeping the caret offset. Returns the number of
    // occurrences replaced.
    int renameIdentifier(const QString& oldName, const QString& newName);

    // Model-known names for the highlighter: model types colour like the
    // built-in types; the method's arguments render italic.
    void setModelTypes(const QSet<QString>& names);
    void setArgumentNames(const QSet<QString>& names);

    // The occurrence-highlight word, set by the owning dialog -- shown in
    // this editor regardless of focus, so the highlight can span both
    // constructor editors and preview what an F2 rename would touch.
    void setHighlightWord(const QString& word);

    // Highlight the word's occurrences inside the header band too (the
    // signature strip -- an argument renamed via F2 changes there as well).
    void setHeaderHighlightWord(const QString& word);

    // The header band's text as plain text (setHeaderText's input) -- lets
    // the dialog count occurrences across editors + signature.
    QString headerPlainText() const { return _headerPlain; }

    // Whole-identifier occurrence count of `word` in `text` (same boundary
    // rule as the editor's own occurrence highlight).
    static int identifierCount(const QString& text, const QString& word);

    // Attach a completion provider (not owned). Enables the popup: auto on
    // '.', '->', '::' and after 2 identifier chars; Ctrl+Space forces it.
    void setCompletionProvider(CodeCompletionProvider* provider);

    // Re-indent the selected lines (or the whole text without a selection)
    // with the same predictor that drives typing, as one undo step. Lines
    // inside block comments are left untouched.
    void reformatCode();

    // Move the selected lines (or the current line) one line up / down as a
    // block, keeping the selection on them. One undo step.
    void moveSelectedLines(bool up);

signals:
    // The identifier at the caret changed (focused editor only; empty when
    // the caret leaves identifiers). The owning dialog spreads the highlight
    // across its editors and signature strip from this.
    void identifierUnderCursorChanged(const QString& word);

    // Cmd+Click (macOS) / Ctrl+Click on an identifier: go to definition.
    // The caret has already been moved to the clicked spot.
    void definitionRequested();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    bool viewportEvent(QEvent* event) override;   // hover tooltips

private:
    // Predicted indent (in spaces) for a fresh line at the cursor -- the MFC
    // CCodeEdit::GetIndent / GetNextLineIndent line-by-line predictor.
    // computeIndentAt predicts for a line starting at `pos`.
    int  computeIndent() const;
    int  computeIndentAt(int pos) const;

    void insertNewlineWithIndent();
    void indentSelection(bool unindent);
    void reindentOpenBrace();
    void reindentClosingBrace();

    // Reserve viewport margins for the marker bands + place them in the frame.
    void updateBandMargins();
    void layoutBands();

    // Recompute the extra selections: the current-line tint plus, when the
    // caret abuts a brace, the highlight of it and its match.
    void updateExtraSelections();

    // Completion plumbing. completionKeyPressEvent eats the keys the visible
    // popup owns (Enter/Tab/arrows/Escape) and Ctrl+Space; maybeTrigger
    // decides after a normal keystroke whether to (re)show or hide the popup.
    bool completionKeyPressEvent(QKeyEvent* event);
    void maybeTriggerCompletion(QKeyEvent* event);
    void triggerCompletion();
    void insertCompletion(const QModelIndex& index);
    int  typedPrefixLength() const;   // identifier chars just before the caret

    // Index in the document of the brace matching the one at `pos` (the char
    // just after `pos` if `forward`, else just before), or -1 if none / unbalanced.
    int matchingBrace(int pos, bool forward) const;

    int _indentSize = 4;

    void renderHeader();             // re-render the header band's HTML

    QLabel* _header = nullptr;       // top marker band, null until first set
    QLabel* _footer = nullptr;       // bottom marker band
    QString _headerPlain;            // header band text before HTML rendering
    QString _headerWord;             // highlight word inside the header band
    QSet<QString> _argumentNames;    // italic in the header band, like the code

    QString _highlightWord;          // dialog-set occurrence-highlight word
    QString _lastEmittedWord;        // identifierUnderCursorChanged de-dup

    CppHighlighter* _highlighter = nullptr;

    CodeCompletionProvider* _provider = nullptr;   // not owned
    QCompleter*             _completer = nullptr;
    QStandardItemModel*     _completionModel = nullptr;
};
