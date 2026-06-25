// qt/InheritDialog.cpp -- the Qt Inherit attributes dialog.
//
// Ported from the MFC CInheritDialog. Edits one Inherit relation of an
// extern class: the base class it inherits from, access (public / protected
// / private), virtual flag, template reference and a note. Non-live -- read
// on OK (applyFieldChanges, the MFC ::Update). Drives the model directly.

#include "InheritDialog.h"
#include "ui_InheritDialog.h"

#include "QtInheritDialog.h"         // Qt_ShowInheritDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "QtCompact.h"               // compactCombo
#include "QtComboHelpers.h"          // Qt_MakeSearchableCombo

#include <QComboBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>

#include <algorithm>
#include <vector>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

InheritDialog::InheritDialog(Inherit* pInherit, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::InheritDialog)
    , _pInherit(pInherit)
{
    _ui->setupUi(this);

    setWindowTitle(toQ("Class " +
        _pInherit->GetExternClass()->GetName() + " inherits"));

    // The current inherit name -- empty when the base class is the extern
    // class itself (i.e. inheritance not yet decided).
    CbString origInherit;
    if (_pInherit->GetExternClass() != _pInherit->GetBaseClass())
        origInherit = _pInherit->GetBaseClass()->GetName();
    const QString origInheritQ  = toQ(origInherit);
    const QString origTemplateQ = toQ(_pInherit->GetTemplate());

    // --- Access / virtual / note ---------------------------------------
    switch (_pInherit->GetAccess())
    {
    case PROTECTED: _ui->radioProtected->setChecked(true); break;
    case PRIVATE:   _ui->radioPrivate->setChecked(true);   break;
    default:        _ui->radioPublic->setChecked(true);    break;
    }
    _ui->checkVirtual->setChecked(_pInherit->GetVirtual());
    _ui->editNote->setPlainText(toQ(_pInherit->GetNote()));

    // --- Populate the "Inherits From" combo ----------------------------
    // Mirrors CInheritDialog::OnInitDialog: the current inheritance is
    // temporarily moved last so the candidate list is computed correctly,
    // then restored at its previous position.
    ExternClass* pExternClass = _pInherit->GetExternClass();
    Class*       pClass       = dynamic_cast<Class*>(pExternClass);
    BaseClass*   pBaseClass   = _pInherit->GetBaseClass();
    DataModelDoc* pDataModelDoc = pExternClass->GetDataModelDoc();

    Inherit* pInheritPos = pBaseClass->GetNextInherit(_pInherit);
    pExternClass->BaseClass::MoveInheritLast(_pInherit);

    std::vector<std::pair<QString, BaseClass*>> items;
    auto add = [&items](BaseClass* pBase)
    {
        items.emplace_back(toQ(pBase->GetName()), pBase);
    };

    if (pClass)
    {
        DataModel* pDataModel = pDataModelDoc->GetDataModel();

        if (pClass->GetSerialize())
        {
            if (pClass == pDataModel->GetDocument())
            {
                add(pBaseClass);
                DataModelDoc::BaseClassIterator baseClass(pDataModelDoc,
                    &BaseClass::IsExternClass);
                while (++baseClass)
                {
                    if (pBaseClass != (BaseClass*)baseClass)
                        add((BaseClass*)baseClass);
                }
            }
            else if (pClass == pDataModel->GetDocumentObject())
            {
                add(pBaseClass);
            }
            else
            {
                DataModelDoc::BaseClassIterator baseClass(pDataModelDoc);
                while (++baseClass)
                {
                    if (baseClass->IsInheritBase(pClass))
                    {
                        Class* pCand =
                            dynamic_cast<Class*>((BaseClass*)baseClass);
                        if (pCand)
                        {
                            if (pCand->GetSerialize() &&
                                pCand != pDataModel->GetDocument())
                                add((BaseClass*)baseClass);
                        }
                        else
                        {
                            add((BaseClass*)baseClass);
                        }
                    }
                }
            }
        }
        else
        {
            DataModelDoc::BaseClassIterator baseClass(pDataModelDoc);
            while (++baseClass)
            {
                if (baseClass->IsInheritBase(pExternClass))
                    add((BaseClass*)baseClass);
            }
        }
    }
    else
    {
        DataModelDoc::BaseClassIterator baseClass(pDataModelDoc,
            &BaseClass::IsExternClass);
        while (++baseClass)
        {
            if (baseClass->IsInheritBase(pExternClass))
                add((BaseClass*)baseClass);
        }
    }

    // Restore the inheritance at its original location.
    if (pInheritPos)
        pBaseClass->MoveInheritBefore(_pInherit, pInheritPos);
    else
        pBaseClass->MoveInheritLast(_pInherit);

    // CBS_SORT -- the MFC combo sorted its strings.
    std::sort(items.begin(), items.end(),
        [](const std::pair<QString, BaseClass*>& a,
           const std::pair<QString, BaseClass*>& b)
        { return a.first < b.first; });
    for (const auto& it : items)
        _ui->comboInherit->addItem(it.first,
            QVariant::fromValue(static_cast<void*>(it.second)));
    compactCombo(_ui->comboInherit);
    Qt_MakeSearchableCombo(_ui->comboInherit);

    if (pClass && pClass->GetDocumentObject())
        _ui->comboInherit->setEnabled(false);

    connect(_ui->comboInherit, &QComboBox::currentIndexChanged,
            this, [this] { onSelchangeInherit(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &InheritDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &InheritDialog::reject);

    // Remember the originals -- used by onSelchangeInherit to decide whether
    // the edited template reference applies.
    setProperty("origInherit",  origInheritQ);
    setProperty("origTemplate", origTemplateQ);

    if (!origInheritQ.isEmpty())
    {
        const int index = _ui->comboInherit->findText(origInheritQ);
        if (index != -1)
            _ui->comboInherit->setCurrentIndex(index);
        onSelchangeInherit();
    }
    else
    {
        _ui->comboInherit->setCurrentIndex(-1);
        _ui->editTemplate->clear();
        _ui->editTemplate->setEnabled(false);
    }
}

InheritDialog::~InheritDialog()
{
    delete _ui;
}

// Selection changed -- the MFC OnSelchangeInherit. Enables / fills the
// template-reference field for the newly selected base class.
void InheritDialog::onSelchangeInherit()
{
    const int sel = _ui->comboInherit->currentIndex();
    if (sel < 0)
    {
        _ui->editTemplate->clear();
        _ui->editTemplate->setEnabled(false);
        return;
    }

    BaseClass* pBaseClass = static_cast<BaseClass*>(
        _ui->comboInherit->itemData(sel).value<void*>());

    if (pBaseClass->GetTemplate().IsEmpty())
    {
        _ui->editTemplate->clear();
        _ui->editTemplate->setEnabled(false);
    }
    else
    {
        _ui->editTemplate->setEnabled(true);
        if (toQ(pBaseClass->GetName()) == property("origInherit").toString())
            _ui->editTemplate->setText(property("origTemplate").toString());
        else
            _ui->editTemplate->setText(toQ(pBaseClass->GetTemplate()));
    }
}

// OK -- validate a class is selected (the MFC DDV_Inherit), then apply.
void InheritDialog::accept()
{
    if (_ui->comboInherit->currentIndex() < 0)
    {
        QMessageBox::warning(this, "Inherit",
                             "Must select class to inherit from");
        return;
    }

    _changed = applyFieldChanges();
    QDialog::accept();
}

// Apply the widgets to the model (the MFC CInheritDialog::Update).
bool InheritDialog::applyFieldChanges()
{
    int access = 0;                            // 0 public
    if (_ui->radioProtected->isChecked()) access = 1;
    else if (_ui->radioPrivate->isChecked()) access = 2;

    const QString inherit  = _ui->comboInherit->currentText();
    const QString tmpl     = _ui->editTemplate->text();
    const QString note     = _ui->editNote->toPlainText();
    const bool    isVirt   = _ui->checkVirtual->isChecked();

    if (access != int(_pInherit->GetAccess()) ||
        note != toQ(_pInherit->GetNote()) ||
        inherit != toQ(_pInherit->GetBaseClass()->GetName()) ||
        tmpl != toQ(_pInherit->GetTemplate()) ||
        isVirt != _pInherit->GetVirtual())
    {
        _pInherit->SaveState(1);
        CbString oldInherit = _pInherit->GetBaseName();

        _pInherit->SetAccess(AccessType(access));
        _pInherit->SetNote(toCb(note));
        _pInherit->SetTemplate(toCb(tmpl));
        _pInherit->SetVirtual(isVirt);

        BaseClass* pBaseClass =
            _pInherit->GetDataModelDoc()->FindBaseClass(toCb(inherit));
        if (pBaseClass && pBaseClass != _pInherit->GetBaseClass())
        {
            ClassDiagram::RemoveInherit(_pInherit);
            pBaseClass->MoveInheritLast(_pInherit);
            ClassDiagram::AddInherit(_pInherit);
        }

        CbString newInherit = _pInherit->GetBaseName();
        if (oldInherit != newInherit &&
            _pInherit->GetExternClass()->IsClass())
        {
            BaseClass::MethodIterator iMethod(_pInherit->GetExternClass());
            while (++iMethod)
            {
                Constructor* pConstructor =
                    dynamic_cast<Constructor*>(iMethod.Get());
                if (pConstructor)
                {
                    pConstructor->ReplaceInInit(oldInherit, newInherit);
                    if (pConstructor->GetPhase() > Implementation_Phase)
                        pConstructor->SetPhaseUpwards(Implementation_Phase);
                }
            }
        }

        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowInheritDialog(Inherit* pInherit, bool& changed, void* ownerHwnd)
{
    Qt_EnsureApplication();

    InheritDialog dlg(pInherit);
    const bool accepted =
        Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
    changed = accepted && dlg.fieldsChanged();
    return accepted;
}
