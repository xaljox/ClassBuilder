// qt/QtCodePrint.cpp -- see QtCodePrint.h.

#include "QtCodePrint.h"

#include "CodeEditor.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QUrl>

namespace {
// Same escape as CodeEditor's, for the band text (plain, uncoloured).
QString esc(const QString& s)
{
    QString o;
    o.reserve(s.size() + 8);
    for (const QChar c : s)
    {
        if (c == '&')      o += QStringLiteral("&amp;");
        else if (c == '<') o += QStringLiteral("&lt;");
        else if (c == '>') o += QStringLiteral("&gt;");
        else               o += c;
    }
    return o;
}

// A file-name-safe form of the title for the temp file.
QString safeName(const QString& title)
{
    QString n;
    for (const QChar c : title)
        n += (c.isLetterOrNumber() || c == '_' || c == '-') ? c : QChar('_');
    if (n.isEmpty())
        n = QStringLiteral("code");
    return n.left(80);
}

// The page's CSS. Black on white for print; a monospace family that resolves on
// every OS. NO line numbers -- a single method body reads better without them
// (JV 2026-07-23). The bands (signature / {//@CODE ... }//@CODE) and the code use
// the SAME monospace and stack tight (margin 0) so a method prints as one block;
// the bands are a touch dimmer so the markers read as frame, not body. @page
// trims the browser's default margins.
const char* const kStyle =
    "<style>"
    "body{font-family:'Segoe UI',-apple-system,'Helvetica Neue',sans-serif;"
    "  color:#000;background:#fff;margin:20px;}"
    "h1{font-size:15px;font-weight:600;margin:0 0 10px;}"
    "pre.band,pre.code{font-family:Consolas,'DejaVu Sans Mono',Menlo,monospace;"
    "  font-size:11px;line-height:1.4;margin:0;white-space:pre-wrap;"
    "  word-break:break-word;}"
    "pre.band{color:#555;}"
    "div.method{margin:0 0 22px;}"
    // Code MUST be allowed to break across pages: a method longer than a page
    // otherwise cannot fit under the title and the whole <pre> is pushed to
    // page 2, leaving page 1 nearly empty (JV 2026-07-23). Only keep a band
    // glued to the code that follows it, so a signature/{ is not orphaned at a
    // page foot -- break-after:avoid moves just that one band, not the body.
    "@media print{body{margin:0;} pre.band{break-after:avoid;"
    "  page-break-after:avoid;}}"
    "@page{margin:14mm;}"
    "</style>";
} // namespace

void Cb_PrintCodeInBrowser(const QString& title, const QList<CodeEditor*>& editors)
{
    QString page =
        "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\">"
        "<title>" + esc(title) + "</title>" + kStyle + "</head><body>";
    page += "<h1>" + esc(title) + "</h1>";

    page += "<div class=\"method\">";
    for (CodeEditor* ed : editors)
    {
        if (!ed)
            continue;
        const QString hdr = ed->headerPlainText();
        const QString ftr = ed->footerPlainText();
        if (!hdr.isEmpty())
            page += "<pre class=\"band\">" + esc(hdr) + "</pre>";
        page += ed->toPrintableHtml();     // <pre class="code"> with the colouring
        if (!ftr.isEmpty())
            page += "<pre class=\"band\">" + esc(ftr) + "</pre>";
    }
    page += "</div></body></html>\n";

    const QString path = QDir(QDir::tempPath())
                             .filePath("CB_" + safeName(title) + ".html");
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;
    f.write(page.toUtf8());
    f.close();

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
