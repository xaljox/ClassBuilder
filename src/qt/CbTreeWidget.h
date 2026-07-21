// qt/CbTreeWidget.h -- QTreeWidget with corrected root-level branch drawing.
//
// The app-wide QTreeView::branch stylesheet (QtApp.cpp) draws the connector
// lines, but a stylesheet cannot tell a root-level leaf from a nested
// last-child leaf -- both get branch_end (an ell). On a root node the ell's
// upward stub dangles toward a non-existent parent. This subclass overpaints
// that one case with branch_top.svg (a plain horizontal stub).
//
// Use for every Qt model tree so they stay consistent (and with the eventual
// Qt main class tree).
#pragma once

#include <QPersistentModelIndex>
#include <QTreeWidget>

class CbTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit CbTreeWidget(QWidget* parent = nullptr);

    // Re-derive the accent-based selection/hover tint from the CURRENT theme
    // accent and repaint. Called by the app-wide accent watcher (QtApp.cpp)
    // when the desktop accent changes while CB is open, so the tree never sits
    // in a stale colour. Cheap (one setStyleSheet + repaint); accent changes
    // are rare.
    void reapplyThemeAccent();

protected:
    void drawBranches(QPainter* painter, const QRect& rect,
                      const QModelIndex& index) const override;
    // macOS: repaint the full viewport when the hovered row or the selection
    // changes -- the transition otherwise leaves thin stale stripes (see .cpp).
    // No-ops beyond the base behavior elsewhere.
    bool viewportEvent(QEvent* event) override;
    void selectionChanged(const QItemSelection& selected,
                          const QItemSelection& deselected) override;

private:
    // Build + apply this tree's stylesheet (row height, font, and the
    // accent-derived selection/hover tint). Called from the constructor and
    // again from reapplyThemeAccent() on a live accent change -- one place that
    // owns the sheet, so the two can never drift.
    void applyThemeStyleSheet();

    // macOS: the row the cursor is on -- only a CHANGE of row triggers the
    // full-viewport repaint in viewportEvent, not every mouse move.
    QPersistentModelIndex _hoverRow;
};
