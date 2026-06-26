// qt/CbTreeWidget.cpp -- QTreeWidget that draws its own branch connectors.
// See CbTreeWidget.h.

#include "CbTreeWidget.h"

#include <QPainter>
#include <QString>
#include <QSize>
#include <QColor>
#include <QPalette>
#include <QApplication>
#ifdef CB_HAVE_SVG
#include <QSvgRenderer>
#include <QHash>
#include <QFile>
#include <QByteArray>
#include <QRegularExpression>
#endif

CbTreeWidget::CbTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    // Compact the tree rows to the text lists' row height so a tree and a
    // list side by side in one dialog (FindMethod, IteratorWizard) line up.
    // The lists size each item via QtCompact's compactItemSize =
    // fontMetrics().height() + 2; match that. (The font itself -- shared with
    // the lists -- is the app-wide one set in Qt_EnsureApplication.)
    //
    // The modern Windows style imposes a generous touch-friendly minimum
    // tree-row height that a delegate sizeHint cannot undercut; an explicit
    // QTreeView::item height in a per-widget stylesheet IS authoritative.
//    const int rowHeight = fontMetrics().height() + 0;  // no extra padding, the default is already roomy
    const int rowHeight = fontMetrics().height() - 1;  // no extra padding, the default is already roomy
    // Re-assert the app font-size in this widget's OWN stylesheet. Setting a
    // stylesheet (the row height) routes the tree through Qt's stylesheet
    // renderer, which otherwise drops the app font (QApplication::setFont) and
    // renders the tree text smaller than the rest of the UI. font() here is
    // still the app font (set before any widget exists), so pointSize() == the
    // app's CB_UI_FONT_PT. (rowHeight above was measured from that same font.)
    const int ptSize = font().pointSize();
    setStyleSheet(QString("QTreeView { font-size: %2pt; }"
                          "QTreeView::item { height: %1px; }")
                      .arg(rowHeight).arg(ptSize));

    // Model icons: size them just inside the row height (a small inset reads
    // best -- lands ~the MFC tree's 24px with a little breathing room; the bare
    // Qt default decoration size was smaller, so the res/*.ico art scaled
    // worse). Set here so EVERY tree (main view + all dialog trees, which are
    // all CbTreeWidget) is consistent. Does not affect the row height above.
    // Sharper icons need SVG model art (the planned redo); MFC-zoom tracking is
    // a later slice -- see qt/PORT_DIALOGS.md.
//    const int kIconInset = 3;
    const int kIconInset = 1;
    setIconSize(QSize(rowHeight - kIconInset, rowHeight - kIconInset));
}

#ifdef CB_HAVE_SVG
namespace {

// Linear blend of two colours: t=0 -> a, t=1 -> b.
QColor blend(const QColor& a, const QColor& b, double t)
{
    return QColor(int(a.red()   * (1 - t) + b.red()   * t),
                  int(a.green() * (1 - t) + b.green() * t),
                  int(a.blue()  * (1 - t) + b.blue()  * t));
}

// Render one branch glyph into a cell, recoloured to `tint`. The SVG art is
// loaded once, its baked grey `fill` colours rewritten to `tint`, and a
// QSvgRenderer built from the edited markup -- so the hierarchy chrome takes
// a single, theme-derived colour. Renderers are cached per (path, tint) and
// kept for the process lifetime (a handful of tiny files).
void renderBranch(QPainter* painter, const QRect& cell, const QString& path,
                  const QColor& tint)
{
    static QHash<QString, QSvgRenderer*> cache;
    const QString key = path + QLatin1Char('|') + tint.name();
    QSvgRenderer*& r = cache[key];
    if (!r)
    {
        QFile file(path);
        QByteArray markup;
        if (file.open(QIODevice::ReadOnly))
            markup = file.readAll();
        // Rewrite every #rrggbb fill in the artwork to the tint colour.
        QString edited = QString::fromUtf8(markup);
        edited.replace(QRegularExpression(QStringLiteral("#[0-9A-Fa-f]{6}")),
                       tint.name());
        r = new QSvgRenderer(edited.toUtf8());
    }
    if (r->isValid() && !cell.isEmpty())
        r->render(painter, QRectF(cell));
}

} // namespace
#endif

// Draw the tree connector lines + expand triangles ourselves. A stylesheet
// cannot do this correctly: it has no notion of depth, so it cannot tell a
// root-level leaf (-> branch_top, a plain stub) from a nested last-child leaf
// (-> branch_end, an ell). Here the glyph is chosen with the depth and
// sibling state in hand. The artwork is the qt/tree/*.svg set.
void CbTreeWidget::drawBranches(QPainter* painter, const QRect& rect,
                                const QModelIndex& index) const
{
#ifndef CB_HAVE_SVG
    QTreeWidget::drawBranches(painter, rect, index);   // no Svg -> native
#else
    const int ind = indentation();

    // Hierarchy chrome takes a colour found in no model icon, so the tree's
    // structure reads as a separate layer from its content: the expand
    // triangles in the theme accent, the connector lines a muted accent
    // (accent blended halfway to the tree background). Both palette-derived,
    // so they follow the system theme.
    // Read from the APPLICATION palette, not this widget's: CbTreeWidget has
    // a stylesheet set (the row height), and a widget with a stylesheet
    // resolves palette() through the stylesheet style -- roles not named in
    // the QSS (Highlight here) fall back to stylesheet-style defaults (a
    // grey), not the theme accent. qApp's palette is the genuine theme one.
    // Use the ACTIVE group explicitly too: the Inactive Highlight is the grey
    // unfocused-selection colour.
    const QPalette appPal = QApplication::palette();
    const QColor accent = appPal.color(QPalette::Active, QPalette::Highlight);
    const QColor lineColour = blend(
        accent, appPal.color(QPalette::Active, QPalette::Base), 0.25);

#ifdef __APPLE__
    // macOS: a SELECTED row's background is QPalette::Highlight -- the very colour
    // the triangle + connector lines use, so they'd be invisible on the selection.
    // Draw the chrome in HighlightedText (the selection foreground) for the
    // selected row instead, so it stays visible. Windows' selection background
    // differs from the accent, so the triangle stays visible there -- no flip
    // needed (and flipping it just read as a colour glitch on selection).
    const bool selected = selectionModel() && selectionModel()->isSelected(index);
    const QColor triColour  = selected
        ? appPal.color(QPalette::Active, QPalette::HighlightedText) : accent;
    const QColor connColour = selected
        ? appPal.color(QPalette::Active, QPalette::HighlightedText) : lineColour;
#else
    const QColor triColour  = accent;
    const QColor connColour = lineColour;
#endif

    // Walk root..item; record, per level, whether that node has a sibling
    // below it. hasNext.last() is the item itself; size-1 == its depth.
    QList<bool> hasNext;
    for (QModelIndex m = index; m.isValid(); m = m.parent())
        hasNext.prepend(m.sibling(m.row() + 1, m.column()).isValid());
    const int depth = hasNext.size() - 1;

    // Ancestor columns: a continuing vertical line where the ancestor at that
    // level still has siblings below.
    for (int level = 0; level < depth; ++level)
    {
        if (hasNext[level])
            renderBranch(painter,
                QRect(rect.left() + level * ind, rect.top(),
                      ind, rect.height()),
                QStringLiteral(":/tree/vline.svg"), connColour);
    }

    // The item's own column.
    const QRect own(rect.left() + depth * ind, rect.top(),
                    ind, rect.height());
    const bool hasChildren = model() && model()->hasChildren(index);

    QString glyph;
    if (hasChildren)
        glyph = isExpanded(index) ? QStringLiteral(":/tree/branch_open.svg")
                                  : QStringLiteral(":/tree/branch_closed.svg");
    else if (depth == 0 && index.row() == 0 && !hasNext[depth])
        // The ONLY top-level node: no sibling above or below to connect to,
        // so a plain stub.
        glyph = QStringLiteral(":/tree/branch_top.svg");
    else if (depth == 0 && index.row() == 0)
        // The FIRST top-level node: nothing above (no parent), sibling below
        // -- stub + lower-half vertical only.
        glyph = QStringLiteral(":/tree/branch_first.svg");
    else if (hasNext[depth])
        glyph = QStringLiteral(":/tree/branch_more.svg");  // tee
    else
        glyph = QStringLiteral(":/tree/branch_end.svg");   // ell

    // Triangles (expand/collapse) get the full accent; the line glyphs the
    // muted line colour. On the selected row both switch to the selection
    // foreground (see triColour/connColour above) so they don't vanish.
    renderBranch(painter, own, glyph, hasChildren ? triColour : connColour);
#endif
}
