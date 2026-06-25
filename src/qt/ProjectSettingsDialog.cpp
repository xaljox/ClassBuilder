// qt/ProjectSettingsDialog.cpp -- the Qt Project Settings dialog.
//
// Ported from the MFC ProjectSettingsDialog. Drives the model directly: it
// holds the live DataModelDoc* and reads / writes it.

#include "ProjectSettingsDialog.h"
#include "ui_ProjectSettingsDialog.h"

#include "QtProjectSettingsDialog.h"   // Qt_ShowProjectSettingsDialog
#include "QtApp.h"                     // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"               // toQ / toCb (CRLF-aware)
#include "QtCompact.h"                 // compactCombo

#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

ProjectSettingsDialog::ProjectSettingsDialog(DataModelDoc* pDataModelDoc,
                                             QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ProjectSettingsDialog)
    , _pDataModelDoc(pDataModelDoc)
{
    _ui->setupUi(this);

    _ui->spinUndoStack->setValue(_pDataModelDoc->GetMaxUndoCount());
    _ui->lineEditAdditionalAllowed->setText(
        toQ(_pDataModelDoc->GetAdditionalAllowed()));
    compactCombo(_ui->comboCommentInitialCode);
    _ui->comboCommentInitialCode->setCurrentText(
        toQ(_pDataModelDoc->GetCommentInitialCode()));
    _ui->plainTextMethodNameList->setPlainText(
        toQ(_pDataModelDoc->GetMethodNameList()));
    _ui->plainTextSimilarLinesList->setPlainText(
        toQ(_pDataModelDoc->GetSimilarLinesList()));

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ProjectSettingsDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ProjectSettingsDialog::reject);
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
    delete _ui;
}

// OK -- apply every changed field to the model (the MFC OnOK).
void ProjectSettingsDialog::accept()
{
    const int     undoCount   = _ui->spinUndoStack->value();
    const QString qAdditional = _ui->lineEditAdditionalAllowed->text();
    const QString qComment    = _ui->comboCommentInitialCode->currentText();
    const QString qMethods    = _ui->plainTextMethodNameList->toPlainText();
    const QString qSimilar    = _ui->plainTextSimilarLinesList->toPlainText();

    if (qAdditional != toQ(_pDataModelDoc->GetAdditionalAllowed()) ||
        qMethods != toQ(_pDataModelDoc->GetMethodNameList()) ||
        qSimilar != toQ(_pDataModelDoc->GetSimilarLinesList()) ||
        qComment != toQ(_pDataModelDoc->GetCommentInitialCode())||
        undoCount != _pDataModelDoc->GetMaxUndoCount())
    {
        _pDataModelDoc->SaveState();

        _pDataModelDoc->SetAdditionalAllowed(toCb(qAdditional));
        _pDataModelDoc->SetMethodNameList(toCb(qMethods));
        _pDataModelDoc->SetSimilarLinesList(toCb(qSimilar));
        _pDataModelDoc->SetCommentInitialCode(toCb(qComment));
        _pDataModelDoc->SetMaxUndoCount(undoCount);
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowProjectSettingsDialog(DataModelDoc* pDataModelDoc, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ProjectSettingsDialog dlg(pDataModelDoc);
    Qt_ExecModal(dlg, ownerHwnd);
}
