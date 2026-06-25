// qt/FindMethodDialog.cpp -- the Qt Find Method dialog.
//
// Ported from the MFC CFindMethodDialog. Edits a FindMethod: a QTreeWidget of
// the relation target's members / owned relations / base classes feeds an
// argument-map list, alongside the method's name / access / properties / note.
//
// Like the MFC original this is mostly NON-live: the field widgets are read
// only on OK (applyFieldChanges, the MFC ::Update); the argument map is
// rebuilt on OK when it changed. Drives the model directly.
//
// The tree carries the same per-row icons as the main class tree (via
// Qt_ModelIcon) so it looks familiar -- the model GetIcon() index maps
// straight onto the shared icon set.

#include "FindMethodDialog.h"
#include "ui_FindMethodDialog.h"

#include "QtFindMethodDialog.h"      // Qt_ShowFindMethodDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "QtModelIcons.h"            // Qt_ModelIcon
#include "QtCompact.h"               // compactItemSize

#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// Store/read a model pointer on a tree or list item (Qt::UserRole). A null
// pointer marks a structural tree node that is not an argument-map target.
void setItemPtr(QTreeWidgetItem* item, void* ptr)
{
    item->setData(0, Qt::UserRole,
                  QVariant::fromValue(reinterpret_cast<qulonglong>(ptr)));
}
template <class T> T* treePtr(const QTreeWidgetItem* item)
{
    return reinterpret_cast<T*>(item->data(0, Qt::UserRole).toULongLong());
}
template <class T> T* listPtr(const QListWidgetItem* item)
{
    return reinterpret_cast<T*>(item->data(Qt::UserRole).toULongLong());
}

} // namespace

FindMethodDialog::FindMethodDialog(FindMethod* pFindMethod, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::FindMethodDialog)
    , _pFindMethod(pFindMethod)
{
    _ui->setupUi(this);

    setWindowTitle("Find Method of class " +
        toQ(_pFindMethod->GetBaseClass()->GetName()) + " at relation '" +
        toQ(_pFindMethod->GetFromRelation()->GetRelation()->GetNotation()) +
        "'");

    // Load the attribute widgets.
    _ui->editName->setText(toQ(_pFindMethod->GetName()));
    _ui->editNote->setPlainText(toQ(_pFindMethod->GetNote()));
    switch (_pFindMethod->GetAccess())
    {
    case PROTECTED: _ui->radioProtected->setChecked(true); break;
    case PRIVATE:   _ui->radioPrivate->setChecked(true);   break;
    default:        _ui->radioPublic->setChecked(true);    break;
    }
    _ui->checkNext->setChecked(_pFindMethod->GetNext());
    _ui->checkReverse->setChecked(_pFindMethod->GetReverse());
    _ui->checkDllExport->setChecked(_pFindMethod->GetDllExport());
    _ui->comboCallConv->setCurrentText(
        toQ(_pFindMethod->GetCallingConvention()));

    // Identify the relation key member, if it is among the arguments.
    Method::ArgumentIterator iArgument(_pFindMethod, &Argument::IsMemberArgument);
    while (++iArgument)
    {
        MemberArgument* pMemberArgument = (MemberArgument*)iArgument.Get();
        if (isMemberKey(pMemberArgument->GetMember()))
            _pMemberKey = pMemberArgument->GetMember();
    }

    // Build the navigation tree under a root node for the relation itself.
    Relation* pRelation = _pFindMethod->GetFromRelation()->GetRelation();
    QTreeWidgetItem* root = new QTreeWidgetItem(_ui->tree);
    root->setText(0, "i" + toQ(pRelation->GetToName()));
    root->setIcon(0, Qt_ModelIcon(_pFindMethod->GetFromRelation()->GetIcon()));
    setItemPtr(root, nullptr);
    fillTree(pRelation->GetToClass(), root);
    root->setExpanded(true);

    fillList();

    if (_pFindMethod->IsFixed())
    {
        _ui->buttonAdd->setEnabled(false);
        _ui->buttonDelete->setEnabled(false);
        _ui->checkNext->setEnabled(false);
        _ui->checkReverse->setEnabled(false);
    }

    connect(_ui->buttonAdd, &QPushButton::clicked,
            this, &FindMethodDialog::onAdd);
    connect(_ui->buttonDelete, &QPushButton::clicked,
            this, &FindMethodDialog::onDeleteMap);
    connect(_ui->tree, &QTreeWidget::itemSelectionChanged,
            this, &FindMethodDialog::onTreeSelChanged);
    connect(_ui->tree, &QTreeWidget::itemDoubleClicked,
            this, [this] { onAdd(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &FindMethodDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &FindMethodDialog::reject);
}

FindMethodDialog::~FindMethodDialog()
{
    delete _ui;
}

// Find a direct child of `parent` whose text matches (the MFC FindTreeItem).
QTreeWidgetItem* FindMethodDialog::findTreeItem(QTreeWidgetItem* parent,
                                                const QString& text)
{
    for (int i = 0; i < parent->childCount(); ++i)
        if (parent->child(i)->text(0) == text)
            return parent->child(i);
    return nullptr;
}

// Recursively populate the tree below `parent` with the navigable members,
// get-member methods, owned relations and base classes of `pClass`.
// `access` is the visibility threshold (3 = all, 2 = base class, 1 = owned
// relation target). Faithfully ported from the MFC FillTree.
void FindMethodDialog::fillTree(Class* pClass, QTreeWidgetItem* parent,
                                int access)
{
    // Top-level call: reset the cycle-guard flag on every class.
    if (!parent->parent())
    {
        DataModel::ClassIterator iClass(pClass->GetDataModel());
        while (++iClass)
            iClass->SetFlag();
    }

    // Members without a get-member method (own class / base class only).
    if (access > 1)
    {
        Class::MemberIterator iMember(pClass);
        while (++iMember)
        {
            if (iMember->GetAccess() < access && !iMember->GetGetMemberMethod())
            {
                const QString str = "->" + toQ(iMember->GetPrefixedName());
                if (!findTreeItem(parent, str))
                {
                    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
                    item->setText(0, str);
                    item->setIcon(0, Qt_ModelIcon(ICON_PUBLIC_MEMBER));
                    setItemPtr(item, iMember.Get());
                }
            }
        }
    }

    // Get-member methods.
    BaseClass::MethodIterator iMethod(pClass);
    while (++iMethod)
    {
        if (iMethod->GetAccess() < access && iMethod->IsGetMemberMethod())
        {
            const QString str = "->" + toQ(iMethod->GetName()) + "()";
            if (!findTreeItem(parent, str))
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(parent);
                item->setText(0, str);
                item->setIcon(0, Qt_ModelIcon(ICON_PUBLIC_MEMBER));
                setItemPtr(item, iMethod.Get());
            }
        }
    }

    // Owned / non-static relations -- recurse into owned ones.
    Class::ToRelationIterator iRelation(pClass);
    while (++iRelation)
    {
        if (!iRelation->GetStatic() &&
            _pFindMethod->GetFromRelation()->GetRelation() != iRelation.Get())
        {
            const QString str = "->Get" + toQ(iRelation->GetFromName()) + "()";
            if (!findTreeItem(parent, str))
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(parent);
                item->setText(0, str);
                item->setIcon(0,
                    Qt_ModelIcon(iRelation->GetToRelation()->GetIcon()));
                setItemPtr(item, iRelation->GetFromClass());
                if (iRelation->GetOwned() &&
                    !iRelation->GetFromClass()->GetFlag())
                {
                    iRelation->GetFromClass()->SetFlag(1);
                    fillTree(iRelation->GetFromClass(), item, 1);
                    iRelation->GetFromClass()->SetFlag();
                }
            }
        }
    }

    // Base classes.
    Class::InheritIterator inherit(pClass);
    while (++inherit)
    {
        Class* pBase = dynamic_cast<Class*>(inherit->GetBaseClass());
        if (pBase)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(parent);
            item->setText(0, toQ(pBase->GetName()));
            item->setIcon(0, Qt_ModelIcon(pBase->GetIcon()));
            setItemPtr(item, nullptr);
            fillTree(pBase, item, 2);
        }
    }
}

// Populate the argument-map list from the method's existing arguments.
void FindMethodDialog::fillList()
{
    Method::ArgumentIterator iArgument(_pFindMethod);
    while (++iArgument)
    {
        MemberArgument* pMemberArgument =
            dynamic_cast<MemberArgument*>(iArgument.Get());
        if (pMemberArgument)
        {
            Member* pMember = pMemberArgument->GetMember();

            if (iArgument->GetPath().IsEmpty())
            {
                if (pMember->GetGetMemberMethod())
                    iArgument->SetPath(CbString("->") +
                        pMember->GetGetMemberMethod()->GetName() + "()");
                else
                    iArgument->SetPath(CbString("->") +
                        pMember->GetPrefixedName());
            }

            QListWidgetItem* item = new QListWidgetItem(
                toQ(iArgument->GetPath()), _ui->listArguments);
            item->setData(Qt::UserRole, QVariant::fromValue(
                reinterpret_cast<qulonglong>(pMember)));
            item->setSizeHint(
                compactItemSize(_ui->listArguments, item->text()));
        }
        else if (!iArgument->GetPath().IsEmpty())
        {
            QListWidgetItem* item = new QListWidgetItem(
                toQ(iArgument->GetPath()), _ui->listArguments);
            item->setData(Qt::UserRole, QVariant::fromValue(
                reinterpret_cast<qulonglong>(iArgument->GetType())));
            item->setSizeHint(
                compactItemSize(_ui->listArguments, item->text()));
        }
    }
}

bool FindMethodDialog::isMemberKey(Member* pMember) const
{
    Relation* pRelation = _pFindMethod->GetFromRelation()->GetRelation();
    return pRelation->GetRelationMember() &&
           pRelation->GetRelationMember()->GetMember() == pMember;
}

// Tree selection changed: remember the node and compose its argument path by
// walking up to the root, taking the text of every non-structural ancestor.
void FindMethodDialog::onTreeSelChanged()
{
    QList<QTreeWidgetItem*> selected = _ui->tree->selectedItems();
    if (selected.isEmpty())
        return;

    QTreeWidgetItem* item = selected.first();
    _pGti = treePtr<Gti>(item);
    _argumentMap.clear();
    for (; item; item = item->parent())
    {
        if (treePtr<Gti>(item))
            _argumentMap = item->text(0) + _argumentMap;
    }

    const QString prefix =
        "i" + toQ(_pFindMethod->GetFromRelation()->GetRelation()->GetToName());
    if (_argumentMap.startsWith(prefix))
        _argumentMap = _argumentMap.mid(prefix.length());
}

// Add: append the selected tree node's path to the argument-map list.
void FindMethodDialog::onAdd()
{
    if (!_pGti)
        return;

    void* data = _pGti;

    GetMemberMethod* pGetMemberMethod = dynamic_cast<GetMemberMethod*>(_pGti);
    Member* pMember = dynamic_cast<Member*>(_pGti);
    if (pGetMemberMethod)
    {
        pMember = pGetMemberMethod->GetMember();
        data = pMember;
    }

    if (pMember && isMemberKey(pMember))
        _pMemberKey = pMember;

    // Skip if this exact path is already mapped.
    for (int i = 0; i < _ui->listArguments->count(); ++i)
        if (_ui->listArguments->item(i)->text() == _argumentMap)
            return;

    QListWidgetItem* item =
        new QListWidgetItem(_argumentMap, _ui->listArguments);
    item->setData(Qt::UserRole,
                  QVariant::fromValue(reinterpret_cast<qulonglong>(data)));
    item->setSizeHint(compactItemSize(_ui->listArguments, _argumentMap));
    _argumentMapChanged = true;
}

// Delete: drop the selected entries from the argument-map list.
void FindMethodDialog::onDeleteMap()
{
    const QList<QListWidgetItem*> selected =
        _ui->listArguments->selectedItems();
    for (QListWidgetItem* item : selected)
    {
        if (_pMemberKey && _pMemberKey == listPtr<Member>(item))
            _pMemberKey = nullptr;
        delete _ui->listArguments->takeItem(_ui->listArguments->row(item));
    }
    if (!selected.isEmpty())
        _argumentMapChanged = true;
}

// Apply the attribute-widget values to the model (the MFC ::Update). Returns
// true if anything changed -- the caller then runs FindMethod::Update.
bool FindMethodDialog::applyFieldChanges()
{
    const QString name     = _ui->editName->text();
    const QString note     = _ui->editNote->toPlainText();
    const QString callConv = _ui->comboCallConv->currentText();
    const int     access   = _ui->radioProtected->isChecked() ? PROTECTED
                           : _ui->radioPrivate->isChecked()   ? PRIVATE
                                                              : PUBLIC;
    const bool dllExport = _ui->checkDllExport->isChecked();
    const bool next      = _ui->checkNext->isChecked();
    const bool reverse   = _ui->checkReverse->isChecked();

    bool update = false;

    if (toQ(_pFindMethod->GetName()) != name ||
        toQ(_pFindMethod->GetNote()) != note ||
        _pFindMethod->GetAccess() != access ||
        toQ(_pFindMethod->GetCallingConvention()) != callConv ||
        _pFindMethod->GetDllExport() != dllExport ||
        _argumentMapChanged)
    {
        _pFindMethod->SaveState();
        _pFindMethod->SetAccess(AccessType(access));
        _pFindMethod->SetCallingConvention(toCb(callConv));
        _pFindMethod->SetDllExport(dllExport);
        _pFindMethod->SetName(toCb(name));
        _pFindMethod->SetNote(toCb(note));
        update = true;
    }

    if (next != _pFindMethod->GetNext())
    {
        _pFindMethod->SaveState();
        const CbString typeName = _pFindMethod->GetType()->GetName();
        const CbString argName =
            (reverse ? CbString("startBefore") : CbString("startAfter")) +
            typeName;

        if (next)
        {
            Argument* pArgument =
                new Argument(_pFindMethod, _pFindMethod->GetType());
            pArgument->SetPointer(1);
            pArgument->SetName(argName);
            pArgument->SetDefault("0");
            pArgument->SetNote(
                "Default argument to give the start position of the find.");
            if (_pFindMethod->GetAdded())
                pArgument->Add();
        }
        else
        {
            Argument* pArgument = _pFindMethod->FindArgument(argName);
            if (pArgument)
                pArgument->Delete();
        }

        _pFindMethod->SetNext(next);
        update = true;
    }

    if (reverse != _pFindMethod->GetReverse())
    {
        _pFindMethod->SaveState();
        const CbString typeName = _pFindMethod->GetType()->GetName();
        const CbString newName =
            (reverse ? CbString("startBefore") : CbString("startAfter")) +
            typeName;
        const CbString oldName =
            (reverse ? CbString("startAfter") : CbString("startBefore")) +
            typeName;

        Argument* pArgument = _pFindMethod->FindArgument(oldName);
        if (pArgument)
        {
            pArgument->SaveState();
            pArgument->SetName(newName);
        }

        _pFindMethod->SetReverse(reverse);
        update = true;
    }

    if (update)
        _pFindMethod->InitCode();

    return update;
}

// OK -- validate, rebuild the argument map if it changed, apply the fields.
void FindMethodDialog::accept()
{
    if (!_ui->listArguments->count())
    {
        QMessageBox::warning(this, "Find Method",
                             "Must select argument(s)");
        return;
    }

    // Argument-map rebuild (the MFC OnOK): delete every argument, then
    // recreate one per list row.
    if (_argumentMapChanged)
    {
        _pFindMethod->SaveState();
        _pFindMethod->SetNext(0);
        Method::ArgumentIterator argument(_pFindMethod);
        while (++argument)
            argument->Delete();

        for (int i = 0; i < _ui->listArguments->count(); ++i)
        {
            QListWidgetItem* row = _ui->listArguments->item(i);
            Gti* pGti = listPtr<Gti>(row);

            Member* pMember = dynamic_cast<Member*>(pGti);
            Class*  pClass  = dynamic_cast<Class*>(pGti);

            Argument* pArgument = nullptr;
            if (pMember)
            {
                pArgument = new MemberArgument(_pFindMethod, pMember);
            }
            else if (pClass)
            {
                pArgument = new Argument(_pFindMethod, pClass);
                pArgument->SetPointer(1);
                pArgument->Variable::SetName(
                    CbString("p") + pClass->GetName());
            }
            if (!pArgument)
                continue;   // data was neither -- skip (MFC would crash)

            pArgument->SetPath(toCb(row->text()));
            if (_pFindMethod->GetAdded())
                pArgument->Add();
        }
    }

    _fieldsChanged = applyFieldChanges();

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowFindMethodDialog(FindMethod* pFindMethod, void* ownerHwnd,
                             bool& fieldsChanged)
{
    Qt_EnsureApplication();

    FindMethodDialog dlg(pFindMethod);
    const bool accepted =
        Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
    fieldsChanged = accepted && dlg.fieldsChanged();
    return accepted;
}
