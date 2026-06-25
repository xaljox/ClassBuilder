// qt/DependencyDialog.cpp -- the Qt Dependency properties dialog.
// Ported from the MFC DependencyDialog.
//
// Edit and Create share applyControls(). Create does NOT build the shape -- the
// bridge creates it AFTER OK and calls applyControls(), because a dependency is
// a standalone diagram shape (no tree object), created directly only on commit.

#include "DependencyDialog.h"
#include "ui_DependencyDialog.h"

#include "QtDependencyDialog.h"   // Qt_Show/CreateDependencyDialog
#include "QtApp.h"                // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"          // toQ / toCb (CRLF-aware)
#include "QtCompact.h"            // compactCombo

#include <QComboBox>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// Populate a class-shape combo, sort it, select the current shape.
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
    combo->model()->sort(0);          // CBS_SORT
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
DependencyDialog::DependencyDialog(DependencyShape* pDependencyShape,
                                   QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::DependencyDialog)
    , _pDependencyShape(pDependencyShape)
{
    _ui->setupUi(this);

    _ui->lineEditName->setText(toQ(_pDependencyShape->GetName()));
    _ui->lineEditStereotype->setText(toQ(_pDependencyShape->GetStereotype()));

    ClassDiagram* pClassDiagram = _pDependencyShape->GetClassDiagram();
    fillClassCombo(_ui->comboFromClass, pClassDiagram,
                   _pDependencyShape->GetFromClassShape());
    fillClassCombo(_ui->comboToClass, pClassDiagram,
                   _pDependencyShape->GetToClassShape());

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &DependencyDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &DependencyDialog::reject);
}

// Create: no shape yet. Seed the combos from the diagram; the shape is built by
// the bridge after OK (see below).
DependencyDialog::DependencyDialog(ClassDiagram* pClassDiagram,
                                   ClassShape* pInitFrom, ClassShape* pInitTo,
                                   QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::DependencyDialog)
    , _pDependencyShape(nullptr)
{
    _ui->setupUi(this);

    _ui->lineEditName->clear();
    _ui->lineEditStereotype->clear();
    fillClassCombo(_ui->comboFromClass, pClassDiagram, pInitFrom);
    fillClassCombo(_ui->comboToClass, pClassDiagram, pInitTo);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &DependencyDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &DependencyDialog::reject);
}

DependencyDialog::~DependencyDialog()
{
    delete _ui;
}

ClassShape* DependencyDialog::fromShape() const
{
    return comboClassShape(_ui->comboFromClass);
}

ClassShape* DependencyDialog::toShape() const
{
    return comboClassShape(_ui->comboToClass);
}

// Write the dialog's fields onto `pShape` -- NO routing, NO SaveState. Shared by
// the edit accept() and the create bridge; the caller does the routing.
void DependencyDialog::applyAttributes(DependencyShape* pShape) const
{
    pShape->SetStereotype(toCb(_ui->lineEditStereotype->text()));
    pShape->SetName(toCb(_ui->lineEditName->text()));
}

// OK on the EDIT path -- apply to the live shape if anything changed. Create mode
// has no shape (_pDependencyShape == null): it only gathers; the bridge builds
// after OK.
void DependencyDialog::accept()
{
    if (_pDependencyShape)
    {
        ClassShape* pFromShape = comboClassShape(_ui->comboFromClass);
        ClassShape* pToShape   = comboClassShape(_ui->comboToClass);

        if (pFromShape != _pDependencyShape->GetFromClassShape() ||
            _ui->lineEditStereotype->text() != toQ(_pDependencyShape->GetStereotype()) ||
            pToShape != _pDependencyShape->GetToClassShape() ||
            _ui->lineEditName->text() != toQ(_pDependencyShape->GetName()))
        {
            _pDependencyShape->SaveState(1);
            applyAttributes(_pDependencyShape);

            bool newRouting = false;
            if (pFromShape != _pDependencyShape->GetFromClassShape())
            {
                newRouting = true;
                pFromShape->MoveFromConnectionShapeLast(_pDependencyShape);
            }
            if (pToShape != _pDependencyShape->GetToClassShape())
            {
                newRouting = true;
                pToShape->MoveToConnectionShapeLast(_pDependencyShape);
            }
            if (newRouting)
            {
                _pDependencyShape->SetStartPoint(
                    pFromShape->ConnectionPoint(pToShape));
                _pDependencyShape->SetEndPoint(
                    pToShape->ConnectionPoint(pFromShape));
                _pDependencyShape->SetInitial(true);
                _pDependencyShape->MakeNewRouting();
            }
            else
            {
                _pDependencyShape->ConvertRouting();
            }
            _modelChanged = true;
        }
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry points
// ---------------------------------------------------------------------------
bool Qt_ShowDependencyDialog(DependencyShape* pDependencyShape,
                             bool& modelChangedOut, void* ownerHwnd)
{
    Qt_EnsureApplication();

    DependencyDialog dlg(pDependencyShape);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    return true;
}

DependencyShape* Qt_CreateDependencyDialog(
    ClassDiagram* pClassDiagram, ClassShape* initFrom, ClassShape* initTo,
    void* ownerHwnd)
{
    Qt_EnsureApplication();

    DependencyDialog dlg(pClassDiagram, initFrom, initTo);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return nullptr;

    ClassShape* from = dlg.fromShape();
    ClassShape* to   = dlg.toShape();
    if (!from || !to)
        return nullptr;

    // Create the shape ONLY now -- after OK -- then apply the gathered fields and
    // route it once (the ctor already routed; MakeNewRouting rebuilds cleanly --
    // a plain ConvertRouting here would double-convert the typed end segment).
    DependencyShape* pShape = new DependencyShape(pClassDiagram, from, to);
    dlg.applyAttributes(pShape);
    pShape->MakeNewRouting();
    return pShape;
}
