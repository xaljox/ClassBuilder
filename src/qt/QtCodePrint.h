// qt/QtCodePrint.h -- print code from a code editor via the system browser.
//
// CB does not link QtPrintSupport (the MFC print path was dropped and never
// replaced). Instead of a Qt print dialog, the editors export their code -- WITH
// the on-screen syntax colouring -- to a small self-contained HTML page and open
// it in the user's default browser, where Ctrl/Cmd+P prints through the normal
// system route (printer, or "Save as PDF"). One HTML file per print, written to
// the temp dir; the browser owns it from there.

#pragma once

#include <QList>
#include <QString>

class CodeEditor;

// Build the print page and open it in the browser. `title` heads the page (and
// names the temp file). Each editor is printed as its marker bands + code stacked
// the way the dialog shows them: the header band (signature and/or {//@CODE...),
// the colour-highlighted body, then the footer band (}//@CODE...). Several
// editors (a constructor's init list + body) stack in order, so the printout
// reads as the whole method. Colouring comes from CodeEditor::toPrintableHtml().
void Cb_PrintCodeInBrowser(const QString& title,
                           const QList<CodeEditor*>& editors);
