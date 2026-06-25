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
#include <QPixmap>
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
                    out.append(QIcon(svg));
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

                out.append(QIcon(QPixmap::fromImage(tile)));
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
