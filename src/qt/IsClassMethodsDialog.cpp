// qt/IsClassMethodsDialog.cpp -- the Qt IsClass Methods dialog.
//
// Ported from the MFC CIsClassMethodsDialog. Lists the `Is<Base>()` methods
// the class could gain from its (transitively) inherited base classes; on OK,
// adds an IsClassMethod for each ticked entry. Drives the model directly.

#include "IsClassMethodsDialog.h"
#include "ui_IsClassMethodsDialog.h"

#include "QtIsClassMethodsDialog.h"  // Qt_ShowIsClassMethodsDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ
#include "QtCompact.h"               // compactItemSize

#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

#include "CbViewLock.h"   // CbViewLock guard (constructed in accept(); the
                         // FORWARD_ONLY master only forward-declares it)

IsClassMethodsDialog::IsClassMethodsDialog(Class* pClass,
                                           MemberAndMethodGroup* pGroup,
                                           QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::IsClassMethodsDialog)
    , _pClass(pClass)
    , _pGroup(pGroup)
{
    _ui->setupUi(this);

    setWindowTitle("IsClass Methods for class " + toQ(_pClass->GetName()));

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &IsClassMethodsDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &IsClassMethodsDialog::reject);
    connect(_ui->buttonSelectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(true); });
    connect(_ui->buttonUnselectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(false); });

    BaseClass::InheritIterator inherit(_pClass);
    while (++inherit)
    {
        Class* pBase = dynamic_cast<Class*>(inherit->GetExternClass());
        if (pBase)
            fillList(pBase);
    }
}

IsClassMethodsDialog::~IsClassMethodsDialog()
{
    delete _ui;
}

// Walk one inherited base class: if `_pClass` does not already have an
// IsClassMethod for `pRefClass`, add a candidate row; then recurse into
// `pRefClass`'s own base classes. Each item carries its Class* in UserRole.
void IsClassMethodsDialog::fillList(Class* pRefClass)
{
    bool present = false;
    Class::IsClassMethodIterator method(pRefClass);
    while (++method)
    {
        if (method->GetBaseClass() == _pClass)
        {
            present = true;
            break;
        }
    }

    if (!present)
    {
        const QString text = "bool Is" + toQ(pRefClass->GetBaseName()) + "()";
        QListWidgetItem* item = new QListWidgetItem(text, _ui->listMethods);
        item->setData(Qt::UserRole, QVariant::fromValue(
            reinterpret_cast<qulonglong>(pRefClass)));
        item->setSizeHint(compactItemSize(_ui->listMethods, text, true));

        // A checkbox per row -- the dialog is a multi-pick list.
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }

    BaseClass::InheritIterator inherit(pRefClass);
    while (++inherit)
    {
        Class* pBase = dynamic_cast<Class*>(inherit->GetExternClass());
        if (pBase)
            fillList(pBase);
    }
}

// Tick or untick every row (the Select All / Unselect All buttons).
void IsClassMethodsDialog::setAllChecked(bool checked)
{
    const Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    for (int i = 0; i < _ui->listMethods->count(); ++i)
        _ui->listMethods->item(i)->setCheckState(state);
}

// OK -- add an IsClassMethod for each ticked row (the MFC OnOK).
void IsClassMethodsDialog::accept()
{
    int checkedCount = 0;
    for (int i = 0; i < _ui->listMethods->count(); ++i)
        if (_ui->listMethods->item(i)->checkState() == Qt::Checked)
            ++checkedCount;

    // Warn about the RTTI compiler setting when adding the model's first
    // IsClass method.
    if (checkedCount)
    {
        bool first = true;
        DataModel::ClassIterator iClass(_pClass->GetDataModel());
        while (first && ++iClass)
        {
            Class::IsClassMethodIterator iMethod(iClass);
            while (first && ++iMethod)
                first = false;
        }

        if (first)
        {
            QMessageBox::information(this, "IsClass Methods",
                "Don't forget to enable the Run Time Type Information (RTTI) "
                "option on your compiler.");
        }
    }

    // Coalesce the per-method tree refreshes into one -- select-all can add many
    // IsClass methods at once (each Add()/NotifyAddMethod otherwise rebuilds the
    // whole Qt tree). Guard runs to the end of accept().
    CbViewLock lock(_pClass->GetDataModelDoc());

    for (int i = 0; i < _ui->listMethods->count(); ++i)
    {
        QListWidgetItem* item = _ui->listMethods->item(i);
        if (item->checkState() != Qt::Checked)
            continue;

        Class* pRefClass = reinterpret_cast<Class*>(
            item->data(Qt::UserRole).toULongLong());

        Method* pNewMethod = new IsClassMethod(_pClass, pRefClass);
        if (_pGroup)
            _pGroup->AddMethodLast(pNewMethod);

        pNewMethod->Add();
        _pClass->NotifyAddMethod(pNewMethod);
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowIsClassMethodsDialog(Class* pClass, MemberAndMethodGroup* pGroup,
                                 void* ownerHwnd)
{
    Qt_EnsureApplication();

    IsClassMethodsDialog dlg(pClass, pGroup);
    Qt_ExecModal(dlg, ownerHwnd);
}
