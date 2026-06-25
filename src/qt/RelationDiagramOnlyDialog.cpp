// qt/RelationDiagramOnlyDialog.cpp -- the Qt "Relation ClassDiagram Only"
// properties dialog. Ported from the MFC RelationDiagramOnlyDialog.
//
// Edit and Create share applyControls() (one apply path, no duplicate to drift).
// Create does NOT build the shape itself -- the bridge creates it AFTER OK and
// calls applyControls(), because the diagram-only connection is a standalone
// shape (no tree object), so it is created directly only on commit.

#include "RelationDiagramOnlyDialog.h"
#include "ui_RelationDiagramOnlyDialog.h"

#include "QtRelationDiagramOnlyDialog.h"   // Qt_Show/CreateRelationDiagramOnlyDialog
#include "QtApp.h"                         // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"                   // toQ / toCb
#include "QtCompact.h"                     // compactCombo

#include <QComboBox>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// Populate a class-shape combo (sorted), select the current shape.
void fillClassCombo(QComboBox* combo, ClassDiagram* pClassDiagram,
                    ClassShape* pSelected)
{
    ClassDiagram::ClassDiagramShapeIterator iShape(pClassDiagram);
    while (++iShape)
    {
        ClassShape* pClassShape = dynamic_cast<ClassShape*>(iShape.Get());
        if (pClassShape)
            combo->addItem(toQ(pClassShape->GetBaseClass()->GetName()),
                           QVariant::fromValue(
                               reinterpret_cast<qulonglong>(pClassShape)));
    }
    combo->model()->sort(0);            // CBS_SORT
    compactCombo(combo);

    for (int i = 0; i < combo->count(); ++i)
    {
        if (reinterpret_cast<ClassShape*>(combo->itemData(i).toULongLong())
 == pSelected)
        {
            combo->setCurrentIndex(i);
            break;
        }
    }
}

ClassShape* comboClassShape(const QComboBox* combo)
{
    return reinterpret_cast<ClassShape*>(combo->currentData().toULongLong());
}

} // namespace

// Edit: handed a live shape, read it into the controls.
RelationDiagramOnlyDialog::RelationDiagramOnlyDialog(
        RelationDiagramOnlyShape* pShape, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::RelationDiagramOnlyDialog)
    , _pShape(pShape)
{
    _ui->setupUi(this);

    _ui->lineEditFromName->setText(toQ(_pShape->GetFromName()));
    _ui->lineEditToName->setText(toQ(_pShape->GetToName()));
    _ui->lineEditFromMultiplicity->setText(toQ(_pShape->GetUmlFrom()));
    _ui->lineEditToMultiplicity->setText(toQ(_pShape->GetUmlTo()));

    // Association type: Single = 0, Multi = 1, Static Multi = 2.
    const int type = _pShape->GetMulti() + _pShape->GetStatic();
    if (type == 2)      _ui->radioStaticMulti->setChecked(true);
    else if (type == 1) _ui->radioMulti->setChecked(true);
    else                _ui->radioSingle->setChecked(true);

    // Association properties: Association = 0, Aggregation = 1, Composition = 2.
    switch (_pShape->GetOwned())
    {
    case 1:  _ui->radioAggregation->setChecked(true); break;
    case 2:  _ui->radioComposition->setChecked(true); break;
    default: _ui->radioAssociation->setChecked(true); break;
    }

    ClassDiagram* pClassDiagram = _pShape->GetClassDiagram();
    fillClassCombo(_ui->comboFromClass, pClassDiagram,
                   _pShape->GetFromClassShape());
    fillClassCombo(_ui->comboToClass, pClassDiagram,
                   _pShape->GetToClassShape());

    wireSignals();
}

// Create: no shape yet. Seed the combos from the diagram and start from the
// defaults a freshly-constructed shape would have (Multi + Aggregation, "1"/"*").
// The shape itself is built by the bridge after OK (see below).
RelationDiagramOnlyDialog::RelationDiagramOnlyDialog(
        ClassDiagram* pClassDiagram, ClassShape* pInitFrom, ClassShape* pInitTo,
        QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::RelationDiagramOnlyDialog)
    , _pShape(nullptr)
{
    _ui->setupUi(this);

    fillClassCombo(_ui->comboFromClass, pClassDiagram, pInitFrom);
    fillClassCombo(_ui->comboToClass, pClassDiagram, pInitTo);

    ClassShape* pFrom = comboClassShape(_ui->comboFromClass);
    ClassShape* pTo   = comboClassShape(_ui->comboToClass);
    _ui->lineEditFromName->setText(
        pFrom ? toQ(pFrom->GetBaseClass()->GetName()) : QString());
    _ui->lineEditToName->setText(
        pTo ? toQ(pTo->GetBaseClass()->GetName()) : QString());
    _ui->lineEditFromMultiplicity->setText("1");
    _ui->lineEditToMultiplicity->setText("*");
    _ui->radioMulti->setChecked(true);
    _ui->radioAggregation->setChecked(true);

    wireSignals();
}

void RelationDiagramOnlyDialog::wireSignals()
{
    // Connect AFTER the initial state is loaded, so seeding the controls does
    // not trigger the auto-fill slots (the MFC click handlers only fire on
    // genuine user interaction).
    connect(_ui->comboFromClass, &QComboBox::currentIndexChanged,
            this, &RelationDiagramOnlyDialog::onFromClassChanged);
    connect(_ui->comboToClass, &QComboBox::currentIndexChanged,
            this, &RelationDiagramOnlyDialog::onToClassChanged);
    connect(_ui->radioAssociation, &QRadioButton::toggled,
            this, &RelationDiagramOnlyDialog::onOwnedToggled);
    connect(_ui->radioAggregation, &QRadioButton::toggled,
            this, &RelationDiagramOnlyDialog::onOwnedToggled);
    connect(_ui->radioComposition, &QRadioButton::toggled,
            this, &RelationDiagramOnlyDialog::onOwnedToggled);
    connect(_ui->radioSingle, &QRadioButton::toggled,
            this, &RelationDiagramOnlyDialog::onTypeToggled);
    connect(_ui->radioMulti, &QRadioButton::toggled,
            this, &RelationDiagramOnlyDialog::onTypeToggled);
    connect(_ui->radioStaticMulti, &QRadioButton::toggled,
            this, &RelationDiagramOnlyDialog::onTypeToggled);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &RelationDiagramOnlyDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &RelationDiagramOnlyDialog::reject);
}

RelationDiagramOnlyDialog::~RelationDiagramOnlyDialog()
{
    delete _ui;
}

void RelationDiagramOnlyDialog::onFromClassChanged()
{
    if (ClassShape* p = comboClassShape(_ui->comboFromClass))
        _ui->lineEditFromName->setText(toQ(p->GetBaseClass()->GetName()));
}

void RelationDiagramOnlyDialog::onToClassChanged()
{
    if (ClassShape* p = comboClassShape(_ui->comboToClass))
        _ui->lineEditToName->setText(toQ(p->GetBaseClass()->GetName()));
}

// Aggregation/Composition imply the "From" end owns exactly one (UML only).
void RelationDiagramOnlyDialog::onOwnedToggled()
{
    const bool owns = _ui->radioAggregation->isChecked() ||
                      _ui->radioComposition->isChecked();
    _ui->lineEditFromMultiplicity->setText(owns ? "1" : "0..1");
}

// Multi associations imply a many ("*") "To" multiplicity.
void RelationDiagramOnlyDialog::onTypeToggled()
{
    const bool multi = _ui->radioMulti->isChecked() ||
                       _ui->radioStaticMulti->isChecked();
    _ui->lineEditToMultiplicity->setText(multi ? "*" : "0..1");
}

ClassShape* RelationDiagramOnlyDialog::fromShape() const
{
    return comboClassShape(_ui->comboFromClass);
}

ClassShape* RelationDiagramOnlyDialog::toShape() const
{
    return comboClassShape(_ui->comboToClass);
}

// Write the dialog's attributes onto `pShape` -- NO routing, NO SaveState.
// Shared by the edit accept() and the create bridge; the caller does the routing
// (edit reroutes only if endpoints changed; create does one clean MakeNewRouting).
void RelationDiagramOnlyDialog::applyAttributes(RelationDiagramOnlyShape* pShape) const
{
    const int type = _ui->radioStaticMulti->isChecked() ? 2
                   : _ui->radioMulti->isChecked()       ? 1
                                                        : 0;
    const int owned = _ui->radioAggregation->isChecked() ? 1
                    : _ui->radioComposition->isChecked() ? 2
                                                         : 0;

    pShape->SetFromName(toCb(_ui->lineEditFromName->text()));
    pShape->SetToName(toCb(_ui->lineEditToName->text()));
    pShape->SetStatic(type == 2);
    pShape->SetMulti(type > 0);
    pShape->SetOwned(owned);
    pShape->SetUmlFrom(toCb(_ui->lineEditFromMultiplicity->text()));
    pShape->SetUmlTo(toCb(_ui->lineEditToMultiplicity->text()));
}

// OK on the EDIT path -- apply to the live shape if anything changed. Create mode
// has no shape (_pShape == null): it only gathers; the bridge builds after OK.
void RelationDiagramOnlyDialog::accept()
{
    if (_pShape)
    {
        ClassShape* pFromShape = comboClassShape(_ui->comboFromClass);
        ClassShape* pToShape   = comboClassShape(_ui->comboToClass);
        const int type = _ui->radioStaticMulti->isChecked() ? 2
                       : _ui->radioMulti->isChecked()       ? 1
                                                            : 0;
        const int owned = _ui->radioAggregation->isChecked() ? 1
                        : _ui->radioComposition->isChecked() ? 2
                                                             : 0;

        if (pFromShape != _pShape->GetFromClassShape() ||
            _ui->lineEditFromName->text() != toQ(_pShape->GetFromName()) ||
            _ui->lineEditFromMultiplicity->text() != toQ(_pShape->GetUmlFrom()) ||
            pToShape != _pShape->GetToClassShape() ||
            _ui->lineEditToName->text() != toQ(_pShape->GetToName()) ||
            _ui->lineEditToMultiplicity->text() != toQ(_pShape->GetUmlTo()) ||
            type != (_pShape->GetMulti() + _pShape->GetStatic()) ||
            owned != _pShape->GetOwned())
        {
            _pShape->SaveState(1);
            applyAttributes(_pShape);

            bool newRouting = false;
            if (pFromShape != _pShape->GetFromClassShape())
            {
                newRouting = true;
                pFromShape->MoveFromConnectionShapeLast(_pShape);
            }
            if (pToShape != _pShape->GetToClassShape())
            {
                newRouting = true;
                pToShape->MoveToConnectionShapeLast(_pShape);
            }
            if (newRouting)
            {
                _pShape->SetStartPoint(pFromShape->ConnectionPoint(pToShape));
                _pShape->SetEndPoint(pToShape->ConnectionPoint(pFromShape));
                _pShape->SetInitial(true);
                _pShape->MakeNewRouting();
            }
            else
            {
                _pShape->ConvertRouting();
            }
            _modelChanged = true;
        }
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry points
// ---------------------------------------------------------------------------
bool Qt_ShowRelationDiagramOnlyDialog(RelationDiagramOnlyShape* pShape,
                                      bool& modelChangedOut, void* ownerHwnd)
{
    Qt_EnsureApplication();

    RelationDiagramOnlyDialog dlg(pShape);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    return true;
}

RelationDiagramOnlyShape* Qt_CreateRelationDiagramOnlyDialog(
    ClassDiagram* pClassDiagram, ClassShape* initFrom, ClassShape* initTo,
    void* ownerHwnd)
{
    Qt_EnsureApplication();

    RelationDiagramOnlyDialog dlg(pClassDiagram, initFrom, initTo);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return nullptr;

    ClassShape* from = dlg.fromShape();
    ClassShape* to   = dlg.toShape();
    if (!from || !to)
        return nullptr;

    // Create the shape ONLY now -- after OK -- then apply the gathered spec and
    // route it once (MakeNewRouting rebuilds segments from scratch for the chosen
    // owned/multi; the ctor already routed with defaults, so a plain ConvertRouting
    // here would double-convert the typed start/end segments and crash).
    RelationDiagramOnlyShape* pShape =
        new RelationDiagramOnlyShape(pClassDiagram, from, to);
    dlg.applyAttributes(pShape);
    pShape->MakeNewRouting();
    return pShape;
}
