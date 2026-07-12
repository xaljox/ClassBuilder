// qt/QtToolBarIcons.cpp -- see header. The toolbar glyphs come from two places,
// in priority order (same modernise-one-at-a-time scheme as QtModelIcons):
//   1. a redrawn vector  qt/icons/tb_<name>.svg   (bundled as :/icons/, needs
//      Qt's Svg module -- CB_HAVE_SVG), else
//   2. the matching tile sliced out of the legacy strip res/Toolbar.bmp.
// <name> is the per-glyph base name below (also the source PNGs the slicer
// writes to qt/icons/toolbar_src/ for tracing). Add a tb_<name>.svg, rebuild,
// and only that button goes vector.

#include "QtToolBarIcons.h"

#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QPolygonF>
#include <QString>
#include <QVector>

namespace {

// Base names in TOOLBAR strip / ToolGlyph order. Keep in sync with the enum
// in QtToolBarIcons.h and the IDR_MAINFRAME toolbar in the (retired) .rc.
const char* const kGlyphNames[] = {
    "file_new", "file_open", "file_save",
    "edit_undo", "edit_redo",
    "read_source", "save_source",
    "edit_cut", "edit_copy", "edit_paste", "edit_delete",
    "file_print", "app_about",
    "zoom_in", "zoom_out", "zoom_full",
    "add_classdiagram", "add_sequencediagram", "add_group",
    "add_relation_diagramonly", "add_dependency",
    "add_note", "add_lifeline", "add_message",
    "add_class", "add_inherit", "add_relation",
    "add_member", "add_function", "add_constructor", "add_argument",
    "add_virtuals", "add_isclass",
    "add_type",
};
const int kGlyphCount = int(sizeof(kGlyphNames) / sizeof(kGlyphNames[0]));

// The MFC toolbar bitmap's mask colour (ScaleToolBar used Cb_RGB(192,192,192) as
// the transparent background when adding the strip to its CImageList).
const QRgb kMask = qRgb(192, 192, 192);

#ifdef __APPLE__
// A clearly-"disabled" rendering of a toolbar glyph: desaturate to a light grey
// and fade the alpha. Qt asks the active QStyle to grey disabled icons, and
// Windows' style does so clearly -- but macOS' style dims them only faintly, so
// the (deliberately green) undo/redo glyphs still read as enabled. On macOS we
// supply an explicit QIcon::Disabled pixmap so "disabled" is unmistakable.
// macOS-only: Windows keeps its native (already-clear) disabled greying.
QImage makeDisabledGlyph(const QImage& src)
{
    QImage out = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < out.height(); ++y)
        for (int x = 0; x < out.width(); ++x)
        {
            const QRgb p = out.pixel(x, y);
            const int a = qAlpha(p);
            if (a == 0)
                continue;
            const int g = qRound(0.30 * qRed(p) + 0.59 * qGreen(p) + 0.11 * qBlue(p));
            const int light = (g + 165) / 2;      // push toward light grey
            out.setPixel(x, y, qRgba(light, light, light, a * 2 / 5));  // ~40% alpha
        }
    return out;
}
#endif  // __APPLE__

const QVector<QIcon>& glyphs()
{
    static const QVector<QIcon> v = [] {
        QVector<QIcon> out;

        QImage strip(":/Toolbar.bmp");
        const int h = strip.isNull() ? 0 : strip.height();        // tile size (16)
        const int n = (h > 0) ? strip.width() / h : 0;            // square tiles
        if (!strip.isNull())
            strip = strip.convertToFormat(QImage::Format_ARGB32);

        const int count = (n > kGlyphCount) ? n : kGlyphCount;
        out.reserve(count);
        for (int i = 0; i < count; ++i)
        {
            // 1. SVG override (qt/icons/tb_<name>.svg -> :/icons/tb_<name>.svg).
#ifdef CB_HAVE_SVG
            if (i < kGlyphCount)
            {
                const QString svg = QString(":/icons/tb_") + kGlyphNames[i] + ".svg";
                if (QFile::exists(svg))
                {
                    QIcon ic(svg);
#ifdef __APPLE__
                    // Attach explicit greyed Disabled pixmaps at the sizes the
                    // toolbars actually render (incl. 2x retina), so disabled
                    // reads clearly on macOS (its style dims SVG icons faintly).
                    for (int s : {16, 20, 24, 32, 40, 48})
                    {
                        const QImage norm = ic.pixmap(QSize(s, s)).toImage();
                        if (!norm.isNull())
                            ic.addPixmap(QPixmap::fromImage(makeDisabledGlyph(norm)),
                                         QIcon::Disabled);
                    }
#endif
                    out.append(ic);
                    continue;
                }
            }
#endif
            // 2. The legacy strip tile.
            if (i < n)
            {
                QImage tile = strip.copy(i * h, 0, h, h);
                for (int y = 0; y < tile.height(); ++y)
                    for (int x = 0; x < tile.width(); ++x)
                        if ((tile.pixel(x, y) & 0x00FFFFFF) == (kMask & 0x00FFFFFF))
                            tile.setPixel(x, y, 0u);   // mask -> transparent

                // Undo/Redo: recolour the flat black glyph to green (keep alpha).
                // A black-on-grey glyph greys to nearly itself when disabled, so
                // enabled-vs-disabled is unreadable; a green glyph greys out
                // clearly. (JV)
                if (i == TG_EDIT_UNDO || i == TG_EDIT_REDO)
                    for (int y = 0; y < tile.height(); ++y)
                        for (int x = 0; x < tile.width(); ++x)
                        {
                            const int a = qAlpha(tile.pixel(x, y));
                            if (a > 0)
                                tile.setPixel(x, y, qRgba(0, 150, 0, a));
                        }

                QIcon ic(QPixmap::fromImage(tile));
#ifdef __APPLE__
                // Explicit greyed Disabled pixmap (macOS dims too faintly).
                ic.addPixmap(QPixmap::fromImage(makeDisabledGlyph(tile)),
                             QIcon::Disabled);
#endif
                out.append(ic);
            }
            else
            {
                out.append(QIcon());
            }
        }
        return out;
    }();
    return v;
}

} // namespace

QIcon Qt_ToolBarIcon(int glyph)
{
    const QVector<QIcon>& g = glyphs();
    if (glyph < 0 || glyph >= g.size())
        return QIcon();
    return g[glyph];
}

QIcon Qt_AddStarBadge(const QIcon& base)
{
    if (base.isNull())
        return base;

    // The 4-point starburst, in the tb_add_*.svg 16-unit coordinate space
    // (same path/colours as those SVGs), so the badge is pixel-identical to
    // the stars on the Add-Class / Add-Member / ... buttons beside it.
    static const double kStar[8][2] = {
        {3.1, 0.4}, {3.9, 2.3}, {5.8, 3.1}, {3.9, 3.9},
        {3.1, 5.8}, {2.3, 3.9}, {0.4, 3.1}, {2.3, 2.3},
    };

    QIcon out;
    // Render at the sizes the view toolbars actually ask for (incl. 2x retina),
    // matching the tb glyphs' explicit-pixmap set.
    for (int s : {16, 20, 24, 32, 40, 48})
    {
        const double u = s / 16.0;                 // svg unit -> device px
        const int inner = qRound(0.72 * s);        // shrunk base glyph
        const int off   = qRound(4.5 * u);         // translate(4.5,4.5)

        QImage img(s, s, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);

        const QPixmap basePx = base.pixmap(QSize(inner, inner));
        if (!basePx.isNull())
            p.drawPixmap(off, off, basePx);

        QPolygonF star;
        for (const auto& pt : kStar)
            star << QPointF(pt[0] * u, pt[1] * u);
        p.setPen(QPen(QColor(0x8A, 0x6D, 0x00), 0.8 * u));
        p.setBrush(QColor(0xF5, 0xC4, 0x00));
        p.drawPolygon(star);
        p.end();

        out.addPixmap(QPixmap::fromImage(img));
#ifdef __APPLE__
        out.addPixmap(QPixmap::fromImage(makeDisabledGlyph(img)), QIcon::Disabled);
#endif
    }
    return out;
}
