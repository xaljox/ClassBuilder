// qt/AddSerializeDialog.cpp -- the Qt "Add Serialize" dialog.
//
// Ported from the MFC AddSerializeDialog. Validates the document-class name
// directly against the live DataModelDoc (the model is MFC-free, so this Qt
// translation unit can include the model aggregator).

#include "AddSerializeDialog.h"
#include "ui_AddSerializeDialog.h"

#include "QtAddSerializeDialog.h"   // Qt_ShowAddSerializeDialog
#include "QtApp.h"                  // Qt_EnsureApplication / Qt_ExecModal

#include <QMessageBox>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

CbString toCb(const QString& q)
{
    QByteArray bytes = q.toLocal8Bit();
    return CbString(bytes.constData());
}

} // namespace

AddSerializeDialog::AddSerializeDialog(DataModelDoc* pDataModelDoc,
                                       QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::AddSerializeDialog)
    , _pDataModelDoc(pDataModelDoc)
{
    _ui->setupUi(this);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &AddSerializeDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &AddSerializeDialog::reject);
}

AddSerializeDialog::~AddSerializeDialog()
{
    delete _ui;
}

// Validation -- the MFC DDV_ClassName routine, against the live model.
void AddSerializeDialog::accept()
{
    const QString qName = _ui->lineEditClassName->text();

    if (qName.isEmpty())
    {
        QMessageBox::warning(this, "Add Serialize",
            "Must give name of the top (document) class");
        return;
    }
    if (_pDataModelDoc->HasNonCSymbols(toCb(qName)))
    {
        QMessageBox::warning(this, "Add Serialize",
            "Class name contains illegal characters");
        return;
    }
    if (_pDataModelDoc->FindBaseClass(toCb(qName)))
    {
        QMessageBox::warning(this, "Add Serialize",
            "Class name is not unique");
        return;
    }
    if (_pDataModelDoc->FindBaseClass(toCb(qName + "Object")))
    {
        QMessageBox::warning(this, "Add Serialize",
            "Resulting object class name is not unique");
        return;
    }

    // Final confirmation -- adding serialization cannot be undone.
    if (QMessageBox::question(this, "Add Serialize",
            "Are you sure, this action can not be undone")
 != QMessageBox::Yes)
    {
        return;
    }

    _className = qName;
    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowAddSerializeDialog(DataModelDoc* pDataModelDoc,
                               std::string&  classNameOut,
                               void*         ownerHwnd)
{
    Qt_EnsureApplication();

    AddSerializeDialog dlg(pDataModelDoc);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    classNameOut = dlg.className().toLocal8Bit().constData();
    return true;
}
