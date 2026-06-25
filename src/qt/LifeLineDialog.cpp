// qt/LifeLineDialog.cpp -- the Qt Life Line dialog.
//
// Ported from the MFC LifeLineDialog. Edits a sequence-diagram life line:
// name, actor-or-class, template reference, show-activations, note. Two modes
// -- edit an existing LifeLineShape (applies on OK), or pick the actor/class
// for a new life line (the caller reads actor()/baseClass()/templateText()).
// Drives the model directly.

#include "LifeLineDialog.h"
#include "ui_LifeLineDialog.h"

#include "QtLifeLineDialog.h"        // Qt_ShowLifeLineDialog*
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "QtCompact.h"               // compactCombo
#include "QtComboHelpers.h"          // Qt_MakeSearchableCombo

#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QList>
#include <QPair>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {
DataModelDocObject* itemObject(QComboBox* combo, int index)
{
    return reinterpret_cast<DataModelDocObject*>(
        combo->itemData(index).toULongLong());
}
}

LifeLineDialog::LifeLineDialog(LifeLineShape* pLifeLineShape, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::LifeLineDialog)
    , _pLifeLineShape(pLifeLineShape)
    , _pDataModelDoc(pLifeLineShape->GetDataModelDoc())
{
    _ui->setupUi(this);
    _ui->editName->setText(toQ(pLifeLineShape->GetName()));
    _ui->editNote->setPlainText(toQ(pLifeLineShape->GetNote()));
    _ui->checkShowActivations->setChecked(
        pLifeLineShape->GetShowActivations());

    if (pLifeLineShape->GetActorLifeLine())
        _pActor = pLifeLineShape->GetActorLifeLine()->GetActor();
    if (pLifeLineShape->GetClassLifeLine())
    {
        _pBaseClass = pLifeLineShape->GetClassLifeLine()->GetBaseClass();
        _origTemplate =
            toQ(pLifeLineShape->GetClassLifeLine()->GetTemplate());
    }
    _origBaseClass = _pBaseClass;

    buildCommon();
}

LifeLineDialog::LifeLineDialog(DataModelDoc* pDataModelDoc, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::LifeLineDialog)
    , _pDataModelDoc(pDataModelDoc)
{
    _ui->setupUi(this);
    // A new life line shows activations by default -- matches the shape that
    // gets created, so the checkbox is honest about the result.
    _ui->checkShowActivations->setChecked(true);
    buildCommon();
}

LifeLineDialog::~LifeLineDialog()
{
    delete _ui;
}

QString LifeLineDialog::templateText() const
{
    return _ui->editTemplate->text();
}

bool LifeLineDialog::showActivations() const
{
    return _ui->checkShowActivations->isChecked();
}

QString LifeLineDialog::nameText() const
{
    return _ui->editName->text();
}

QString LifeLineDialog::noteText() const
{
    return _ui->editNote->toPlainText();
}

// Shared construction: fill the actor/class combo, select the current one,
// wire signals.
void LifeLineDialog::buildCommon()
{
    _ui->editTemplate->setEnabled(false);

    QList<QPair<QString, DataModelDocObject*>> entries;
    if (!_pBaseClass)
    {
        DataModelDoc::ActorIterator iActor(_pDataModelDoc);
        while (++iActor)
            entries.append({ toQ(iActor->GetName()), iActor.Get() });
    }
    if (!_pActor)
    {
        DataModelDoc::BaseClassIterator iBaseClass(_pDataModelDoc);
        while (++iBaseClass)
            entries.append({ toQ(iBaseClass->Type::GetName()),
                              iBaseClass.Get() });
    }
    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    for (const auto& e : entries)
        _ui->comboActorClass->addItem(e.first, QVariant::fromValue(
            reinterpret_cast<qulonglong>(e.second)));
    compactCombo(_ui->comboActorClass);
    Qt_MakeSearchableCombo(_ui->comboActorClass);

    // Select the current actor / class.
    DataModelDocObject* current = _pActor
        ? static_cast<DataModelDocObject*>(_pActor)
        : static_cast<DataModelDocObject*>(_pBaseClass);
    for (int i = 0; current && i < _ui->comboActorClass->count(); ++i)
    {
        if (itemObject(_ui->comboActorClass, i) == current)
        {
            _ui->comboActorClass->setCurrentIndex(i);
            break;
        }
    }

    // A class life line that already has activations bound to methods cannot
    // be re-pointed.
    if (_pBaseClass && _pLifeLineShape)
    {
        LifeLineShape::ChildActivationShapeIterator
            iChildActivationShape(_pLifeLineShape);
        while (++iChildActivationShape)
        {
            if (iChildActivationShape->GetMethod())
            {
                _ui->comboActorClass->setEnabled(false);
                break;
            }
        }
    }

    connect(_ui->comboActorClass,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this] { onSelectionChanged(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &LifeLineDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &LifeLineDialog::reject);

    onSelectionChanged();
}

// Combo selection changed: flip the label between Actor / Class and manage
// the template field.
void LifeLineDialog::onSelectionChanged()
{
    const int sel = _ui->comboActorClass->currentIndex();
    if (sel < 0)
        return;

    DataModelDocObject* obj = itemObject(_ui->comboActorClass, sel);

    if (dynamic_cast<Actor*>(obj))
    {
        _ui->labelActorClass->setText("&Actor:");
        _ui->editTemplate->clear();
        _ui->editTemplate->setEnabled(false);
    }
    else if (BaseClass* pBaseClass = dynamic_cast<BaseClass*>(obj))
    {
        _ui->labelActorClass->setText("&Class:");
        if (pBaseClass->GetTemplate().IsEmpty())
        {
            _ui->editTemplate->clear();
            _ui->editTemplate->setEnabled(false);
        }
        else
        {
            _ui->editTemplate->setEnabled(true);
            _ui->editTemplate->setText(pBaseClass == _origBaseClass
                ? _origTemplate
                : toQ(pBaseClass->GetTemplate()));
        }
    }
}

// OK -- the MFC OnOK: in edit mode apply the changes to the model; always
// record the chosen actor / class.
void LifeLineDialog::accept()
{
    const int sel = _ui->comboActorClass->currentIndex();
    if (sel < 0)
    {
        QMessageBox::warning(this, "Life line object",
                             "Must select a Class or Actor");
        return;
    }

    DataModelDocObject* obj = itemObject(_ui->comboActorClass, sel);
    Actor*     pCurActor     = dynamic_cast<Actor*>(obj);
    BaseClass* pCurBaseClass = dynamic_cast<BaseClass*>(obj);

    if (_pLifeLineShape)
    {
        const QString name = _ui->editName->text();
        const QString note = _ui->editNote->toPlainText();
        const bool showAct  = _ui->checkShowActivations->isChecked();
        const QString tmpl  = _ui->editTemplate->text();

        bool saveState = false;
        if (toQ(_pLifeLineShape->GetName()) != name ||
            toQ(_pLifeLineShape->GetNote()) != note ||
            _pLifeLineShape->GetShowActivations() != showAct)
        {
            saveState = true;
            _pLifeLineShape->SaveState(1);
            _pLifeLineShape->SetName(toCb(name));
            _pLifeLineShape->SetNote(toCb(note));
            _pLifeLineShape->SetShowActivations(showAct);
        }

        if (_pActor && _pActor != pCurActor)
        {
            if (!saveState) { saveState = true; _pLifeLineShape->SaveState(1); }
            pCurActor->MoveActorLifeLineShapeLast(
                (ActorLifeLineShape*)_pLifeLineShape);
        }

        if (_pBaseClass && _pBaseClass != pCurBaseClass)
        {
            if (!saveState) { saveState = true; _pLifeLineShape->SaveState(1); }
            pCurBaseClass->MoveClassLifeLineShapeLast(
                _pLifeLineShape->GetClassLifeLine());
        }

        if (_pBaseClass &&
            toQ(_pLifeLineShape->GetClassLifeLine()->GetTemplate()) != tmpl)
        {
            if (!saveState) { saveState = true; _pLifeLineShape->SaveState(1); }
            _pLifeLineShape->GetClassLifeLine()->SetTemplate(toCb(tmpl));
        }

        if (saveState)
        {
            _pLifeLineShape->GetSequenceDiagram()
                ->UpdateSequenceDiagramViews();
            _pLifeLineShape->GetDataModelDoc()->MarkLastUndo();
        }
    }

    _pActor     = pCurActor;
    _pBaseClass = pCurBaseClass;

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry points
// ---------------------------------------------------------------------------
void Qt_ShowLifeLineDialog(LifeLineShape* pLifeLineShape, void* ownerHwnd)
{
    Qt_EnsureApplication();

    LifeLineDialog dlg(pLifeLineShape);
    Qt_ExecModal(dlg, ownerHwnd);
}

bool Qt_ShowLifeLineDialogNew(DataModelDoc* pDataModelDoc, void* ownerHwnd,
                              Actor*& pActor, BaseClass*& pBaseClass,
                              CbString& templ, bool& showActivations,
                              CbString& name, CbString& note)
{
    Qt_EnsureApplication();

    LifeLineDialog dlg(pDataModelDoc);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    pActor          = dlg.actor();
    pBaseClass      = dlg.baseClass();
    templ           = toCb(dlg.templateText());
    showActivations = dlg.showActivations();
    name            = toCb(dlg.nameText());
    note            = toCb(dlg.noteText());
    return true;
}
