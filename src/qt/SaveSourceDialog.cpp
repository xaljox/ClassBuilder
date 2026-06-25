// qt/SaveSourceDialog.cpp -- the Qt "Save source files" dialog.
//
// Ported from the MFC CSaveSourceDialog. The dialog implements
// SourceLogInterface; the model's codegen (SaveAllFiles / SaveModifiedFiles)
// calls AddLog* / StepProgress back into it during the (synchronous) save.

#include "SaveSourceDialog.h"
#include "ui_SaveSourceDialog.h"

#include "QtSaveSourceDialog.h"      // Qt_ShowSaveSourceDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb

#include <QCoreApplication>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QMessageBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

SaveSourceDialog::SaveSourceDialog(DataModel* pDataModel, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::SaveSourceDialog)
    , _pDataModel(pDataModel)
{
    _ui->setupUi(this);

    _ui->editAuthor->setText(toQ(_pDataModel->GetAuthor()));
    _ui->progressBar->setRange(0, _pDataModel->GetClassCount() + 1);
    _ui->progressBar->setValue(0);

    connect(_ui->buttonSaveAll, &QPushButton::clicked,
            this, [this] { runSave(false); });
    connect(_ui->buttonSaveModifications, &QPushButton::clicked,
            this, [this] { runSave(true); });
    connect(_ui->buttonClose, &QPushButton::clicked,
            this, &QDialog::accept);
}

SaveSourceDialog::~SaveSourceDialog()
{
    delete _ui;
}

// --- SourceLogInterface ----------------------------------------------------
// The save runs synchronously on the GUI thread; processEvents keeps the log
// and progress bar painting (the MFC dialog used LockWindowUpdate/UpdateWindow).

void SaveSourceDialog::AddLog(const CbString& log)
{
    _ui->editLog->appendPlainText(toQ(log));
    QCoreApplication::processEvents();
}

void SaveSourceDialog::AddLogError(const CbString& log)
{
    AddLog(log);
    _error = true;
}

void SaveSourceDialog::AddLogWarning(const CbString& log)
{
    AddLog(log);
    _warning = true;
}

void SaveSourceDialog::StepProgress()
{
    _ui->progressBar->setValue(_ui->progressBar->value() + 1);
    QCoreApplication::processEvents();
}

// --- Save ------------------------------------------------------------------
void SaveSourceDialog::runSave(bool modifiedOnly)
{
    const QString author = _ui->editAuthor->text();
    if (author.isEmpty())
    {
        QMessageBox::warning(this, "Save source files",
                             "Must fill in the author and/or note");
        return;
    }

    _pDataModel->SetAuthor(toCb(author));
    _ui->editLog->clear();
    _error   = false;
    _warning = false;
    _ui->progressBar->setValue(0);

    setCursor(Qt::WaitCursor);
    if (modifiedOnly)
        _pDataModel->SaveModifiedFiles(this);
    else
        _pDataModel->SaveAllFiles(this);
    unsetCursor();

    if (_error && _warning)
        QMessageBox::warning(this, "Save source files",
            "Errors and warnings have occured during writing source, "
            "see log transscript");
    else if (_error)
        QMessageBox::warning(this, "Save source files",
            "Errors have occured during writing source, see log transscript");
    else if (_warning)
        QMessageBox::warning(this, "Save source files",
            "Warnings have occured during writing source, "
            "see log transscript");
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowSaveSourceDialog(DataModel* pDataModel, void* ownerHwnd)
{
    Qt_EnsureApplication();

    SaveSourceDialog dlg(pDataModel);
    Qt_ExecModal(dlg, ownerHwnd);
}
