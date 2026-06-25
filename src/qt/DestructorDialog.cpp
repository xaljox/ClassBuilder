// qt/DestructorDialog.cpp -- the Qt Destructor properties dialog.
//
// Ported from the MFC CDestructorDialog (the constructor-dialog twin, with
// Virtual/Pure in place of Explicit). Drives the model directly.

#include "DestructorDialog.h"
#include "ui_DestructorDialog.h"

#include "QtDestructorDialog.h"   // Qt_ShowDestructorDialog
#include "QtApp.h"                // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"          // toQ / toCb (CRLF-aware)
#include "QtCompact.h"            // compactCombo

#include <QCheckBox>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

DestructorDialog::DestructorDialog(Destructor* pDestructor, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::DestructorDialog)
    , _pDestructor(pDestructor)
{
    _ui->setupUi(this);

    setWindowTitle("Destructor of class " +
                   toQ(_pDestructor->GetBaseClass()->GetName()));

    // Access radio: 0 = Public, 1 = Protected, 2 = Private.
    switch (_pDestructor->GetAccess())
    {
    case PROTECTED: _ui->radioProtected->setChecked(true); break;
    case PRIVATE:   _ui->radioPrivate->setChecked(true);   break;
    default:        _ui->radioPublic->setChecked(true);    break;
    }

    _ui->checkVirtual->setChecked(_pDestructor->GetVirtual());
    _ui->checkPure->setChecked(_pDestructor->GetPure());
    _ui->checkInline->setChecked(_pDestructor->GetInline());
    _ui->checkDllExport->setChecked(_pDestructor->GetDllExport());
    _ui->checkDeclare->setChecked(_pDestructor->GetDeclare());
    _ui->checkImplement->setChecked(_pDestructor->GetImplement());
    compactCombo(_ui->comboCallingConvention);
    _ui->comboCallingConvention->setCurrentText(
        toQ(_pDestructor->GetCallingConvention()));
    _ui->plainTextNote->setPlainText(toQ(_pDestructor->GetNote()));

    // A destructor on a non-class base can't be private / declared /
    // implemented (mirrors the MFC OnInitDialog).
    if (!_pDestructor->GetBaseClass()->IsClass())
    {
        _ui->radioPrivate->setEnabled(false);
        _ui->checkDeclare->setEnabled(false);
        _ui->checkImplement->setEnabled(false);
    }

    connect(_ui->checkImplement, &QCheckBox::toggled,
            this, &DestructorDialog::onImplementToggled);
    connect(_ui->checkVirtual, &QCheckBox::toggled,
            this, &DestructorDialog::onVirtualToggled);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &DestructorDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &DestructorDialog::reject);

    onVirtualToggled();     // initial Pure gating
    onImplementToggled();   // initial Inline gating
}

DestructorDialog::~DestructorDialog()
{
    delete _ui;
}

// Inline is only meaningful for an implemented destructor on a class base.
void DestructorDialog::onImplementToggled()
{
    if (_ui->checkImplement->isChecked() &&
        _pDestructor->GetBaseClass()->IsClass())
    {
        _ui->checkInline->setEnabled(true);
        _ui->checkInline->setChecked(_pDestructor->GetInline());
    }
    else
    {
        _ui->checkInline->setChecked(false);
        _ui->checkInline->setEnabled(false);
    }
}

// Pure only applies to a virtual destructor.
void DestructorDialog::onVirtualToggled()
{
    if (_ui->checkVirtual->isChecked())
    {
        _ui->checkPure->setEnabled(true);
    }
    else
    {
        _ui->checkPure->setChecked(false);
        _ui->checkPure->setEnabled(false);
    }
}

// OK -- apply every changed field to the model (the MFC Update()).
void DestructorDialog::accept()
{
    const int access = _ui->radioPrivate->isChecked()   ? PRIVATE
                      : _ui->radioProtected->isChecked() ? PROTECTED
                                                         : PUBLIC;
    const bool    virtual_   = _ui->checkVirtual->isChecked();
    const bool    pure       = _ui->checkPure->isChecked();
    const bool    inLine     = _ui->checkInline->isChecked();
    const bool    dllExport  = _ui->checkDllExport->isChecked();
    const bool    declare    = _ui->checkDeclare->isChecked();
    const bool    implement  = _ui->checkImplement->isChecked();
    const QString qConv      = _ui->comboCallingConvention->currentText();
    const QString qNote      = _ui->plainTextNote->toPlainText();

    if (_pDestructor->GetAccess() != access ||
        toQ(_pDestructor->GetCallingConvention()) != qConv ||
        _pDestructor->GetDllExport() != dllExport ||
        toQ(_pDestructor->GetNote()) != qNote ||
        _pDestructor->GetVirtual() != virtual_ ||
        _pDestructor->GetPure() != pure ||
        _pDestructor->GetInline() != inLine ||
        _pDestructor->GetDeclare() != declare ||
        _pDestructor->GetImplement() != implement)
    {
        _pDestructor->SaveState();
        _pDestructor->SetAccess(AccessType(access));
        _pDestructor->SetCallingConvention(toCb(qConv));
        _pDestructor->SetDllExport(dllExport);
        _pDestructor->SetNote(toCb(qNote));
        _pDestructor->SetVirtual(virtual_);
        _pDestructor->SetPure(pure);
        _pDestructor->SetInline(inLine);
        _pDestructor->SetDeclare(declare);
        _pDestructor->SetImplement(implement);
        _modelChanged = true;
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowDestructorDialog(Destructor* pDestructor, bool& modelChangedOut,
                             void* ownerHwnd)
{
    Qt_EnsureApplication();

    DestructorDialog dlg(pDestructor);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    return true;
}
