// qt/ExceptionSpecificationDialog.cpp -- the Qt Exception Specification dialog.
//
// Ported from the MFC ExceptionSpecificationDialog. Edits a method's throw
// clause: a transfer pair (thrown types <-> available types) with Add/Remove,
// plus a property panel for the selected thrown type. Each edit mutates the
// model immediately (the property checkboxes SaveState first); the caller
// wraps the dialog in MarkLastUndo / RollBack so Cancel undoes the sequence.

#include "ExceptionSpecificationDialog.h"
#include "ui_ExceptionSpecificationDialog.h"

#include "QtExceptionSpecificationDialog.h" // Qt_ShowExceptionSpecificationDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "QtCompact.h"               // compactItemSize

#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

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

ExceptionSpecificationDialog::ExceptionSpecificationDialog(Method* pMethod,
                                                           QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ExceptionSpecificationDialog)
    , _pMethod(pMethod)
{
    _ui->setupUi(this);

    setWindowTitle("Exception specification of method " +
                   toQ(_pMethod->GetBaseClass()->GetName()) + "::" +
                   toQ(_pMethod->GetName()));

    // Thrown types already on the method's exception specification.
    if (ExceptionSpecification* pES = _pMethod->GetExceptionSpecification())
    {
        ExceptionSpecification::ExceptionSpecificationTypeIterator iESType(pES);
        while (++iESType)
            addItem(_ui->listThrow,
                    toQ(iESType->GetType()->GetName()), iESType.Get());
    }

    // Every named model type is an Add candidate (skip the empty / "..." ones).
    DataModelDoc::TypeIterator iType(_pMethod->GetDataModelDoc());
    while (++iType)
    {
        const CbString name = iType->GetName();
        if (name != "" && name != "...")
            addItem(_ui->listTypes, toQ(name), iType.Get());
    }

    // The throw clause exists iff the box is ticked.
    _ui->checkEnable->setChecked(_pMethod->GetExceptionSpecification() != 0);

    connect(_ui->checkEnable, &QCheckBox::toggled,
            this, &ExceptionSpecificationDialog::onEnableToggled);
    connect(_ui->buttonAdd, &QPushButton::clicked,
            this, &ExceptionSpecificationDialog::onAdd);
    connect(_ui->buttonRemove, &QPushButton::clicked,
            this, &ExceptionSpecificationDialog::onRemove);
    connect(_ui->listThrow, &QListWidget::itemSelectionChanged,
            this, &ExceptionSpecificationDialog::onThrowSelChanged);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ExceptionSpecificationDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ExceptionSpecificationDialog::reject);

    // Property panel -- each edit SaveStates then writes the model. clicked()
    // / textEdited() fire only on real user input, so updateGui()'s
    // programmatic setChecked/setText calls do not re-enter these.
    auto saveAndSet = [this](void (ExceptionSpecificationType::*set)(bool),
                             bool value)
    {
        if (!_pESType)
            return;
        _pESType->SaveState();
        (_pESType->*set)(value);
    };
    connect(_ui->checkConst, &QCheckBox::clicked, this, [this, saveAndSet](bool v)
    { saveAndSet(&ExceptionSpecificationType::SetConst, v); _pMethod->Update(); });
    connect(_ui->checkReference, &QCheckBox::clicked, this, [this, saveAndSet](bool v)
    { saveAndSet(&ExceptionSpecificationType::SetReference, v); _pMethod->Update(); });
    connect(_ui->checkConstPointer, &QCheckBox::clicked, this, [this, saveAndSet](bool v)
    { saveAndSet(&ExceptionSpecificationType::SetConstPointer, v); _pMethod->Update(); });
    connect(_ui->checkPointerPointer, &QCheckBox::clicked, this, [this, saveAndSet](bool v)
    { saveAndSet(&ExceptionSpecificationType::SetPointerPointer, v); _pMethod->Update(); });
    connect(_ui->checkPointer, &QCheckBox::clicked, this, [this](bool v)
    {
        if (!_pESType)
            return;
        _pESType->SaveState();
        _pESType->SetPointer(v);
        if (!v)
        {
            // Pointer cleared: the pointer-dependent flags no longer apply --
            // clear them too, else they stay set (disabled but still ticked)
            // and produce a nonsense type string. (Bug carried from the MFC
            // original, which left them set.)
            _pESType->SetConstPointer(0);
            _pESType->SetPointerPointer(0);
        }
        updateGui();
    });
    connect(_ui->checkArray, &QCheckBox::clicked, this, [this, saveAndSet](bool v)
    {
        saveAndSet(&ExceptionSpecificationType::SetArray, v);
        updateGui();
        if (v)
            _ui->editArraySize->setFocus();
    });

    connect(_ui->editArraySize, &QLineEdit::textEdited, this, [this](const QString& t)
    {
        if (!_pESType)
            return;
        _pESType->SaveState();
        _pESType->SetArraySizeStr(toCb(t));
        _pMethod->Update();
    });
    connect(_ui->editArraySize, &QLineEdit::editingFinished,
            this, &ExceptionSpecificationDialog::validateArraySize);
    connect(_ui->editTemplate, &QLineEdit::textEdited, this, [this](const QString& t)
    {
        if (!_pESType)
            return;
        _pESType->SaveState();
        _pESType->SetTemplate(toCb(t));
        _pMethod->Update();
    });

    onEnableToggled(_ui->checkEnable->isChecked());  // initial enable state
    onThrowSelChanged();                             // initial property panel
}

ExceptionSpecificationDialog::~ExceptionSpecificationDialog()
{
    delete _ui;
}

// The "has a throw clause" checkbox: create the ExceptionSpecification on
// tick, destroy it (and every thrown type) on untick.
void ExceptionSpecificationDialog::onEnableToggled(bool enabled)
{
    if (enabled)
    {
        if (!_pMethod->GetExceptionSpecification())
            (void)new ExceptionSpecification(_pMethod);
    }
    else if (ExceptionSpecification* pES =
                 _pMethod->GetExceptionSpecification())
    {
        for (int i = _ui->listThrow->count() - 1; i >= 0; --i)
        {
            QListWidgetItem* item = _ui->listThrow->item(i);
            itemPtr<ExceptionSpecificationType>(item)->Delete();
        }
        _ui->listThrow->clear();
        pES->Delete();
        _pESType = nullptr;
    }

    _ui->throwGroup->setEnabled(enabled);
    _ui->typesGroup->setEnabled(enabled);
    _ui->buttonAdd->setEnabled(enabled);
    _ui->buttonRemove->setEnabled(enabled);
    if (!enabled)
        _ui->selectedGroup->setEnabled(false);

    _pMethod->Update();
}

// Add: create an ExceptionSpecificationType for each selected available type.
void ExceptionSpecificationDialog::onAdd()
{
    ExceptionSpecification* pES = _pMethod->GetExceptionSpecification();
    if (!pES)
        return;

    QListWidgetItem* lastAdded = nullptr;
    const QList<QListWidgetItem*> selected = _ui->listTypes->selectedItems();
    for (QListWidgetItem* item : selected)
    {
        Type* pType = itemPtr<Type>(item);
        ExceptionSpecificationType* pESType =
            new ExceptionSpecificationType(pES, pType);
        lastAdded = addItem(_ui->listThrow, toQ(pType->GetName()), pESType);
    }

    if (lastAdded)
        _ui->listThrow->setCurrentItem(lastAdded);  // -> onThrowSelChanged
    _pMethod->Update();
}

// Remove: delete the selected thrown type from the model and the list.
void ExceptionSpecificationDialog::onRemove()
{
    const QList<QListWidgetItem*> selected = _ui->listThrow->selectedItems();
    for (QListWidgetItem* item : selected)
    {
        itemPtr<ExceptionSpecificationType>(item)->Delete();
        delete _ui->listThrow->takeItem(_ui->listThrow->row(item));
    }
    onThrowSelChanged();
    _pMethod->Update();
}

// Throw-list selection changed: bind the property panel to the selected type.
void ExceptionSpecificationDialog::onThrowSelChanged()
{
    const QList<QListWidgetItem*> selected = _ui->listThrow->selectedItems();
    if (selected.count() == 1)
    {
        _pESType = itemPtr<ExceptionSpecificationType>(selected.first());
        _ui->selectedGroup->setEnabled(true);
        updateGui();
    }
    else
    {
        _pESType = nullptr;
        _ui->selectedGroup->setEnabled(false);
    }
}

// Load the property panel from the selected type (the MFC UpdateGui).
void ExceptionSpecificationDialog::updateGui()
{
    if (!_pESType)
        return;

    const bool array   = _pESType->GetArray();
    const bool pointer = _pESType->GetPointer();

    _ui->checkArray->setChecked(array);
    _ui->editArraySize->setEnabled(array);
    _ui->editArraySize->setText(toQ(_pESType->GetArraySizeStr()));

    _ui->checkConst->setChecked(_pESType->GetConst());
    _ui->checkReference->setChecked(_pESType->GetReference());

    _ui->checkPointer->setChecked(pointer);
    _ui->checkConstPointer->setEnabled(pointer);
    _ui->checkConstPointer->setChecked(_pESType->GetConstPointer());
    _ui->checkPointerPointer->setEnabled(pointer);
    _ui->checkPointerPointer->setChecked(_pESType->GetPointerPointer());

    _ui->editTemplate->setEnabled(
        !_pESType->GetType()->GetTemplate().IsEmpty());
    _ui->editTemplate->setText(toQ(_pESType->GetTemplate()));

    _pMethod->Update();
}

// Array checked but no size given: warn -- OK to fix it, Cancel drops Array.
void ExceptionSpecificationDialog::validateArraySize()
{
    if (!_pESType || !_ui->checkArray->isChecked())
        return;
    if (!_ui->editArraySize->text().isEmpty())
        return;

    const QString text = "Must specify a size for " +
        toQ(_pESType->GetType()->GetName()) + " array";
    if (QMessageBox::warning(this, "Exception Specification", text,
            QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
        _ui->editArraySize->setFocus();
    }
    else
    {
        _pESType->SaveState();
        _pESType->SetArray(0);
        updateGui();
        _ui->checkArray->setFocus();
    }
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowExceptionSpecificationDialog(Method* pMethod, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ExceptionSpecificationDialog dlg(pMethod);
    return Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
}
