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

#include <QTreeWidget>

class CbTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit CbTreeWidget(QWidget* parent = nullptr);

protected:
    void drawBranches(QPainter* painter, const QRect& rect,
                      const QModelIndex& index) const override;
    void changeEvent(QEvent* event) override;

private:
    // Whether a selected row's real, style-painted background collides with
    // the accent colour the branch chrome is drawn in (see drawBranches).
    // Determined by actually rendering a probe selected cell through this
    // widget's style/palette and sampling the pixel -- see .cpp -- instead of
    // guessing from a palette colour or the style/platform name, neither of
    // which reliably predicts how a given theme renders selection. Lazily
    // computed and cached; invalidated by changeEvent on palette/style change.
    bool selectionChromeShouldFlip() const;
    mutable bool _chromeFlipCached = false;
    mutable bool _chromeFlip = false;
};
