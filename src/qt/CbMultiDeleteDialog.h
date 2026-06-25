// qt/CbMultiDeleteDialog.h -- bulk ("delete multiple") dialog launched from the
// main tree's context menu.
//
// Shows the selected node's subtree (the same scoping as "New Sub Window").
// Deletable items get a checkbox; structurally-needed-but-non-deletable items
// are shown WITHOUT one (visible for context only). Type-filter checkboxes
// narrow what's visible (like the iterator Filter dialog). Checking a parent
// implies its contents. OK deletes the checked items as ONE undo step --
// cascade-correct and guarded against double-free via the undo stack.
#pragma once

#include <QDialog>
#include <QVector>

class Gti;
class DataModelDoc;
class CbTreeWidget;

QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
class QCheckBox;
class QLabel;
QT_END_NAMESPACE

class CbMultiDeleteDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CbMultiDeleteDialog(Gti* pRoot, QWidget* parent = nullptr);

private slots:
    void onItemChanged(QTreeWidgetItem* item, int column);
    void selectAllVisible();
    void unselectAllVisible();
    void accept() override;

private:
    void rebuildTree();                                    // (re)populate honouring the filters
    QTreeWidgetItem* buildNode(Gti* pGti, bool isRoot);    // detached item, or nullptr if pruned
    void collectChecked(QTreeWidgetItem* item, QVector<QTreeWidgetItem*>& out) const;
    void setSubtreeChecked(QTreeWidgetItem* item, bool checked);
    int  countChecked() const;
    void updateCount();

    Gti*          _pRoot;
    DataModelDoc* _pDoc;
    CbTreeWidget* _tree       = nullptr;
    QLabel*       _countLabel = nullptr;
    QVector<QCheckBox*> _filters;          // one per category (index == Category)
    bool          _propagating = false;    // suppress itemChanged during programmatic checks
};
