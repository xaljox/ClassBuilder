// qt/SelectMembersAndMethods.cpp -- the Qt "select members/methods" dialog.
//
// Ported from the MFC SelectMembersAndMethods. A checkbox tree: each class
// shape of a class diagram, with its (filtered) members and methods beneath;
// a checked member/method has a shape on the class shape. OK / Apply create
// or delete those shapes; Cancel rolls back to the undo point taken at
// construction. Drives the model directly.

#include "SelectMembersAndMethods.h"
#include "ui_SelectMembersAndMethods.h"

#include "QtSelectMembersAndMethods.h"   // Qt_ShowSelectMembersAndMethodsDialog
#include "QtApp.h"                        // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"                  // toQ
#include "QtModelIcons.h"                 // Qt_ModelIcon

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTreeWidgetItem>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

bool SelectMembersAndMethods::_privateMembers      = true;
bool SelectMembersAndMethods::_protectedMembers    = true;
bool SelectMembersAndMethods::_publicMembers       = true;
bool SelectMembersAndMethods::_privateMethods      = true;
bool SelectMembersAndMethods::_protectedMethods    = true;
bool SelectMembersAndMethods::_publicMethods       = true;
bool SelectMembersAndMethods::_classBuilderMethods = false;
bool SelectMembersAndMethods::_relationMethods     = false;
bool SelectMembersAndMethods::_getSetMethods       = false;

namespace {
// Tree items carry a model pointer; children carry the Gti (a Method or
// Member), roots carry the ClassShape.
void setPtr(QTreeWidgetItem* item, void* p)
{
    item->setData(0, Qt::UserRole, QVariant::fromValue(p));
}
void* getPtr(QTreeWidgetItem* item)
{
    return item->data(0, Qt::UserRole).value<void*>();
}
}

SelectMembersAndMethods::SelectMembersAndMethods(
        ClassDiagram* pClassDiagram,
        SelectMembersApplyCallback onApply,
        void* applyContext,
        QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::SelectMembersAndMethods)
    , _pClassDiagram(pClassDiagram)
    , _onApply(onApply)
    , _applyContext(applyContext)
{
    _ui->setupUi(this);

    setWindowTitle(toQ(CbString(
        "Select members and/or methods of class diagram '") +
        _pClassDiagram->GetName() + "'"));

    // Undo point: Cancel rolls the model back to here.
    _pUndoBase = _pClassDiagram->GetDataModelDoc()->MarkLastUndo();

    _ui->checkPrivateMembers->setChecked(_privateMembers);
    _ui->checkProtectedMembers->setChecked(_protectedMembers);
    _ui->checkPublicMembers->setChecked(_publicMembers);
    _ui->checkPrivateMethods->setChecked(_privateMethods);
    _ui->checkProtectedMethods->setChecked(_protectedMethods);
    _ui->checkPublicMethods->setChecked(_publicMethods);
    _ui->checkClassBuilderMethods->setChecked(_classBuilderMethods);
    _ui->checkRelationMethods->setChecked(_relationMethods);
    _ui->checkGetSetMethods->setChecked(_getSetMethods);

    QCheckBox* filters[] = {
        _ui->checkPrivateMembers, _ui->checkProtectedMembers,
        _ui->checkPublicMembers, _ui->checkPrivateMethods,
        _ui->checkProtectedMethods, _ui->checkPublicMethods,
        _ui->checkClassBuilderMethods, _ui->checkRelationMethods,
        _ui->checkGetSetMethods };
    for (QCheckBox* c : filters)
        connect(c, &QCheckBox::clicked,
                this, [this] { onFilterChanged(); });

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SelectMembersAndMethods::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &SelectMembersAndMethods::reject);
    connect(_ui->buttonBox->button(QDialogButtonBox::Apply),
            &QPushButton::clicked,
            this, [this]
            {
                applyChecks();
                // Close the edit like every other diagram edit BEFORE refreshing:
                // MarkLastUndo recomputes the class box (RecalculateAllDiagrams ->
                // RecalculateRect -> reroute) and folds the shape add/removes + that
                // reroute into one undo step. Without it the box stays un-laid-out and
                // the undo stack is left with inconsistent segments. Apply keeps the
                // dialog open; refresh the host's views so the change shows immediately
                // (the MFC OnApply did UpdateAllViews here).
                _pClassDiagram->GetDataModelDoc()->MarkLastUndo();
                if (_onApply)
                    _onApply(_applyContext);
            });

    fillTree();
}

SelectMembersAndMethods::~SelectMembersAndMethods()
{
    delete _ui;
}

void SelectMembersAndMethods::fillTree()
{
    _ui->tree->clear();

    ClassDiagram::ClassDiagramShapeIterator
        iShape(_pClassDiagram, &ClassDiagramShape::IsClassShape);
    while (++iShape)
    {
        ClassShape* pClassShape = (ClassShape*)iShape.Get();
        BaseClass*  pBaseClass  = pClassShape->GetBaseClass();

        QTreeWidgetItem* root = new QTreeWidgetItem(_ui->tree);
        root->setText(0, toQ(pBaseClass->GetItemText()));
        root->setIcon(0, Qt_ModelIcon(pBaseClass->GetIcon()));
        setPtr(root, static_cast<void*>(pClassShape));

        BaseClass::MethodIterator iMethod(pBaseClass);
        while (++iMethod)
        {
            const int access = iMethod->GetAccess();
            if (!((_privateMethods && access == PRIVATE) ||
                  (_protectedMethods && access == PROTECTED) ||
                  (_publicMethods && access == PUBLIC)))
                continue;
            if ((iMethod->IsFixedMethod() && !_classBuilderMethods) ||
                (iMethod->IsMacroMethod() && !_relationMethods) ||
                (iMethod->IsGetMemberMethod() && !_getSetMethods) ||
                (iMethod->IsSetMemberMethod() && !_getSetMethods))
                continue;

            QTreeWidgetItem* item = new QTreeWidgetItem(root);
            item->setText(0, toQ(iMethod->GetItemText()));
            item->setIcon(0, Qt_ModelIcon(iMethod->GetIcon()));
            setPtr(item, static_cast<void*>(
                static_cast<Gti*>(iMethod.Get())));
            item->setCheckState(0,
                iMethod->FindMethodShape(pClassShape) ? Qt::Checked
                                                      : Qt::Unchecked);
        }

        BaseClass::MemberIterator iMember(pBaseClass);
        while (++iMember)
        {
            const int access = iMember->GetAccess();
            if (!((_privateMembers && access == PRIVATE) ||
                  (_protectedMembers && access == PROTECTED) ||
                  (_publicMembers && access == PUBLIC)))
                continue;

            QTreeWidgetItem* item = new QTreeWidgetItem(root);
            item->setText(0, toQ(iMember->GetItemText()));
            item->setIcon(0, Qt_ModelIcon(iMember->GetIcon()));
            setPtr(item, static_cast<void*>(
                static_cast<Gti*>(iMember.Get())));
            item->setCheckState(0,
                iMember->FindMemberShape(pClassShape) ? Qt::Checked
                                                      : Qt::Unchecked);
        }
    }

    _ui->tree->sortItems(0, Qt::AscendingOrder);
    _ui->tree->expandAll();
}

// A filter checkbox toggled -- store the flags and rebuild the tree.
void SelectMembersAndMethods::onFilterChanged()
{
    _privateMembers      = _ui->checkPrivateMembers->isChecked();
    _protectedMembers    = _ui->checkProtectedMembers->isChecked();
    _publicMembers       = _ui->checkPublicMembers->isChecked();
    _privateMethods      = _ui->checkPrivateMethods->isChecked();
    _protectedMethods    = _ui->checkProtectedMethods->isChecked();
    _publicMethods       = _ui->checkPublicMethods->isChecked();
    _classBuilderMethods = _ui->checkClassBuilderMethods->isChecked();
    _relationMethods     = _ui->checkRelationMethods->isChecked();
    _getSetMethods       = _ui->checkGetSetMethods->isChecked();

    fillTree();
}

// Sync one child item's check state to the model: a checked item must have a
// member/method shape on the class shape, an unchecked item must not.
void SelectMembersAndMethods::checkItem(QTreeWidgetItem* item,
                                        ClassShape* pClassShape)
{
    Gti* pGti = static_cast<Gti*>(getPtr(item));
    const bool checked = item->checkState(0) == Qt::Checked;

    if (Method* pMethod = dynamic_cast<Method*>(pGti))
    {
        MethodShape* pShape = pMethod->FindMethodShape(pClassShape);
        if (pShape && !checked)
            pShape->Delete();
        else if (!pShape && checked)
            (void)new MethodShape(pClassShape, pMethod);
    }
    else if (Member* pMember = dynamic_cast<Member*>(pGti))
    {
        MemberShape* pShape = pMember->FindMemberShape(pClassShape);
        if (pShape && !checked)
            pShape->Delete();
        else if (!pShape && checked)
            (void)new MemberShape(pClassShape, pMember);
    }
}

void SelectMembersAndMethods::applyChecks()
{
    for (int i = 0; i < _ui->tree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* root = _ui->tree->topLevelItem(i);
        ClassShape* pClassShape =
            static_cast<ClassShape*>(getPtr(root));
        for (int c = 0; c < root->childCount(); ++c)
            checkItem(root->child(c), pClassShape);
    }
}

void SelectMembersAndMethods::accept()
{
    applyChecks();
    // Close the edit like every other diagram edit: MarkLastUndo recomputes the class
    // box (RecalculateAllDiagrams -> RecalculateRect -> reroute) and folds the shape
    // add/removes + that reroute into ONE undo step. Without it the box stays
    // un-laid-out (empty) and undo restores inconsistent connection segments -> crash.
    // Then refresh the host's views so the change shows.
    _pClassDiagram->GetDataModelDoc()->MarkLastUndo();
    if (_onApply)
        _onApply(_applyContext);
    QDialog::accept();
}

void SelectMembersAndMethods::reject()
{
    _pClassDiagram->GetDataModelDoc()->RollBack(_pUndoBase);
    QDialog::reject();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowSelectMembersAndMethodsDialog(ClassDiagram* pClassDiagram,
                                          void* ownerHwnd,
                                          SelectMembersApplyCallback onApply,
                                          void* applyContext)
{
    Qt_EnsureApplication();

    SelectMembersAndMethods dlg(pClassDiagram, onApply, applyContext);
    Qt_ExecModal(dlg, ownerHwnd);
}
