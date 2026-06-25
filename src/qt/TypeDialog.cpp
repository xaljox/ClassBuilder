// qt/TypeDialog.cpp -- the Qt Type (OtherType) properties dialog.
//
// Ported from the MFC CTypeDialog. Drives the model directly: it holds the
// live OtherType* and reads / writes it.

#include "TypeDialog.h"
#include "ui_TypeDialog.h"

#include "QtTypeDialog.h"    // Qt_ShowTypeDialog
#include "QtApp.h"           // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"     // toQ / toCb (CRLF-aware)

#include <QMessageBox>
#include <QDialogButtonBox>
#include <QFontDatabase>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

TypeDialog::TypeDialog(OtherType* pOtherType, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::TypeDialog)
    , _pOtherType(pOtherType)
{
    _ui->setupUi(this);

    _ui->lineEditName->setText(toQ(_pOtherType->GetName()));
    _ui->plainTextDeclaration->setPlainText(toQ(_pOtherType->GetDeclaration()));

    // Serialize-map radio: 0 = None, 1 = Int (the MFC DDX_Radio index).
    if (_pOtherType->GetSerializeMap() == 1)
        _ui->radioIntMap->setChecked(true);
    else
        _ui->radioNoneMap->setChecked(true);

    // The declaration is C++ source -- show it in a fixed-pitch font (the MFC
    // dialog used 10pt Courier).
    _ui->plainTextDeclaration->setFont(
        QFontDatabase::systemFont(QFontDatabase::FixedFont));

    // A pending (unnamed) type that is not the document's current "last type"
    // is not editable -- mirrors the MFC OnInitDialog.
    DataModelDoc* pDoc = _pOtherType->GetDataModelDoc();
    if (_pOtherType->GetName().IsEmpty() &&
        pDoc->GetLastType() != _pOtherType)
    {
        _ui->lineEditName->setEnabled(false);
        _ui->plainTextDeclaration->setEnabled(false);
    }

    // Serialize mapping only applies when the model has serialization on.
    if (!pDoc->GetDataModel()->GetSerialize())
    {
        _ui->radioNoneMap->setEnabled(false);
        _ui->radioIntMap->setEnabled(false);
    }

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &TypeDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &TypeDialog::reject);
}

TypeDialog::~TypeDialog()
{
    delete _ui;
}

// OK -- validate, then apply edits to the model (the MFC Update()).
void TypeDialog::accept()
{
    const QString qName        = _ui->lineEditName->text();
    const QString qDeclaration = _ui->plainTextDeclaration->toPlainText();
    const bool    serializeMap = _ui->radioIntMap->isChecked();

    // Validation (the MFC DDV_Name).
    if (qName.isEmpty())
    {
        QMessageBox::warning(this, "Type", "Must give type a name");
        return;
    }
    Type* pFound = _pOtherType->GetDataModelDoc()->FindType(toCb(qName));
    if (pFound && pFound != _pOtherType)
    {
        QMessageBox::warning(this, "Type", "Type name must be unique");
        return;
    }

    if (toQ(_pOtherType->GetName()) != qName ||
        toQ(_pOtherType->GetDeclaration()) != qDeclaration ||
        _pOtherType->GetSerializeMap() != serializeMap)
    {
        _pOtherType->SaveState();
        // SetDeclaration before SetName -- SetName triggers the automatic
        // _declaration update, so the declaration must already be in place.
        _pOtherType->SetDeclaration(toCb(qDeclaration));
        _pOtherType->SetName(toCb(qName));
        _pOtherType->SetSerializeMap(serializeMap);
        _modelChanged = true;
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowTypeDialog(OtherType* pOtherType, bool& modelChangedOut,
                       void* ownerHwnd)
{
    Qt_EnsureApplication();

    TypeDialog dlg(pOtherType);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    return true;
}
