// qt/MethodDialog.cpp -- the Qt Method attributes dialog.
//
// Ported from the MFC CMethodDialog. Edits a method: return type, name,
// template reference, the type-property / access / property flags, calling
// convention and a note. Non-live -- read on OK (applyFieldChanges, the MFC
// ::Update, which also propagates the change to virtual overrides in derived
// classes). Drives the model directly.

#include "MethodDialog.h"
#include "ui_MethodDialog.h"

#include "QtMethodDialog.h"          // Qt_ShowMethodDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "QtCompact.h"               // compactCombo
#include "QtComboHelpers.h"          // Qt_MakeSearchableCombo

#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QMessageBox>

#include <algorithm>
#include <vector>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

MethodDialog::MethodDialog(Method* pMethod, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::MethodDialog)
    , _pMethod(pMethod)
{
    _ui->setupUi(this);

    setWindowTitle(toQ("Method of class " +
        _pMethod->GetBaseClass()->GetName()));

    // --- Name combo: the model's method-name list ----------------------
    const QString nameList = toQ(_pMethod->GetDataModelDoc()
                                     ->GetMethodNameList());
    for (const QString& name : nameList.split('\n', Qt::SkipEmptyParts))
        _ui->comboName->addItem(name);
    // Compact BEFORE setting the edit text: compacting changes item data,
    // which on an editable combo re-syncs the line edit to the current item.
    compactCombo(_ui->comboName);
    Qt_MakeSearchableCombo(_ui->comboName);
    _ui->comboName->setEditText(toQ(_pMethod->GetName()));

    // --- Type combo: every type except "..." (sorted) ------------------
    {
        std::vector<std::pair<QString, Type*>> types;
        DataModelDoc::TypeIterator type(_pMethod->GetDataModelDoc());
        while (++type)
        {
            if (type->GetName() != "...")
                types.emplace_back(toQ(type->GetName()), type.Get());
        }
        std::sort(types.begin(), types.end(),
            [](const std::pair<QString, Type*>& a,
               const std::pair<QString, Type*>& b)
            { return a.first < b.first; });
        for (const auto& t : types)
        {
            _ui->comboType->addItem(t.first,
                QVariant::fromValue(static_cast<void*>(t.second)));
            if (t.second == _pMethod->GetType())
                _ui->comboType->setCurrentIndex(
                    _ui->comboType->count() - 1);
        }
    }

    compactCombo(_ui->comboType);
    Qt_MakeSearchableCombo(_ui->comboType);

    _ui->editTemplate->setText(toQ(_pMethod->GetTemplate()));
    _ui->editArraySize->setText(toQ(_pMethod->GetArraySizeStr()));
    _ui->editNote->setPlainText(toQ(_pMethod->GetNote()));
    compactCombo(_ui->comboCallingConvention);
    _ui->comboCallingConvention->setEditText(
        toQ(_pMethod->GetCallingConvention()));

    // --- Initial flag state --------------------------------------------
    switch (_pMethod->GetAccess())
    {
    case PROTECTED: _ui->radioProtected->setChecked(true); break;
    case PRIVATE:   _ui->radioPrivate->setChecked(true);   break;
    default:        _ui->radioPublic->setChecked(true);    break;
    }
    _ui->checkArray->setChecked(_pMethod->GetArray());
    _ui->checkConst->setChecked(_pMethod->Variable::GetConst());
    _ui->checkConstMethod->setChecked(_pMethod->GetConst());
    _ui->checkPointer->setChecked(_pMethod->GetPointer());
    _ui->checkReference->setChecked(_pMethod->GetReference());
    _ui->checkStatic->setChecked(_pMethod->GetStatic());
    _ui->checkPure->setChecked(_pMethod->GetPure());
    _ui->checkVirtual->setChecked(_pMethod->GetVirtual());
    _ui->checkInline->setChecked(_pMethod->GetInline());
    _ui->checkPointerPointer->setChecked(
        _pMethod->GetPointerPointer());
    _ui->checkConstPointer->setChecked(_pMethod->GetConstPointer());
    _ui->checkDllExport->setChecked(_pMethod->GetDllExport());
    _ui->checkDeclare->setChecked(_pMethod->GetDeclare());
    _ui->checkImplement->setChecked(_pMethod->GetImplement());
    _ui->checkDelete->setChecked(_pMethod->GetDelete());

    // Array is not applicable to methods -- hidden, as in the MFC dialog.
    _ui->checkArray->setVisible(false);
    _ui->editArraySize->setVisible(false);

    if (!_pMethod->GetBaseClass()->IsClass())
    {
        _ui->radioPrivate->setEnabled(false);
        _ui->checkDeclare->setEnabled(false);
        _ui->checkImplement->setEnabled(false);
        _ui->checkDelete->setEnabled(false);
    }

    if (_pMethod->IsFixed())
    {
        _ui->checkArray->setEnabled(false);
        _ui->editArraySize->setEnabled(false);
        _ui->checkConst->setEnabled(false);
        _ui->comboName->setEnabled(false);
        _ui->checkPointer->setEnabled(false);
        _ui->checkReference->setEnabled(false);
        _ui->comboType->setEnabled(false);
        _ui->checkStatic->setEnabled(false);
        _ui->checkPure->setEnabled(false);
        _ui->checkVirtual->setEnabled(false);
        _ui->editTemplate->setEnabled(false);
        _ui->checkDeclare->setEnabled(false);
        _ui->checkImplement->setEnabled(false);
        _ui->checkDelete->setEnabled(false);

        // An IsClassMethod may still be made const or not.
        if (!_pMethod->IsIsClassMethod())
            _ui->checkConstMethod->setEnabled(false);
    }

    if (_pMethod->IsGetMemberMethod())
    {
        _ui->editTemplate->setEnabled(false);
        _ui->comboType->setEnabled(false);
        _ui->checkStatic->setEnabled(false);
    }
    if (_pMethod->IsSetMemberMethod())
    {
        _ui->checkStatic->setEnabled(false);
    }

    connect(_ui->checkArray, &QCheckBox::clicked,
            this, [this] { onArray(); });
    connect(_ui->checkVirtual, &QCheckBox::clicked,
            this, [this] { onVirtual(); });
    connect(_ui->checkStatic, &QCheckBox::clicked,
            this, [this] { onStatic(); });
    connect(_ui->checkPointer, &QCheckBox::clicked,
            this, [this] { onPointer(); });
    connect(_ui->checkPure, &QCheckBox::clicked,
            this, [this] { onPure(); });
    connect(_ui->checkImplement, &QCheckBox::clicked,
            this, [this] { onImplement(); });
    connect(_ui->checkDelete, &QCheckBox::clicked,
            this, [this] { onDelete(); });
    connect(_ui->comboType, &QComboBox::currentIndexChanged,
            this, [this] { onSelchangeType(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &MethodDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &MethodDialog::reject);

    onArray();
    onVirtual();
    onPure();
    onStatic();
    onPointer();
    onImplement();
    onSelchangeType();
    _ui->checkImplement->setChecked(_pMethod->GetImplement());
    onDelete();   // final say: a deleted method forces Implement off + disabled
}

MethodDialog::~MethodDialog()
{
    delete _ui;
}

void MethodDialog::onArray()
{
    if (_ui->checkArray->isChecked())
    {
        _ui->editArraySize->setEnabled(true);
        _ui->editArraySize->setVisible(true);
        _ui->editArraySize->setFocus();
    }
    else
    {
        _ui->editArraySize->setVisible(false);
    }
}

void MethodDialog::onVirtual()
{
    if (_ui->checkVirtual->isChecked())
    {
        _ui->checkStatic->setChecked(false);
        _ui->checkStatic->setEnabled(false);
        _ui->checkPure->setEnabled(true);
    }
    else
    {
        if (_pMethod->IsFixed() ||
            _pMethod->IsGetMemberMethod() ||
            _pMethod->IsSetMemberMethod())
            _ui->checkStatic->setEnabled(false);
        else
            _ui->checkStatic->setEnabled(true);
        _ui->checkPure->setChecked(false);
        _ui->checkPure->setEnabled(false);
    }
    onPure();
}

void MethodDialog::onPure()
{
    if (_ui->checkPure->isChecked())
    {
        // Pure and "= delete" are mutually exclusive definition states.
        _ui->checkDelete->setChecked(false);
        _ui->checkImplement->setChecked(false);
        _ui->checkInline->setChecked(false);
        _ui->checkInline->setEnabled(false);
    }
    else if (_pMethod->GetBaseClass()->IsClass() &&
             !_ui->checkDelete->isChecked())
    {
        _ui->checkImplement->setEnabled(true);
        _ui->checkImplement->setChecked(
            _pMethod->GetImplement() || _pMethod->GetPure());
        _ui->checkInline->setEnabled(true);
        _ui->checkInline->setChecked(_pMethod->GetInline());
    }
}

void MethodDialog::onStatic()
{
    if (_ui->checkStatic->isChecked())
    {
        _ui->checkVirtual->setChecked(false);
        _ui->checkPure->setChecked(false);
        _ui->checkConstMethod->setChecked(false);
        _ui->checkVirtual->setEnabled(false);
        _ui->checkPure->setEnabled(false);
        _ui->checkConstMethod->setEnabled(false);
    }
    else
    {
        _ui->checkVirtual->setEnabled(true);
        _ui->checkConstMethod->setEnabled(true);
    }
}

void MethodDialog::onPointer()
{
    if (_ui->checkPointer->isChecked())
    {
        _ui->checkPointerPointer->setEnabled(true);
        _ui->checkConstPointer->setEnabled(true);
    }
    else
    {
        _ui->checkPointerPointer->setEnabled(false);
        _ui->checkPointerPointer->setChecked(false);
        _ui->checkConstPointer->setEnabled(false);
        _ui->checkConstPointer->setChecked(false);
    }
}

void MethodDialog::onSelchangeType()
{
    const int sel = _ui->comboType->currentIndex();
    Type* pType = sel >= 0 ? static_cast<Type*>(
        _ui->comboType->itemData(sel).value<void*>()) : nullptr;
    if (pType)
    {
        if (pType->GetTemplate().IsEmpty())
        {
            _ui->editTemplate->clear();
            _ui->editTemplate->setEnabled(false);
        }
        else
        {
            _ui->editTemplate->setEnabled(true);
            if (pType->GetName() == _pMethod->GetType()->GetName())
                _ui->editTemplate->setText(toQ(_pMethod->GetTemplate()));
            else
                _ui->editTemplate->setText(toQ(pType->GetTemplate()));
        }
    }
    else
    {
        _ui->editTemplate->clear();
        _ui->editTemplate->setEnabled(false);
    }

    if (_pMethod->IsGetMemberMethod())
        _ui->editTemplate->setEnabled(false);
}

void MethodDialog::onImplement()
{
    if (_ui->checkImplement->isChecked() &&
        _pMethod->GetBaseClass()->IsClass())
    {
        _ui->checkInline->setEnabled(true);
        _ui->checkInline->setChecked(_pMethod->GetInline());
    }
    else
    {
        _ui->checkInline->setChecked(false);
        _ui->checkInline->setEnabled(false);
    }
}

// A "= delete"d method has no definition: force Implement off and lock it.
// Declare stays user-toggleable (so the declaration itself can be suppressed
// for a debug compile). Unchecking restores Implement's normal availability,
// deferring to onPure for the pure-virtual case.
void MethodDialog::onDelete()
{
    if (_ui->checkDelete->isChecked())
    {
        // Pure and "= delete" are mutually exclusive definition states.
        _ui->checkPure->setChecked(false);
        _ui->checkImplement->setChecked(false);
        _ui->checkImplement->setEnabled(false);
    }
    else if (_pMethod->GetBaseClass()->IsClass() &&
             !_ui->checkPure->isChecked())
    {
        _ui->checkImplement->setEnabled(true);
        _ui->checkImplement->setChecked(_pMethod->GetImplement());
    }
    onImplement();   // sync Inline off the resulting Implement state
}

// OK -- run the MFC DDV validations, then apply.
void MethodDialog::accept()
{
    const QString type = _ui->comboType->currentText();
    const QString name = _ui->comboName->currentText();

    // DDV_Type
    if (type.isEmpty() && !name.startsWith("operator "))
    {
        QMessageBox::warning(this, "Method", "Must select type of method");
        return;
    }
    // DDV_Name
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Method", "Must give method a name");
        return;
    }
    if (_pMethod->GetDataModelDoc()->HasNonCSymbols(toCb(name)) &&
        !name.startsWith("operator"))
    {
        QMessageBox::warning(this, "Method",
                             "Method name contains illegal characters");
        return;
    }
    // The type combo is editable, so currentText() may not resolve to a known
    // Type. applyFieldChanges would then FindType()->null and crash on
    // MoveVariableLast; reject it here with a warning (mirrors the pipe path).
    if (!type.isEmpty() &&
        !_pMethod->GetDataModelDoc()->FindType(toCb(type)) &&
        !name.startsWith("operator"))
    {
        QMessageBox::warning(this, "Method",
                             "Unknown return type '" + type + "'");
        return;
    }

    _changed = applyFieldChanges();
    QDialog::accept();
}

// Apply the widgets to the model (the MFC CMethodDialog::Update). Also
// propagates the change to virtual overrides in derived classes.
bool MethodDialog::applyFieldChanges()
{
    int access = PUBLIC;
    if (_ui->radioProtected->isChecked()) access = PROTECTED;
    else if (_ui->radioPrivate->isChecked()) access = PRIVATE;

    const QString type      = _ui->comboType->currentText();
    const QString name      = _ui->comboName->currentText();
    const QString note      = _ui->editNote->toPlainText();
    const QString tmpl      = _ui->editTemplate->text();
    const QString arraySize = _ui->editArraySize->text();
    const QString callConv  = _ui->comboCallingConvention->currentText();
    const bool array        = _ui->checkArray->isChecked();
    const bool isConst      = _ui->checkConst->isChecked();
    const bool constMethod  = _ui->checkConstMethod->isChecked();
    const bool pointer      = _ui->checkPointer->isChecked();
    const bool reference    = _ui->checkReference->isChecked();
    const bool isStatic     = _ui->checkStatic->isChecked();
    const bool pure         = _ui->checkPure->isChecked();
    const bool isVirtual    = _ui->checkVirtual->isChecked();
    const bool isInline     = _ui->checkInline->isChecked();
    const bool pointerPtr   = _ui->checkPointerPointer->isChecked();
    const bool constPointer = _ui->checkConstPointer->isChecked();
    const bool dllExport    = _ui->checkDllExport->isChecked();
    const bool declare      = _ui->checkDeclare->isChecked();
    const bool implement    = _ui->checkImplement->isChecked();
    const bool isDelete     = _ui->checkDelete->isChecked();

    Type* pType = _pMethod->GetDataModelDoc()->FindType(toCb(type));

    if (access == int(_pMethod->GetAccess()) &&
        array == (_pMethod->GetArray()) &&
        arraySize == toQ(_pMethod->GetArraySizeStr()) &&
        isConst == (_pMethod->Variable::GetConst()) &&
        callConv == toQ(_pMethod->GetCallingConvention()) &&
        constMethod == (_pMethod->GetConst()) &&
        dllExport == (_pMethod->GetDllExport()) &&
        isInline == _pMethod->GetInline() &&
        name == toQ(_pMethod->GetName()) &&
        note == toQ(_pMethod->GetNote()) &&
        pointer == (_pMethod->GetPointer()) &&
        pointerPtr == (_pMethod->GetPointerPointer()) &&
        constPointer== (_pMethod->GetConstPointer()) &&
        pure == (_pMethod->GetPure()) &&
        reference == (_pMethod->GetReference()) &&
        isStatic == (_pMethod->GetStatic()) &&
        isVirtual == (_pMethod->GetVirtual()) &&
        pType == _pMethod->GetType() &&
        tmpl == toQ(_pMethod->GetTemplate()) &&
        declare == (_pMethod->GetDeclare()) &&
        implement == (_pMethod->GetImplement()) &&
        isDelete == _pMethod->GetDelete())
    {
        return false;
    }

    _pMethod->SaveState(1);

    // For a virtual method, gather the overrides in derived classes so the
    // change can be propagated to them.
    std::vector<Method*> overrides;
    if (isVirtual)
    {
        DataModelDoc::TypeIterator iType(_pMethod->GetDataModelDoc(),
                                         &Type::IsExternClass);
        while (++iType)
        {
            ExternClass* pDerivedClass = (ExternClass*)iType.Get();
            if (pDerivedClass->IsBaseClass(_pMethod->GetBaseClass()))
            {
                Method* pFound =
                    pDerivedClass->FindSimilarMethod(_pMethod);
                if (pFound)
                    overrides.push_back(pFound);
            }
        }
    }

    if (_pMethod->GetImplement() != implement)
        _pMethod->SetPhase(implement ? Implementation_Phase
                                     : Complete_Phase);

    _pMethod->SetAccess(AccessType(access));
    _pMethod->SetArray(array);
    _pMethod->SetArraySizeStr(toCb(arraySize));
    _pMethod->Variable::SetConst(isConst);
    _pMethod->SetCallingConvention(toCb(callConv));
    _pMethod->SetConst(constMethod);
    _pMethod->SetDllExport(dllExport);
    _pMethod->SetInline(isInline);
    _pMethod->SetName(toCb(name));
    _pMethod->SetNote(toCb(note));
    _pMethod->SetPointer(pointer);
    _pMethod->SetPointerPointer(pointerPtr);
    _pMethod->SetConstPointer(constPointer);
    _pMethod->SetPure(pure);
    _pMethod->SetReference(reference);
    _pMethod->SetStatic(isStatic);
    _pMethod->SetVirtual(isVirtual);
    _pMethod->SetTemplate(toCb(tmpl));
    _pMethod->SetDeclare(declare);
    _pMethod->SetImplement(implement);
    _pMethod->SetDelete(isDelete);

    if (pType && pType != _pMethod->GetType())   // pType null-guard (defensive)
    {
        Class* pClass = dynamic_cast<Class*>(_pMethod->GetBaseClass());
        if (pClass && !_pMethod->GetType()->GetName().IsEmpty())
        {
            CbString str = "@Updated return type of method '" +
                _pMethod->GetName() + "'";
            pClass->AddModified(str);
        }
        pType->MoveVariableLast(_pMethod);
    }

    for (Method* pOverride : overrides)
    {
        pOverride->SaveState(1);
        pType->MoveVariableLast(pOverride);
        CbString note    = pOverride->GetNote();
        CbString code    = pOverride->GetCode();
        int      pureF   = pOverride->GetPure();
        bool     declF   = pOverride->GetDeclare();
        bool     implF   = pOverride->GetImplement();
        bool     delF    = pOverride->GetDelete();
        pOverride->CopyValuesFrom(*_pMethod);
        pOverride->SetNote(note);
        pOverride->SetCode(code);
        pOverride->SetPure(pureF);
        pOverride->SetDeclare(declF);
        pOverride->SetImplement(implF);
        pOverride->SetDelete(delF);
        pOverride->Update();
    }

    return true;
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowMethodDialog(Method* pMethod, bool& changed, void* ownerHwnd)
{
    Qt_EnsureApplication();

    MethodDialog dlg(pMethod);
    const bool accepted =
        Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
    changed = accepted && dlg.fieldsChanged();
    return accepted;
}
