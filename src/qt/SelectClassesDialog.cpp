// qt/SelectClassesDialog.cpp -- the Qt Select Classes dialog.
//
// Ported from the MFC SelectClassesDialog. A checkbox tree of the model's
// classes (data model + meta groups + extern classes); on OK every class
// shape on the diagram is added (placed at a free spot) or removed to match
// the ticked set. Drives the model directly.

#include "SelectClassesDialog.h"
#include "ui_SelectClassesDialog.h"

#include "QtSelectClassesDialog.h"   // Qt_ShowSelectClassesDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ
#include "QtModelIcons.h"            // Qt_ModelIcon

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDialogButtonBox>

#include <algorithm>
#include <vector>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {
Gti* itemGti(const QTreeWidgetItem* item)
{
    return reinterpret_cast<Gti*>(item->data(0, Qt::UserRole).toULongLong());
}
}

SelectClassesDialog::SelectClassesDialog(ClassDiagram* pClassDiagram,
                                         QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::SelectClassesDialog)
    , _pClassDiagram(pClassDiagram)
{
    _ui->setupUi(this);

    setWindowTitle("Select classes of class diagram '" +
                   toQ(_pClassDiagram->GetName()) + "'");

    DataModel* pDataModel =
        _pClassDiagram->GetDataModelDoc()->GetDataModel();
    ExternClasses* pExternClasses =
        _pClassDiagram->GetDataModelDoc()->GetExternClasses();

    QTreeWidgetItem* root = fillTreeItems(pDataModel, nullptr);
    root->setExpanded(true);

    DataModel::MetaGroupIterator iMetaGroup(pDataModel);
    while (++iMetaGroup)
        fillTreeItems(iMetaGroup, nullptr);

    fillTreeItems(pExternClasses, nullptr);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SelectClassesDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &SelectClassesDialog::reject);
}

SelectClassesDialog::~SelectClassesDialog()
{
    delete _ui;
}

// Build a tree node for `pGti`. A BaseClass node is checkable (ticked when it
// already has a shape on the diagram); a group node is not, and recurses into
// its base-class / class-group children.
QTreeWidgetItem* SelectClassesDialog::fillTreeItems(Gti* pGti,
                                                    QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = parent ? new QTreeWidgetItem(parent)
                                   : new QTreeWidgetItem(_ui->tree);
    item->setText(0, toQ(pGti->GetItemText()));
    item->setIcon(0, Qt_ModelIcon(pGti->GetIcon()));
    item->setData(0, Qt::UserRole,
                  QVariant::fromValue(reinterpret_cast<qulonglong>(pGti)));

    BaseClass* pBaseClass = dynamic_cast<BaseClass*>(pGti);
    if (pBaseClass)
    {
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0,
            pBaseClass->FindClassShape(_pClassDiagram) ? Qt::Checked
                                                       : Qt::Unchecked);
    }
    else
    {
        // Main-tree display order (Gti::CompareTreeOrder, stable ties), not
        // the alphabetical sort the dialog used to apply -- the rows must
        // read like the tree (JV 2026-07-13).
        std::vector<Gti*> children;
        Gti::ChildIterator iChild(pGti);
        while (++iChild)
        {
            if (iChild->IsBaseClass() || iChild->IsClassGroup())
                children.push_back(iChild.Get());
        }
        std::stable_sort(children.begin(), children.end(),
                         [](Gti* a, Gti* b)
                         { return Gti::CompareTreeOrder(a, b) < 0; });
        for (Gti* pChild : children)
            fillTreeItems(pChild, item);
    }

    return item;
}

// OK -- add / remove class shapes to match the ticked set (the MFC OnOK).
void SelectClassesDialog::checkItem(QTreeWidgetItem* item)
{
    BaseClass* pBaseClass = dynamic_cast<BaseClass*>(itemGti(item));
    if (pBaseClass)
    {
        const bool checked = item->checkState(0) == Qt::Checked;
        ClassShape* pClassShape =
            pBaseClass->FindClassShape(_pClassDiagram);

        if (pClassShape)
        {
            if (!checked)
            {
                while (pClassShape)
                {
                    pClassShape->Delete();
                    pClassShape = pBaseClass->FindClassShape(_pClassDiagram);
                }
            }
        }
        else if (checked)
        {
            // The diagram owns the free-cell placement scan.
            CbPoint point = _pClassDiagram->FindFreeShapePlacement();
            (void)new ClassShape(_pClassDiagram, pBaseClass, point);
        }
    }
    else
    {
        for (int i = 0; i < item->childCount(); ++i)
            checkItem(item->child(i));
    }
}

void SelectClassesDialog::accept()
{
    for (int i = 0; i < _ui->tree->topLevelItemCount(); ++i)
        checkItem(_ui->tree->topLevelItem(i));

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowSelectClassesDialog(ClassDiagram* pClassDiagram, void* ownerHwnd)
{
    Qt_EnsureApplication();

    SelectClassesDialog dlg(pClassDiagram);
    return Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
}
