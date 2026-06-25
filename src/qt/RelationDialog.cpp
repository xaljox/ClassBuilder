// qt/RelationDialog.cpp -- the Qt Relation attributes dialog.
//
// Ported from the MFC CRelationDialog. Edits a relation between two classes:
// from/to class+name, association type, aggregation/critical/filter, the
// container implementation (standard / value-tree / avl-tree) and a note.
// Non-live -- read on OK (applyFieldChanges, the MFC ::Update). Drives the
// model directly.

#include "RelationDialog.h"
#include "ui_RelationDialog.h"

#include "QtRelationDialog.h"        // Qt_ShowRelationDialog
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

#include "CbViewLock.h"   // coalesce the on-OK relation re-wiring cascade

namespace {
Class* comboClass(QComboBox* box)
{
    const int i = box->currentIndex();
    return i < 0 ? nullptr
                 : static_cast<Class*>(box->itemData(i).value<void*>());
}
}

RelationDialog::RelationDialog(Relation* pRelation, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::RelationDialog)
    , _pRelation(pRelation)
{
    _ui->setupUi(this);

    if (_pRelation->GetRelationMember())
    {
        _pMemberOrg = _pMember =
            _pRelation->GetRelationMember()->GetMember();
        if (_pRelation->GetRelationMember()->IsUniqueValueTree())
            _uniqueValueTreeOrg = true;
    }

    QString fromName = toQ(_pRelation->GetFromName());
    if (fromName.isEmpty())
        fromName = toQ(_pRelation->GetFromClass()->GetBaseName());
    QString toName = toQ(_pRelation->GetToName());
    if (toName.isEmpty())
        toName = toQ(_pRelation->GetToClass()->GetBaseName());

    _ui->editFromName->setText(fromName);
    _ui->editToName->setText(toName);
    _ui->editNote->setPlainText(toQ(_pRelation->GetNote()));
    _ui->checkOwned->setChecked(_pRelation->GetOwned());
    _ui->checkCritical->setChecked(_pRelation->GetCritical());
    _ui->checkFilter->setChecked(_pRelation->GetFilter());
    _ui->checkUniqueValueTree->setChecked(_uniqueValueTreeOrg);

    const int type = _pRelation->GetMulti() + _pRelation->GetStatic();
    if (type == 2)      _ui->radioStaticMulti->setChecked(true);
    else if (type == 1) _ui->radioMulti->setChecked(true);
    else                _ui->radioSingle->setChecked(true);

    switch (_pRelation->GetImplementation())
    {
    case 1:  _ui->radioValueTree->setChecked(true); break;
    case 2:  _ui->radioAvlTree->setChecked(true);   break;
    default: _ui->radioStandard->setChecked(true);  break;
    }

    // --- Populate the From / To class combos ---------------------------
    DataModel* pDataModel = _pRelation->GetFromClass()->GetDataModel();
    {
        std::vector<std::pair<QString, Class*>> classes;
        DataModel::ClassIterator iClass(pDataModel);
        while (++iClass)
            classes.emplace_back(toQ(iClass->GetName()), iClass.Get());
        std::sort(classes.begin(), classes.end(),
            [](const std::pair<QString, Class*>& a,
               const std::pair<QString, Class*>& b)
            { return a.first < b.first; });
        for (const auto& c : classes)
        {
            QVariant data = QVariant::fromValue(static_cast<void*>(c.second));
            _ui->comboFromClass->addItem(c.first, data);
            _ui->comboToClass->addItem(c.first, data);
            if (c.second == _pRelation->GetFromClass())
                _ui->comboFromClass->setCurrentIndex(
                    _ui->comboFromClass->count() - 1);
            if (c.second == _pRelation->GetToClass())
                _ui->comboToClass->setCurrentIndex(
                    _ui->comboToClass->count() - 1);
        }
    }
    compactCombo(_ui->comboFromClass);
    Qt_MakeSearchableCombo(_ui->comboFromClass);
    compactCombo(_ui->comboToClass);
    Qt_MakeSearchableCombo(_ui->comboToClass);

    connect(_ui->comboFromClass, &QComboBox::activated,
            this, [this] { onSelchangeFromClass(); });
    connect(_ui->comboToClass, &QComboBox::activated,
            this, [this] { onSelchangeToClass(); });
    connect(_ui->radioSingle, &QRadioButton::clicked,
            this, [this] { onMulti(); });
    connect(_ui->radioMulti, &QRadioButton::clicked,
            this, [this] { onMulti(); });
    connect(_ui->radioStaticMulti, &QRadioButton::clicked,
            this, [this] { onMulti(); });
    connect(_ui->radioStandard, &QRadioButton::clicked,
            this, [this] { onImplementation(); });
    connect(_ui->radioValueTree, &QRadioButton::clicked,
            this, [this] { onImplementation(); });
    connect(_ui->radioAvlTree, &QRadioButton::clicked,
            this, [this] { onImplementation(); });
    connect(_ui->comboMember, &QComboBox::activated,
            this, [this] { onSelchangeMember(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &RelationDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &RelationDialog::reject);

    onMulti();
    fillMemberList(_pRelation->GetToClass());
    onCheckTemplate();

    if (_pRelation->GetFromClass() == pDataModel->GetDocument() &&
        _pRelation->GetToClass() == pDataModel->GetDocumentObject() &&
        _pRelation->GetFromClass()->GetFirstFromRelation() == _pRelation)
    {
        _ui->comboFromClass->setEnabled(false);
        _ui->comboToClass->setEnabled(false);
        _ui->radioSingle->setEnabled(false);
        _ui->radioMulti->setEnabled(false);
        _ui->radioStaticMulti->setEnabled(false);
        _ui->checkOwned->setEnabled(false);
    }
}

RelationDialog::~RelationDialog()
{
    delete _ui;
}

int RelationDialog::associationType() const
{
    if (_ui->radioMulti->isChecked())       return 1;
    if (_ui->radioStaticMulti->isChecked()) return 2;
    return 0;
}

int RelationDialog::implementation() const
{
    if (_ui->radioValueTree->isChecked()) return 1;
    if (_ui->radioAvlTree->isChecked())   return 2;
    return 0;
}

void RelationDialog::onSelchangeFromClass()
{
    Class* pClass = comboClass(_ui->comboFromClass);
    if (pClass)
        _ui->editFromName->setText(toQ(pClass->GetBaseName()));
    onCheckTemplate();
}

void RelationDialog::onSelchangeToClass()
{
    Class* pClass = comboClass(_ui->comboToClass);
    if (pClass)
    {
        _ui->editToName->setText(toQ(pClass->GetBaseName()));
        fillMemberList(pClass);
    }
    onCheckTemplate();
}

// Static-multi is only available for plain (non-template, non-serialize)
// classes -- the MFC OnCheckTemplate.
void RelationDialog::onCheckTemplate()
{
    Class* pFromClass = comboClass(_ui->comboFromClass);
    Class* pToClass   = comboClass(_ui->comboToClass);
    if (!pFromClass || !pToClass)
        return;

    if (pFromClass->GetTemplate().IsEmpty() &&
        pToClass->GetTemplate().IsEmpty() &&
        !pFromClass->GetSerialize() && !pToClass->GetSerialize())
    {
        _ui->radioStaticMulti->setEnabled(true);
    }
    else
    {
        if (_ui->radioStaticMulti->isChecked())
            _ui->radioMulti->setChecked(true);
        _ui->radioStaticMulti->setChecked(false);
        _ui->radioStaticMulti->setEnabled(false);
        onMulti();
    }
}

void RelationDialog::fillMemberList(BaseClass* pBaseClass, bool first)
{
    if (first)
    {
        _ui->comboMember->clear();
        _pMember = nullptr;
    }

    Class::MemberIterator iMember(pBaseClass);
    while (++iMember)
    {
        if (first || !iMember->GetSetMemberMethod())
        {
            if (iMember->IsPublic() ||
                (iMember->GetGetMemberMethod() &&
                 iMember->GetGetMemberMethod()->GetAccess() == PUBLIC))
            {
                if (!iMember->GetStatic() && !iMember->GetPointer() &&
                    !iMember->GetReference())
                {
                    CbString str = iMember->GetPrefixedName();
                    if (!first)
                        str = pBaseClass->GetName() + "::" + str;
                    _ui->comboMember->addItem(toQ(str),
                        QVariant::fromValue(
                            static_cast<void*>(iMember.Get())));
                    if (iMember.Get() == _pMemberOrg)
                    {
                        _ui->comboMember->setCurrentIndex(
                            _ui->comboMember->count() - 1);
                        _pMember = _pMemberOrg;
                    }
                }
            }
        }
    }

    Class* pClass = dynamic_cast<Class*>(pBaseClass);
    if (pClass)
    {
        ExternClass::InheritIterator iInherit(pClass);
        while (++iInherit)
            fillMemberList(iInherit->GetBaseClass(), false);
    }

    if (first)
        compactCombo(_ui->comboMember);
}

void RelationDialog::onMulti()
{
    const bool multi       = _ui->radioMulti->isChecked();
    const bool staticMulti = _ui->radioStaticMulti->isChecked();

    if (multi || staticMulti)
    {
        _ui->checkFilter->setEnabled(true);
    }
    else
    {
        _ui->checkFilter->setChecked(false);
        _ui->checkFilter->setEnabled(false);
    }

    if (multi)
    {
        _ui->radioValueTree->setEnabled(true);
        _ui->radioAvlTree->setEnabled(true);
    }
    else
    {
        _ui->radioStandard->setChecked(true);
        _ui->radioValueTree->setChecked(false);
        _ui->radioValueTree->setEnabled(false);
        _ui->radioAvlTree->setChecked(false);
        _ui->radioAvlTree->setEnabled(false);
    }
    onImplementation();
}

void RelationDialog::onImplementation()
{
    if (_ui->radioValueTree->isChecked())
    {
        _ui->checkUniqueValueTree->setEnabled(true);
        _ui->labelMember->setEnabled(true);
        _ui->labelMember->setVisible(true);
        _ui->comboMember->setEnabled(true);
        _ui->comboMember->setVisible(true);
        _ui->comboMember->setFocus();
    }
    else if (_ui->radioAvlTree->isChecked())
    {
        _ui->checkUniqueValueTree->setEnabled(false);
        _ui->labelMember->setEnabled(true);
        _ui->labelMember->setVisible(true);
        _ui->comboMember->setEnabled(true);
        _ui->comboMember->setVisible(true);
        _ui->comboMember->setFocus();
    }
    else
    {
        _ui->checkUniqueValueTree->setEnabled(false);
        _ui->checkUniqueValueTree->setChecked(false);
        _ui->labelMember->setEnabled(false);
        _ui->labelMember->setVisible(true);
        _ui->comboMember->setVisible(false);
    }
}

void RelationDialog::onSelchangeMember()
{
    const int i = _ui->comboMember->currentIndex();
    if (i >= 0)
        _pMember = static_cast<Member*>(
            _ui->comboMember->itemData(i).value<void*>());
}

void RelationDialog::accept()
{
    const QString fromClass = _ui->comboFromClass->currentText();
    const QString toClass   = _ui->comboToClass->currentText();
    const QString fromName  = _ui->editFromName->text();
    const QString toName    = _ui->editToName->text();
    const bool owned        = _ui->checkOwned->isChecked();
    const int  type         = associationType();

    DataModelDoc* pDoc = _pRelation->GetDataModelDoc();
    DataModel* pDataModel = _pRelation->GetFromClass()->GetDataModel();

    // DDV_FromClass
    if (fromClass.isEmpty())
    {
        QMessageBox::warning(this, "Relation",
                             "Must select 'From Class' for relation");
        return;
    }
    {
        BaseClass* pFrom = pDoc->FindBaseClass(toCb(fromClass));
        BaseClass* pTo   = pDoc->FindBaseClass(toCb(toClass));
        if (pFrom && pTo &&
            pFrom->GetTemplate() != pTo->GetTemplate() &&
            pFrom->GetTemplateDeclaration() != pTo->GetTemplateDeclaration())
        {
            QMessageBox::warning(this, "Relation",
                toQ("The template of class '" + pFrom->GetName() +
                    "' and class '" + pTo->GetName() +
                    "' must be the same"));
            return;
        }
    }

    // DDV_ToClass
    if (toClass.isEmpty())
    {
        QMessageBox::warning(this, "Relation",
                             "Must select 'To Class' for relation");
        return;
    }

    // DDV_FromName
    if (fromName.isEmpty())
    {
        QMessageBox::warning(this, "Relation",
                             "Must select 'From Name' for relation");
        return;
    }
    if (pDoc->HasNonCSymbols(toCb(fromName)))
    {
        QMessageBox::warning(this, "Relation",
                             "From name contains illegal characters");
        return;
    }
    {
        Class::ToRelationIterator tRel(pDataModel->FindClass(toCb(toClass)));
        while (++tRel)
        {
            if (((Relation*)tRel) != _pRelation &&
                tRel->GetFromName() == toCb(fromName))
            {
                QMessageBox::warning(this, "Relation",
                    "Must select unique 'From Name' for relation");
                return;
            }
        }
    }

    // DDV_ToName
    if (toName.isEmpty())
    {
        QMessageBox::warning(this, "Relation",
                             "Must select 'To Name' for relation");
        return;
    }
    if (pDoc->HasNonCSymbols(toCb(toName)))
    {
        QMessageBox::warning(this, "Relation",
                             "To name contains illegal characters");
        return;
    }
    {
        Class::FromRelationIterator fRel(
            pDataModel->FindClass(toCb(fromClass)));
        while (++fRel)
        {
            if (((Relation*)fRel) != _pRelation &&
                fRel->GetToName() == toCb(toName))
            {
                QMessageBox::warning(this, "Relation",
                    "Must select unique 'To Name' for relation");
                return;
            }
        }
    }

    if (fromClass == toClass && fromName == toName && owned && type == 0)
    {
        QMessageBox::warning(this, "Relation",
            "Can't have a single aggregation with same role on both "
            "sides (From Name == To Name)");
        return;
    }

    _changed = applyFieldChanges();
    QDialog::accept();
}

// Apply the widgets to the model (the MFC CRelationDialog::Update).
bool RelationDialog::applyFieldChanges()
{
    const QString note      = _ui->editNote->toPlainText();
    const QString fromClass = _ui->comboFromClass->currentText();
    const QString toClass   = _ui->comboToClass->currentText();
    const QString fromName  = _ui->editFromName->text();
    const QString toName    = _ui->editToName->text();
    const int  type         = associationType();
    const bool owned        = _ui->checkOwned->isChecked();
    const bool critical     = _ui->checkCritical->isChecked();
    const bool filter       = _ui->checkFilter->isChecked();
    const int  impl         = implementation();
    const bool uniqueVT     = _ui->checkUniqueValueTree->isChecked();

    if (note == toQ(_pRelation->GetNote()) &&
        fromClass == toQ(_pRelation->GetFromClassName()) &&
        toClass == toQ(_pRelation->GetToClassName()) &&
        fromName == toQ(_pRelation->GetFromName()) &&
        toName == toQ(_pRelation->GetToName()) &&
        type == (_pRelation->GetMulti() +
                      _pRelation->GetStatic()) &&
        owned == (_pRelation->GetOwned()) &&
        critical == (_pRelation->GetCritical()) &&
        filter == (_pRelation->GetFilter()) &&
        impl == _pRelation->GetImplementation() &&
        uniqueVT == _uniqueValueTreeOrg &&
        _pMember == _pMemberOrg)
    {
        return false;
    }

    // Coalesce the relation re-wiring (delete/regenerate find + macro methods,
    // reparent, container rebuild -- each SetX otherwise refreshes the tree)
    // into a single view refresh.
    CbViewLock lock(_pRelation->GetDataModelDoc());

    _pRelation->SaveState(1);

    DataModel* pDataModel = _pRelation->GetFromClass()->GetDataModel();
    Class* pFromClass = pDataModel->FindClass(toCb(fromClass));
    Class* pToClass   = pDataModel->FindClass(toCb(toClass));
    Class* pOldFromClass = _pRelation->GetFromClass();
    Class* pOldToClass   = _pRelation->GetToClass();

    const bool xStatic = (type == 2);
    const bool multi   = (type > 0);
    const bool single  = (type == 0);

    const bool fromNameChanged = fromName != toQ(_pRelation->GetFromName());
    const bool toNameChanged   = toName != toQ(_pRelation->GetToName());
    const bool namesChanged    = fromNameChanged || toNameChanged;
    const bool staticChanged   = _pRelation->GetStatic() != xStatic;
    const bool ownedChanged    = _pRelation->GetOwned() != owned;
    const bool criticalChanged = _pRelation->GetCritical() != critical;

    // Transition from multi to single -- discard the (find) methods.
    if (_pRelation->GetMulti() && single)
    {
        FromRelation::MethodIterator method(
            _pRelation->GetFromRelation());
        while (++method)
            method->Delete();

        if (_pRelation->GetFromRelation()->GetFromRelationMacroMethods())
            _pRelation->GetFromRelation()
                ->GetFromRelationMacroMethods()->Delete();
    }

    _pRelation->SetNote(toCb(note));
    _pRelation->SetFromName(toCb(fromName));
    _pRelation->SetToName(toCb(toName));
    _pRelation->SetStatic(xStatic);
    _pRelation->SetMulti(multi);
    _pRelation->SetSingle(single);
    _pRelation->SetOwned(owned);
    _pRelation->SetCritical(critical);
    _pRelation->SetFilter(filter);

    // Drop the macro methods -- regenerated below for the new situation.
    if (_pRelation->GetFromRelation()->GetFromRelationMacroMethods())
        _pRelation->GetFromRelation()
            ->GetFromRelationMacroMethods()->Delete();
    if (_pRelation->GetToRelation()->GetToRelationMacroMethods())
        _pRelation->GetToRelation()
            ->GetToRelationMacroMethods()->Delete();

    if (pOldFromClass != pFromClass || pOldToClass != pToClass)
    {
        ClassDiagram::RemoveRelation(_pRelation);

        if (pOldFromClass != pFromClass)
        {
            _pRelation->GetFromRelation()->Remove();
            pFromClass->MoveFromRelation(_pRelation);
            _pRelation->GetFromRelation()->Add();
        }
        if (pOldToClass != pToClass)
        {
            _pRelation->GetToRelation()->Remove();
            pToClass->MoveToRelation(_pRelation);
            _pRelation->GetToRelation()->Add();
        }

        ClassDiagram::AddRelation(_pRelation);
    }
    else if (ownedChanged || fromNameChanged || staticChanged)
    {
        pToClass->GetConstructorIncludeMethod()->UpdateArguments();
    }

    if (!_pRelation->GetFromRelation()->GetAdded())
        _pRelation->GetFromRelation()->Add();
    if (!_pRelation->GetToRelation()->GetAdded())
        _pRelation->GetToRelation()->Add();

    if (staticChanged)
    {
        FromRelation::MethodIterator method(
            _pRelation->GetFromRelation());
        while (++method)
        {
            method->SaveState();
            method->SetStatic(xStatic);
            method->InitCode();
        }
    }

    if (impl != _pRelation->GetImplementation() ||
        uniqueVT != _uniqueValueTreeOrg ||
        _pMember != _pMemberOrg ||
        pOldToClass != pToClass)
    {
        if (_pRelation->GetRelationMember())
            _pRelation->GetRelationMember()->Delete();

        if (_pMember)
        {
            if (impl == 1)
            {
                if (uniqueVT)
                    (void)new UniqueValueTree(_pRelation, _pMember);
                else
                    (void)new ValueTree(_pRelation, _pMember);
            }
            else if (impl == 2)
            {
                (void)new AvlTree(_pRelation, _pMember);
            }
        }
    }
    else if (impl == 1 && criticalChanged)
    {
        FromRelation::MethodIterator iMethod(
            _pRelation->GetFromRelation(),
            &FromRelationMethod::IsFindMethod);
        while (++iMethod)
        {
            if (iMethod->IsFindUniqueValueTreeMethod() ||
                iMethod->IsFindValueTreeMethod() ||
                iMethod->IsFindReverseValueTreeMethod())
            {
                iMethod->SaveState();
                iMethod->InitCode();
            }
        }
    }
    else if (impl == 2 && criticalChanged)
    {
        FromRelation::MethodIterator iMethod(
            _pRelation->GetFromRelation(),
            &FromRelationMethod::IsFindMethod);
        while (++iMethod)
        {
            if (iMethod->IsFindAvlTreeMethod() ||
                iMethod->IsFindReverseAvlTreeMethod() ||
                iMethod->IsFindEqualOrSmallerAvlTreeMethod() ||
                iMethod->IsFindEqualOrBiggerAvlTreeMethod())
            {
                iMethod->SaveState();
                iMethod->InitCode();
            }
        }
    }

    if (!_pRelation->GetToRelation()->GetToRelationMacroMethods())
    {
        (void)new ToRelationMacroMethods(_pRelation->GetToRelation());
        _pRelation->GetToRelation()->GetToRelationMacroMethods()->Add();
    }
    if (!_pRelation->GetFromRelation()->GetFromRelationMacroMethods())
    {
        (void)new FromRelationMacroMethods(
            _pRelation->GetFromRelation());
        _pRelation->GetFromRelation()
            ->GetFromRelationMacroMethods()->Add();
    }

    Relation::RelationShapeIterator iRelationShape(_pRelation);
    while (++iRelationShape)
    {
        iRelationShape->SaveState();
        if (namesChanged)
        {
            if (_pRelation->GetFromName() ==
                    _pRelation->GetFromClass()->GetBaseName() &&
                _pRelation->GetToName() ==
                    _pRelation->GetToClass()->GetBaseName())
                iRelationShape->SetVerbosity(0);
            else
                iRelationShape->SetVerbosity(1);
        }
        iRelationShape->ConvertRouting();
    }

    return true;
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowRelationDialog(Relation* pRelation, bool& changed,
                           void* ownerHwnd)
{
    Qt_EnsureApplication();

    RelationDialog dlg(pRelation);
    const bool accepted =
        Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
    changed = accepted && dlg.fieldsChanged();
    return accepted;
}
