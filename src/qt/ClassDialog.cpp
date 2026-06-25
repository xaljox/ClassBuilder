// qt/ClassDialog.cpp -- the Qt Class attributes dialog.
//
// Ported from the MFC CClassDialog. Edits a class: name, source / include
// files, template declaration+reference, the property flags and a note.
// Non-live -- read on OK (applyFieldChanges, the MFC ::Update). Drives the
// model directly.

#include "ClassDialog.h"
#include "ui_ClassDialog.h"

#include "QtClassDialog.h"           // Qt_ShowClassDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb

#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

#include "CbViewLock.h"   // coalesce the on-OK setter cascade into one refresh

namespace {

// Whether the Serialize checkbox should be toggleable.
//
// The MFC dialog used two recursive predicates (CheckRelationsToClass /
// CheckRelationsFromClass) that walked outgoing/incoming relations and
// recursed through bases. The recursion through the auto-Inherit to
// DocumentObject then iterated EVERY owned object in the model and
// disabled the checkbox in practically all cases -- so every fresh
// serialize-on class became uncheckable on re-open, and dropping
// Serialize required the workaround of temporarily reparenting the
// class to a non-serialize base.
//
// SetSerialize already cascades the toggle symmetrically (ON adds the
// auto-Inherit to DocumentObject and creates the serialize methods,
// OFF drops them). The only legitimate disable case is a Serialize-on
// SUBCLASS deriving from this class: dropping Serialize here would
// break that subclass's serialize chain (the single-inheritance rule).
//
// BaseClass::InheritIterator walks the Inherits where THIS class is
// the base; each Inherit's ExternClass is the derived class.
bool serializeOnSubclassExists(Class* pClass)
{
    BaseClass::InheritIterator iSub(pClass);
    while (++iSub)
    {
        Class* pDerived = dynamic_cast<Class*>(iSub->GetExternClass());
        if (pDerived && pDerived->GetSerialize())
            return true;
    }
    return false;
}

} // namespace

ClassDialog::ClassDialog(Class* pClass, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ClassDialog)
    , _pClass(pClass)
{
    _ui->setupUi(this);
    _ui->editMemberPrefix->setMaxLength(3);

    QString name = toQ(_pClass->Type::GetName());
    if (name.isEmpty())
        name = toQ(_pClass->GetDataModel()->GetClassPrefix());

    _ui->editName->setText(name);
    _ui->editCppFile->setText(toQ(_pClass->GetCppFile()));
    _ui->editHFile->setText(toQ(_pClass->GetHFile()));
    _ui->editNote->setPlainText(toQ(_pClass->GetNote()));
    _ui->editMemberPrefix->setText(toQ(_pClass->GetMemberPrefix()));
    _ui->editTemplateDecl->setText(toQ(_pClass->GetTemplateDeclaration()));
    _ui->editTemplate->setText(toQ(_pClass->GetTemplate()));

    _ui->checkReplace->setChecked(_pClass->GetReplace());
    _ui->checkDllExport->setChecked(_pClass->GetDllExport());
    _ui->checkSerialize->setChecked(_pClass->GetSerialize());
    _ui->checkStruct->setChecked(_pClass->GetStruct());
    _ui->checkRelationMacrosLast->setChecked(
        _pClass->GetRelationMacrosLast());

    // --- Whether the Serialize checkbox may be toggled -----------------
    // Enabled iff: model has Serialize on, this isn't the document object,
    // and no Serialize-on subclass depends on this class (see helper).
    _serializeEnable = _pClass->GetDataModel()->GetSerialize();
    if (_serializeEnable &&
        _pClass == _pClass->GetDataModel()->GetDocumentObject())
        _serializeEnable = false;
    if (_serializeEnable && serializeOnSubclassExists(_pClass))
        _serializeEnable = false;

    // --- Whether the Template checkbox may be toggled ------------------
    _templateEnable = true;
    Class::FromRelationIterator iFromRelation(_pClass);
    while (_templateEnable && ++iFromRelation)
    {
        if (iFromRelation->GetCritical() || iFromRelation->GetStatic())
            _templateEnable = false;
    }
    Class::ToRelationIterator iToRelation(_pClass);
    while (_templateEnable && ++iToRelation)
    {
        if (iToRelation->GetCritical() || iToRelation->GetStatic())
            _templateEnable = false;
    }

    connect(_ui->editName, &QLineEdit::textEdited,
            this, [this] { onChangeName(); });
    connect(_ui->editCppFile, &QLineEdit::textEdited,
            this, [this] { onChangeCppFile(); });
    connect(_ui->checkSerialize, &QCheckBox::clicked,
            this, [this] { onSerialize(); });
    connect(_ui->checkTemplate, &QCheckBox::clicked,
            this, [this] { onTemplateCheck(); });
    connect(_ui->editTemplateDecl, &QLineEdit::textEdited,
            this, [this] { onChangeTemplateDecl(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ClassDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ClassDialog::reject);

    // --- Initial enable / check state ----------------------------------
    if (!_serializeEnable)
        _ui->checkSerialize->setEnabled(false);
    onSerialize();

    _ui->checkTemplate->setChecked(
        !_pClass->GetTemplateDeclaration().IsEmpty());
    if (!_templateEnable)
        _ui->checkTemplate->setEnabled(false);
    onTemplateCheck();

    // Place the caret just after the class prefix (the MFC OnSetfocusName).
    const int prefixLen =
        toQ(_pClass->GetDataModel()->GetClassPrefix()).length();
    _ui->editName->setFocus();
    _ui->editName->setCursorPosition(prefixLen);
}

ClassDialog::~ClassDialog()
{
    delete _ui;
}

// Editing the name re-derives the source / include file names.
void ClassDialog::onChangeName()
{
    const QString className = _ui->editName->text();
    QString baseName = className;

    const QString prefix = toQ(_pClass->GetDataModel()->GetClassPrefix());
    if (!prefix.isEmpty() && className.startsWith(prefix))
        baseName = className.mid(prefix.length());

    CbString cbBase = toCb(baseName);
    _pClass->GetDataModelDoc()->ConvertNonCSymbols(cbBase);
    baseName = toQ(cbBase);

    _ui->editCppFile->setText(baseName + ".cpp");
    _ui->editHFile->setText(baseName + ".h");
}

// Editing the source file re-derives the include file name.
void ClassDialog::onChangeCppFile()
{
    QString name = _ui->editCppFile->text();
    const int dot = name.lastIndexOf('.');
    if (dot != -1)
        name = name.left(dot);
    _ui->editHFile->setText(name + ".h");
}

// Serialize and Template are mutually exclusive; Serialize forces off Struct.
void ClassDialog::onSerialize()
{
    if (_ui->checkSerialize->isChecked())
    {
        _ui->checkStruct->setChecked(false);
        _ui->checkStruct->setEnabled(false);
        _ui->checkTemplate->setEnabled(false);
    }
    else
    {
        _ui->checkStruct->setEnabled(true);
        _ui->checkTemplate->setEnabled(_templateEnable);
    }
}

// The Template checkbox enables / clears the template fields.
void ClassDialog::onTemplateCheck()
{
    if (_ui->checkTemplate->isChecked())
    {
        _ui->editTemplateDecl->setEnabled(true);
        _ui->editTemplate->setEnabled(true);
        if (_pClass->GetTemplateDeclaration().IsEmpty())
        {
            _ui->editTemplateDecl->setText("template<>");
            _ui->editTemplate->setText("<>");
        }
        else
        {
            _ui->editTemplateDecl->setText(
                toQ(_pClass->GetTemplateDeclaration()));
            _ui->editTemplate->setText(toQ(_pClass->GetTemplate()));
        }
        _ui->editTemplateDecl->setFocus();
        _ui->editTemplateDecl->setCursorPosition(9);
        _ui->checkSerialize->setEnabled(false);
    }
    else
    {
        _ui->editTemplateDecl->clear();
        _ui->editTemplate->clear();
        _ui->editTemplateDecl->setEnabled(false);
        _ui->editTemplate->setEnabled(false);
        _ui->checkSerialize->setEnabled(_serializeEnable);
    }
}

// Derive the template *reference* ("<T, N>") from the *declaration*
// ("template<class T, int N>") -- the MFC OnChangeTemplatedeclaration.
void ClassDialog::onChangeTemplateDecl()
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
            decl = decl.mid(1);
            decl.replace('&', ' ');
            decl.replace('*', ' ');
            decl.replace("const ", "");
            decl.replace("volatile ", "");
            decl.replace("signed ", "");
            decl.replace("unsigned ", "");
            decl = decl.trimmed();

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

// OK -- run the MFC DDV validations, then apply.
void ClassDialog::accept()
{
    const QString name    = _ui->editName->text();
    const QString cppFile = _ui->editCppFile->text();
    const QString hFile   = _ui->editHFile->text();

    // DDV_Name
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Class", "Must give Class a name");
        return;
    }
    if (_pClass->GetDataModelDoc()->HasNonCSymbols(toCb(name)))
    {
        QMessageBox::warning(this, "Class",
                             "Class name contains illegal characters");
        return;
    }
    Type* pTypeFind = _pClass->GetDataModelDoc()->FindType(toCb(name));
    if (pTypeFind && pTypeFind != _pClass)
    {
        QMessageBox::warning(this, "Class", "Class name must be unique");
        return;
    }

    const QString masterInclude = toQ(_pClass->GetDataModel()->GetHFile());

    // DDV_CppFile
    if (cppFile.isEmpty())
    {
        QMessageBox::warning(this, "Class", "Must give an source file");
        return;
    }
    if (cppFile.compare(masterInclude, Qt::CaseInsensitive) == 0)
    {
        QMessageBox::warning(this, "Class",
            "Can not give source file the same name as master include file");
        return;
    }
    // DDV_HFile
    if (hFile.isEmpty())
    {
        QMessageBox::warning(this, "Class", "Must give an include file");
        return;
    }
    if (hFile.compare(masterInclude, Qt::CaseInsensitive) == 0)
    {
        QMessageBox::warning(this, "Class",
            "Can not give include file the same name as master include file");
        return;
    }
    {
        DataModel::ClassIterator iClass(_pClass->GetDataModel());
        while (++iClass)
        {
            if (iClass.Get() == _pClass)
                continue;
            const QString otherCpp = toQ(iClass->GetCppFile());
            const QString otherH   = toQ(iClass->GetHFile());
            if (cppFile.compare(otherCpp, Qt::CaseInsensitive) == 0 ||
                cppFile.compare(otherH,   Qt::CaseInsensitive) == 0)
            {
                QMessageBox::warning(this, "Class",
                    "Must give an unique name to source file");
                return;
            }
            if (hFile.compare(otherCpp, Qt::CaseInsensitive) == 0 ||
                hFile.compare(otherH,   Qt::CaseInsensitive) == 0)
            {
                QMessageBox::warning(this, "Class",
                    "Must give an unique name to include file");
                return;
            }
        }
    }

    _changed = applyFieldChanges();
    QDialog::accept();
}

// Apply the widgets to the model (the MFC CClassDialog::Update).
bool ClassDialog::applyFieldChanges()
{
    const QString name     = _ui->editName->text();
    const QString cppFile  = _ui->editCppFile->text();
    const QString hFile    = _ui->editHFile->text();
    const QString note     = _ui->editNote->toPlainText();
    const QString prefix   = _ui->editMemberPrefix->text();
    const QString tmplDecl = _ui->editTemplateDecl->text();
    const QString tmpl     = _ui->editTemplate->text();
    const bool replace     = _ui->checkReplace->isChecked();
    const bool dllExport   = _ui->checkDllExport->isChecked();
    const bool serialize   = _ui->checkSerialize->isChecked();
    const bool isStruct    = _ui->checkStruct->isChecked();
    const bool macrosLast  = _ui->checkRelationMacrosLast->isChecked();

    if (cppFile != toQ(_pClass->GetCppFile()) ||
        hFile != toQ(_pClass->GetHFile()) ||
        prefix != toQ(_pClass->GetMemberPrefix()) ||
        name != toQ(_pClass->GetName()) ||
        note != toQ(_pClass->GetNote()) ||
        replace != _pClass->GetReplace() ||
        serialize != _pClass->GetSerialize() ||
        isStruct != _pClass->GetStruct() ||
        dllExport != _pClass->GetDllExport() ||
        macrosLast != _pClass->GetRelationMacrosLast() ||
        tmplDecl != toQ(_pClass->GetTemplateDeclaration()) ||
        tmpl != toQ(_pClass->GetTemplate()))
    {
        // Coalesce the setter cascade (each SetX refreshes the tree; SetSerialize
        // also seeds the serialize methods) into a single view refresh.
        CbViewLock lock(_pClass->GetDataModelDoc());

        _pClass->SaveState();

        _pClass->SetCppFile(toCb(cppFile));
        _pClass->SetHFile(toCb(hFile));
        _pClass->SetMemberPrefix(toCb(prefix));
        _pClass->SetName(toCb(name));
        _pClass->SetNote(toCb(note));
        _pClass->SetReplace(replace);
        _pClass->SetDllExport(dllExport);
        _pClass->SetSerialize(serialize);
        _pClass->SetStruct(isStruct);
        _pClass->SetRelationMacrosLast(macrosLast);

        if (tmplDecl != toQ(_pClass->GetTemplateDeclaration()) ||
            tmpl != toQ(_pClass->GetTemplate()))
        {
            if (_pClass->GetFromRelationCount() ||
                _pClass->GetToRelationCount())
            {
                QMessageBox::information(this, "Class",
                    "All template definitions of associated classes "
                    "are updated");
            }
            _pClass->SetTemplate(toCb(tmplDecl), toCb(tmpl));
        }

        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowClassDialog(Class* pClass, bool& changed, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ClassDialog dlg(pClass);
    const bool accepted =
        Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
    changed = accepted && dlg.fieldsChanged();
    return accepted;
}
