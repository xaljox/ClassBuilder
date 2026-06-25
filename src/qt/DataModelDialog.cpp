// qt/DataModelDialog.cpp -- the Qt DataModel properties dialog.
//
// Ported from the MFC CDataModelDialog. Unlike the earlier ported dialogs
// (About / CommentHeader / Search) this one drives the model directly: it
// holds the live DataModel* and reads / writes it. The model is MFC-free, so
// this Qt translation unit can include the whole model aggregator.

#include "DataModelDialog.h"
#include "ui_DataModelDialog.h"

#include "QtDataModelDialog.h"      // Qt_ShowDataModelDialog + result struct
#include "QtCommentHeaderDialog.h"  // Qt_ShowCommentHeaderDialog
#include "QtApp.h"                  // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"            // toQ / toCb (CRLF-aware)

#include <QMessageBox>
#include <QPushButton>

// keeps windows.h's min/max macros away from the Qt headers above.

// FORWARD_ONLY collapses the 3 MFC-GUI view classes to forward-decls -- this
// TU never needs their MFC definitions.
#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

// ---------------------------------------------------------------------------
// CbString <-> QString. The model is a MultiByte (ANSI) build, so round-trip
// through the local 8-bit codepage to match how the MFC side stores text.
// ---------------------------------------------------------------------------
namespace {

// "Update All Headers" callbacks for the Qt comment-header editor. The Qt
// dialog gets these as plain function pointers; userData is the DataModel*.
void UpdateAllCppHeaders(void* userData)
{
    DataModel* pDataModel = static_cast<DataModel*>(userData);
    DataModel::ClassIterator iClass(pDataModel);
    while (++iClass)
        iClass->SetCppHeader("");
}

void UpdateAllHHeaders(void* userData)
{
    DataModel* pDataModel = static_cast<DataModel*>(userData);
    DataModel::ClassIterator iClass(pDataModel);
    while (++iClass)
        iClass->SetHHeader("");
}

} // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
DataModelDialog::DataModelDialog(DataModel* pDataModel,
                                 const QString& classNameIn,
                                 QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::DataModelDialog)
    , _pDataModel(pDataModel)
    , _className(classNameIn)
{
    _ui->setupUi(this);

    // --- controls <- model -------------------------------------------------
    _ui->lineEditName->setText(toQ(_pDataModel->GetName()));
    _ui->lineEditFile->setText(toQ(_pDataModel->GetHFile()));
    _ui->lineEditNamespace->setText(toQ(_pDataModel->GetNamespace()));
    _ui->lineEditClassName->setText(_className);
    _ui->lineEditMemberPrefix->setText(toQ(_pDataModel->GetMemberPrefix()));
    _ui->lineEditClassPrefix->setText(toQ(_pDataModel->GetClassPrefix()));
    _ui->plainTextNote->setPlainText(toQ(_pDataModel->GetNote()));
    _ui->spinIndentSize->setValue(_pDataModel->GetIndentSize());
    _ui->spinVersion->setValue(_pDataModel->GetDataModelDoc()->GetVersion());

    _ui->checkStdAfx->setChecked(_pDataModel->GetStdAfx());
    _ui->checkSerialize->setChecked(_pDataModel->GetSerialize());
    _ui->checkUndoRedo->setChecked(_pDataModel->GetUndoRedo());
    _ui->checkModifiers->setChecked(_pDataModel->GetModifiers());
    _ui->checkPhaseSupport->setChecked(_pDataModel->GetPhaseSupport());
    _ui->checkShowDllExport->setChecked(_pDataModel->GetShowDllExport());
    _ui->checkTemplateHeaderOnly->setChecked(
        _pDataModel->GetTemplateClassHeaderOnly());

    // --- signal wiring -----------------------------------------------------
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &DataModelDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &DataModelDialog::reject);
    connect(_ui->checkSerialize, &QCheckBox::toggled,
            this, &DataModelDialog::onSerializeToggled);
    connect(_ui->lineEditName, &QLineEdit::textChanged,
            this, &DataModelDialog::onNameChanged);
    connect(_ui->buttonCppHeader, &QPushButton::clicked,
            this, &DataModelDialog::onCppHeader);
    connect(_ui->buttonHHeader, &QPushButton::clicked,
            this, &DataModelDialog::onHHeader);
    connect(_ui->buttonCompactVersion, &QPushButton::clicked,
            this, &DataModelDialog::onCompactVersion);

    // --- enable / lock rules (mirrors the MFC OnInitDialog) ----------------
    // An already-named model is an existing one: Serialize is locked and the
    // document class name is fixed.
    const bool existing = !_pDataModel->GetName().IsEmpty();
    if (existing)
    {
        _ui->checkSerialize->setEnabled(false);
        _ui->lineEditClassName->setEnabled(false);
    }

    // Undo/Redo: once on it stays on (turning it off does not remove the
    // Undo/Redo classes from the model) -- so lock it. Otherwise it is only
    // meaningful with Serialize on.
    if (_ui->checkUndoRedo->isChecked())
        _ui->checkUndoRedo->setEnabled(false);

    onSerializeToggled();   // sets className-row visibility + Undo/Redo enable
}

DataModelDialog::~DataModelDialog()
{
    delete _ui;
}

// Records that the dialog has written into the model, and marks the document
// dirty. DELIBERATELY KEPT despite the two-place dirty rule: some of this
// dialog's paths mutate via plain setters with NO SaveState (the version
// compaction loops over SetInitialVersion/SetVersion), so the flag would
// otherwise be lost. The lone surviving manual dirty set on the Qt side.
void DataModelDialog::markModelChanged()
{
    _modelChanged = true;
    _pDataModel->GetDataModelDoc()->SetModifiedFlag();
}

// ---------------------------------------------------------------------------
// Serialize toggled -- show/hide the document-class-name row, gate Undo/Redo
// ---------------------------------------------------------------------------
void DataModelDialog::onSerializeToggled()
{
    const bool on = _ui->checkSerialize->isChecked();

    _ui->labelClassName->setVisible(on);
    _ui->lineEditClassName->setVisible(on);
    if (on)
        _ui->lineEditClassName->setFocus();

    // Undo/Redo only makes sense with Serialize on. Leave it alone when it is
    // already permanently on (locked in the constructor).
    if (_ui->checkUndoRedo->isEnabled() || !_ui->checkUndoRedo->isChecked())
    {
        _ui->checkUndoRedo->setEnabled(on);
        if (!on)
            _ui->checkUndoRedo->setChecked(false);
    }
}

// ---------------------------------------------------------------------------
// Name edited -- auto-fill the master include file as <name>.h
// ---------------------------------------------------------------------------
void DataModelDialog::onNameChanged()
{
    CbString cName = toCb(_ui->lineEditName->text());
    _pDataModel->GetDataModelDoc()->ConvertNonCSymbols(cName);
    _ui->lineEditFile->setText(toQ(cName) + ".h");
}

// ---------------------------------------------------------------------------
// Comment-header editors (the already-Qt CommentHeader dialog)
// ---------------------------------------------------------------------------
void DataModelDialog::onCppHeader()
{
    std::string comment = (const char*)_pDataModel->GetCppHeader();
    if (Qt_ShowCommentHeaderDialog(comment,
            "Template comment header for *.cpp",
            "Are you sure? 'Update All Headers' destroys the current "
            "comment header of all source files.",
            &UpdateAllCppHeaders, _pDataModel,
            reinterpret_cast<void*>(winId())))
    {
        CbString edited(comment.c_str());
        if (edited != _pDataModel->GetCppHeader())
        {
            _pDataModel->SaveState();
            _pDataModel->SetCppHeader(edited);
            markModelChanged();
        }
    }
}

void DataModelDialog::onHHeader()
{
    std::string comment = (const char*)_pDataModel->GetHHeader();
    if (Qt_ShowCommentHeaderDialog(comment,
            "Template comment header for *.h",
            "Are you sure? 'Update All Headers' destroys the current "
            "comment header of all include files.",
            &UpdateAllHHeaders, _pDataModel,
            reinterpret_cast<void*>(winId())))
    {
        CbString edited(comment.c_str());
        if (edited != _pDataModel->GetHHeader())
        {
            _pDataModel->SaveState();
            _pDataModel->SetHHeader(edited);
            markModelChanged();
        }
    }
}

// ---------------------------------------------------------------------------
// Compact Version -- scan Serialize/SerializeRelations bodies for the highest
// version-gated read, propose (max + 1) as the new document version.
// ---------------------------------------------------------------------------
void DataModelDialog::onCompactVersion()
{
    int maxUsed = -1;
    DataModelDoc::GtiIterator iGti(_pDataModel->GetDataModelDoc());
    while (++iGti)
    {
        if (!iGti->IsMethod())
            continue;
        Method* pMethod = (Method*)iGti.operator->();
        const CbString& name = pMethod->GetName();
        if (name != "Serialize" && name != "SerializeRelations")
            continue;

        const CbString& code = pMethod->GetCode();
        const CbString needle = "<= _objectVersion";
        int pos = 0;
        while (pos < code.GetLength())
        {
            int hit = code.Find(needle, pos);
            if (hit < 0)
                break;
            int j = hit - 1;
            while (j >= 0 && code[j] == ' ')
                j--;
            int end = j;
            while (j >= 0 && code[j] >= '0' && code[j] <= '9')
                j--;
            if (end > j)
            {
                CbString num = code.Mid(j + 1, end - j);
                int n = atoi((const char*)num);
                if (n > maxUsed)
                    maxUsed = n;
            }
            pos = hit + needle.GetLength();
        }
    }

    if (maxUsed < 0)
    {
        QMessageBox::information(this, "Compact Version",
            "No version-gated reads found in any "
            "Serialize / SerializeRelations body.");
        return;
    }

    const int version  = _ui->spinVersion->value();
    const int proposed = maxUsed + 1;
    if (proposed >= version)
    {
        QMessageBox::information(this, "Compact Version",
            QString("Highest used: %1\nCurrent Version: %2\n"
                    "Nothing to compact.").arg(maxUsed).arg(version));
        return;
    }

    // Count Gti objects whose _initialVersion is above the proposed version --
    // those get clamped down so the version-gate logic stays consistent.
    int clampCount = 0;
    {
        DataModelDoc::GtiIterator iGti2(_pDataModel->GetDataModelDoc());
        while (++iGti2)
            if (iGti2->GetInitialVersion() > proposed)
                clampCount++;
    }

    const QString msg =
        QString("Highest used: %1\nCurrent Version: %2\n"
                "Set Version to %3?\n"
                "Also clamp _initialVersion on %4 Gti object(s) > %3.")
            .arg(maxUsed).arg(version).arg(proposed).arg(clampCount);
    if (QMessageBox::question(this, "Compact Version", msg) != QMessageBox::Yes)
        return;

    DataModelDoc::GtiIterator iGti3(_pDataModel->GetDataModelDoc());
    while (++iGti3)
        if (iGti3->GetInitialVersion() > proposed)
            iGti3->SetInitialVersion(proposed);

    _ui->spinVersion->setValue(proposed);
    markModelChanged();
}

// ---------------------------------------------------------------------------
// OK -- validate, then apply every edit to the model (the MFC Update()).
// ---------------------------------------------------------------------------
void DataModelDialog::accept()
{
    const QString qName      = _ui->lineEditName->text();
    const QString qFile      = _ui->lineEditFile->text();
    const QString qNamespace = _ui->lineEditNamespace->text();
    const QString qClassName = _ui->lineEditClassName->text();
    DataModelDoc* pDoc       = _pDataModel->GetDataModelDoc();

    // --- validation (the MFC DDV_* routines) -------------------------------
    if (qName.isEmpty())
    {
        QMessageBox::warning(this, "DataModel", "Must give DataModel a name");
        return;
    }
    if (qFile.isEmpty())
    {
        QMessageBox::warning(this, "DataModel",
                             "Must give a master include file");
        return;
    }
    {
        DataModel::ClassIterator iClass(_pDataModel);
        while (++iClass)
        {
            if (qFile.compare(toQ(iClass->GetCppFile()), Qt::CaseInsensitive) == 0 ||
                qFile.compare(toQ(iClass->GetHFile()),   Qt::CaseInsensitive) == 0)
            {
                QMessageBox::warning(this, "DataModel",
                    "Must give an unique name to master include file");
                return;
            }
        }
    }
    if (pDoc->HasNonCSymbols(toCb(qNamespace)))
    {
        QMessageBox::warning(this, "DataModel",
                             "Namespace contains illegal characters");
        return;
    }
    if (_ui->checkSerialize->isChecked())
    {
        if (qClassName.isEmpty())
        {
            QMessageBox::warning(this, "DataModel",
                "Must give name of the top (document) class");
            return;
        }
        if (pDoc->HasNonCSymbols(toCb(qClassName)))
        {
            QMessageBox::warning(this, "DataModel",
                "Class name contains illegal characters");
            return;
        }
    }

    // --- apply: version change --------------------------------------------
    const int  version    = _ui->spinVersion->value();
    const bool serialize  = _ui->checkSerialize->isChecked();
    const bool undoRedo   = _ui->checkUndoRedo->isChecked();
    const bool stdAfx     = _ui->checkStdAfx->isChecked();
    const bool modifiers  = _ui->checkModifiers->isChecked();
    const bool phaseSup   = _ui->checkPhaseSupport->isChecked();
    const bool showDll    = _ui->checkShowDllExport->isChecked();
    const bool tmplHdr    = _ui->checkTemplateHeaderOnly->isChecked();
    const QString qNote   = _ui->plainTextNote->toPlainText();
    const QString qMember = _ui->lineEditMemberPrefix->text();
    const QString qClassP = _ui->lineEditClassPrefix->text();
    const int  indentSize = _ui->spinIndentSize->value();

    if (version != pDoc->GetVersion())
    {
        pDoc->SetVersion(version);
        DataModelDoc::GtiIterator iGti(pDoc);
        while (++iGti)
        {
            if (iGti->GetVersion() >= version)
            {
                iGti->SaveState();
                if (iGti->GetInitialVersion() < version)
                    iGti->SetVersion(version);
                else
                    iGti->SetInitialVersion(version);
            }
        }
        markModelChanged();
    }

    // --- apply: scalar fields ---------------------------------------------
    if (toQ(_pDataModel->GetName()) != qName ||
        toQ(_pDataModel->GetNote()) != qNote ||
        toQ(_pDataModel->GetHFile()) != qFile ||
        toQ(_pDataModel->GetMemberPrefix()) != qMember ||
        toQ(_pDataModel->GetClassPrefix()) != qClassP ||
        _pDataModel->GetStdAfx() != stdAfx ||
        _pDataModel->GetModifiers() != modifiers ||
        toQ(_pDataModel->GetNamespace()) != qNamespace ||
        _pDataModel->GetSerialize() != serialize ||
        _pDataModel->GetUndoRedo() != undoRedo ||
        _pDataModel->GetTemplateClassHeaderOnly() != tmplHdr ||
        _pDataModel->GetIndentSize() != indentSize ||
        _pDataModel->GetShowDllExport() != showDll)
    {
        _pDataModel->SaveState();

        // Newly enabling Serialize on an already-named model adds the
        // serialization scaffolding for the document class.
        if (!_pDataModel->GetName().IsEmpty() && serialize &&
            serialize != _pDataModel->GetSerialize())
        {
            _pDataModel->AddSerialize(toCb(qClassName));
        }

        _pDataModel->SetName(toCb(qName));
        _pDataModel->SetNote(toCb(qNote));
        _pDataModel->SetHFile(toCb(qFile));
        _pDataModel->SetMemberPrefix(toCb(qMember));
        _pDataModel->SetClassPrefix(toCb(qClassP));
        _pDataModel->SetStdAfx(stdAfx);
        _pDataModel->SetModifiers(modifiers);
        _pDataModel->SetNamespace(toCb(qNamespace));
        _pDataModel->SetUndoRedo(undoRedo);
        _pDataModel->SetIndentSize(indentSize);
        _pDataModel->SetSerialize(serialize);
        _pDataModel->SetTemplateClassHeaderOnly(tmplHdr);
        _pDataModel->SetShowDllExport(showDll);

        markModelChanged();
    }

    // --- apply: phase support ---------------------------------------------
    if (phaseSup != _pDataModel->GetPhaseSupport())
    {
        if (!_modelChanged)
            _pDataModel->SaveState(1);

        _pDataModel->SetPhaseSupport(phaseSup);
        DataModelDoc::GtiIterator iGti(pDoc);
        while (++iGti)
        {
            if (iGti->GetPhase() != None_Phase)
                iGti->Gti::Update();
        }
        markModelChanged();
    }

    _className = qClassName;
    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
DataModelDialogResult Qt_ShowDataModelDialog(DataModel*         pDataModel,
                                             const std::string& classNameIn,
                                             void*              ownerHwnd)
{
    Qt_EnsureApplication();

    DataModelDialog dlg(pDataModel,
                        QString::fromLocal8Bit(classNameIn.c_str()));

    DataModelDialogResult result;
    result.accepted     = (Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted);
    // modelChanged is reported even on Cancel: the Compact Version button
    // applies its edits to the model immediately, so a compact-then-Cancel
    // still leaves the model (and hence the document) dirty.
    result.modelChanged = dlg.modelChanged();
    if (result.accepted)
        result.className = dlg.className().toLocal8Bit().constData();
    return result;
}
