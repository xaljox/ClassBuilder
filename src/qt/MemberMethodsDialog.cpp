// qt/MemberMethodsDialog.cpp -- the Qt Member Methods dialog.
//
// Ported from the MFC CMemberMethodsDialog. Lists the public methods of the
// member's (class) type; on OK, wraps each selected method onto the member.
// Drives the model directly.

#include "MemberMethodsDialog.h"
#include "ui_MemberMethodsDialog.h"

#include "QtMemberMethodsDialog.h"   // Qt_ShowMemberMethodsDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ
#include "QtCompact.h"               // compactItemSize

#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QPushButton>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

MemberMethodsDialog::MemberMethodsDialog(Member* pMember, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::MemberMethodsDialog)
    , _pMember(pMember)
{
    _ui->setupUi(this);

    setWindowTitle(toQ(_pMember->GetBaseClass()->GetName()) + "::" +
                   toQ(_pMember->GetPrefixedName()) + " Member Methods");

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &MemberMethodsDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &MemberMethodsDialog::reject);
    connect(_ui->buttonSelectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(true); });
    connect(_ui->buttonUnselectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(false); });

    fillList();
}

MemberMethodsDialog::~MemberMethodsDialog()
{
    delete _ui;
}

// Populate the list with the public methods of the member's class type that
// aren't already wrapped, and aren't constructors / destructors / fixed.
// Each item carries its Method* in Qt::UserRole.
void MemberMethodsDialog::fillList()
{
    BaseClass* pBaseClass = dynamic_cast<BaseClass*>(_pMember->GetType());
    if (!pBaseClass)
        return;

    BaseClass::MethodIterator method(pBaseClass, &Method::IsPublicMethod);
    while (++method)
    {
        if (!_pMember->FindSimilarMethod(method) &&
            !method->IsConstructor() && !method->IsDestructor() &&
            !method->IsFixedMethod())
        {
            const QString text = toQ(method->GetItemText());
            QListWidgetItem* item =
                new QListWidgetItem(text, _ui->listMethods);
            item->setData(Qt::UserRole, QVariant::fromValue(
                reinterpret_cast<qulonglong>((Method*)method)));
            item->setSizeHint(compactItemSize(_ui->listMethods, text, true));

            // A checkbox per row -- the dialog is a multi-pick list, and a
            // checkbox makes "these are selectable, tick the ones you want"
            // unmistakable (clearer than Ctrl/Shift extended-selection).
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
        }
    }
}

// Tick or untick every row (the Select All / Unselect All buttons).
void MemberMethodsDialog::setAllChecked(bool checked)
{
    const Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    for (int i = 0; i < _ui->listMethods->count(); ++i)
        _ui->listMethods->item(i)->setCheckState(state);
}

// OK -- wrap each checked method onto the member (the MFC OnOK).
void MemberMethodsDialog::accept()
{
    for (int i = 0; i < _ui->listMethods->count(); ++i)
    {
        QListWidgetItem* item = _ui->listMethods->item(i);
        if (item->checkState() != Qt::Checked)
            continue;

        Method* pMethod = reinterpret_cast<Method*>(
            item->data(Qt::UserRole).toULongLong());

        WrapMemberMethod* pNewMethod = new WrapMemberMethod(_pMember, pMethod);
        pNewMethod->Add();
        pNewMethod->GetBaseClass()->NotifyAddMethod(pNewMethod);
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowMemberMethodsDialog(Member* pMember, void* ownerHwnd)
{
    Qt_EnsureApplication();

    MemberMethodsDialog dlg(pMember);
    Qt_ExecModal(dlg, ownerHwnd);
}
