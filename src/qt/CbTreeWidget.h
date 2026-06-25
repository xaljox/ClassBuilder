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
};
