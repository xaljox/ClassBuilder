// qt/VariableMethodDialog.cpp -- the Qt Variable->Method() Wizard dialog.
//
// Ported from the MFC VariableMethodDialog. Launched from the code editor's
// Insert -> Variable->Method(): a tree of the in-scope variables ("this",
// arguments, locals parsed from the body) expanded into their reachable
// relations / methods / members. Selecting a node yields an access-path
// string (e.g. "this->GetChild()->Update()") spliced into the editor.
// Drives the model directly.

#include "VariableMethodDialog.h"
#include "ui_VariableMethodDialog.h"

#include "QtVariableMethodDialog.h"  // Qt_ShowVariableMethodDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ
#include "QtModelIcons.h"            // Qt_ModelIcon

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDialogButtonBox>
#include <QMessageBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// C-identifier character (the MFC __iscsym): letter, digit or underscore.
bool isCSym(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_'); }

// "->Name(arg1, arg2)" for a method node.
QString methodString(Method* pMethod)
{
    QString str = "->" + toQ(pMethod->GetName()) + "(";
    Method::ArgumentIterator iArgument(pMethod);
    while (++iArgument)
    {
        str += toQ(iArgument->GetVariableName());
        if (!iArgument.IsLast())
            str += ", ";
    }
    str += ")";
    return str;
}

// A node "carries data" (MFC SetItemData != 0) iff it contributes its text to
// a descendant's composed path. Stored as a bool in Qt::UserRole.
void setNode(QTreeWidgetItem* item, bool carriesData)
{
    item->setData(0, Qt::UserRole, carriesData);
}
bool nodeCarriesData(const QTreeWidgetItem* item)
{
    return item->data(0, Qt::UserRole).toBool();
}

} // namespace

VariableMethodDialog::VariableMethodDialog(Method* pMethod,
                                           const QString& code,
                                           QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::VariableMethodDialog)
    , _pMethod(pMethod)
    , _code(code)
{
    _ui->setupUi(this);

    fillVariableList();

    connect(_ui->tree, &QTreeWidget::itemSelectionChanged,
            this, &VariableMethodDialog::onSelectionChanged);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &VariableMethodDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &VariableMethodDialog::reject);
}

VariableMethodDialog::~VariableMethodDialog()
{
    delete _ui;
}

// Find a top-level item by text (the MFC FindTreeItem, TVI_ROOT only).
QTreeWidgetItem* VariableMethodDialog::findTreeItem(const QString& text)
{
    for (int i = 0; i < _ui->tree->topLevelItemCount(); ++i)
        if (_ui->tree->topLevelItem(i)->text(0) == text)
            return _ui->tree->topLevelItem(i);
    return nullptr;
}

// Recurse a class/extern class: relation macro methods, owned relations,
// methods, members (+ their methods), base classes.
void VariableMethodDialog::fillVariableTree(BaseClass* pBaseClass,
                                            QTreeWidgetItem* parent)
{
    pBaseClass->SortMethod(Method::CompareTreeSaveState);

    Class* pClass = dynamic_cast<Class*>(pBaseClass);

    // Top-level call: reset the cycle-guard flag on every class.
    if (!parent->parent())
    {
        DataModel::ClassIterator iClass(
            pBaseClass->GetDataModelDoc()->GetDataModel());
        while (++iClass)
            iClass->SetFlag();
    }

    if (pClass)
    {
        // From-relations that expose macro methods.
        Class::FromRelationIterator iFromRelation(pClass);
        while (++iFromRelation)
        {
            FromRelationMacroMethods* pMacro =
                iFromRelation->GetFromRelation()->GetFromRelationMacroMethods();
            if (iFromRelation->GetStatic() || !pMacro)
                continue;

            QTreeWidgetItem* item = new QTreeWidgetItem(parent);
            item->setText(0,
                toQ(iFromRelation->GetFromRelation()->GetItemText()));
            item->setIcon(0,
                Qt_ModelIcon(iFromRelation->GetFromRelation()->GetIcon()));
            setNode(item, false);

            MacroMethods::MacroMethodIterator iMethod(pMacro);
            while (++iMethod)
            {
                QTreeWidgetItem* m = new QTreeWidgetItem(item);
                m->setText(0, methodString(iMethod));
                m->setIcon(0, Qt_ModelIcon(iMethod->GetIcon()));
                setNode(m, true);
            }
        }

        // To-relations -- recurse into owned ones.
        Class::ToRelationIterator iToRelation(pClass);
        while (++iToRelation)
        {
            if (iToRelation->GetStatic())
                continue;
            QTreeWidgetItem* item = new QTreeWidgetItem(parent);
            item->setText(0,
                "->Get" + toQ(iToRelation->GetFromName()) + "()");
            item->setIcon(0,
                Qt_ModelIcon(iToRelation->GetToRelation()->GetIcon()));
            setNode(item, true);
            if (iToRelation->GetOwned() &&
                !iToRelation->GetFromClass()->GetFlag())
            {
                iToRelation->GetFromClass()->SetFlag(1);
                fillVariableTree(iToRelation->GetFromClass(), item);
                iToRelation->GetFromClass()->SetFlag(0);
            }
        }
    }

    // Plain methods (skip ctor/dtor/member-method/fixed, but keep IsClass
    // methods and the fixed find methods).
    BaseClass::MethodIterator iMethod(pBaseClass);
    while (++iMethod)
    {
        if ((!iMethod->IsConstructor() && !iMethod->IsDestructor() &&
             !iMethod->IsMemberMethod() && !iMethod->IsFixed()) ||
            iMethod->IsIsClassMethod() ||
            (iMethod->IsFindMethod() && iMethod->IsFixed()))
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(parent);
            item->setText(0, methodString(iMethod));
            item->setIcon(0, Qt_ModelIcon(iMethod->GetIcon()));
            setNode(item, true);
        }
    }

    // Members, each with its own methods.
    BaseClass::MemberIterator iMember(pBaseClass);
    while (++iMember)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent);
        item->setText(0, "->" + toQ(iMember->GetPrefixedName()));
        item->setIcon(0, Qt_ModelIcon(iMember->GetIcon()));
        setNode(item, false);

        Member::MethodIterator iMemberMethod(iMember);
        while (++iMemberMethod)
        {
            QTreeWidgetItem* m = new QTreeWidgetItem(item);
            m->setText(0, methodString(iMemberMethod));
            m->setIcon(0, Qt_ModelIcon(iMemberMethod->GetIcon()));
            setNode(m, true);
        }
    }

    // Base classes.
    if (pClass)
    {
        Class::InheritIterator iInherit(pClass);
        while (++iInherit)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(parent);
            item->setText(0, toQ(iInherit->GetBaseName()));
            item->setIcon(0,
                Qt_ModelIcon(iInherit->GetBaseClass()->GetIcon()));
            setNode(item, false);
            fillVariableTree(iInherit->GetBaseClass(), item);
        }
    }
}

// Scan `code` for `TypeName <var>` declarations, adding each found variable as
// a top-level node. (The MFC FindAndAddVariables.)
void VariableMethodDialog::findAndAddVariables(Class* pClass, QString code,
                                               const QString& typeName,
                                               bool iterator)
{
    const int len = typeName.length();
    int index;
    while ((index = code.indexOf(typeName)) != -1)
    {
        const int afterIndex = index + len;
        if (index > 0 && isCSym(code[index - 1]))
        {
            code = code.mid(afterIndex + 1);
            continue;
        }
        if (afterIndex < code.length() && isCSym(code[afterIndex]))
        {
            code = code.mid(afterIndex + 1);
            continue;
        }
        code = code.mid(afterIndex + 1);

        int pointer = 0;
        int reference = 0;
        for (int i = 0; i < code.length(); ++i)
        {
            const QChar c = code[i];
            if (c == QLatin1Char('*')) pointer = i + 1;
            if (c == QLatin1Char('&')) reference = i + 1;

            if (c == QLatin1Char('=') || c == QLatin1Char(';') ||
                (c == QLatin1Char('(') && iterator))
            {
                int start = pointer;
                if (reference > start)
                    start = reference;

                const QString variable =
                    code.mid(start, i - start).trimmed();
                if (!findTreeItem(variable))
                {
                    QTreeWidgetItem* item = new QTreeWidgetItem(_ui->tree);
                    item->setText(0, variable);
                    item->setIcon(0, Qt_ModelIcon(ICON_ARGUMENT));
                    setNode(item, true);
                    fillVariableTree(pClass, item);
                }
                break;
            }
            if ((c == QLatin1Char('(') && !iterator) ||
                c == QLatin1Char(')') || c == QLatin1Char('[') ||
                c == QLatin1Char('.'))
            {
                break;
            }
        }
    }
}

// Build the whole variable tree (the MFC FillVariableList).
void VariableMethodDialog::fillVariableList()
{
    Class* pClass = dynamic_cast<Class*>(_pMethod->GetBaseClass());
    if (!pClass)
        return;

    // A non-static method gets the "this" variable first.
    if (!_pMethod->GetStatic())
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(_ui->tree);
        item->setText(0, "this");
        item->setIcon(0, Qt_ModelIcon(ICON_ARGUMENT));
        setNode(item, true);
        fillVariableTree(pClass, item);
    }

    // Arguments.
    Method::ArgumentIterator argument(_pMethod);
    while (++argument)
    {
        BaseClass* pArgClass = dynamic_cast<BaseClass*>(argument->GetType());
        if (pArgClass)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(_ui->tree);
            item->setText(0, toQ(argument->GetName()));
            item->setIcon(0, Qt_ModelIcon(ICON_ARGUMENT));
            setNode(item, true);
            fillVariableTree(pArgClass, item);
        }
        else if (argument->GetType()->GetName() != "...")
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(_ui->tree);
            item->setText(0, toQ(argument->GetName()));
            item->setIcon(0, Qt_ModelIcon(ICON_ARGUMENT));
            setNode(item, false);
        }
    }

    // Variable declarations parsed out of the method body.
    DataModel::ClassIterator iClass(pClass->GetDataModel());
    while (++iClass)
    {
        findAndAddVariables(iClass, _code, toQ(iClass->GetName()));
        Class::FromRelationIterator rel(iClass, &Relation::GetMulti);
        while (++rel)
        {
            findAndAddVariables(rel->GetToClass(), _code,
                toQ(iClass->GetName()) + "::" +
                toQ(rel->GetToName()) + "Iterator", true);

            if (iClass.Get() == pClass || pClass->IsBaseClass(iClass))
                findAndAddVariables(rel->GetToClass(), _code,
                    toQ(rel->GetToName()) + "Iterator", true);
        }
    }
}

// Selection changed: compose the access path -- the selected node's own text,
// prefixed by every ancestor that carries data. Strip a leading "this->".
void VariableMethodDialog::onSelectionChanged()
{
    QList<QTreeWidgetItem*> selected = _ui->tree->selectedItems();
    if (selected.isEmpty())
        return;

    QTreeWidgetItem* item = selected.first();
    _insertCode = item->text(0);
    for (QTreeWidgetItem* it = item->parent(); it; it = it->parent())
        if (nodeCarriesData(it))
            _insertCode = it->text(0) + _insertCode;

    if (_insertCode.startsWith("this->"))
        _insertCode = _insertCode.mid(6);
}

// OK -- a variable must have been picked (the MFC DDV_Variable).
void VariableMethodDialog::accept()
{
    if (_insertCode.isEmpty())
    {
        QMessageBox::warning(this, "Variable->Method() Wizard",
                             "Must select a variable");
        return;
    }
    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowVariableMethodDialog(Method* pMethod, const char* code,
                                 CbString& insertCode, void* ownerHwnd)
{
    Qt_EnsureApplication();

    VariableMethodDialog dlg(pMethod, QString::fromLocal8Bit(code));
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    insertCode = CbString(dlg.insertCode().toLocal8Bit().constData());
    return true;
}
