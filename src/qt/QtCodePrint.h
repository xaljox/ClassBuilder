// qt/QtCodePrint.h -- print code via the system browser.
//
// CB does not link QtPrintSupport (the MFC print path was dropped and never
// replaced). Instead of a Qt print dialog, code is exported -- WITH the syntax
// colouring -- to a small self-contained HTML page and opened in the user's
// default browser, where Ctrl/Cmd+P prints through the normal system route
// (printer, or "Save as PDF"). One HTML file per print, written to the temp dir;
// the browser owns it from there.
//
// Two entry points share the colouring:
//   * a code EDITOR's content (a method / constructor body) -- CodeEditor::
//     toPrintableHtml(), which drives Cb_PrintCodeInBrowser below;
//   * a whole GENERATED FILE (a class's .h / .cpp) -- Cb_PrintGeneratedFile,
//     which colours plain generated text through a throwaway highlighter.

#pragma once

#include <QList>
#include <QSet>
#include <QString>

class CodeEditor;
class QTextDocument;

// Walk a document's blocks and emit the syntax-coloured lines as HTML: <span>s
// carrying each block layout's highlighter formats (colour/bold/italic), gaps
// left as plain text. `lineNumbers` false wraps the lines in <pre class="code">
// (newline-separated, no numbers -- a single body); true wraps them in
// <ol class="code"> so the browser numbers each line (a whole file). Shared by
// both entry points so their colouring is one implementation.
QString Cb_HighlightedLinesHtml(const QTextDocument* doc, bool lineNumbers);

// Print one or more code editors. `title` heads the page (and names the temp
// file). Each editor prints as its marker bands + code stacked as the dialog
// shows them: the header band (signature and/or {//@CODE...), the coloured body,
// then the footer band (}//@CODE...). Several editors (a constructor's init list
// + body) stack in order, so the printout reads as the whole method. No line
// numbers.
void Cb_PrintCodeInBrowser(const QString& title,
                           const QList<CodeEditor*>& editors);

// Print a whole generated file. `code` is the file text as the codegen produced
// it (Class::WriteHFileBody / WriteCppFileBody); `modelTypes` are the model type
// / class names to colour like built-in types, matching the editors. Rendered
// WITH line numbers -- a full file earns them.
void Cb_PrintGeneratedFile(const QString& title, const QString& code,
                           const QSet<QString>& modelTypes);
