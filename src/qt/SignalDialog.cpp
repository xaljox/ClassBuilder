// qt/SignalDialog.cpp -- the Qt Signal (Message) dialog.
//
// Ported from the MFC SignalDialog. Edits a sequence-diagram signal: the
// linked method (picked from the receiver class's methods, or a free message
// name typed into the combo), the clause, label, return text, note and the
// display flags (argument types/names, scope, async/merge, creation/
// destruction). Drives the model directly; applies on OK.

#include "SignalDialog.h"
#include "ui_SignalDialog.h"

#include "QtSignalDialog.h"          // Qt_ShowSignalDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "QtCompact.h"               // compactCombo
#include "QtComboHelpers.h"          // Qt_MakeSearchableCombo

#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

bool SignalDialog::_classBuilderMethods = false;
bool SignalDialog::_relationMethods     = false;
bool SignalDialog::_getSetMethods       = false;

SignalDialog::SignalDialog(SignalShape* pSignalShape, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::SignalDialog)
    , _pSignalShape(pSignalShape)
{
    _ui->setupUi(this);

    // Receiver / sender base classes (null for actor life lines).
    _pBaseClass = nullptr;
    if (_pSignalShape->GetReceiver()->GetLifeLineShape()->GetClassLifeLine())
        _pBaseClass = _pSignalShape->GetReceiver()->GetLifeLineShape()
                          ->GetClassLifeLine()->GetBaseClass();
    _pSenderBaseClass = nullptr;
    if (_pSignalShape->GetSender()->GetLifeLineShape()->GetClassLifeLine())
        _pSenderBaseClass = _pSignalShape->GetSender()->GetLifeLineShape()
                                ->GetClassLifeLine()->GetBaseClass();

    _pMethod = _pSignalShape->GetReceiver()->GetMethod();
    if (_pMethod->IsFixedMethod())
        _classBuilderMethods = true;
    if (_pMethod->IsMacroMethod())
        _relationMethods = true;
    // Note: the MFC source tested IsGetMemberMethod twice -- corrected here
    // to also pick up setter methods.
    if (_pMethod->IsGetMemberMethod() || _pMethod->IsSetMemberMethod())
        _getSetMethods = true;

    // --- Static fields -------------------------------------------------
    _ui->editClause->setText(toQ(_pSignalShape->GetClause()));
    _ui->editLabel->setPlainText(toQ(_pSignalShape->GetLabel()));
    _ui->editNote->setPlainText(toQ(_pSignalShape->GetNote()));
    _ui->comboMethod->setEditText(toQ(_pSignalShape->GetName()));

    _ui->checkArguments->setChecked(_pSignalShape->GetArguments());
    _ui->checkArgumentNames->setChecked(
        _pSignalShape->GetArgumentNames());
    _ui->checkScope->setChecked(_pSignalShape->GetScope());
    _ui->checkAsync->setChecked(_pSignalShape->GetAsync());
    _ui->checkEnableReturn->setChecked(
        _pSignalShape->GetEnableReturn());
    _ui->checkCreation->setChecked(
        _pSignalShape->GetReceiver()->GetCreation());
    _ui->checkDestruction->setChecked(
        _pSignalShape->GetReceiver()->GetDestruction());
    _ui->checkMerge->setChecked(_pSignalShape->GetReceiver()->GetMerge());

    _ui->checkClassBuilder->setChecked(_classBuilderMethods);
    _ui->checkRelation->setChecked(_relationMethods);
    _ui->checkGetSet->setChecked(_getSetMethods);

    setMethod(_pMethod);

    if (_pBaseClass)
    {
        _ui->buttonNewMethod->setEnabled(true);
        fillMethodList();
        // Narrow the method list as you type (matches the other ported
        // dialogs); typing the exact signature then links to that method,
        // anything else stays a free-text message.
        Qt_MakeSearchableCombo(_ui->comboMethod);
    }
    else
    {
        _ui->buttonNewMethod->setEnabled(false);
        _ui->checkArguments->setEnabled(false);
        _ui->checkArgumentNames->setEnabled(false);
        _ui->checkScope->setEnabled(false);
        _ui->checkCreation->setEnabled(false);
        _ui->checkDestruction->setEnabled(false);
        _ui->checkClassBuilder->setEnabled(false);
        _ui->checkRelation->setEnabled(false);
        _ui->checkGetSet->setEnabled(false);
    }

    if (_pSignalShape->IsRecursiveActivation())
    {
        _ui->checkEnableReturn->setChecked(false);
        _ui->checkEnableReturn->setEnabled(false);
    }
    onEnableReturn();
    onAsync();

    // --- Signals -------------------------------------------------------
    connect(_ui->comboMethod, &QComboBox::activated,
            this, [this] { onSelchangeMethod(); });
    connect(_ui->comboMethod->lineEdit(), &QLineEdit::textEdited,
            this, [this] { onEditchangeMethod(); });
    connect(_ui->checkArguments, &QCheckBox::clicked,
            this, [this] { fillMethodList(); });
    connect(_ui->checkArgumentNames, &QCheckBox::clicked,
            this, [this] { fillMethodList(); });
    connect(_ui->checkScope, &QCheckBox::clicked,
            this, [this] { fillMethodList(); });
    connect(_ui->checkClassBuilder, &QCheckBox::clicked, this, [this]
            { _classBuilderMethods = _ui->checkClassBuilder->isChecked();
              fillMethodList(); });
    connect(_ui->checkRelation, &QCheckBox::clicked, this, [this]
            { _relationMethods = _ui->checkRelation->isChecked();
              fillMethodList(); });
    connect(_ui->checkGetSet, &QCheckBox::clicked, this, [this]
            { _getSetMethods = _ui->checkGetSet->isChecked();
              fillMethodList(); });
    connect(_ui->checkEnableReturn, &QCheckBox::clicked,
            this, [this] { onEnableReturn(); });
    connect(_ui->checkAsync, &QCheckBox::clicked,
            this, [this] { onAsync(); });
    connect(_ui->buttonNewMethod, &QPushButton::clicked,
            this, [this] { onNewMethod(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SignalDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &SignalDialog::reject);
}

SignalDialog::~SignalDialog()
{
    delete _ui;
}

// Reflect a (possibly null) linked method into the link indicator and the
// creation / destruction checkboxes -- the MFC SetMethod.
void SignalDialog::setMethod(Method* pMethod)
{
    if (pMethod)
    {
        _ui->checkMethodLink->setChecked(true);
        if (pMethod->IsConstructor())
        {
            _ui->checkCreation->setChecked(true);
            _ui->checkCreation->setEnabled(false);
            _ui->checkDestruction->setEnabled(true);
            if (!_pMethod->IsConstructor())
                _ui->checkDestruction->setChecked(false);
        }
        else
        {
            _ui->checkCreation->setChecked(false);
            _ui->checkCreation->setEnabled(false);

            if (pMethod->IsDestructor())
            {
                _ui->checkCreation->setChecked(false);
                _ui->checkDestruction->setChecked(true);
                _ui->checkDestruction->setEnabled(false);
            }
            else
            {
                _ui->checkDestruction->setEnabled(true);
                if (_pMethod != pMethod)
                    _ui->checkDestruction->setChecked(false);
            }
        }
    }
    else
    {
        _ui->checkMethodLink->setChecked(false);
        _ui->checkCreation->setEnabled(true);
        _ui->checkDestruction->setEnabled(true);
    }
    _pMethod = pMethod;
}

// Repopulate the method combo, preserving any free-typed text -- the MFC
// FillMethodList() (no args).
void SignalDialog::fillMethodList()
{
    const QString text = _ui->comboMethod->currentText();
    _ui->comboMethod->clear();

    int access = PUBLIC;
    if (_pBaseClass == _pSenderBaseClass)
    {
        access = PRIVATE;
    }
    else
    {
        ExternClass* pExternClass =
            dynamic_cast<ExternClass*>(_pSenderBaseClass);
        if (pExternClass && pExternClass->IsBaseClass(_pBaseClass))
            access = PROTECTED;
    }

    Method* wanted = _pMethod;
    fillMethodList(_pBaseClass, access);

    // Compact now -- before adjusting the selection / edit text below, since
    // compacting changes item data and would otherwise re-sync the editable
    // combo's line edit back to the current item.
    compactCombo(_ui->comboMethod);

    // Did the linked method survive the (re)filtering? Qt auto-selects item 0
    // when the first entry is added, so currentIndex() can't be trusted --
    // search the items explicitly.
    int found = -1;
    for (int i = 0; found < 0 && i < _ui->comboMethod->count(); ++i)
    {
        if (static_cast<Method*>(
                _ui->comboMethod->itemData(i).value<void*>()) == wanted)
            found = i;
    }
    if (found >= 0)
    {
        _ui->comboMethod->setCurrentIndex(found);
    }
    else if (wanted && _ui->comboMethod->count() > 0)
    {
        // A linked method was filtered out -- select the first remaining
        // entry and link to it, so the shown name and the link always agree.
        _ui->comboMethod->setCurrentIndex(0);
        setMethod(static_cast<Method*>(
            _ui->comboMethod->itemData(0).value<void*>()));
    }
    else
    {
        // No method to link to (a free-text message, or nothing left after
        // filtering) -- keep the typed text, unlinked.
        setMethod(nullptr);
        _ui->comboMethod->setEditText(text);
    }
}

// Recursive worker -- the MFC FillMethodList(pBox, pBaseClass, access).
void SignalDialog::fillMethodList(BaseClass* pBaseClass, int access)
{
    if (!pBaseClass)
        return;

    pBaseClass->SortMethod(Method::CompareTreeSaveState);
    BaseClass::MethodIterator iMethod(pBaseClass);
    while (++iMethod)
    {
        if (iMethod->GetAccess() <= access)
        {
            const bool arguments     = _ui->checkArguments->isChecked();
            const bool argumentNames = _ui->checkArgumentNames->isChecked();
            const bool scope         = _ui->checkScope->isChecked();

            if (!((iMethod->IsFixedMethod() && !_classBuilderMethods) ||
                  (iMethod->IsMacroMethod() && !_relationMethods) ||
                  (iMethod->IsGetMemberMethod() && !_getSetMethods) ||
                  (iMethod->IsSetMemberMethod() && !_getSetMethods)))
            {
                _ui->comboMethod->addItem(
                    toQ(iMethod->GetSignalShapeText(arguments,
                                                    argumentNames, scope)),
                    QVariant::fromValue(
                        static_cast<void*>(iMethod.Get())));
                if (iMethod.Get() == _pMethod)
                    _ui->comboMethod->setCurrentIndex(
                        _ui->comboMethod->count() - 1);
            }
        }
    }

    if (access == PRIVATE)
        access = PROTECTED;

    ExternClass* pExternClass = dynamic_cast<ExternClass*>(pBaseClass);
    if (pExternClass)
    {
        ExternClass::InheritIterator iInherit(pExternClass);
        while (++iInherit)
            fillMethodList(iInherit->GetBaseClass(), access);
    }
}

void SignalDialog::onSelchangeMethod()
{
    const int sel = _ui->comboMethod->currentIndex();
    if (sel >= 0)
        setMethod(static_cast<Method*>(
            _ui->comboMethod->itemData(sel).value<void*>()));
    else
        setMethod(nullptr);
}

void SignalDialog::onEditchangeMethod()
{
    // If the current text exactly matches a listed method, link to it (so the
    // link indicator tracks what you typed/completed); otherwise it is a
    // free-text message. accept() re-resolves the same way, so the final link
    // always agrees with the final text regardless of which signal fired.
    setMethod(methodForText(_ui->comboMethod->currentText()));
}

// Exact-match the combo text to a listed method's signature (the item text),
// returning its Method* or null for a free-text message.
Method* SignalDialog::methodForText(const QString& text) const
{
    for (int i = 0; i < _ui->comboMethod->count(); ++i)
    {
        if (_ui->comboMethod->itemText(i) == text)
            return static_cast<Method*>(
                _ui->comboMethod->itemData(i).value<void*>());
    }
    return nullptr;
}

// The "Return" checkbox enables / clears the return-text field.
void SignalDialog::onEnableReturn()
{
    if (_ui->checkEnableReturn->isChecked())
    {
        _ui->editReturn->setReadOnly(false);
        _ui->editReturn->setText(toQ(_pSignalShape->GetReturn()));
    }
    else
    {
        _ui->editReturn->clear();
        _ui->editReturn->setReadOnly(true);
    }
}

// "Async" gates the "Merge" checkbox -- the MFC OnAsync.
void SignalDialog::onAsync()
{
    ChildActivationShape* pReceiver = _pSignalShape->GetReceiver();
    if (_ui->checkAsync->isChecked() && pReceiver->CanMerge())
    {
        _ui->checkMerge->setEnabled(true);
    }
    else
    {
        _ui->checkMerge->setChecked(false);
        _ui->checkMerge->setEnabled(false);
    }
}

// "New Method" -- create a method on the receiver class and edit it. This
// still routes through the MFC Method::OnEditAttributes (CMethodDialog);
// once MethodDialog is ported the nested dialog becomes Qt.
void SignalDialog::onNewMethod()
{
    if (!_pBaseClass)
        return;

    _pBaseClass->GetDataModelDoc()->MarkLastUndo();
    Method* pMethod = new Method(_pBaseClass,
        _pBaseClass->GetDataModelDoc()->FindType(""));

    if (!_pSenderBaseClass || _pSenderBaseClass != _pBaseClass)
        pMethod->SetAccess(PUBLIC);

    if (pMethod->OnEditAttributes())
    {
        pMethod->Add();
        _pBaseClass->NotifyAddMethod(pMethod);
        setMethod(pMethod);
        fillMethodList();
    }
    else
    {
        _pBaseClass->GetDataModelDoc()->RollBack();
    }
}

// OK -- validate, then apply (the MFC OnOK).
void SignalDialog::accept()
{
    const QString message = _ui->comboMethod->currentText();
    if (message.isEmpty())
    {
        QMessageBox::warning(this, "Message",
            "Must select a method, or provide a message name");
        return;
    }

    // Resolve the link from the final text: an exact match to a listed method
    // links to it, anything else is a free message. This covers the case where
    // the searchable completer set the text without firing edit/select.
    _pMethod = methodForText(message);

    const QString clause = _ui->editClause->text();
    const QString label  = _ui->editLabel->toPlainText();
    const QString note   = _ui->editNote->toPlainText();
    const QString ret    = _ui->editReturn->text();
    const bool arguments     = _ui->checkArguments->isChecked();
    const bool argumentNames = _ui->checkArgumentNames->isChecked();
    const bool scope         = _ui->checkScope->isChecked();
    const bool async         = _ui->checkAsync->isChecked();
    const bool enableReturn  = _ui->checkEnableReturn->isChecked();
    const bool creation      = _ui->checkCreation->isChecked();
    const bool destruction   = _ui->checkDestruction->isChecked();
    const bool merge         = _ui->checkMerge->isChecked();

    bool update = false;

    if (clause != toQ(_pSignalShape->GetClause()) ||
        arguments != _pSignalShape->GetArguments() ||
        argumentNames != _pSignalShape->GetArgumentNames() ||
        label != toQ(_pSignalShape->GetLabel()) ||
        message != toQ(_pSignalShape->GetName()) ||
        note != toQ(_pSignalShape->GetNote()) ||
        scope != _pSignalShape->GetScope() ||
        enableReturn != _pSignalShape->GetEnableReturn() ||
        ret != toQ(_pSignalShape->GetReturn()) ||
        async != _pSignalShape->GetAsync())
    {
        _pSignalShape->SaveState(1);

        _pSignalShape->SetClause(toCb(clause));
        _pSignalShape->SetArguments(arguments);
        _pSignalShape->SetArgumentNames(argumentNames);
        _pSignalShape->SetLabel(toCb(label));
        _pSignalShape->SetName(toCb(message));
        _pSignalShape->SetNote(toCb(note));
        _pSignalShape->SetScope(scope);
        _pSignalShape->SetEnableReturn(enableReturn);
        _pSignalShape->SetReturn(toCb(ret));
        _pSignalShape->SetAsync(async);

        update = true;
    }

    ChildActivationShape* pReceiver = _pSignalShape->GetReceiver();
    if (creation != pReceiver->GetCreation() ||
        destruction != pReceiver->GetDestruction() ||
        merge != pReceiver->GetMerge() ||
        _pMethod != pReceiver->GetMethod())
    {
        pReceiver->SaveState(1);
        pReceiver->SetCreation(creation);
        pReceiver->SetDestruction(destruction);
        pReceiver->SetMerge(merge);

        update = true;

        if (_pMethod != pReceiver->GetMethod())
        {
            if (_pMethod)
            {
                if (pReceiver->GetMethod())
                    pReceiver->GetMethod()
                        ->RemoveChildActivationShape(pReceiver);
                _pMethod->AddChildActivationShapeLast(pReceiver);
            }
            else
            {
                pReceiver->GetMethod()
                    ->RemoveChildActivationShape(pReceiver);
            }
        }
    }

    if (update)
    {
        _pSignalShape->GetSequenceDiagram()->CheckAndUpdatePhase();
        _pSignalShape->GetSequenceDiagram()->UpdateSequenceDiagramViews();
        _pSignalShape->GetDataModelDoc()->MarkLastUndo();
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowSignalDialog(SignalShape* pSignalShape, void* ownerHwnd)
{
    Qt_EnsureApplication();

    SignalDialog dlg(pSignalShape);
    return Qt_ExecModal(dlg, ownerHwnd) == QDialog::Accepted;
}
