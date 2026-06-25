// qt/ArgumentDialog.cpp -- the Qt Argument attributes dialog.
//
// Ported from the MFC CArgumentDialog. Edits a method argument: type, name,
// const / reference / pointer / pointer-pointer / const-pointer / array,
// default value, note, template reference. Non-live -- the widgets are read
// on OK (applyFieldChanges, the MFC ::Update, which also propagates the
// change to the matching argument of every virtual override). Drives the
// model directly.

#include "ArgumentDialog.h"
#include "ui_ArgumentDialog.h"

#include "QtArgumentDialog.h"        // Qt_ShowArgumentDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "QtComboHelpers.h"          // Qt_MakeSearchableCombo
#include "QtCompact.h"               // compactCombo

#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QVector>

#include <algorithm>
#include <utility>
#include <vector>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

ArgumentDialog::ArgumentDialog(Argument* pArgument, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ArgumentDialog)
    , _pArgument(pArgument)
{
    _ui->setupUi(this);

    // Title -- the method signature, trimmed before any "//" comment.
    QString title = "Argument of '" +
        toQ(_pArgument->GetMethod()->GetInterfaceCpp()) + "'";
    const int slash = title.indexOf("//");
    if (slash != -1)
        title = title.left(slash) + "'";
    setWindowTitle(title);

    // Load the widgets.
    _ui->editName->setText(toQ(_pArgument->GetName()));
    _ui->editNote->setPlainText(toQ(_pArgument->GetNote()));
    _ui->editDefault->setText(toQ(_pArgument->GetDefault()));
    _ui->editTemplate->setText(toQ(_pArgument->GetTemplate()));
    _ui->editArraySize->setText(toQ(_pArgument->GetArraySizeStr()));
    _ui->checkArray->setChecked(_pArgument->GetArray());
    _ui->checkConst->setChecked(_pArgument->GetConst());
    _ui->checkPointer->setChecked(_pArgument->GetPointer());
    _ui->checkPointerPointer->setChecked(_pArgument->GetPointerPointer());
    _ui->checkConstPointer->setChecked(_pArgument->GetConstPointer());
    _ui->checkReference->setChecked(_pArgument->GetReference());

    // Type picker -- every named model type, alphabetical (the model
    // iterates in creation order). compactCombo tightens row height;
    // Qt_MakeSearchableCombo makes typing accumulate and narrow the
    // dropdown (default non-editable type-to-find is per-letter only).
    {
        std::vector<std::pair<QString, Type*>> types;
        DataModelDoc::TypeIterator type(_pArgument->GetDataModelDoc());
        while (++type)
        {
            if (type->GetName() == "")
                continue;
            types.emplace_back(toQ(type->GetName()), type.Get());
        }
        std::sort(types.begin(), types.end(),
            [](const std::pair<QString, Type*>& a,
               const std::pair<QString, Type*>& b)
            { return a.first.compare(b.first, Qt::CaseInsensitive) < 0; });
        for (const auto& t : types)
        {
            _ui->comboType->addItem(t.first,
                QVariant::fromValue(reinterpret_cast<qulonglong>(t.second)));
            if (t.second == _pArgument->GetType())
                _ui->comboType->setCurrentIndex(
                    _ui->comboType->count() - 1);
        }
    }
    compactCombo(_ui->comboType);
    Qt_MakeSearchableCombo(_ui->comboType);

    _prevType = toQ(_pArgument->GetType()->GetName());

    connect(_ui->comboType, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this] { onTypeChanged(); });
    connect(_ui->checkArray, &QCheckBox::toggled,
            this, [this] { onArrayToggled(); });
    connect(_ui->checkPointer, &QCheckBox::clicked,
            this, [this] { onPointerToggled(); });
    connect(_ui->checkPointerPointer, &QCheckBox::clicked,
            this, [this] { onUpdateName(); });
    connect(_ui->checkReference, &QCheckBox::clicked,
            this, [this] { onUpdateName(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ArgumentDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ArgumentDialog::reject);

    onArrayToggled();
    onTypeChanged();
}

ArgumentDialog::~ArgumentDialog()
{
    delete _ui;
}

// Array-size field enabled only when "Array" is ticked.
void ArgumentDialog::onArrayToggled()
{
    _ui->editArraySize->setEnabled(_ui->checkArray->isChecked());
}

// Pointer-pointer / const-pointer are usable only when Pointer is both
// ticked and itself enabled; clearing Pointer also clears them.
void ArgumentDialog::onPointerToggled()
{
    if (_ui->checkPointer->isChecked())
    {
        const bool on = _ui->checkPointer->isEnabled();
        _ui->checkPointerPointer->setEnabled(on);
        _ui->checkConstPointer->setEnabled(on);
    }
    else
    {
        _ui->checkPointerPointer->setEnabled(false);
        _ui->checkPointerPointer->setChecked(false);
        _ui->checkConstPointer->setEnabled(false);
        _ui->checkConstPointer->setChecked(false);
    }
    onUpdateName();
}

// Auto-name a pointer/reference argument from its type, when the current name
// still looks auto-generated (the MFC OnUpdateName).
void ArgumentDialog::onUpdateName()
{
    const bool pointer        = _ui->checkPointer->isChecked();
    const bool pointerPointer = _ui->checkPointerPointer->isChecked();
    const bool reference      = _ui->checkReference->isChecked();

    const QString type = _ui->comboType->currentText();

    QString baseType = type;
    if (Class* pClass = _pArgument->GetDataModelDoc()->GetDataModel()
                            ->FindClass(toCb(type)))
        baseType = toQ(pClass->GetBaseName());

    QString baseOldType = _prevType;
    if (Class* pClass = _pArgument->GetDataModelDoc()->GetDataModel()
                            ->FindClass(toCb(_prevType)))
        baseOldType = toQ(pClass->GetBaseName());

    const QString name = _ui->editName->text();

    if (pointer || reference)
    {
        int lenDiff = 0;
        bool proceed = false;
        if (name.isEmpty())
        {
            proceed = true;
        }
        else
        {
            const bool tail = name.right(baseOldType.length())
                                  .compare(baseOldType, Qt::CaseInsensitive) == 0;
            lenDiff = qAbs(name.length() - baseOldType.length());
            proceed = tail && lenDiff <= 3;
        }

        if (proceed)
        {
            const bool ok =
                (lenDiff == 0 && name.isEmpty()) ||
                (lenDiff == 1 && name.length() >= 1 &&
                    (name[0] == 'p' || name[0] == 'r')) ||
                (lenDiff == 2 && name.length() >= 2 &&
                    name[0] == 'p' && (name[1] == 'p' || name[1] == 'r')) ||
                (lenDiff == 3 && name.length() >= 3 &&
                    name[0] == 'p' && name[1] == 'p' && name[2] == 'r');
            if (ok)
            {
                QString newName;
                if (pointer)        newName += "p";
                if (pointerPointer) newName += "p";
                if (reference)      newName += "r";
                newName += baseType.left(1).toUpper() + baseType.mid(1);
                _ui->editName->setText(newName);
            }
        }
    }

    _prevType = type;
}

// Type selection changed: template field, the "..." special case, and the
// per-method-kind enable matrix (the MFC OnSelchangeType).
void ArgumentDialog::onTypeChanged()
{
    const int sel = _ui->comboType->currentIndex();
    if (sel < 0)
    {
        _ui->editTemplate->clear();
        _ui->editTemplate->setEnabled(false);
        onUpdateName();
        return;
    }

    Type* pType = reinterpret_cast<Type*>(
        _ui->comboType->itemData(sel).toULongLong());

    // Template reference field.
    if (pType->GetTemplate().IsEmpty())
    {
        _ui->editTemplate->clear();
        _ui->editTemplate->setEnabled(false);
    }
    else
    {
        _ui->editTemplate->setEnabled(
            !_pArgument->GetMethod()->IsSetMemberMethod());
        if (toQ(pType->GetName()) == toQ(_pArgument->GetType()->GetName()))
            _ui->editTemplate->setText(toQ(_pArgument->GetTemplate()));
        else
            _ui->editTemplate->setText(toQ(pType->GetTemplate()));
    }

    if (pType->GetName() == "...")
    {
        // Ellipsis argument -- no name/default/properties.
        _ui->editName->clear();
        _ui->editName->setEnabled(false);
        _ui->editDefault->clear();
        _ui->editDefault->setEnabled(false);
        _ui->checkArray->setChecked(false);
        _ui->checkArray->setEnabled(false);
        onArrayToggled();
        _ui->checkPointer->setChecked(false);
        _ui->checkPointer->setEnabled(false);
        _ui->checkPointerPointer->setChecked(false);
        _ui->checkPointerPointer->setEnabled(false);
        _ui->checkReference->setChecked(false);
        _ui->checkReference->setEnabled(false);
        _ui->checkConst->setChecked(false);
        _ui->checkConst->setEnabled(false);
    }
    else
    {
        Method* pMethod = _pArgument->GetMethod();
        if (pMethod->IsFixed())
        {
            _ui->checkArray->setEnabled(false);
            _ui->checkConst->setEnabled(false);
            _ui->editDefault->setEnabled(false);
            _ui->editName->setEnabled(false);
            _ui->checkPointer->setEnabled(false);
            _ui->checkReference->setEnabled(false);
            _ui->comboType->setEnabled(false);
        }
        else if (pMethod->IsFindMethod() || pMethod->IsGetMemberMethod() ||
                 pMethod->IsSetMemberMethod())
        {
            _ui->checkArray->setEnabled(false);
            _ui->checkConst->setEnabled(true);
            _ui->editDefault->setEnabled(true);
            _ui->editName->setEnabled(true);
            _ui->checkPointer->setEnabled(true);
            _ui->checkReference->setEnabled(true);
            _ui->comboType->setEnabled(false);
        }
        else
        {
            _ui->checkArray->setEnabled(true);
            _ui->checkConst->setEnabled(true);
            _ui->editDefault->setEnabled(true);
            _ui->editName->setEnabled(true);
            _ui->checkPointer->setEnabled(true);
            _ui->checkReference->setEnabled(true);
            _ui->comboType->setEnabled(true);
        }
        onPointerToggled();
    }

    onUpdateName();
}

// OK -- validate (the MFC DDV_Type / DDV_Name), then apply.
void ArgumentDialog::accept()
{
    const QString type = _ui->comboType->currentText();
    const QString name = _ui->editName->text();

    if (type.isEmpty())
    {
        QMessageBox::warning(this, "Argument",
                             "Must select type of argument");
        return;
    }
    if (type != "..." &&
        _pArgument->GetDataModelDoc()->HasNonCSymbols(toCb(name)))
    {
        QMessageBox::warning(this, "Argument",
                             "Argument name contains illegal characters");
        return;
    }
    Argument* pFind = _pArgument->GetMethod()->FindArgument(toCb(name));
    if (!name.isEmpty() && pFind && pFind != _pArgument)
    {
        QMessageBox::warning(this, "Argument",
                             "Argument name must be unique");
        return;
    }

    _changed = applyFieldChanges();
    QDialog::accept();
}

// Apply the widget values to the model (the MFC CArgumentDialog::Update).
// Returns true if anything changed. Also propagates to the matching argument
// of every virtual override.
bool ArgumentDialog::applyFieldChanges()
{
    const QString type     = _ui->comboType->currentText();
    const QString name     = _ui->editName->text();
    const QString note     = _ui->editNote->toPlainText();
    const QString def      = _ui->editDefault->text();
    const QString tmpl     = _ui->editTemplate->text();
    const QString arraySz  = _ui->editArraySize->text();
    const bool array       = _ui->checkArray->isChecked();
    const bool isConst     = _ui->checkConst->isChecked();
    const bool pointer     = _ui->checkPointer->isChecked();
    const bool pointerPtr  = _ui->checkPointerPointer->isChecked();
    const bool constPtr    = _ui->checkConstPointer->isChecked();
    const bool reference   = _ui->checkReference->isChecked();

    Type* pType = _pArgument->GetDataModelDoc()->FindType(toCb(type));

    bool update = false;

    // Collect the matching argument of each virtual override (before any
    // change), so they can be moved + copied afterward.
    QVector<Argument*> overrides;
    if (_pArgument->GetMethod()->GetVirtual())
    {
        DataModelDoc::TypeIterator iType(_pArgument->GetDataModelDoc(),
                                         &Type::IsExternClass);
        while (++iType)
        {
            ExternClass* pDerived = (ExternClass*)iType.Get();
            if (pDerived->IsBaseClass(_pArgument->GetMethod()->GetBaseClass()))
            {
                Method* pMethod =
                    pDerived->FindSimilarMethod(_pArgument->GetMethod());
                if (pMethod)
                    overrides.append(
                        pMethod->GetArgumentAtSamePosition(_pArgument));
            }
        }
    }

    if (pType != _pArgument->GetType())
    {
        _pArgument->SaveState(1);
        pType->MoveVariableLast(_pArgument);
        update = true;
    }

    if (_pArgument->GetArray() != array ||
        toQ(_pArgument->GetArraySizeStr()) != arraySz ||
        _pArgument->GetConst() != isConst ||
        toQ(_pArgument->GetName()) != name ||
        toQ(_pArgument->GetNote()) != note ||
        toQ(_pArgument->GetDefault()) != def ||
        _pArgument->GetPointer() != pointer ||
        _pArgument->GetPointerPointer() != pointerPtr ||
        _pArgument->GetConstPointer() != constPtr ||
        _pArgument->GetReference() != reference ||
        toQ(_pArgument->GetTemplate()) != tmpl)
    {
        if (!update)
        {
            _pArgument->SaveState();
            update = true;
        }
        _pArgument->SetArray(array);
        _pArgument->SetArraySizeStr(toCb(arraySz));
        _pArgument->SetConst(isConst);
        _pArgument->SetDefault(toCb(def));
        _pArgument->SetName(toCb(name));
        _pArgument->SetNote(toCb(note));
        _pArgument->SetPointer(pointer);
        _pArgument->SetPointerPointer(pointerPtr);
        _pArgument->SetConstPointer(constPtr);
        _pArgument->SetReference(reference);
        _pArgument->SetTemplate(toCb(tmpl));
    }

    if (update)
    {
        for (Argument* pOverride : overrides)
        {
            pOverride->SaveState(1);
            pType->MoveVariableLast(pOverride);
            pOverride->CopyValuesFrom(*_pArgument);
            pOverride->Update();
        }
    }

    return update;
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowArgumentDialog(Argument* pArgument, bool& changed, void* ownerHwnd)
{
    Qt_EnsureApplication();

    ArgumentDialog dlg(pArgument);
    const bool accepted =
        Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
    changed = accepted && dlg.fieldsChanged();
    return accepted;
}
