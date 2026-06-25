// qt/ReadSourceDialog.cpp -- the Qt "Read source files" dialog.
//
// Ported from the MFC CReadSourceDialog. The dialog implements
// ParseLogInterface; the model's reader (DataModel::ReadAllFiles) and the
// flex/yacc parser call AddLog* / StepProgress back into it during the
// (synchronous) read. Modifications are read automatically on open.

#include "ReadSourceDialog.h"
#include "ui_ReadSourceDialog.h"

#include "QtReadSourceDialog.h"      // Qt_ShowReadSourceDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal

#include <QCoreApplication>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QTimer>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

ReadSourceDialog::ReadSourceDialog(DataModel* pDataModel, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ReadSourceDialog)
    , _pDataModel(pDataModel)
{
    _ui->setupUi(this);

    _ui->progressBar->setRange(0, _pDataModel->GetClassCount() + 1);
    _ui->progressBar->setValue(0);

    connect(_ui->buttonRead, &QPushButton::clicked,
            this, [this] { runRead(false); });
    connect(_ui->buttonReadAll, &QPushButton::clicked,
            this, [this] { runRead(true); });
    connect(_ui->buttonClose, &QPushButton::clicked,
            this, &QDialog::accept);

    // The MFC dialog auto-read modifications on show -- do the same once the
    // dialog is up (singleShot fires when the event loop starts running).
    QTimer::singleShot(0, this, [this] { runRead(false); });
}

ReadSourceDialog::~ReadSourceDialog()
{
    delete _ui;
}

// --- ParseLogInterface -----------------------------------------------------
// The read runs synchronously on the GUI thread; processEvents keeps the log
// and progress bar painting.

void ReadSourceDialog::AddLog(const char* log)
{
    _ui->editLog->appendPlainText(QString::fromLocal8Bit(log));
    QCoreApplication::processEvents();
}

void ReadSourceDialog::AddLogError(const char* log)
{
    AddLog(log);
    _error = true;
}

void ReadSourceDialog::AddLogWarning(const char* log)
{
    AddLog(log);
    _warning = true;
}

void ReadSourceDialog::StepProgress()
{
    _ui->progressBar->setValue(_ui->progressBar->value() + 1);
    QCoreApplication::processEvents();
}

// --- Read ------------------------------------------------------------------
void ReadSourceDialog::runRead(bool readAll)
{
    _ui->editLog->clear();
    _error   = false;
    _warning = false;
    _ui->progressBar->setValue(0);

    setCursor(Qt::WaitCursor);
    _pDataModel->ReadAllFiles(this, readAll);
    unsetCursor();

    if (_error && _warning)
        QMessageBox::warning(this, "Read source files",
            "Errors and warnings have occured during reading source, "
            "see log transscript");
    else if (_error)
        QMessageBox::warning(this, "Read source files",
            "Errors have occured during reading source, see log transscript");
    else if (_warning)
        QMessageBox::warning(this, "Read source files",
            "Warnings have occured during reading source, "
            "see log transscript");
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowReadSourceDialog(DataModel* pDataModel, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ReadSourceDialog dlg(pDataModel);
    Qt_ExecModal(dlg, ownerHwnd);
}
