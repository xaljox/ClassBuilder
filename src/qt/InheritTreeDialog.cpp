// qt/InheritTreeDialog.cpp -- the Qt inheritance-tree dialog.
//
// Ported from the MFC InheritByTree + InheritFromTree. Both were tiny modal
// tree windows; merged here into one class with a mode. The MFC font /
// image-list / sort-callback plumbing is gone -- CbTreeWidget supplies the
// font and branch glyphs, QtModelIcons the icons, and Qt sorts the items.

#include "InheritTreeDialog.h"
#include "ui_InheritTreeDialog.h"

#include "QtInheritTreeDialog.h"     // Qt_ShowInherits*Dialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ
#include "QtModelIcons.h"            // Qt_ModelIcon

#include <QTreeWidgetItem>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// Create a tree item under `parent`, or a top-level item when parent is null.
QTreeWidgetItem* makeItem(QTreeWidget* tree, QTreeWidgetItem* parent)
{
    return parent ? new QTreeWidgetItem(parent)
                  : new QTreeWidgetItem(tree);
}

} // namespace

InheritTreeDialog::InheritTreeDialog(BaseClass* pClass, Mode mode,
                                     QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::InheritTreeDialog)
{
    _ui->setupUi(this);

    if (mode == InheritedBy)
    {
        setWindowTitle(toQ(CbString("Classes derived from class '") +
                           pClass->GetName() + "'"));
        fillInheritedBy(pClass, nullptr);
    }
    else
    {
        setWindowTitle(toQ(CbString("Base classes of class '") +
                           pClass->GetName() + "'"));
        fillInheritsFrom(pClass, nullptr);
    }

    _ui->tree->sortItems(0, Qt::AscendingOrder);
    _ui->tree->expandAll();
}

InheritTreeDialog::~InheritTreeDialog()
{
    delete _ui;
}

// The derivation tree downward -- the MFC InheritByTree::FillTree.
void InheritTreeDialog::fillInheritedBy(BaseClass* pBaseClass,
                                        QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = makeItem(_ui->tree, parent);
    item->setText(0, toQ(pBaseClass->GetName()));
    item->setIcon(0, Qt_ModelIcon(pBaseClass->GetIcon()));

    BaseClass::InheritIterator iInherit(pBaseClass);
    while (++iInherit)
        fillInheritedBy(iInherit->GetExternClass(), item);
}

// The base-class chain upward -- the MFC InheritFromTree::FillTree. A base
// that is itself an extern class recurses; a plain base is a leaf.
void InheritTreeDialog::fillInheritsFrom(BaseClass* pClass,
                                         QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = makeItem(_ui->tree, parent);
    item->setText(0, toQ(pClass->GetItemText()));
    item->setIcon(0, Qt_ModelIcon(pClass->GetIcon()));

    ExternClass* pExternClass = dynamic_cast<ExternClass*>(pClass);
    if (pExternClass)
    {
        Class::InheritIterator iInherit(pExternClass);
        while (++iInherit)
            fillInheritsFrom(iInherit->GetBaseClass(), item);
    }
}

// ---------------------------------------------------------------------------
// MFC entry points
// ---------------------------------------------------------------------------
void Qt_ShowInheritsFromDialog(ExternClass* pExternClass, void* ownerHwnd)
{
    Qt_EnsureApplication();

    InheritTreeDialog dlg(pExternClass, InheritTreeDialog::InheritsFrom);
    Qt_ExecModal(dlg, ownerHwnd);
}

void Qt_ShowInheritedByDialog(BaseClass* pBaseClass, void* ownerHwnd)
{
    Qt_EnsureApplication();

    InheritTreeDialog dlg(pBaseClass, InheritTreeDialog::InheritedBy);
    Qt_ExecModal(dlg, ownerHwnd);
}
