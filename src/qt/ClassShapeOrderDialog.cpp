// qt/ClassShapeOrderDialog.cpp -- the Qt Class Shape Order dialog.
//
// Ported from the MFC CClassShapeOrderDialog. Two single-select lists -- the
// member and method shapes the diagram shape displays -- with Move Up / Move
// Down buttons. On OK the model's shape order is rewritten to match the list
// order. Drives the model directly.

#include "ClassShapeOrderDialog.h"
#include "ui_ClassShapeOrderDialog.h"

#include "QtClassShapeOrderDialog.h" // Qt_ShowClassShapeOrderDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ
#include "QtCompact.h"               // compactItemSize

#include <QListWidget>
#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QPushButton>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

// Add one row to `list` carrying `ptr` in Qt::UserRole.
static void addRow(QListWidget* list, const QString& text, void* ptr)
{
    QListWidgetItem* item = new QListWidgetItem(text, list);
    item->setData(Qt::UserRole, QVariant::fromValue(
        reinterpret_cast<qulonglong>(ptr)));
    item->setSizeHint(compactItemSize(list, text));
}

ClassShapeOrderDialog::ClassShapeOrderDialog(ClassShape* pClassShape,
                                             QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ClassShapeOrderDialog)
    , _pClassShape(pClassShape)
{
    _ui->setupUi(this);

    setWindowTitle(toQ(_pClassShape->GetBaseClass()->GetName()));

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ClassShapeOrderDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ClassShapeOrderDialog::reject);
    connect(_ui->buttonMembersUp, &QPushButton::clicked,
            this, [this] { moveRow(_ui->listMembers, -1); });
    connect(_ui->buttonMembersDown, &QPushButton::clicked,
            this, [this] { moveRow(_ui->listMembers, +1); });
    connect(_ui->buttonMethodsUp, &QPushButton::clicked,
            this, [this] { moveRow(_ui->listMethods, -1); });
    connect(_ui->buttonMethodsDown, &QPushButton::clicked,
            this, [this] { moveRow(_ui->listMethods, +1); });

    fillLists();
}

ClassShapeOrderDialog::~ClassShapeOrderDialog()
{
    delete _ui;
}

// Populate both lists in the shape's current member/method order. Each item
// carries its MemberShape* / MethodShape* in Qt::UserRole.
void ClassShapeOrderDialog::fillLists()
{
    ClassShape::MemberShapeIterator iMemberShape(_pClassShape);
    while (++iMemberShape)
        addRow(_ui->listMembers,
               toQ(iMemberShape->GetMember()->GetShapeText(VERBOSITY_STATIC)),
               iMemberShape.Get());

    const VerbosityType verbosity =
        VerbosityType(_pClassShape->GetVerbosity());
    ClassShape::MethodShapeIterator iMethodShape(_pClassShape);
    while (++iMethodShape)
        addRow(_ui->listMethods,
               toQ(iMethodShape->GetMethod()->GetShapeText(verbosity)),
               iMethodShape.Get());
}

// Shift the selected row of `list` by `delta` (-1 up, +1 down), keeping it
// selected. No-op if nothing is selected or the move runs off either end.
void ClassShapeOrderDialog::moveRow(QListWidget* list, int delta)
{
    const int row = list->currentRow();
    if (row < 0)
        return;
    const int newRow = row + delta;
    if (newRow < 0 || newRow >= list->count())
        return;

    QListWidgetItem* item = list->takeItem(row);
    list->insertItem(newRow, item);
    list->setCurrentRow(newRow);
}

// OK -- rewrite the model's shape order to match the list order (the MFC
// OnOK: SaveState then MoveXxxShapeLast each shape, top to bottom).
void ClassShapeOrderDialog::accept()
{
    for (int i = 0; i < _ui->listMembers->count(); ++i)
    {
        MemberShape* pMemberShape = reinterpret_cast<MemberShape*>(
            _ui->listMembers->item(i)->data(Qt::UserRole).toULongLong());
        pMemberShape->SaveState(1);
        _pClassShape->MoveMemberShapeLast(pMemberShape);
    }

    for (int i = 0; i < _ui->listMethods->count(); ++i)
    {
        MethodShape* pMethodShape = reinterpret_cast<MethodShape*>(
            _ui->listMethods->item(i)->data(Qt::UserRole).toULongLong());
        pMethodShape->SaveState(1);
        _pClassShape->MoveMethodShapeLast(pMethodShape);
    }

    // Close the edit: MarkLastUndo recomputes the box so the rows re-lay-out in the new
    // order (RecalculateRect lays them out in relation order) and folds the reorder +
    // reroute into ONE undo step -- without it the reorder isn't applied visually and the
    // undo stack is left open (the caller only refreshes, it doesn't close the edit).
    _pClassShape->GetDataModelDoc()->MarkLastUndo();
    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowClassShapeOrderDialog(ClassShape* pClassShape, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ClassShapeOrderDialog dlg(pClassShape);
    return Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
}
