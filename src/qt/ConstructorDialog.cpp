// qt/ConstructorDialog.cpp -- the Qt Constructor properties dialog.
//
// Ported from the MFC CConstructorDialog. Drives the model directly: it holds
// the live Constructor* and reads / writes it.

#include "ConstructorDialog.h"
#include "ui_ConstructorDialog.h"

#include "QtConstructorDialog.h"   // Qt_ShowConstructorDialog
#include "QtApp.h"                 // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"           // toQ / toCb (CRLF-aware)
#include "QtCompact.h"             // compactCombo

#include <QCheckBox>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

ConstructorDialog::ConstructorDialog(Constructor* pConstructor,
                                     QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ConstructorDialog)
    , _pConstructor(pConstructor)
{
    _ui->setupUi(this);

    setWindowTitle("Constructor of class " +
                   toQ(_pConstructor->GetBaseClass()->GetName()));

    // Access radio: 0 = Public, 1 = Protected, 2 = Private.
    switch (_pConstructor->GetAccess())
    {
    case PROTECTED: _ui->radioProtected->setChecked(true); break;
    case PRIVATE:   _ui->radioPrivate->setChecked(true);   break;
    default:        _ui->radioPublic->setChecked(true);    break;
    }

    _ui->checkInline->setChecked(_pConstructor->GetInline());
    _ui->checkExplicit->setChecked(_pConstructor->GetExplicit());
    _ui->checkDllExport->setChecked(_pConstructor->GetDllExport());
    _ui->checkDeclare->setChecked(_pConstructor->GetDeclare());
    _ui->checkImplement->setChecked(_pConstructor->GetImplement());
    _ui->checkDelete->setChecked(_pConstructor->GetDelete());
    compactCombo(_ui->comboCallingConvention);
    _ui->comboCallingConvention->setCurrentText(
        toQ(_pConstructor->GetCallingConvention()));
    _ui->plainTextNote->setPlainText(toQ(_pConstructor->GetNote()));

    // A replace-constructor, or one on a non-class base, can't be made
    // private / declared / implemented (mirrors the MFC OnInitDialog).
    if (_pConstructor->IsReplaceConstructor() ||
        !_pConstructor->GetBaseClass()->IsClass())
    {
        _ui->radioPrivate->setEnabled(false);
        _ui->checkDeclare->setEnabled(false);
        _ui->checkImplement->setEnabled(false);
        _ui->checkDelete->setEnabled(false);
    }

    // Explicit only where the model allows it.
    if (!_pConstructor->ExplicitAllowed())
    {
        _ui->checkExplicit->setChecked(false);
        _ui->checkExplicit->setEnabled(false);
    }

    connect(_ui->checkImplement, &QCheckBox::toggled,
            this, &ConstructorDialog::onImplementToggled);
    connect(_ui->checkDelete, &QCheckBox::toggled,
            this, &ConstructorDialog::onDeleteToggled);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ConstructorDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ConstructorDialog::reject);

    onImplementToggled();   // initial Inline gating
    onDeleteToggled();      // a deleted ctor forces Implement off + disabled
}

ConstructorDialog::~ConstructorDialog()
{
    delete _ui;
}

// Inline is only meaningful for an implemented constructor on a class base.
void ConstructorDialog::onImplementToggled()
{
    if (_ui->checkImplement->isChecked() &&
        _pConstructor->GetBaseClass()->IsClass())
    {
        _ui->checkInline->setEnabled(true);
        _ui->checkInline->setChecked(_pConstructor->GetInline());
    }
    else
    {
        _ui->checkInline->setChecked(false);
        _ui->checkInline->setEnabled(false);
    }
}

// A "= delete"d constructor has no definition: force Implement off and lock
// it (which in turn drops Inline via onImplementToggled). Declare stays
// user-toggleable so the declaration itself can be suppressed for a debug
// compile. Unchecking restores Implement where the base allows it.
void ConstructorDialog::onDeleteToggled()
{
    if (_ui->checkDelete->isChecked())
    {
        _ui->checkImplement->setChecked(false);
        _ui->checkImplement->setEnabled(false);
    }
    else if (!_pConstructor->IsReplaceConstructor() &&
             _pConstructor->GetBaseClass()->IsClass())
    {
        _ui->checkImplement->setEnabled(true);
        _ui->checkImplement->setChecked(_pConstructor->GetImplement());
    }
}

// OK -- apply every changed field to the model (the MFC Update()).
void ConstructorDialog::accept()
{
    const int access = _ui->radioPrivate->isChecked()   ? PRIVATE
                      : _ui->radioProtected->isChecked() ? PROTECTED
                                                         : PUBLIC;
    const bool    inLine     = _ui->checkInline->isChecked();
    const bool    explicit_  = _ui->checkExplicit->isChecked();
    const bool    dllExport  = _ui->checkDllExport->isChecked();
    const bool    declare    = _ui->checkDeclare->isChecked();
    const bool    implement  = _ui->checkImplement->isChecked();
    const bool    isDelete   = _ui->checkDelete->isChecked();
    const QString qConv      = _ui->comboCallingConvention->currentText();
    const QString qNote      = _ui->plainTextNote->toPlainText();

    if (_pConstructor->GetAccess() != access ||
        toQ(_pConstructor->GetCallingConvention()) != qConv ||
        _pConstructor->GetDllExport() != dllExport ||
        toQ(_pConstructor->GetNote()) != qNote ||
        _pConstructor->GetInline() != inLine ||
        _pConstructor->GetExplicit() != explicit_ ||
        _pConstructor->GetDeclare() != declare ||
        _pConstructor->GetImplement() != implement ||
        _pConstructor->GetDelete() != isDelete)
    {
        _pConstructor->SaveState();
        _pConstructor->SetAccess(AccessType(access));
        _pConstructor->SetCallingConvention(toCb(qConv));
        _pConstructor->SetDllExport(dllExport);
        _pConstructor->SetNote(toCb(qNote));
        _pConstructor->SetInline(inLine);
        _pConstructor->SetExplicit(explicit_);
        _pConstructor->SetDeclare(declare);
        _pConstructor->SetImplement(implement);
        _pConstructor->SetDelete(isDelete);
        _modelChanged = true;
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowConstructorDialog(Constructor* pConstructor, bool& modelChangedOut,
                              void* ownerHwnd)
{
    Qt_EnsureApplication();

    ConstructorDialog dlg(pConstructor);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    return true;
}
