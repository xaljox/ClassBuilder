// qt/QtCompact.h -- tighten item-view row heights.
//
// The modern Windows style gives item-view rows (list widgets, combo-box
// drop-downs) a generous, touch-friendly height. For these ported dialogs a
// text-height row reads better and matches the old MFC look. One place for
// the row-size logic: list-widget items use compactItemSize() at creation,
// combo-box drop-downs use compactCombo() once the items are populated.
#pragma once

#include <QComboBox>
#include <QFontMetrics>
#include <QSize>
#include <QString>
#include <QWidget>

// Compact size hint for an item-view row: text-height tall, text-advance wide
// (so a horizontal scrollbar still shows for long entries). `view` supplies
// the font.
//
// A plain row is the font's line height minus 1px, matching CbTreeWidget's row
// height (fontMetrics().height() - 1) so a list and a tree side by side in one
// dialog (FindMethod, IteratorWizard) line up. The -1 still leaves the leading,
// so text is not cramped. `checkable` rows keep a little extra room: the
// checkbox indicator is taller than the text and would otherwise crowd. Use for
// QListWidgetItem::setSizeHint.
inline QSize compactItemSize(const QWidget* view, const QString& text,
                             bool checkable = false)
{
    const QFontMetrics fm = view->fontMetrics();
    return QSize(fm.horizontalAdvance(text) + 12,
                 fm.height() + (checkable ? 2 : -1));
}

// Compact the row height of a combo box's drop-down list. Call once after the
// combo's items are populated.
inline void compactCombo(QComboBox* combo)
{
    for (int i = 0; i < combo->count(); ++i)
        combo->setItemData(i, compactItemSize(combo, combo->itemText(i)),
                           Qt::SizeHintRole);
}
