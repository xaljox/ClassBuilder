// qt/TypeVariableDialog.cpp -- the Qt "Type Variable Wizard" dialog.
//
// Ported from the MFC TypeVariableDialog. Builds a variable-declaration
// snippet from a model type + name + const/pointer/reference/array; the caller
// (a code-edit dialog) splices the result into the code being edited.
//
// NOTE: the MFC original had the Const and Array checkboxes but never folded
// them into the inserted snippet (always just `type[*][&] name;`) -- an
// unfinished feature. The Qt port completes it: `const` prefixes the type and
// `[size]` is appended after the name.

#include "TypeVariableDialog.h"
#include "ui_TypeVariableDialog.h"

#include "QtTypeVariableDialog.h"   // Qt_ShowTypeVariableDialog
#include "QtApp.h"                  // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"            // toQ / toCb
#include "QtCompact.h"              // compactCombo
#include "QtComboHelpers.h"         // Qt_MakeSearchableCombo

#include <QComboBox>
#include <QMessageBox>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

TypeVariableDialog::TypeVariableDialog(DataModelDoc* pDataModelDoc,
                                       QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::TypeVariableDialog)
    , _pDataModelDoc(pDataModelDoc)
{
    _ui->setupUi(this);

    // Populate the type combo with the model's named types.
    DataModelDoc::TypeIterator type(_pDataModelDoc);
    while (++type)
    {
        const CbString& name = type->GetName();
        if (name != "" && name != "...")
            _ui->comboType->addItem(toQ(name));
    }
    _ui->comboType->model()->sort(0);   // CBS_SORT
    compactCombo(_ui->comboType);
    Qt_MakeSearchableCombo(_ui->comboType);
    _ui->comboType->setCurrentIndex(-1);

    connect(_ui->comboType, &QComboBox::currentTextChanged,
            this, &TypeVariableDialog::autoFillName);
    connect(_ui->checkPointer, &QCheckBox::toggled,
            this, &TypeVariableDialog::autoFillName);
    connect(_ui->checkReference, &QCheckBox::toggled,
            this, &TypeVariableDialog::autoFillName);
    connect(_ui->checkArray, &QCheckBox::toggled,
            this, &TypeVariableDialog::onArrayToggled);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &TypeVariableDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &TypeVariableDialog::reject);

    onArrayToggled();   // array-size field starts hidden unless Array is on
}

TypeVariableDialog::~TypeVariableDialog()
{
    delete _ui;
}

// When the name is still empty and pointer/reference is set, suggest a name
// from the Hungarian prefix + the capitalised type (the MFC OnUpdateName).
void TypeVariableDialog::autoFillName()
{
    if (!_ui->lineEditName->text().isEmpty())
        return;

    const bool pointer   = _ui->checkPointer->isChecked();
    const bool reference = _ui->checkReference->isChecked();
    if (!pointer && !reference)
        return;

    QString name;
    if (pointer)
        name += "p";
    if (reference)
        name += "r";

    QString type = _ui->comboType->currentText();
    if (!type.isEmpty())
        name += type.left(1).toUpper() + type.mid(1);

    const int lt = name.indexOf('<');     // drop any template arguments
    if (lt != -1)
        name = name.left(lt);

    _ui->lineEditName->setText(name);
}

void TypeVariableDialog::onArrayToggled()
{
    _ui->spinArraySize->setVisible(_ui->checkArray->isChecked());
}

// OK -- validate, then build the declaration snippet:
//   [const ]type[*][&] name[ [size] ];
void TypeVariableDialog::accept()
{
    const QString type = _ui->comboType->currentText();
    const QString name = _ui->lineEditName->text();

    if (type.isEmpty())
    {
        QMessageBox::warning(this, "Type Variable",
                             "Must select type of argument");
        return;
    }
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Type Variable",
                             "Must give variable a name");
        return;
    }
    if (_pDataModelDoc->HasNonCSymbols(toCb(name)))
    {
        QMessageBox::warning(this, "Type Variable",
                             "Variable name contains illegal characters");
        return;
    }

    _insertCode.clear();
    if (_ui->checkConst->isChecked())
        _insertCode += "const ";
    _insertCode += type;
    if (_ui->checkPointer->isChecked())
        _insertCode += "*";
    if (_ui->checkReference->isChecked())
        _insertCode += "&";
    _insertCode += " " + name;
    if (_ui->checkArray->isChecked())
        _insertCode += "[" + QString::number(_ui->spinArraySize->value()) + "]";
    _insertCode += ";";

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowTypeVariableDialog(DataModelDoc* pDataModelDoc,
                               std::string&  insertCodeOut,
                               void*         ownerHwnd)
{
    Qt_EnsureApplication();

    TypeVariableDialog dlg(pDataModelDoc);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    insertCodeOut = dlg.insertCode().toLocal8Bit().constData();
    return true;
}
