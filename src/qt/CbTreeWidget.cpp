// qt/CbTreeWidget.cpp -- QTreeWidget that draws its own branch connectors.
// See CbTreeWidget.h.

#include "CbTreeWidget.h"

#include <QPainter>
#include <QString>
#include <QSize>
#include <QColor>
#include <QPalette>
#include <QApplication>
#include <QImage>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QEvent>
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
    QString sheet = QString("QTreeView { font-size: %2pt; }"
                            "QTreeView::item { height: %1px; }")
                        .arg(rowHeight).arg(ptSize);
#ifdef __APPLE__
    // macOS' style does NOT hover-highlight item-view rows (Windows does), so the
    // row under the cursor gave no feedback here. Add a subtle hover background so
    // it reads the same on both -- only on non-selected rows, so it doesn't fight
    // the selection highlight. (A `:hover` QSS rule also makes Qt enable hover
    // tracking on the view.) Windows keeps its native hover -- this is macOS-only.
    sheet += "QTreeView::item:hover:!selected {"
             "  background: rgba(10, 77, 168, 0.10);"   // accent #0A4DA8 @ ~10%
             "}";
#endif
    setStyleSheet(sheet);

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

void CbTreeWidget::changeEvent(QEvent* event)
{
    QTreeWidget::changeEvent(event);
    // The desktop theme (and so the answer selectionChromeShouldFlip cached)
    // can change while the app is running -- a live qt6ct theme switch, a
    // light/dark toggle -- so drop the cache and re-probe on the next paint.
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange)
        _chromeFlipCached = false;
}

// Whether the branch triangle/connector chrome (drawn in the theme accent)
// would be invisible against a REAL selected row's background. Neither the
// OS nor the QStyle object name reliably predicts this: two Linux desktops
// can report the same style yet render selection differently depending on
// the desktop theme (translucency, gradients, a narrower rounded indicator
// vs. a custom-themed flat opaque fill in exactly the accent colour). So
// instead of guessing from a colour or a name, render a throwaway selected
// cell through this widget's OWN style()/palette() -- the exact inputs the
// real row paint uses -- and sample the pixel it actually produced. Lazily
// computed once per theme (see changeEvent) since it never varies per-row.
bool CbTreeWidget::selectionChromeShouldFlip() const
{
    if (_chromeFlipCached)
        return _chromeFlip;
    _chromeFlipCached = true;
    _chromeFlip = false;

    const int w = 40, h = 24;
    QImage probe(w, h, QImage::Format_ARGB32_Premultiplied);
    probe.fill(Qt::transparent);
    {
        QPainter p(&probe);
        QStyleOptionViewItem opt;
        opt.initFrom(this);
        opt.rect = probe.rect();
        opt.state = QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_Active;
        opt.palette = palette();
        opt.showDecorationSelected = true;
        opt.viewItemPosition = QStyleOptionViewItem::OnlyOne;
        // No icon/text: an empty item still gets the real selection fill,
        // and an empty rect keeps the sampled centre pixel clean of glyphs.
        style()->drawControl(QStyle::CE_ItemViewItem, &opt, &p, this);
    }
    const QColor painted = probe.pixelColor(w / 2, h / 2);
    if (painted.alpha() == 0)
        return false;   // style painted nothing here -- nothing to collide with

    const QPalette appPal = QApplication::palette();
    const QColor accent = appPal.color(QPalette::Active, QPalette::Highlight);
    auto lum = [](const QColor& c) {
        return 0.2126 * c.redF() + 0.7152 * c.greenF() + 0.0722 * c.blueF();
    };
    _chromeFlip = qAbs(lum(painted) - lum(accent)) < 0.12;
    return _chromeFlip;
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

    // On the selected row the triangle + connector chrome can vanish when the
    // row's REAL, style-painted background collides with the accent they're
    // drawn in (Raspberry Pi's qt6ct config, and macOS). Where the painted
    // selection contrasts with the accent -- Windows, and Ubuntu's native
    // style -- keep the accent triangle; flipping there is LESS visible.
    // Neither the OS nor the QStyle object name is a reliable proxy for this:
    // two Linux desktops can share a style name (or even an OS) and still
    // render selection differently (translucency, gradients, a narrower
    // rounded indicator, a custom-themed flat fill). selectionChromeShouldFlip()
    // sidesteps all guessing by rendering a probe selected cell through this
    // widget's own style/palette and sampling the pixel it actually painted.
    QColor triColour  = accent;
    QColor connColour = lineColour;
    if (selectionModel() && selectionModel()->isSelected(index) && selectionChromeShouldFlip())
    {
        triColour  = appPal.color(QPalette::Active, QPalette::HighlightedText);
        connColour = triColour;
    }

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
