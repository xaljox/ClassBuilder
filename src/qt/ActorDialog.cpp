// qt/ActorDialog.cpp -- the Qt Actor properties dialog.
//
// Ported from the MFC ActorDialog. Drives the model directly: it holds the
// live Actor* and reads / writes it (the model is MFC-free, so this Qt
// translation unit can include the model aggregator).

#include "ActorDialog.h"
#include "ui_ActorDialog.h"

#include "QtActorDialog.h"   // Qt_ShowActorDialog
#include "QtApp.h"           // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"     // toQ / toCb (CRLF-aware)

#include <QMessageBox>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

ActorDialog::ActorDialog(Actor* pActor, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ActorDialog)
    , _pActor(pActor)
{
    _ui->setupUi(this);

    _ui->lineEditName->setText(toQ(_pActor->GetName()));
    _ui->plainTextNote->setPlainText(toQ(_pActor->GetNote()));

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ActorDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ActorDialog::reject);
}

ActorDialog::~ActorDialog()
{
    delete _ui;
}

// OK -- validate, then apply edits to the model (the MFC Update()).
void ActorDialog::accept()
{
    const QString qName = _ui->lineEditName->text();
    const QString qNote = _ui->plainTextNote->toPlainText();

    if (qName.isEmpty())
    {
        QMessageBox::warning(this, "Actor", "Must give Actor a name");
        return;
    }

    if (toQ(_pActor->GetName()) != qName ||
        toQ(_pActor->GetNote()) != qNote)
    {
        _pActor->SaveState();
        _pActor->SetName(toCb(qName));
        _pActor->SetNote(toCb(qNote));
        _modelChanged = true;
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowActorDialog(Actor* pActor, bool& modelChangedOut, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ActorDialog dlg(pActor);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    return true;
}
