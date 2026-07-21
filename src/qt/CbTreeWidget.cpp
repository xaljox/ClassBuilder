// qt/CbTreeWidget.cpp -- QTreeWidget that draws its own branch connectors.
// See CbTreeWidget.h.

#include "CbTreeWidget.h"
#include "QtSoftSelection.h"   // Qt_SoftSelectionColor -- shared opaque accent tint

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
#include <QHoverEvent>
#include <QItemSelection>
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
    applyThemeStyleSheet();

    // Model icons: size them just inside the row height (a small inset reads
    // best -- lands ~the MFC tree's 24px with a little breathing room; the bare
    // Qt default decoration size was smaller, so the res/*.ico art scaled
    // worse). Set here so EVERY tree (main view + all dialog trees, which are
    // all CbTreeWidget) is consistent. Does not affect the row height above.
    // Sharper icons need SVG model art (the planned redo); MFC-zoom tracking is
    // a later slice -- see qt/PORT_DIALOGS.md.
    const int kIconInset = 1;
    const int rowHeight = fontMetrics().height() - 1;
    setIconSize(QSize(rowHeight - kIconInset, rowHeight - kIconInset));
}

void CbTreeWidget::reapplyThemeAccent()
{
    // The desktop accent changed while CB is open -- re-derive the tint (and
    // drop the probe cache) so the tree matches the new accent live. The
    // branch triangles/lines already read the live accent every paint, so a
    // repaint is all they need.
    _chromeFlipCached = false;
    applyThemeStyleSheet();
    if (viewport())
        viewport()->update();
}

void CbTreeWidget::applyThemeStyleSheet()
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
    // Re-assert the app font WEIGHT too. A QSS font rule (the font-size above)
    // otherwise resets the weight to Normal(400), so the app's chosen weight
    // (CB_UI_FONT_WEIGHT) never reached the tree -- which is most of the UI text.
    // Passing font().weight() makes the tree follow that one knob on every OS.
    const int wt = font().weight();
    // outline:0 -- no item focus rectangle: the style draws a hard (near-black)
    // outline around the current row when the view has focus, which reads as an
    // odd box over the soft selection tint. The selection is marked instead by a
    // full-accent stripe at the row's left edge (see drawBranches) -- JV
    // 2026-07-18.
    QString sheet = QString("QTreeView { font-size: %2pt; font-weight: %3;"
                            " outline: 0px; }"
                            "QTreeView::item { height: %1px; }")
                        .arg(rowHeight).arg(ptSize).arg(wt);
    // Selection + hover: the SAME soft accent tint on EVERY platform, matching
    // the editor's completion / who-calls-me popups (Qt_ApplySoftSelection), so
    // tree and popups read as one look. The native selections differed per OS
    // (macOS' full accent + white text, GNOME's full accent, Windows' saturated
    // -- greenish -- highlight); a translucent LIVE-accent wash (0.28) with the
    // normal dark text kept -- plus the popups' hover tint (0.10) on
    // non-selected rows -- unifies them, and reads clearer than any native
    // saturated fill (JV 2026-07-18: the completion popup look is better than
    // the native Windows tree selection -- make the tree match it). Live accent
    // (not a hardcoded blue) so it tracks the user's highlight colour, which on
    // Windows is the greener system highlight. See also drawBranches() and
    // QtSoftSelection.h.
    // OPAQUE accent tint (not a translucent rgba): a translucent wash lets the
    // platform style's own selection fill show through underneath -- on GNOME a
    // saturated blue unrelated to the accent -- which split the selected row into
    // an accent gutter + a native-blue item body (and tinted hover blue). The
    // opaque fill covers that native paint, so the row is one accent-derived
    // colour on every platform. Same blend weights (0.28 / 0.10) and same helper
    // as the editor popups, so tree and popups stay identical. See
    // QtSoftSelection.h and drawBranches() (whose gutter uses the same blend).
    const QColor sel   = Qt_SoftSelectionColor(0.28);
    const QColor hover = Qt_SoftSelectionColor(0.10);
    sheet += QString("QTreeView::item:selected {"
                     "  background: rgb(%1, %2, %3);"
                     "  color: palette(text);"
                     "}"
                     "QTreeView::item:hover:!selected {"
                     "  background: rgb(%4, %5, %6);"
                     "}")
                 .arg(sel.red()).arg(sel.green()).arg(sel.blue())
                 .arg(hover.red()).arg(hover.green()).arg(hover.blue());
    setStyleSheet(sheet);
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

// macOS: when a row's hover/selection tint goes away, Qt invalidates only
// that row's exact rect -- but the layered row paint (native full-row
// selection under the QSS tint) can bleed a pixel outside it, so a thin
// blue stripe is sometimes left behind on the transition. Repaint the whole
// viewport on those transitions instead: they are discrete events (a row
// crossed, a selection change), the trees are small, and a full repaint
// kills every remnant whatever its exact origin. Elsewhere the base
// behavior is enough.
bool CbTreeWidget::viewportEvent(QEvent* event)
{
#ifdef __APPLE__
    const QEvent::Type t = event->type();
    if (t == QEvent::HoverMove || t == QEvent::HoverEnter ||
        t == QEvent::HoverLeave)
    {
        QPersistentModelIndex row;
        if (t != QEvent::HoverLeave)
            row = indexAt(
                static_cast<QHoverEvent*>(event)->position().toPoint());
        if (row != _hoverRow)
        {
            _hoverRow = row;
            viewport()->update();
        }
    }
#endif
    return QTreeWidget::viewportEvent(event);
}

void CbTreeWidget::selectionChanged(const QItemSelection& selected,
                                    const QItemSelection& deselected)
{
    QTreeWidget::selectionChanged(selected, deselected);
#ifdef __APPLE__
    viewport()->update();
#endif
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
    // Two colours off ONE accent: the row-sized tint blends the accent AS
    // CHOSEN, while the small solid glyphs take Qt_ChromeAccent() -- the same
    // accent with its lightness clamped so a light one still carries at glyph
    // size (shared by every platform, see QtSoftSelection.h).
    const QPalette appPal = QApplication::palette();
    const QColor accent = appPal.color(QPalette::Active, QPalette::Highlight);
    const QColor glyphColour = Qt_ChromeAccent();
    const QColor lineColour = blend(
        glyphColour, appPal.color(QPalette::Active, QPalette::Base), 0.25);

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
    // The expand triangle and the selection stripe are small SOLID glyphs, so
    // they take the lightness-clamped accent (Qt_ChromeAccent, computed above);
    // the clamp itself now lives in that one shared helper instead of here, so
    // every glyph site on every platform uses the same rule (JV 2026-07-19 /
    // 2026-07-21).
    QColor triColour  = glyphColour;
    QColor connColour = lineColour;
    // EVERY platform now uses the soft ::item:selected tint (constructor QSS).
    // The base view paints its native selection across the WHOLE row -- including
    // this branch/indent gutter -- before drawBranches runs, so without this the
    // gutter kept the saturated native fill while the item columns took the light
    // tint, splitting the row into two colours. Repaint the gutter with the SAME
    // soft tint so the selected row is one colour. rgba(accent,0.28) over Base ==
    // blend(Base, accent, 0.28); the solid form fills opaquely over the already-
    // drawn native selection. No chrome flip needed: the gutter is light, so the
    // accent triangle + connectors stay visible. (Windows used to keep its native
    // saturated selection + a chrome flip here; unified with Mac/Linux -- JV
    // 2026-07-18. selectionChromeShouldFlip() is now unused.)
    if (selectionModel() && selectionModel()->isSelected(index))
    {
        painter->fillRect(rect, blend(
            appPal.color(QPalette::Active, QPalette::Base), accent, 0.28));
        // A stripe at the row's LEFT edge marks the selection (replaces the
        // focus outline, JV 2026-07-18). Drawn in the SAME colour as the
        // expand/collapse triangle (the lightness-clamped accent, see
        // triColour above) so the tree's palette stays limited (JV's
        // suggestion). Width scales a touch with the row height.
        const int barW = qMax(3, rect.height() / 8);   // ~3px (integer-safe)
        painter->fillRect(QRect(rect.left(), rect.top(), barW, rect.height()),
                          triColour);
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
