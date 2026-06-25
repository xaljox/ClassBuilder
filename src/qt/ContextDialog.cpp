// qt/ContextDialog.cpp -- the Qt "Assign Context" dialog.
//
// Ported from the MFC ContextDialog. Two-list transfer dialog. Each Add/Remove
// mutates the model immediately (CreateContext / Context::Delete); the caller
// wraps the dialog in MarkLastUndo / RollBack so Cancel undoes the sequence.

#include "ContextDialog.h"
#include "ui_ContextDialog.h"

#include "QtContextDialog.h"   // Qt_ShowContextDialog
#include "QtApp.h"             // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"       // toQ
#include "QtCompact.h"         // compactItemSize

#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// Add an item to a list: text + a model pointer in Qt::UserRole, with a
// compact row height (the modern Windows style's default rows are tall).
QListWidgetItem* addItem(QListWidget* list, const QString& text, void* ptr)
{
    QListWidgetItem* item = new QListWidgetItem(text, list);
    item->setData(Qt::UserRole,
                  QVariant::fromValue(reinterpret_cast<qulonglong>(ptr)));
    item->setSizeHint(compactItemSize(list, text));
    return item;
}

template <class T>
T* itemPtr(const QListWidgetItem* item)
{
    return reinterpret_cast<T*>(item->data(Qt::UserRole).toULongLong());
}

} // namespace

ContextDialog::ContextDialog(Gti* pGti, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ContextDialog)
    , _pGti(pGti)
{
    _ui->setupUi(this);
    setWindowTitle(windowTitle() + toQ(_pGti->GetItemText()));

    // Right list: context declarations not yet assigned to this Gti.
    DataModel::ContextDeclarationIterator
        iDecl(_pGti->GetDataModelDoc()->GetDataModel());
    while (++iDecl)
    {
        if (!iDecl->FindContext(_pGti))
            addItem(_ui->listUnassigned, toQ(iDecl->GetName()), iDecl.Get());
    }

    // Left list: contexts already assigned to this Gti.
    Context* pContext = _pGti->GetFirstContext();
    while (pContext)
    {
        addItem(_ui->listAssigned,
                toQ(pContext->GetContextDeclaration()->GetName()), pContext);
        pContext = _pGti->GetNextContext(pContext);
    }

    connect(_ui->buttonAdd, &QPushButton::clicked,
            this, &ContextDialog::onAdd);
    connect(_ui->buttonRemove, &QPushButton::clicked,
            this, &ContextDialog::onRemove);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ContextDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ContextDialog::reject);
}

ContextDialog::~ContextDialog()
{
    delete _ui;
}

// Add: assign each selected declaration -> CreateContext, move it to the
// assigned list (its item now carries the new Context*).
void ContextDialog::onAdd()
{
    const QList<QListWidgetItem*> selected =
        _ui->listUnassigned->selectedItems();
    for (QListWidgetItem* item : selected)
    {
        ContextDeclaration* pDecl = itemPtr<ContextDeclaration>(item);
        Context* pContext = _pGti->CreateContext(pDecl);

        delete _ui->listUnassigned->takeItem(_ui->listUnassigned->row(item));
        addItem(_ui->listAssigned, toQ(pDecl->GetName()), pContext);
    }
}

// Remove: unassign each selected context -> Context::Delete, move it back to
// the unassigned list (its item now carries the ContextDeclaration*).
void ContextDialog::onRemove()
{
    const QList<QListWidgetItem*> selected =
        _ui->listAssigned->selectedItems();
    for (QListWidgetItem* item : selected)
    {
        Context* pContext = itemPtr<Context>(item);
        ContextDeclaration* pDecl = pContext->GetContextDeclaration();

        delete _ui->listAssigned->takeItem(_ui->listAssigned->row(item));
        pContext->Delete();
        addItem(_ui->listUnassigned, toQ(pDecl->GetName()), pDecl);
    }
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowContextDialog(Gti* pGti, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ContextDialog dlg(pGti);
    return Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
}
