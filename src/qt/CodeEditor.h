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
#include <QIcon>
#include <QPair>
#include <QList>
#include <QSet>
#include <QVector>

class QCompleter;
class QFocusEvent;
class QKeyEvent;
class QModelIndex;
class QResizeEvent;
class QShowEvent;
class QWheelEvent;
class QLabel;
class QStandardItemModel;
class QTimer;
class CppHighlighter;

// One completion candidate. `display` is shown in the popup (a method shows
// its argument TYPES), `insert` replaces the typed prefix (a method inserts
// its argument NAMES), and `caretBack` steps the caret back after insertion.
// When selectLen > 0, the range ending `selectBack` chars before the end of
// the insert is selected instead -- the first argument name, ready to be
// overtyped. `icon` is the model icon for the row (kind = method / member /
// argument / type / iterator); `detail` is the muted right-aligned text (a
// method's return type, a variable's type, "class").
struct CodeCompletionItem
{
    QString display;
    QString insert;
    QIcon   icon;
    QString detail;
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

    // Rich-text parameter hint for the innermost open call at the caret
    // (`textToCursor` is the editor text up to the caret); empty = no open
    // call / nothing resolvable -- the editor hides the hint then.
    virtual QString parameterHint(const QString& textToCursor)
    {
        Q_UNUSED(textToCursor)
        return QString();
    }

    // Ranges [start, length] of qualified calls in `text` whose receiver
    // resolves to a real modeled class that has no such method -- the editor
    // wave-underlines them (a visible cue inviting a hover, which then shows
    // the method-not-found warning). Empty by default.
    virtual QVector<QPair<int, int>> unresolvedCalls(const QString& text)
    {
        Q_UNUSED(text)
        return {};
    }
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget* parent = nullptr);
    ~CodeEditor() override;

    // Indent step in spaces (the MFC DataModel::GetIndentSize). Default 4.
    void setIndentSize(int spaces);
    int  indentSize() const { return _indentSize; }

    // Font zoom as commands, so the hosting dialog's View menu drives the same
    // path as Ctrl+= / Ctrl+- / Ctrl+0 and Ctrl+wheel. The level is SHARED by
    // every open editor (applySharedZoom), so the panes of one logical edit
    // scale together whichever one is zoomed.
    void zoomStep(int delta);      // +1 in, -1 out
    void zoomReset();              // back to the code font's own size

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

    // The header/footer band text as plain text (setHeaderText/setFooterText's
    // input). The header lets the dialog count occurrences across editors +
    // signature; both let the print reproduce the marker bands ({//@CODE ...
    // }//@CODE) that frame the body but are not part of the document.
    QString headerPlainText() const { return _headerPlain; }
    QString footerPlainText() const { return _footerPlain; }

    // Global rect of the header band (signature strip); null when there is
    // no visible header. Anchors the who-calls-me popup right under it.
    QRect headerGlobalRect() const;

    // Whole-identifier occurrence count of `word` in `text` (same boundary
    // rule as the editor's own occurrence highlight).
    static int identifierCount(const QString& text, const QString& word);

    // Attach a completion provider (not owned). Enables the popup: auto on
    // '.', '->', '::' and after 2 identifier chars; Ctrl+Space forces it.
    void setCompletionProvider(CodeCompletionProvider* provider);

    // Re-pin the text-selection colours to the CURRENT theme accent. Called by
    // the app-wide accent watcher (QtApp.cpp) when the desktop accent changes
    // while CB is open, so the editor selection follows the accent live.
    void reapplyThemeAccent();

    // Re-indent the selected lines (or the whole text without a selection)
    // with the same predictor that drives typing, as one undo step. Lines
    // inside block comments are left untouched.
    void reformatCode();

    // Move the selected lines (or the current line) one line up / down as a
    // block, keeping the selection on them. One undo step.
    void moveSelectedLines(bool up);

    // Toggle `//` line comments on the selected lines (or the current line):
    // uncomment when every non-blank line is already commented, else comment.
    void toggleLineComment();

    // Wrap the selection (or the current line) in a `/* ... */` block
    // comment; unwrap when it already is one. One undo step.
    void toggleBlockComment();

    // A print-ready HTML fragment of this editor's code, carrying the SAME
    // syntax colouring shown on screen (read from the highlighter, not
    // re-tokenised). Each source line is one <li> so the browser numbers and
    // page-breaks it. Drives the File > Print preview (see QtCodePrint.h).
    QString toPrintableHtml();

signals:
    // The identifier at the caret changed (focused editor only; empty when
    // the caret leaves identifiers). The owning dialog spreads the highlight
    // across its editors and signature strip from this.
    void identifierUnderCursorChanged(const QString& word);

    // Cmd+Click (macOS) / Ctrl+Click on an identifier: go to definition.
    // The caret has already been moved to the clicked spot.
    void definitionRequested();

    // Cmd+Click (macOS) / Ctrl+Click on the header band (signature strip):
    // "who calls me" -- the strip is the definition, so the definition
    // gesture on it means show-the-references.
    void whoCallsMeRequested();

protected:
    bool event(QEvent* event) override;             // claim Ctrl+zoom keys
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;   // Ctrl+wheel = font zoom
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    bool viewportEvent(QEvent* event) override;   // hover tooltips
    bool eventFilter(QObject* obj, QEvent* event) override;  // header band

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

    // Set the editor's font point size, overriding the app-wide stylesheet
    // (a bare `QWidget { font-size }` rule would otherwise pin it, defeating
    // setFont/zoom); recomputes the tab stops. `pt` is clamped.
    void applyEditorFont(int pt);
    // Font zoom is SHARED across every open code editor so the panes of one
    // logical edit (e.g. a constructor's init list + body, or the six User
    // Sections editors) always scale together -- zooming one zooms them all.
    // The user-zoom gestures call applySharedZoom(); it stores the level and
    // re-applies it to every live editor. applyEditorFont() above is the
    // per-editor apply it drives.
    static void applySharedZoom(int pt);
    static QList<CodeEditor*>& liveEditors();
    // Pin QPalette::Highlight/HighlightedText to the live theme accent (the
    // text-selection colour). One place, called from the constructor and from
    // reapplyThemeAccent() on a live accent change.
    void applyThemeAccent();

    // Auto-close pairs: an opener inserts its closer (caret between); a closer
    // typed where it already sits steps over it; Backspace on an empty pair
    // deletes both. Returns true when the key was fully handled. Also expands
    // `{|}` on Enter into an indented three-line block.
    bool autoCloseKeyPressEvent(QKeyEvent* event);

    // Reserve viewport margins for the marker bands + place them in the frame.
    void updateBandMargins();
    void layoutBands();
    // Apply the CURRENT zoom font to the marker bands (font + matching
    // font-size in their own stylesheet, so their height and rendered text
    // both track the editor zoom) and re-reserve their margins. Guarded on
    // null, so it is safe to call before a band exists.
    void refreshBands();

    // Recompute the extra selections: the current-line tint plus, when the
    // caret abuts a brace, the highlight of it and its match.
    void updateExtraSelections();

    // Re-query the provider for method-not-found call ranges (debounced off
    // textChanged) and refresh the wave underlines.
    void updateDiagnostics();

    // Completion plumbing. completionKeyPressEvent eats the keys the visible
    // popup owns (Enter/Tab/arrows/Escape) and Ctrl+Space; maybeTrigger
    // decides after a normal keystroke whether to (re)show or hide the popup.
    bool completionKeyPressEvent(QKeyEvent* event);
    void maybeTriggerCompletion(QKeyEvent* event);
    void triggerCompletion();
    void insertCompletion(const QModelIndex& index);
    int  typedPrefixLength() const;   // identifier chars just before the caret

    // Parameter hint: a tooltip-styled label above the caret's line with the
    // called method's signature, the active argument bold. Shown on '('/',',
    // re-resolved on every caret move while visible, hidden when the caret
    // leaves the call (provider returns empty) / Esc / focus loss.
    void updateParameterHint(bool allowShow);
    void hideParameterHint();

    // Index in the document of the brace matching the one at `pos` (the char
    // just after `pos` if `forward`, else just before), or -1 if none / unbalanced.
    int matchingBrace(int pos, bool forward) const;

    int _indentSize = 4;

    void renderHeader();             // re-render the header band's HTML

    QLabel* _header = nullptr;       // top marker band, null until first set
    QLabel* _footer = nullptr;       // bottom marker band
    QString _headerPlain;            // header band text before HTML rendering
    QString _footerPlain;            // footer band text before HTML rendering
    QString _headerWord;             // highlight word inside the header band
    QSet<QString> _argumentNames;    // italic in the header band, like the code

    QString _highlightWord;          // dialog-set occurrence-highlight word
    QString _lastEmittedWord;        // identifierUnderCursorChanged de-dup

    CppHighlighter* _highlighter = nullptr;

    CodeCompletionProvider* _provider = nullptr;   // not owned
    QCompleter*             _completer = nullptr;
    QStandardItemModel*     _completionModel = nullptr;

    QLabel* _paramHint = nullptr;    // parameter-hint label, created on demand

    int _basePt = 11;                // codeFont() point size (zoom baseline)
    int _zoomPt = 11;                // current editor point size
    static int s_zoomPt;             // shared zoom level for all editors (-1 = unset)

    // Method-not-found diagnostics: [start, length] call ranges to wave-
    // underline, refreshed (debounced) by _diagnosticTimer off textChanged.
    QVector<QPair<int, int>> _diagnosticRanges;
    QTimer* _diagnosticTimer = nullptr;
};
