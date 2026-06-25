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

class QKeyEvent;
class QResizeEvent;
class QShowEvent;
class QLabel;

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

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    // Predicted indent (in spaces) for a fresh line at the cursor -- the MFC
    // CCodeEdit::GetIndent / GetNextLineIndent line-by-line predictor.
    int  computeIndent() const;

    void insertNewlineWithIndent();
    void indentSelection(bool unindent);
    void reindentClosingBrace();

    // Reserve viewport margins for the marker bands + place them in the frame.
    void updateBandMargins();
    void layoutBands();

    int _indentSize = 4;

    QLabel* _header = nullptr;       // top marker band, null until first set
    QLabel* _footer = nullptr;       // bottom marker band
};
