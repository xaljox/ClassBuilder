// qt/MemberDialog.cpp -- the Qt Member attributes dialog.
//
// Ported from the MFC CMemberDialog. Edits a member: type / name / template,
// the type-property and access flags, get/set-method access, bit-field,
// initial value and a note. Non-live -- read on OK (applyFieldChanges, the
// MFC ::Update). Drives the model directly.

#include "MemberDialog.h"
#include "ui_MemberDialog.h"

#include "QtMemberDialog.h"          // Qt_ShowMemberDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "QtCompact.h"               // compactCombo
#include "QtComboHelpers.h"          // Qt_MakeSearchableCombo

#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>

#include <algorithm>
#include <vector>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {
// The type combo's previous selection -- the MFC m_type, used by
// onUpdateName to decide whether to re-derive the member name.
QString g_oldType;
}

MemberDialog::MemberDialog(Member* pMember, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::MemberDialog)
    , _pMember(pMember)
{
    _ui->setupUi(this);

    setWindowTitle(toQ("Member of class " +
        _pMember->GetBaseClass()->GetName()));

    g_oldType = toQ(_pMember->GetType()->GetName());

    // --- Type combo: every named type except "..." (sorted) ------------
    {
        std::vector<std::pair<QString, Type*>> types;
        DataModelDoc::TypeIterator type(_pMember->GetDataModelDoc());
        while (++type)
        {
            if (type->GetName() != "" && type->GetName() != "...")
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
            if (t.second == _pMember->GetType())
                _ui->comboType->setCurrentIndex(
                    _ui->comboType->count() - 1);
        }
    }
    compactCombo(_ui->comboType);
    Qt_MakeSearchableCombo(_ui->comboType);

    _ui->editName->setText(toQ(_pMember->GetName()));
    _ui->editTemplate->setText(toQ(_pMember->GetTemplate()));
    _ui->editArraySize->setText(toQ(_pMember->GetArraySizeStr()));
    _ui->editInitialization->setPlainText(
        toQ(_pMember->GetInitialization()));
    _ui->editNote->setPlainText(toQ(_pMember->GetNote()));
    _ui->spinBitFieldSize->setValue(_pMember->GetBitFieldSize());

    _ui->checkArray->setChecked(_pMember->GetArray());
    _ui->checkPointer->setChecked(_pMember->GetPointer());
    _ui->checkReference->setChecked(_pMember->GetReference());
    _ui->checkDelete->setChecked(_pMember->GetDelete());
    _ui->checkStatic->setChecked(_pMember->GetStatic());
    _ui->checkSerialize->setChecked(_pMember->GetSerialize());
    _ui->checkPointerPointer->setChecked(
        _pMember->GetPointerPointer());
    _ui->checkBitField->setChecked(_pMember->GetBitField());
    _ui->checkConst->setChecked(_pMember->GetConst());
    _ui->checkConstPointer->setChecked(_pMember->GetConstPointer());
    _ui->checkMutable->setChecked(_pMember->GetMutable());

    switch (_pMember->GetAccess())
    {
    case PROTECTED: _ui->radioAccessProtected->setChecked(true); break;
    case PRIVATE:   _ui->radioAccessPrivate->setChecked(true);   break;
    default:        _ui->radioAccessPublic->setChecked(true);    break;
    }

    int get = NONE;
    if (_pMember->GetGetMemberMethod())
        get = _pMember->GetGetMemberMethod()->GetAccess();
    switch (get)
    {
    case PUBLIC:    _ui->radioGetPublic->setChecked(true);    break;
    case PROTECTED: _ui->radioGetProtected->setChecked(true); break;
    case PRIVATE:   _ui->radioGetPrivate->setChecked(true);   break;
    default:        _ui->radioGetNone->setChecked(true);      break;
    }

    int set = NONE;
    if (_pMember->GetSetMemberMethod())
        set = _pMember->GetSetMemberMethod()->GetAccess();
    switch (set)
    {
    case PUBLIC:    _ui->radioSetPublic->setChecked(true);    break;
    case PROTECTED: _ui->radioSetProtected->setChecked(true); break;
    case PRIVATE:   _ui->radioSetPrivate->setChecked(true);   break;
    default:        _ui->radioSetNone->setChecked(true);      break;
    }

    connect(_ui->comboType, &QComboBox::currentIndexChanged,
            this, [this] { onSelchangeType(); });
    connect(_ui->checkArray, &QCheckBox::clicked,
            this, [this] { onArray(); });
    connect(_ui->checkPointer, &QCheckBox::clicked,
            this, [this] { onPointer(); });
    connect(_ui->checkStatic, &QCheckBox::clicked,
            this, [this] { onStatic(); });
    connect(_ui->checkReference, &QCheckBox::clicked,
            this, [this] { onUpdateName(); });
    connect(_ui->checkPointerPointer, &QCheckBox::clicked,
            this, [this] { onUpdateName(); });
    connect(_ui->checkBitField, &QCheckBox::clicked,
            this, [this] { onBitField(); });
    connect(_ui->checkConst, &QCheckBox::clicked,
            this, [this] { onConst(); });
    connect(_ui->checkMutable, &QCheckBox::clicked,
            this, [this] { onMutable(); });
    connect(_ui->checkConstPointer, &QCheckBox::clicked,
            this, [this] { onConstPointer(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &MemberDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &MemberDialog::reject);

    handleInitialization();
    handleBitField();
    handleBitFieldSize();
    handleArraySize();
    handleDelete();
    handleConstPointer();
    handlePointerPointer();
    handleSet();
    handleGet();
    handleConst();
    handleMutable();
    handleStatic();
    handleTemplate();

    Class* pClass = dynamic_cast<Class*>(_pMember->GetBaseClass());
    if (!pClass || !pClass->GetSerialize())
    {
        _ui->checkSerialize->setEnabled(false);
        if (!pClass)
        {
            _ui->radioAccessPrivate->setEnabled(false);
            _ui->radioGetPrivate->setEnabled(false);
            _ui->radioSetPrivate->setEnabled(false);
        }
    }
}

MemberDialog::~MemberDialog()
{
    delete _ui;
}

int MemberDialog::getAccess() const
{
    if (_ui->radioGetPublic->isChecked())    return PUBLIC;
    if (_ui->radioGetProtected->isChecked()) return PROTECTED;
    if (_ui->radioGetPrivate->isChecked())   return PRIVATE;
    return NONE;
}

int MemberDialog::setAccess() const
{
    if (_ui->radioSetPublic->isChecked())    return PUBLIC;
    if (_ui->radioSetProtected->isChecked()) return PROTECTED;
    if (_ui->radioSetPrivate->isChecked())   return PRIVATE;
    return NONE;
}

// --- Message handlers ------------------------------------------------------

void MemberDialog::onSelchangeType()
{
    handleTemplate();
    onUpdateName();
}

void MemberDialog::onUpdateName()
{
    const bool pointer        = _ui->checkPointer->isChecked();
    const bool pointerPointer = _ui->checkPointerPointer->isChecked();
    const bool reference      = _ui->checkReference->isChecked();

    const QString type = _ui->comboType->currentText();
    QString baseType = type;
    int index = baseType.indexOf('<');
    if (index != -1)
        baseType = baseType.left(index);

    Class* pClass =
        _pMember->GetDataModelDoc()->GetDataModel()->FindClass(toCb(type));
    if (pClass)
        baseType = toQ(pClass->GetBaseName());

    QString baseOldType = g_oldType;
    index = baseOldType.indexOf('<');
    if (index != -1)
        baseOldType = baseOldType.left(index);
    pClass = _pMember->GetDataModelDoc()->GetDataModel()
                 ->FindClass(toCb(g_oldType));
    if (pClass)
        baseOldType = toQ(pClass->GetBaseName());

    QString name = _ui->editName->text();

    int lenDiff = 0;
    if ((pointer || reference) &&
        (name.isEmpty() ||
         (name.right(baseOldType.length())
              .compare(baseOldType, Qt::CaseInsensitive) == 0 &&
          (lenDiff = qAbs(name.length() - baseOldType.length())) <= 3)))
    {
        if ((lenDiff == 0 && name.isEmpty()) ||
            (lenDiff == 1 && (name[0] == 'p' || name[0] == 'r')) ||
            (lenDiff == 2 && name[0] == 'p' &&
                 (name[1] == 'p' || name[1] == 'r')) ||
            (lenDiff == 3 && name[0] == 'p' && name[1] == 'p' &&
                 name[2] == 'r'))
        {
            name.clear();
            if (pointer)        name += "p";
            if (pointerPointer) name += "p";
            if (reference)      name += "r";

            QString tmp = baseType.left(1).toUpper() + baseType.mid(1);
            name += tmp;
            _ui->editName->setText(name);
        }
    }

    g_oldType = type;
    handleBitField();
}

void MemberDialog::onArray()
{
    handleSet();
    handleGet();
    handleArraySize();
    handleInitialization();
    handleBitField();
}

void MemberDialog::onPointer()
{
    handleDelete();
    handleConstPointer();
    handlePointerPointer();
    handleSet();
    onUpdateName();
}

void MemberDialog::onStatic()
{
    handleInitialization();
    handleMutable();
    handleBitField();
}

void MemberDialog::onBitField()
{
    handleBitFieldSize();
}

void MemberDialog::onConstPointer()
{
    handleSet();
}

void MemberDialog::onConst()
{
    handleSet();
    handleMutable();
}

void MemberDialog::onMutable()
{
    handleConst();
    handleStatic();
}

// --- Handle* enable / disable helpers --------------------------------------

void MemberDialog::handleInitialization()
{
    if (_ui->checkArray->isChecked() && !_ui->checkStatic->isChecked())
    {
        _ui->editInitialization->clear();
        _ui->editInitialization->setEnabled(false);
    }
    else
    {
        _ui->editInitialization->setEnabled(true);
    }
}

void MemberDialog::handleArraySize()
{
    if (_ui->checkArray->isChecked())
    {
        _ui->editArraySize->setEnabled(true);
        _ui->editArraySize->setVisible(true);
        _ui->editArraySize->setFocus();
    }
    else
    {
        _ui->editArraySize->clear();
        _ui->editArraySize->setVisible(false);
    }
}

void MemberDialog::handleDelete()
{
    if (_ui->checkPointer->isChecked())
    {
        _ui->checkDelete->setEnabled(true);
    }
    else
    {
        _ui->checkDelete->setChecked(false);
        _ui->checkDelete->setEnabled(false);
    }
}

void MemberDialog::handleConstPointer()
{
    if (_ui->checkPointer->isChecked())
    {
        _ui->checkConstPointer->setEnabled(true);
    }
    else
    {
        _ui->checkConstPointer->setChecked(false);
        _ui->checkConstPointer->setEnabled(false);
    }
}

void MemberDialog::handlePointerPointer()
{
    if (_ui->checkPointer->isChecked())
    {
        _ui->checkPointerPointer->setEnabled(true);
    }
    else
    {
        _ui->checkPointerPointer->setChecked(false);
        _ui->checkPointerPointer->setEnabled(false);
    }
}

void MemberDialog::handleBitField()
{
    Type* pType = nullptr;
    const int sel = _ui->comboType->currentIndex();
    if (sel >= 0)
        pType = static_cast<Type*>(
            _ui->comboType->itemData(sel).value<void*>());

    if (_ui->checkPointer->isChecked() ||
        _ui->checkArray->isChecked() ||
        _ui->checkStatic->isChecked() ||
        _ui->checkReference->isChecked() ||
        !pType || pType->IsExternClass())
    {
        _ui->checkBitField->setChecked(false);
        _ui->checkBitField->setEnabled(false);
    }
    else
    {
        _ui->checkBitField->setEnabled(true);
    }
}

void MemberDialog::handleBitFieldSize()
{
    if (_ui->checkBitField->isChecked())
    {
        _ui->spinBitFieldSize->setEnabled(true);
        _ui->spinBitFieldSize->setVisible(true);
        _ui->spinBitFieldSize->setFocus();
    }
    else
    {
        _ui->spinBitFieldSize->setVisible(false);
    }
}

void MemberDialog::handleConst()
{
    if (_ui->checkMutable->isChecked())
    {
        _ui->checkConst->setEnabled(false);
        _ui->checkConst->setChecked(false);
    }
    else
    {
        _ui->checkConst->setEnabled(true);
    }
}

void MemberDialog::handleStatic()
{
    if (_ui->checkMutable->isChecked())
    {
        _ui->checkStatic->setEnabled(false);
        _ui->checkStatic->setChecked(false);
    }
    else
    {
        _ui->checkStatic->setEnabled(true);
    }
}

void MemberDialog::handleMutable()
{
    if (_ui->checkConst->isChecked() || _ui->checkStatic->isChecked())
    {
        _ui->checkMutable->setEnabled(false);
        _ui->checkMutable->setChecked(false);
    }
    else
    {
        _ui->checkMutable->setEnabled(true);
    }
}

void MemberDialog::handleSet()
{
    bool disable = false;
    Member::RelationMemberIterator iRelationMember(_pMember);
    while (!disable && ++iRelationMember)
    {
        if (_pMember->GetBaseClass() !=
            iRelationMember->GetRelation()->GetToClass())
            disable = true;
    }

    if (disable || _ui->checkArray->isChecked() ||
        _ui->checkConstPointer->isChecked() ||
        (_ui->checkConst->isChecked() && !_ui->checkPointer->isChecked()))
    {
        // Checking None unchecks the others (auto-exclusive group).
        _ui->radioSetNone->setChecked(true);
        _ui->radioSetPublic->setEnabled(false);
        _ui->radioSetProtected->setEnabled(false);
        _ui->radioSetPrivate->setEnabled(false);
    }
    else
    {
        _ui->radioSetPublic->setEnabled(true);
        _ui->radioSetProtected->setEnabled(true);
        _ui->radioSetPrivate->setEnabled(true);
    }
}

void MemberDialog::handleGet()
{
    if (_ui->checkArray->isChecked())
    {
        // Checking None unchecks the others (auto-exclusive group).
        _ui->radioGetNone->setChecked(true);
        _ui->radioGetPublic->setEnabled(false);
        _ui->radioGetProtected->setEnabled(false);
        _ui->radioGetPrivate->setEnabled(false);
    }
    else
    {
        _ui->radioGetPublic->setEnabled(true);
        _ui->radioGetProtected->setEnabled(true);
        _ui->radioGetPrivate->setEnabled(true);
    }
}

void MemberDialog::handleTemplate()
{
    const int sel = _ui->comboType->currentIndex();
    if (sel >= 0)
    {
        Type* pType = static_cast<Type*>(
            _ui->comboType->itemData(sel).value<void*>());
        if (pType->GetTemplate().IsEmpty())
        {
            _ui->editTemplate->clear();
            _ui->editTemplate->setEnabled(false);
        }
        else
        {
            _ui->editTemplate->setEnabled(true);
            if (pType->GetName() == _pMember->GetType()->GetName())
                _ui->editTemplate->setText(toQ(_pMember->GetTemplate()));
            else
                _ui->editTemplate->setText(toQ(pType->GetTemplate()));
        }
    }
    else
    {
        _ui->editTemplate->clear();
        _ui->editTemplate->setEnabled(false);
    }
}

// --- OK / apply ------------------------------------------------------------

void MemberDialog::accept()
{
    const QString type = _ui->comboType->currentText();
    const QString name = _ui->editName->text();
    const QString init = _ui->editInitialization->toPlainText();
    const bool pointer   = _ui->checkPointer->isChecked();
    const bool reference = _ui->checkReference->isChecked();
    const bool isStatic  = _ui->checkStatic->isChecked();
    const bool bitField  = _ui->checkBitField->isChecked();
    const bool array     = _ui->checkArray->isChecked();
    const bool serialize = _ui->checkSerialize->isChecked();

    // DDV_Type
    if (type.isEmpty())
    {
        QMessageBox::warning(this, "Member", "Must select type of member");
        return;
    }
    if (type == toQ(_pMember->GetBaseClass()->GetName()) &&
        !pointer && !reference && !isStatic)
    {
        QMessageBox::warning(this, "Member",
                             "A class can not contain itself");
        return;
    }

    // DDV_Name
    if (name.isEmpty() && !bitField)
    {
        QMessageBox::warning(this, "Member", "Must give member a name");
        return;
    }
    if (_pMember->GetDataModelDoc()->HasNonCSymbols(toCb(name)))
    {
        QMessageBox::warning(this, "Member",
                             "Member name contains illegal characters");
        return;
    }
    {
        Member* pFind = _pMember->GetBaseClass()->FindMember(toCb(name));
        if (pFind && pFind != _pMember && !bitField)
        {
            QMessageBox::warning(this, "Member",
                                 "Member name must be unique");
            return;
        }
    }

    // DDV_Serialize
    if (!isStatic)
    {
        Type* pType = _pMember->GetDataModelDoc()->FindType(toCb(type));
        ExternClass* pExternClass = dynamic_cast<ExternClass*>(pType);
        if (!pExternClass || pointer)
        {
            Class* pClass =
                dynamic_cast<Class*>(_pMember->GetBaseClass());
            if (pClass && pClass->GetSerialize() && !serialize &&
                init.isEmpty() && !array)
            {
                QMessageBox::warning(this, "Member",
                    "Must specify an initial value for non serialize "
                    "members");
                return;
            }
        }
    }

    _changed = applyFieldChanges();
    QDialog::accept();
}

// Apply the widgets to the model (the MFC CMemberDialog::Update).
bool MemberDialog::applyFieldChanges()
{
    int update = 0;

    const QString typeStr   = _ui->comboType->currentText();
    const QString name      = _ui->editName->text();
    const QString note      = _ui->editNote->toPlainText();
    const QString tmpl      = _ui->editTemplate->text();
    const QString arraySize = _ui->editArraySize->text();
    const QString init      = _ui->editInitialization->toPlainText();
    const bool array        = _ui->checkArray->isChecked();
    const bool pointer      = _ui->checkPointer->isChecked();
    const bool reference    = _ui->checkReference->isChecked();
    const bool isStatic     = _ui->checkStatic->isChecked();
    const bool serialize    = _ui->checkSerialize->isChecked();
    const bool pointerPtr   = _ui->checkPointerPointer->isChecked();
    const bool bitField     = _ui->checkBitField->isChecked();
    const unsigned bitFieldSize = unsigned(_ui->spinBitFieldSize->value());
    const bool isConst      = _ui->checkConst->isChecked();
    const bool constPointer = _ui->checkConstPointer->isChecked();
    const bool isMutable    = _ui->checkMutable->isChecked();
    const bool del          = _ui->checkDelete->isChecked();
    const int  access       = _ui->radioAccessProtected->isChecked()
                                ? PROTECTED
                                : _ui->radioAccessPrivate->isChecked()
                                    ? PRIVATE : PUBLIC;
    const int  get          = getAccess();
    const int  set          = setAccess();

    Type* pType = _pMember->GetDataModelDoc()->FindType(toCb(typeStr));
    if (pType != _pMember->GetType())
    {
        _pMember->SaveState(1);
        pType->MoveVariableLast(_pMember);
        Member::MemberArgumentIterator iMemberArgument(_pMember);
        while (++iMemberArgument)
        {
            iMemberArgument->SaveState(1);
            pType->MoveVariableLast(iMemberArgument);
        }
        update = 1;

        if (_pMember->GetGetMemberMethod())
            _pMember->GetGetMemberMethod()->Delete();
        if (_pMember->GetSetMemberMethod() &&
            !_pMember->GetSetMemberMethod()->GetFirstArgument()
                 ->IsMemberArgument())
            _pMember->GetSetMemberMethod()->Delete();
    }

    if (toQ(_pMember->GetTemplate()) != tmpl)
    {
        if (!update) { _pMember->SaveState(); update = 1; }

        _pMember->SetTemplate(toCb(tmpl));
        Member::MemberArgumentIterator iMemberArgument(_pMember);
        while (++iMemberArgument)
        {
            iMemberArgument->SaveState();
            iMemberArgument->SetTemplate(toCb(tmpl));
        }
        if (_pMember->GetGetMemberMethod())
        {
            _pMember->GetGetMemberMethod()->SaveState();
            _pMember->GetGetMemberMethod()->SetTemplate(toCb(tmpl));
        }
    }

    if (_pMember->GetStatic() != isStatic ||
        _pMember->GetConst() != isConst ||
        _pMember->GetMutable() != isMutable ||
        _pMember->GetPointer() != pointer ||
        _pMember->GetPointerPointer() != pointerPtr ||
        _pMember->GetConstPointer() != constPointer ||
        _pMember->GetDelete() != del ||
        _pMember->GetReference() != reference ||
        _pMember->GetArray() != array ||
        toQ(_pMember->GetArraySizeStr()) != arraySize ||
        _pMember->GetBitField() != bitField ||
        _pMember->GetBitFieldSize() != bitFieldSize)
    {
        if (!update) { _pMember->SaveState(); update = 1; }

        _pMember->SetStatic(isStatic);
        _pMember->SetConst(isConst);
        _pMember->SetMutable(isMutable);
        _pMember->SetPointer(pointer);
        _pMember->SetPointerPointer(pointerPtr);
        _pMember->SetConstPointer(constPointer);
        _pMember->SetDelete(del);
        _pMember->SetReference(reference);
        _pMember->SetArray(array);
        _pMember->SetArraySizeStr(toCb(arraySize));
        _pMember->SetBitField(bitField);
        _pMember->SetBitFieldSize(bitFieldSize);

        if (_pMember->GetGetMemberMethod())
            _pMember->GetGetMemberMethod()->Delete();
        if (_pMember->GetSetMemberMethod())
            _pMember->GetSetMemberMethod()->Delete();
    }

    if (toQ(_pMember->GetName()) != name)
    {
        if (!update) { _pMember->SaveState(); update = 1; }
        _pMember->SetName(toCb(name));
    }

    // Delete the get / set methods that are switched off.
    if (get == NONE && _pMember->GetGetMemberMethod())
    {
        update = 1;
        _pMember->GetGetMemberMethod()->Delete();
    }
    if (set == NONE && _pMember->GetSetMemberMethod())
    {
        update = 1;
        _pMember->GetSetMemberMethod()->Delete();
    }

    // Create the get / set methods that are newly switched on.
    if (get != NONE && !_pMember->GetGetMemberMethod())
    {
        update = 1;
        (void)new GetMemberMethod(_pMember);
    }
    if (set != NONE && !_pMember->GetSetMemberMethod())
    {
        update = 1;
        (void)new SetMemberMethod(_pMember);
    }

    if (_pMember->GetGetMemberMethod() &&
        (_pMember->GetGetMemberMethod()->GetAccess() != get ||
         (_pMember->GetGetMemberMethod()->GetStatic()) != isStatic))
    {
        update = 1;
        _pMember->GetGetMemberMethod()->SaveState();
        _pMember->GetGetMemberMethod()->SetAccess(get);
        _pMember->GetGetMemberMethod()->SetStatic(isStatic);
        if (isStatic)
            _pMember->GetGetMemberMethod()->SetConst(0);
    }
    if (_pMember->GetSetMemberMethod() &&
        (_pMember->GetSetMemberMethod()->GetAccess() != set ||
         (_pMember->GetSetMemberMethod()->GetStatic()) != isStatic))
    {
        update = 1;
        _pMember->GetSetMemberMethod()->SaveState();
        _pMember->GetSetMemberMethod()->SetAccess(set);
        _pMember->GetSetMemberMethod()->SetStatic(isStatic);
    }

    if (toQ(_pMember->GetNote()) != note ||
        toQ(_pMember->GetInitialization()) != init ||
        int(_pMember->GetAccess()) != access ||
        _pMember->GetSerialize() != serialize)
    {
        if (!update) { _pMember->SaveState(); }

        _pMember->SetAccess(AccessType(access));
        _pMember->SetNote(toCb(note));
        _pMember->SetInitialization(toCb(init));
        _pMember->SetSerialize(serialize);

        update = 1;
    }

    return update != 0;
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowMemberDialog(Member* pMember, bool& changed, void* ownerHwnd)
{
    Qt_EnsureApplication();

    MemberDialog dlg(pMember);
    const bool accepted =
        Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
    changed = accepted && dlg.fieldsChanged();
    return accepted;
}
