// qt/VirtualMethodsDialog.cpp -- the Qt Virtual Methods dialog.
//
// Ported from the MFC CVirtualMethodsDialog. A multi-select checkbox list,
// two modes:
//   * extern-class -- list the virtual methods an ExternClass inherits but
//     does not yet override; on OK create an override Method for each ticked.
//   * method -- list the (extern) base classes one Method could be overridden
//     into; on OK create the override in each ticked base.
// Drives the model directly.

#include "VirtualMethodsDialog.h"
#include "ui_VirtualMethodsDialog.h"

#include "QtVirtualMethodsDialog.h"  // Qt_ShowVirtualMethodsDialog*
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ
#include "QtCompact.h"               // compactItemSize

#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QPushButton>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

// Strip a trailing " = 0" -- collapses a pure-virtual signature to its plain
// form so the same method seen pure in one base and non-pure in another
// dedups to a single entry (the MFC PureVirtual2Virtual).
static QString pureToVirtual(const QString& text)
{
    return text.endsWith(" = 0") ? text.left(text.length() - 4) : text;
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
VirtualMethodsDialog::VirtualMethodsDialog(ExternClass* pExternClass,
                                           MemberAndMethodGroup* pGroup,
                                           QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::VirtualMethodsDialog)
    , _pExternClass(pExternClass)
    , _pGroup(pGroup)
{
    _ui->setupUi(this);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &VirtualMethodsDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &VirtualMethodsDialog::reject);
    connect(_ui->buttonSelectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(true); });
    connect(_ui->buttonUnselectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(false); });

    initExternClass();
}

VirtualMethodsDialog::VirtualMethodsDialog(Method* pMethod, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::VirtualMethodsDialog)
    , _pMethod(pMethod)
{
    _ui->setupUi(this);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &VirtualMethodsDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &VirtualMethodsDialog::reject);
    connect(_ui->buttonSelectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(true); });
    connect(_ui->buttonUnselectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(false); });

    initMethod();
}

VirtualMethodsDialog::~VirtualMethodsDialog()
{
    delete _ui;
}

// ---------------------------------------------------------------------------
// Extern-class mode
// ---------------------------------------------------------------------------
void VirtualMethodsDialog::initExternClass()
{
    setWindowTitle("Virtual Methods for class " +
                   toQ(_pExternClass->GetName()));
    _ui->methodsGroup->setTitle("Inherited virtual methods to override");

    if (!_pExternClass->GetInheritCount())
        return;

    // Seed the dedup set with the virtual methods the class already declares,
    // so an inherited method it has already overridden is not offered again.
    QSet<QString> seen;
    Class::MethodIterator method(_pExternClass, &Method::GetVirtual);
    while (++method)
        seen.insert(pureToVirtual(toQ(method->GetItemText())));

    Class::InheritIterator inherit(_pExternClass);
    while (++inherit)
        fillExternClass(inherit->GetBaseClass(), seen);
}

// Recurse a base class (bases first, then its own virtual methods), adding
// each not-yet-seen virtual method as a candidate row.
void VirtualMethodsDialog::fillExternClass(BaseClass* pBaseClass,
                                           QSet<QString>& seen)
{
    ExternClass* pClass = dynamic_cast<ExternClass*>(pBaseClass);
    if (pClass)
    {
        Class::InheritIterator inherit(pClass);
        while (++inherit)
            fillExternClass(inherit->GetBaseClass(), seen);
    }

    Class::MethodIterator method(pBaseClass, &Method::GetVirtual);
    while (++method)
    {
        if (method->IsDestructor())
            continue;
        const QString key = pureToVirtual(toQ(method->GetItemText()));
        if (seen.contains(key))
            continue;
        seen.insert(key);
        addExternRow(method.Get());
    }
}

// One candidate row -- abstract (pure) methods are flagged and pre-ticked.
void VirtualMethodsDialog::addExternRow(Method* pMethod)
{
    QString text = toQ(pMethod->GetSignalShapeText(true, false, true));
    const bool pure = pMethod->GetPure();
    if (pure)
        text = "<<Abstract>> " + text;

    QListWidgetItem* item = new QListWidgetItem(text, _ui->listMethods);
    item->setData(Qt::UserRole, QVariant::fromValue(
        reinterpret_cast<qulonglong>(pMethod)));
    item->setSizeHint(compactItemSize(_ui->listMethods, text, true));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(pure ? Qt::Checked : Qt::Unchecked);
}

// ---------------------------------------------------------------------------
// Method mode
// ---------------------------------------------------------------------------
void VirtualMethodsDialog::initMethod()
{
    setWindowTitle("Override virtual method '" +
                   toQ(_pMethod->GetBaseClass()->GetName()) + "::" +
                   toQ(_pMethod->GetName()) + "'");
    _ui->methodsGroup->setTitle("Override into base class");

    fillMethod(_pMethod->GetBaseClass());
}

// Recurse the method's base classes; offer each extern base that does not
// already carry a similar method as an override target.
void VirtualMethodsDialog::fillMethod(BaseClass* pBaseClass)
{
    if (_pMethod->GetBaseClass() != pBaseClass &&
        !pBaseClass->FindSimilarMethod(_pMethod) &&
        pBaseClass->IsExternClass())
    {
        QString sig = toQ(_pMethod->GetSignalShapeText(true, false, true));
        const int sep = sig.indexOf("::");
        const QString text = toQ(pBaseClass->GetName()) +
                              (sep >= 0 ? sig.mid(sep) : sig);

        QListWidgetItem* item = new QListWidgetItem(text, _ui->listMethods);
        item->setData(Qt::UserRole, QVariant::fromValue(
            reinterpret_cast<qulonglong>(pBaseClass)));
        item->setSizeHint(compactItemSize(_ui->listMethods, text, true));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }

    BaseClass::InheritIterator iInherit(pBaseClass);
    while (++iInherit)
        fillMethod(iInherit->GetExternClass());
}

// ---------------------------------------------------------------------------
// Shared
// ---------------------------------------------------------------------------
void VirtualMethodsDialog::setAllChecked(bool checked)
{
    const Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    for (int i = 0; i < _ui->listMethods->count(); ++i)
        _ui->listMethods->item(i)->setCheckState(state);
}

// Build one override Method as a non-pure, declared+implemented copy of
// `pSource` placed on `pTarget` (the shared tail of both MFC OnOk paths).
static Method* makeOverride(BaseClass* pTarget, Method* pSource)
{
    Method* pNewMethod = new Method(pTarget, pSource);
    pNewMethod->SetPure(0);
    pNewMethod->SetDeclare(1);
    pNewMethod->SetImplement(1);
    pNewMethod->SetCode("");
    pNewMethod->InitCode();
    return pNewMethod;
}

// OK -- create an override per ticked row (the MFC OnOkExternClass /
// OnOkMethod, branched on mode).
void VirtualMethodsDialog::accept()
{
    for (int i = 0; i < _ui->listMethods->count(); ++i)
    {
        QListWidgetItem* item = _ui->listMethods->item(i);
        if (item->checkState() != Qt::Checked)
            continue;

        const qulonglong ptr = item->data(Qt::UserRole).toULongLong();

        if (_pMethod)
        {
            // Method mode: the row carries the target base class.
            BaseClass* pBaseClass = reinterpret_cast<BaseClass*>(ptr);
            Method* pNewMethod = makeOverride(pBaseClass, _pMethod);
            pNewMethod->Add();
            pBaseClass->NotifyAddMethod(pNewMethod);
        }
        else
        {
            // Extern-class mode: the row carries the inherited method.
            Method* pMethod = reinterpret_cast<Method*>(ptr);
            Method* pNewMethod = makeOverride(_pExternClass, pMethod);
            if (_pGroup)
                _pGroup->AddMethodLast(pNewMethod);
            pNewMethod->Add();
            _pExternClass->NotifyAddMethod(pNewMethod);
        }
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry points
// ---------------------------------------------------------------------------
void Qt_ShowVirtualMethodsDialog(ExternClass* pExternClass,
                                 MemberAndMethodGroup* pGroup, void* ownerHwnd)
{
    Qt_EnsureApplication();

    VirtualMethodsDialog dlg(pExternClass, pGroup);
    Qt_ExecModal(dlg, ownerHwnd);
}

void Qt_ShowVirtualMethodsDialogForMethod(Method* pMethod, void* ownerHwnd)
{
    Qt_EnsureApplication();

    VirtualMethodsDialog dlg(pMethod);
    Qt_ExecModal(dlg, ownerHwnd);
}
