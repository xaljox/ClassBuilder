// qt/ExternClassDialog.cpp -- the Qt Extern Class attributes dialog.
//
// Ported from the MFC CExternClassDialog. Edits an extern class: name, struct
// flag, member prefix, template declaration/reference, suppress-forward-
// declaration. Non-live -- read on OK (applyFieldChanges, the MFC ::Update).
// Drives the model directly.

#include "ExternClassDialog.h"
#include "ui_ExternClassDialog.h"

#include "QtExternClassDialog.h"     // Qt_ShowExternClassDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb

#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

ExternClassDialog::ExternClassDialog(ExternClass* pExternClass,
                                     QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ExternClassDialog)
    , _pExternClass(pExternClass)
{
    _ui->setupUi(this);

    _ui->editName->setText(toQ(_pExternClass->Type::GetName()));
    _ui->editMemberPrefix->setText(toQ(_pExternClass->GetMemberPrefix()));
    _ui->checkStruct->setChecked(_pExternClass->GetStruct());
    _ui->checkSuppress->setChecked(
        _pExternClass->GetSuppressForwardDeclaration());
    _ui->editTemplateDecl->setText(
        toQ(_pExternClass->GetTemplateDeclaration()));
    _ui->editTemplate->setText(toQ(_pExternClass->GetTemplate()));

    _ui->checkTemplate->setChecked(
        !_pExternClass->GetTemplateDeclaration().IsEmpty());

    // Suppress-forward is meaningful only when the class has no base classes
    // and every variable of it is a pointer / reference (a forward declaration
    // then suffices).
    bool enableSuppress = false;
    if (_pExternClass->BaseClass::GetInheritCount() == 0 &&
        _pExternClass->GetVariableCount())
    {
        enableSuppress = true;
        Type::VariableIterator iVariable(_pExternClass);
        while (enableSuppress && ++iVariable)
        {
            if (!iVariable->GetPointer() && !iVariable->GetReference())
                enableSuppress = false;
        }
    }
    _ui->checkSuppress->setEnabled(enableSuppress);

    connect(_ui->checkTemplate, &QCheckBox::toggled,
            this, [this] { onTemplateToggled(); });
    connect(_ui->editTemplateDecl, &QLineEdit::textEdited,
            this, [this] { onTemplateDeclChanged(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ExternClassDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ExternClassDialog::reject);

    onTemplateToggled();
}

ExternClassDialog::~ExternClassDialog()
{
    delete _ui;
}

// The "Template" checkbox enables / clears the template fields.
void ExternClassDialog::onTemplateToggled()
{
    if (_ui->checkTemplate->isChecked())
    {
        _ui->editTemplateDecl->setEnabled(true);
        _ui->editTemplate->setEnabled(true);
        if (_pExternClass->GetTemplateDeclaration().IsEmpty())
        {
            _ui->editTemplateDecl->setText("template<>");
            _ui->editTemplate->setText("<>");
        }
        else
        {
            _ui->editTemplateDecl->setText(
                toQ(_pExternClass->GetTemplateDeclaration()));
            _ui->editTemplate->setText(toQ(_pExternClass->GetTemplate()));
        }
        _ui->editTemplateDecl->setFocus();
    }
    else
    {
        _ui->editTemplateDecl->clear();
        _ui->editTemplate->clear();
        _ui->editTemplateDecl->setEnabled(false);
        _ui->editTemplate->setEnabled(false);
    }
}

// Derive the template *reference* ("<T, N>") from the *declaration*
// ("template<class T, int N>") -- the MFC OnChangeTemplatedeclaration.
void ExternClassDialog::onTemplateDeclChanged()
{
    CbString stripped(_ui->editTemplateDecl->text().trimmed()
                          .toLocal8Bit().constData());
    BaseClass::StripTemplateDeclaration(stripped);
    QString decl = toQ(stripped);

    QString ref;
    if (decl.startsWith("template"))
    {
        decl = decl.mid(8).trimmed();
        if (decl.startsWith("<"))
        {
            ref = "<";
            decl = decl.mid(1).trimmed();

            int start = decl.indexOf(' ');
            while (start != -1)
            {
                decl = decl.mid(start).trimmed();

                const int stop = decl.indexOf(',');
                if (stop != -1)
                {
                    ref += decl;
                    decl = decl.mid(stop + 1).trimmed();
                    ref = ref.left(ref.length() - decl.length());
                }
                else
                {
                    ref += decl;
                    decl.clear();
                }
                start = decl.indexOf(' ');
            }

            if (!ref.endsWith('>'))
                ref += ">";
        }
    }

    _ui->editTemplate->setText(ref);
}

// OK -- validate the name (the MFC DDV_Name), then apply.
void ExternClassDialog::accept()
{
    const QString name = _ui->editName->text();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Extern Class",
                             "Must give Class a name");
        return;
    }

    // Strip embedded "::" (but keep a trailing one) for the symbol check.
    QString bare = name;
    int index = bare.indexOf("::");
    while (index != -1 && index + 2 != bare.length())
    {
        bare = bare.left(index) + bare.mid(index + 2);
        index = bare.indexOf("::");
    }
    if (_pExternClass->GetDataModelDoc()->HasNonCSymbols(toCb(bare)))
    {
        QMessageBox::warning(this, "Extern Class",
                             "Class name contains illegal characters");
        return;
    }

    Type* pFind = _pExternClass->GetDataModelDoc()->FindType(toCb(name));
    if (pFind && pFind != _pExternClass)
    {
        QMessageBox::warning(this, "Extern Class",
                             "Class name must be unique");
        return;
    }

    _changed = applyFieldChanges();
    QDialog::accept();
}

// Apply the widgets to the model (the MFC CExternClassDialog::Update).
bool ExternClassDialog::applyFieldChanges()
{
    const QString name     = _ui->editName->text();
    const QString prefix   = _ui->editMemberPrefix->text();
    const QString tmplDecl = _ui->editTemplateDecl->text();
    const QString tmpl     = _ui->editTemplate->text();
    const bool isStruct    = _ui->checkStruct->isChecked();
    const bool suppress    = _ui->checkSuppress->isChecked();

    if (toQ(_pExternClass->GetMemberPrefix()) != prefix ||
        toQ(_pExternClass->GetName()) != name ||
        _pExternClass->GetStruct() != isStruct ||
        _pExternClass->GetSuppressForwardDeclaration() != suppress ||
        toQ(_pExternClass->GetTemplateDeclaration()) != tmplDecl ||
        toQ(_pExternClass->GetTemplate()) != tmpl)
    {
        _pExternClass->SaveState();
        _pExternClass->SetMemberPrefix(toCb(prefix));
        _pExternClass->SetName(toCb(name));
        _pExternClass->SetStruct(isStruct);
        _pExternClass->SetSuppressForwardDeclaration(suppress);
        _pExternClass->SetTemplateDeclaration(toCb(tmplDecl));
        _pExternClass->SetTemplate(toCb(tmpl));
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowExternClassDialog(ExternClass* pExternClass, bool& changed,
                              void* ownerHwnd)
{
    Qt_EnsureApplication();

    ExternClassDialog dlg(pExternClass);
    const bool accepted =
        Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
    changed = accepted && dlg.fieldsChanged();
    return accepted;
}
