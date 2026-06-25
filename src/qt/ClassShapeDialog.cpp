// qt/ClassShapeDialog.cpp -- the Qt Class Shape dialog.
//
// Ported from the MFC CClassShapeDialog. Two checkbox lists -- the class's
// members and methods -- picking which ones the diagram shape displays. On
// OK the shape's MemberShape / MethodShape children are rebuilt to match the
// ticked set. Drives the model directly.

#include "ClassShapeDialog.h"
#include "ui_ClassShapeDialog.h"

#include "QtClassShapeDialog.h"      // Qt_ShowClassShapeDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ
#include "QtCompact.h"               // compactItemSize

#include <QListWidget>
#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QPushButton>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

// Add one checkable row to `list` carrying `ptr` in Qt::UserRole.
static void addRow(QListWidget* list, const QString& text, void* ptr,
                   bool checked)
{
    QListWidgetItem* item = new QListWidgetItem(text, list);
    item->setData(Qt::UserRole, QVariant::fromValue(
        reinterpret_cast<qulonglong>(ptr)));
    item->setSizeHint(compactItemSize(list, text, true));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

ClassShapeDialog::ClassShapeDialog(ClassShape* pClassShape, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ClassShapeDialog)
    , _pClassShape(pClassShape)
{
    _ui->setupUi(this);

    setWindowTitle(toQ(_pClassShape->GetBaseClass()->GetName()));

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ClassShapeDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ClassShapeDialog::reject);
    connect(_ui->buttonMembersSelectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(_ui->listMembers, true); });
    connect(_ui->buttonMembersUnselectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(_ui->listMembers, false); });
    connect(_ui->buttonMethodsSelectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(_ui->listMethods, true); });
    connect(_ui->buttonMethodsUnselectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(_ui->listMethods, false); });

    fillLists();
}

ClassShapeDialog::~ClassShapeDialog()
{
    delete _ui;
}

// Populate both lists: members/methods already shown by the shape go in
// first, ticked; the remaining (not-yet-shown) ones follow, unticked.
void ClassShapeDialog::fillLists()
{
    BaseClass* pBaseClass = _pClassShape->GetBaseClass();

    // Members ---------------------------------------------------------------
    ClassShape::MemberShapeIterator iMemberShape(_pClassShape);
    while (++iMemberShape)
    {
        Member* pMember = iMemberShape->GetMember();
        addRow(_ui->listMembers, toQ(pMember->GetShapeText(VERBOSITY_STATIC)),
               pMember, true);
    }
    BaseClass::MemberIterator iMember(pBaseClass);
    while (++iMember)
    {
        if (!iMember->FindMemberShape(_pClassShape))
            addRow(_ui->listMembers,
                   toQ(iMember->GetShapeText(VERBOSITY_STATIC)),
                   iMember.Get(), false);
    }

    // Methods ---------------------------------------------------------------
    const VerbosityType verbosity =
        VerbosityType(_pClassShape->GetVerbosity());

    ClassShape::MethodShapeIterator iMethodShape(_pClassShape);
    while (++iMethodShape)
    {
        Method* pMethod = iMethodShape->GetMethod();
        addRow(_ui->listMethods, toQ(pMethod->GetShapeText(verbosity)),
               pMethod, true);
    }
    pBaseClass->SortMethod(Method::CompareTreeSaveState);
    BaseClass::MethodIterator iMethod(pBaseClass, &Method::IsNonMacroMethod);
    while (++iMethod)
    {
        if (!iMethod->FindMethodShape(_pClassShape))
            addRow(_ui->listMethods, toQ(iMethod->GetShapeText(verbosity)),
                   iMethod.Get(), false);
    }
}

// Tick or untick every row of one list (its Select All / Unselect All).
void ClassShapeDialog::setAllChecked(QListWidget* list, bool checked)
{
    const Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    for (int i = 0; i < list->count(); ++i)
        list->item(i)->setCheckState(state);
}

// OK -- rebuild the shape's MemberShape / MethodShape children from the
// ticked rows (the MFC OnOK: delete all existing, recreate the ticked set).
void ClassShapeDialog::accept()
{
    // Members ---------------------------------------------------------------
    ClassShape::MemberShapeIterator iMemberShape(_pClassShape);
    while (++iMemberShape)
        iMemberShape->Delete();

    for (int i = 0; i < _ui->listMembers->count(); ++i)
    {
        QListWidgetItem* item = _ui->listMembers->item(i);
        if (item->checkState() != Qt::Checked)
            continue;
        Member* pMember = reinterpret_cast<Member*>(
            item->data(Qt::UserRole).toULongLong());
        new MemberShape(_pClassShape, pMember);
    }

    // Methods ---------------------------------------------------------------
    ClassShape::MethodShapeIterator iMethodShape(_pClassShape);
    while (++iMethodShape)
        iMethodShape->Delete();

    for (int i = 0; i < _ui->listMethods->count(); ++i)
    {
        QListWidgetItem* item = _ui->listMethods->item(i);
        if (item->checkState() != Qt::Checked)
            continue;
        Method* pMethod = reinterpret_cast<Method*>(
            item->data(Qt::UserRole).toULongLong());
        new MethodShape(_pClassShape, pMethod);
    }

    // Close the edit like every diagram edit: MarkLastUndo recomputes the box (lays out
    // the recreated member/method rows via RecalculateRect and reroutes the connections)
    // and folds the delete-all/recreate + reroute into ONE undo step. Without it the
    // recreated shapes have no positions -> empty box, and undo restores inconsistent
    // connection segments -> crash. The caller refreshes the views after the dialog.
    _pClassShape->GetDataModelDoc()->MarkLastUndo();
    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowClassShapeDialog(ClassShape* pClassShape, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ClassShapeDialog dlg(pClassShape);
    return Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
}
