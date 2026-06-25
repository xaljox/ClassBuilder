// qt/ContextDeclarationDialog.cpp -- the Qt Context Declarations dialog.
//
// Ported from the MFC ContextDeclarationDialog. A list of context
// declarations + a property panel. The panel is flushed to the model when the
// selection changes and on OK; Add/Remove create/delete declarations. The
// caller wraps the dialog in MarkLastUndo / RollBack so Cancel undoes it.

#include "ContextDeclarationDialog.h"
#include "ui_ContextDeclarationDialog.h"

#include "QtContextDeclarationDialog.h"  // Qt_ShowContextDeclarationDialog
#include "QtApp.h"                       // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"                 // toQ / toCb
#include "QtCompact.h"                   // compactItemSize

#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {
ContextDeclaration* itemDecl(const QListWidgetItem* item)
{
    return reinterpret_cast<ContextDeclaration*>(
        item->data(Qt::UserRole).toULongLong());
}
}

ContextDeclarationDialog::ContextDeclarationDialog(DataModel* pDataModel,
                                                   QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ContextDeclarationDialog)
    , _pDataModel(pDataModel)
{
    _ui->setupUi(this);

    DataModel::ContextDeclarationIterator iDecl(_pDataModel);
    while (++iDecl)
    {
        QListWidgetItem* item = new QListWidgetItem(
            toQ(iDecl->GetName()), _ui->listContexts);
        item->setData(Qt::UserRole, QVariant::fromValue(
            reinterpret_cast<qulonglong>(iDecl.Get())));
        item->setSizeHint(compactItemSize(_ui->listContexts, item->text()));
    }
    if (_ui->listContexts->count())
        _ui->listContexts->setCurrentRow(0);

    // setCurrentRow above fires no signal yet (itemSelectionChanged is connected
    // below), so onSelectionChanged never runs for the initial selection -- bind
    // _current to it explicitly, otherwise the panel loads blank for the
    // pre-selected first context until the selection changes once (you had to
    // add a context, then reselect Forward Only, to see its definition).
    if (QListWidgetItem* sel = _ui->listContexts->currentItem())
        _current = itemDecl(sel);

    loadPanel();

    connect(_ui->listContexts, &QListWidget::itemSelectionChanged,
            this, &ContextDeclarationDialog::onSelectionChanged);
    connect(_ui->buttonAdd, &QPushButton::clicked,
            this, &ContextDeclarationDialog::onAdd);
    connect(_ui->buttonRemove, &QPushButton::clicked,
            this, &ContextDeclarationDialog::onRemove);
    connect(_ui->editName, &QLineEdit::textEdited,
            this, &ContextDeclarationDialog::onNameEdited);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ContextDeclarationDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ContextDeclarationDialog::reject);
}

ContextDeclarationDialog::~ContextDeclarationDialog()
{
    delete _ui;
}

// Write the panel widgets back to the current declaration.
void ContextDeclarationDialog::flushPanel()
{
    if (!_current)
        return;
    _current->SetName(toCb(_ui->editName->text()));
    _current->SetNote(toCb(_ui->editNote->toPlainText()));
    _current->SetEnableDefineDeclaration(
        _ui->checkEnableDefine->isChecked());
    _current->SetDefineDeclaration(toCb(_ui->editDefine->text()));
    _current->SetStartContextDeclaration(
        toCb(_ui->editStartDecl->toPlainText()));
    _current->SetEndContextDeclaration(
        toCb(_ui->editEndDecl->toPlainText()));
    _current->SetStartContextImplementation(
        toCb(_ui->editStartImpl->toPlainText()));
    _current->SetEndContextImplementation(
        toCb(_ui->editEndImpl->toPlainText()));
}

// Load the current declaration into the panel; the panel is disabled when
// there is no selection.
void ContextDeclarationDialog::loadPanel()
{
    _ui->panelGroup->setEnabled(_current != nullptr);
    if (!_current)
    {
        _ui->editName->clear();
        _ui->editNote->clear();
        _ui->checkEnableDefine->setChecked(false);
        _ui->editDefine->clear();
        _ui->editStartDecl->clear();
        _ui->editEndDecl->clear();
        _ui->editStartImpl->clear();
        _ui->editEndImpl->clear();
        return;
    }
    _ui->editName->setText(toQ(_current->GetName()));
    _ui->editNote->setPlainText(toQ(_current->GetNote()));
    _ui->checkEnableDefine->setChecked(
        _current->GetEnableDefineDeclaration());
    _ui->editDefine->setText(toQ(_current->GetDefineDeclaration()));
    _ui->editStartDecl->setPlainText(
        toQ(_current->GetStartContextDeclaration()));
    _ui->editEndDecl->setPlainText(
        toQ(_current->GetEndContextDeclaration()));
    _ui->editStartImpl->setPlainText(
        toQ(_current->GetStartContextImplementation()));
    _ui->editEndImpl->setPlainText(
        toQ(_current->GetEndContextImplementation()));
}

// Selection changed: flush the old declaration, bind + load the new one.
void ContextDeclarationDialog::onSelectionChanged()
{
    flushPanel();

    QList<QListWidgetItem*> selected = _ui->listContexts->selectedItems();
    _current = selected.isEmpty() ? nullptr : itemDecl(selected.first());

    loadPanel();
}

void ContextDeclarationDialog::onAdd()
{
    flushPanel();

    ContextDeclaration* pDecl =
        new ContextDeclaration(_pDataModel, "New Context");
    pDecl->SetDefineDeclaration("#define ");

    QListWidgetItem* item = new QListWidgetItem(
        toQ(pDecl->GetName()), _ui->listContexts);
    item->setData(Qt::UserRole,
                  QVariant::fromValue(reinterpret_cast<qulonglong>(pDecl)));
    item->setSizeHint(compactItemSize(_ui->listContexts, item->text()));

    _ui->listContexts->setCurrentItem(item);   // -> onSelectionChanged
    _ui->editName->setFocus();
    _ui->editName->selectAll();
}

void ContextDeclarationDialog::onRemove()
{
    QListWidgetItem* item = _ui->listContexts->currentItem();
    if (!item || !_current)
        return;

    _current->Delete();
    _current = nullptr;             // so onSelectionChanged does not flush it
    delete _ui->listContexts->takeItem(_ui->listContexts->row(item));
    // takeItem changes the selection -> onSelectionChanged already ran.
}

// Live name edit -- keep the list entry's text (and sort order) in step.
void ContextDeclarationDialog::onNameEdited(const QString& name)
{
    if (!_current)
        return;
    _current->SetName(toCb(name));
    if (QListWidgetItem* item = _ui->listContexts->currentItem())
        item->setText(name);        // sortingEnabled re-sorts
}

void ContextDeclarationDialog::accept()
{
    flushPanel();
    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowContextDeclarationDialog(DataModel* pDataModel, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ContextDeclarationDialog dlg(pDataModel);
    return Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
}
