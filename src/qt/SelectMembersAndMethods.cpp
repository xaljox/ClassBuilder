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

#include <algorithm>
#include <vector>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {
// Children of pGti in main-tree DISPLAY order: the model comparator
// Gti::CompareTreeOrder, stable for ties -- the same rule MainTreeQtView's
// sortedChildren applies. Raw child order is NOT what the tree shows.
std::vector<Gti*> treeOrderedChildren(Gti* pGti)
{
    std::vector<Gti*> children;
    Gti::ChildIterator iChild(pGti);
    while (++iChild)
        children.push_back(iChild.Get());

    std::stable_sort(children.begin(), children.end(),
                     [](Gti* a, Gti* b) { return Gti::CompareTreeOrder(a, b) < 0; });
    return children;
}
} // namespace

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

    // Both levels follow the MODEL's tree order, exactly like the main tree:
    // the class roots come from a depth-first walk over the model's children
    // (document class first, groups in place), and the rows below each root
    // from the class's own child order. (The old fill added all methods,
    // then all members, then alphabetically sorted the whole tree -- neither
    // level matched the main tree, JV 2026-07-13.)
    std::vector<ClassShape*> shapes;
    ClassDiagram::ClassDiagramShapeIterator
        iShape(_pClassDiagram, &ClassDiagramShape::IsClassShape);
    while (++iShape)
        shapes.push_back((ClassShape*)iShape.Get());

    addClassRoots(_pClassDiagram->GetDataModelDoc()->GetDataModel(), shapes);

    _ui->tree->expandAll();
}

// Depth-first over the model's children in tree order; every class that has
// a shape on this diagram becomes a root. Non-class containers (class
// groups, the Extern Classes node) are entered; a class's own children are
// handled by addFeatureRows.
void SelectMembersAndMethods::addClassRoots(
    Gti* pParent, const std::vector<ClassShape*>& shapes)
{
    for (Gti* pGti : treeOrderedChildren(pParent))
    {
        ClassShape* pClassShape = nullptr;
        for (ClassShape* pShape : shapes)
            if (static_cast<Gti*>(pShape->GetBaseClass()) == pGti)
            {
                pClassShape = pShape;
                break;
            }

        if (pClassShape)
        {
            BaseClass* pBaseClass = pClassShape->GetBaseClass();

            QTreeWidgetItem* root = new QTreeWidgetItem(_ui->tree);
            root->setText(0, toQ(pBaseClass->GetItemText()));
            root->setIcon(0, Qt_ModelIcon(pBaseClass->GetIcon()));
            setPtr(root, static_cast<void*>(pClassShape));

            addFeatureRows(root, pBaseClass, pClassShape);
        }
        else
        {
            addClassRoots(pGti, shapes);
        }
    }
}

// Depth-first over `pParent`'s children in model order -- the exact order
// the main tree shows -- adding a row for every member/method that passes
// the filters. Containers flatten in place: group folders, the ClassBuilder/
// Relation methods folders, and members (whose get/set methods are their
// children) all keep their features at the tree position.
void SelectMembersAndMethods::addFeatureRows(QTreeWidgetItem* root,
                                             Gti* pParent,
                                             ClassShape* pClassShape)
{
    for (Gti* pGti : treeOrderedChildren(pParent))
    {
        if (pGti->IsMethod())
        {
            Method* pMethod = static_cast<Method*>(pGti);
            const int access = pMethod->GetAccess();
            if (!((_privateMethods && access == PRIVATE) ||
                  (_protectedMethods && access == PROTECTED) ||
                  (_publicMethods && access == PUBLIC)))
                continue;
            if ((pMethod->IsFixedMethod() && !_classBuilderMethods) ||
                (pMethod->IsMacroMethod() && !_relationMethods) ||
                (pMethod->IsGetMemberMethod() && !_getSetMethods) ||
                (pMethod->IsSetMemberMethod() && !_getSetMethods))
                continue;

            QTreeWidgetItem* item = new QTreeWidgetItem(root);
            item->setText(0, toQ(pMethod->GetItemText()));
            item->setIcon(0, Qt_ModelIcon(pMethod->GetIcon()));
            setPtr(item, static_cast<void*>(static_cast<Gti*>(pMethod)));
            item->setCheckState(0,
                pMethod->FindMethodShape(pClassShape) ? Qt::Checked
                                                      : Qt::Unchecked);
        }
        else if (pGti->IsMember())
        {
            Member* pMember = static_cast<Member*>(pGti);
            const int access = pMember->GetAccess();
            if (!((_privateMembers && access == PRIVATE) ||
                  (_protectedMembers && access == PROTECTED) ||
                  (_publicMembers && access == PUBLIC)))
                continue;

            QTreeWidgetItem* item = new QTreeWidgetItem(root);
            item->setText(0, toQ(pMember->GetItemText()));
            item->setIcon(0, Qt_ModelIcon(pMember->GetIcon()));
            setPtr(item, static_cast<void*>(static_cast<Gti*>(pMember)));
            item->setCheckState(0,
                pMember->FindMemberShape(pClassShape) ? Qt::Checked
                                                      : Qt::Unchecked);
        }
    }
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
