// qt/QtCodePrint.cpp -- see QtCodePrint.h.

#include "QtCodePrint.h"

#include "CodeEditor.h"
#include "CppHighlighter.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QUrl>

#include <algorithm>

namespace {
// Escape the characters that matter inside HTML text.
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

// One text block -> the inner HTML of its line: characters wrapped in <span>s
// that carry the highlighter's colour/bold/italic, gaps between ranges left as
// plain (default black) text.
QString blockToHtml(const QTextBlock& block)
{
    const QString text = block.text();
    if (text.isEmpty())
        return QString();

    QList<QTextLayout::FormatRange> fmts;
    if (block.layout())
        fmts = block.layout()->formats();
    std::sort(fmts.begin(), fmts.end(),
              [](const QTextLayout::FormatRange& a,
                 const QTextLayout::FormatRange& b) { return a.start < b.start; });

    QString html;
    int i = 0;
    const auto emitPlain = [&](int from, int to) {
        if (to > from)
            html += esc(text.mid(from, to - from));
    };
    for (const QTextLayout::FormatRange& r : fmts)
    {
        const int start = qBound(0, r.start, text.length());
        const int end   = qBound(start, r.start + r.length, text.length());
        if (start > i)                 // uncoloured gap before this range
            emitPlain(i, start);
        if (end <= i)                  // already past (overlap guard)
            continue;

        QString style;
        const QTextCharFormat& f = r.format;
        if (f.foreground().style() != Qt::NoBrush)
            style += "color:" + f.foreground().color().name() + ';';
        if (f.fontWeight() >= QFont::Bold)
            style += "font-weight:bold;";
        if (f.fontItalic())
            style += "font-style:italic;";
        const int from = i > start ? i : start;
        const QString chunk = esc(text.mid(from, end - from));
        if (style.isEmpty())
            html += chunk;
        else
            html += "<span style=\"" + style + "\">" + chunk + "</span>";
        i = end;
    }
    emitPlain(i, text.length());       // trailing uncoloured run
    return html;
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

// The one style block for every print. Black on white; a monospace family that
// resolves on every OS. Bands (signature / {//@CODE) a touch dimmer so the
// markers read as frame, not body; the numbered form (whole file) puts faint
// numbers in the <ol> gutter. Code MUST break freely across pages -- keeping a
// long <pre>/<ol> on one page pushes it wholesale to page 2, leaving page 1
// empty (JV 2026-07-23); only a band is glued to the code that follows it.
const char* const kStyle =
    "<style>"
    "body{font-family:'Segoe UI',-apple-system,'Helvetica Neue',sans-serif;"
    "  color:#000;background:#fff;margin:20px;}"
    "h1{font-size:15px;font-weight:600;margin:0 0 10px;}"
    "pre.band,pre.code{font-family:Consolas,'DejaVu Sans Mono',Menlo,monospace;"
    "  font-size:11px;line-height:1.4;margin:0;white-space:pre-wrap;"
    "  word-break:break-word;}"
    "pre.band{color:#555;}"
    "ol.code{font-family:Consolas,'DejaVu Sans Mono',Menlo,monospace;"
    "  font-size:11px;line-height:1.4;margin:0;padding:0 0 0 3.6em;}"
    "ol.code li{white-space:pre-wrap;word-break:break-word;}"
    "ol.code li::marker{color:#b0b0b0;font-size:9px;}"
    "div.method{margin:0 0 22px;}"
    "@media print{body{margin:0;} pre.band{break-after:avoid;"
    "  page-break-after:avoid;}}"
    "@page{margin:14mm;}"
    "</style>";

// Write the assembled <body> content to a temp .html and open it in the browser.
void writeAndOpen(const QString& title, const QString& bodyHtml)
{
    QString page =
        "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\">"
        "<title>" + esc(title) + "</title>" + kStyle + "</head><body>" +
        "<h1>" + esc(title) + "</h1>" + bodyHtml + "</body></html>\n";

    const QString path = QDir(QDir::tempPath())
                             .filePath("CB_" + safeName(title) + ".html");
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;
    f.write(page.toUtf8());
    f.close();

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
} // namespace

QString Cb_HighlightedLinesHtml(const QTextDocument* doc, bool lineNumbers)
{
    QString inner;
    bool first = true;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next())
    {
        const QString line = blockToHtml(b);
        if (lineNumbers)
        {
            inner += "<li>" + line + "</li>";
        }
        else
        {
            if (!first)
                inner += '\n';
            inner += line;             // empty for a blank line -> preserved by \n
        }
        first = false;
    }
    return lineNumbers ? "<ol class=\"code\">" + inner + "</ol>"
                       : "<pre class=\"code\">" + inner + "</pre>";
}

void Cb_PrintCodeInBrowser(const QString& title, const QList<CodeEditor*>& editors)
{
    QString body = "<div class=\"method\">";
    for (CodeEditor* ed : editors)
    {
        if (!ed)
            continue;
        const QString hdr = ed->headerPlainText();
        const QString ftr = ed->footerPlainText();
        if (!hdr.isEmpty())
            body += "<pre class=\"band\">" + esc(hdr) + "</pre>";
        body += ed->toPrintableHtml();     // <pre class="code"> with the colouring
        if (!ftr.isEmpty())
            body += "<pre class=\"band\">" + esc(ftr) + "</pre>";
    }
    body += "</div>";
    writeAndOpen(title, body);
}

void Cb_PrintGeneratedFile(const QString& title, const QString& code,
                           const QSet<QString>& modelTypes)
{
    // Colour the plain generated text through a throwaway document + the same
    // highlighter the editors use. CRLF (the codegen's NL on Windows) would
    // leave stray \r in the blocks, so normalise to \n first.
    QString normalised = code;
    normalised.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalised.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    QTextDocument doc;
    doc.setPlainText(normalised);
    CppHighlighter hl(&doc);
    hl.setModelTypes(modelTypes);
    hl.rehighlight();

    writeAndOpen(title, Cb_HighlightedLinesHtml(&doc, /*lineNumbers=*/true));
}
