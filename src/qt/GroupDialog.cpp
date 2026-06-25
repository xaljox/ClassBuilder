// qt/GroupDialog.cpp -- the Qt Group properties dialog.
//
// Ported from the MFC CGroupDialog. Drives the model directly: it holds the
// live Group* and reads / writes it.

#include "GroupDialog.h"
#include "ui_GroupDialog.h"

#include "QtGroupDialog.h"   // Qt_ShowGroupDialog
#include "QtApp.h"           // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"     // toQ / toCb (CRLF-aware)

#include <QMessageBox>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

GroupDialog::GroupDialog(Group* pGroup, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::GroupDialog)
    , _pGroup(pGroup)
{
    _ui->setupUi(this);

    // Title reflects the group kind (the MFC OnInitDialog did this).
    if (_pGroup->IsClassGroup())
        setWindowTitle("Class group");
    else if (_pGroup->IsMetaGroup())
        setWindowTitle("Meta group");
    else
        setWindowTitle("Member and method group");

    _ui->lineEditName->setText(toQ(_pGroup->GetName()));
    _ui->plainTextNote->setPlainText(toQ(_pGroup->GetNote()));

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &GroupDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &GroupDialog::reject);
}

GroupDialog::~GroupDialog()
{
    delete _ui;
}

// OK -- validate, then apply edits to the model (the MFC Update()).
void GroupDialog::accept()
{
    const QString qName = _ui->lineEditName->text();
    const QString qNote = _ui->plainTextNote->toPlainText();

    if (qName.isEmpty())
    {
        QMessageBox::warning(this, "Group", "Must give Group a name");
        return;
    }

    if (toQ(_pGroup->GetName()) != qName ||
        toQ(_pGroup->GetNote()) != qNote)
    {
        _pGroup->SaveState();
        _pGroup->SetName(toCb(qName));
        _pGroup->SetNote(toCb(qNote));
        _modelChanged = true;
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowGroupDialog(Group* pGroup, bool& modelChangedOut, void* ownerHwnd)
{
    Qt_EnsureApplication();

    GroupDialog dlg(pGroup);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    return true;
}
