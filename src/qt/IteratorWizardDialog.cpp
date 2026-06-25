// qt/IteratorWizardDialog.cpp -- the Qt Iterator Wizard dialog.
//
// Ported from the MFC CIteratorWizardDialog. Launched from the code editor's
// Insert -> Iterator: pick a variable (tree), a multi relation (list) and an
// optional filter method (list), name the iterator, choose reset / backward,
// and OK produces a C++ iterator snippet to splice into the method body.
// Drives the model directly.

#include "IteratorWizardDialog.h"
#include "ui_IteratorWizardDialog.h"

#include "QtIteratorWizardDialog.h"  // Qt_ShowIteratorWizardDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "QtModelIcons.h"            // Qt_ModelIcon
#include "QtCompact.h"               // compactItemSize

#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// C-identifier character (the MFC __iscsym): letter, digit or underscore.
bool isCSym(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_'); }

// Tree node payload: a model Class* (null on a base-class header node), plus
// a flag marking the static "Class::" entries (their selection drives the
// relation list from the class's *static* relations).
void setNode(QTreeWidgetItem* item, Class* pClass, bool staticClass)
{
    item->setData(0, Qt::UserRole,
                  QVariant::fromValue(reinterpret_cast<qulonglong>(pClass)));
    item->setData(0, Qt::UserRole + 1, staticClass);
}
Class* nodeClass(const QTreeWidgetItem* item)
{
    return reinterpret_cast<Class*>(item->data(0, Qt::UserRole).toULongLong());
}
bool nodeStatic(const QTreeWidgetItem* item)
{
    return item->data(0, Qt::UserRole + 1).toBool();
}

template <class T>
T* listPtr(const QListWidgetItem* item)
{
    return reinterpret_cast<T*>(item->data(Qt::UserRole).toULongLong());
}

void addListItem(QListWidget* list, const QString& text, void* ptr)
{
    QListWidgetItem* item = new QListWidgetItem(text, list);
    item->setData(Qt::UserRole,
                  QVariant::fromValue(reinterpret_cast<qulonglong>(ptr)));
    item->setSizeHint(compactItemSize(list, text));
}

} // namespace

IteratorWizardDialog::IteratorWizardDialog(Method* pMethod,
                                           const QString& code,
                                           QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::IteratorWizardDialog)
    , _pMethod(pMethod)
    , _code(code)
{
    _ui->setupUi(this);

    // Build the initial state directly (the MFC OnInitDialog): the variable
    // tree with "this" selected, and the relation list for the method's class.
    fillVariableList();
    fillRelationList(dynamic_cast<Class*>(_pMethod->GetBaseClass()));

    connect(_ui->tree, &QTreeWidget::itemSelectionChanged,
            this, &IteratorWizardDialog::onVariableChanged);
    connect(_ui->listRelation, &QListWidget::itemSelectionChanged,
            this, &IteratorWizardDialog::onRelationChanged);
    connect(_ui->listFilter, &QListWidget::itemSelectionChanged,
            this, &IteratorWizardDialog::onFilterChanged);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &IteratorWizardDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &IteratorWizardDialog::reject);
}

IteratorWizardDialog::~IteratorWizardDialog()
{
    delete _ui;
}

// ---------------------------------------------------------------------------
// Variable tree
// ---------------------------------------------------------------------------

// Find a top-level item (parent == null) or direct child by text.
QTreeWidgetItem* IteratorWizardDialog::findTreeItem(const QString& text,
                                                    QTreeWidgetItem* parent)
{
    if (parent)
    {
        for (int i = 0; i < parent->childCount(); ++i)
            if (parent->child(i)->text(0) == text)
                return parent->child(i);
    }
    else
    {
        for (int i = 0; i < _ui->tree->topLevelItemCount(); ++i)
            if (_ui->tree->topLevelItem(i)->text(0) == text)
                return _ui->tree->topLevelItem(i);
    }
    return nullptr;
}

// Recurse a class: owned non-static multi relations + base classes, building
// the navigable variable sub-tree below `parent`.
void IteratorWizardDialog::fillVariableTree(Class* pClass,
                                            QTreeWidgetItem* parent)
{
    // Top-level call: reset the cycle-guard flag on every class.
    if (!parent->parent())
    {
        DataModel::ClassIterator iClass(pClass->GetDataModel());
        while (++iClass)
            iClass->SetFlag();
    }

    Class::ToRelationIterator iRelation(pClass, &Relation::GetOwned);
    while (++iRelation)
    {
        if (iRelation->GetStatic())
            continue;
        const QString str = "->Get" + toQ(iRelation->GetFromName()) + "()";
        if (findTreeItem(str, parent))
            continue;

        QTreeWidgetItem* item = new QTreeWidgetItem(parent);
        item->setText(0, str);
        item->setIcon(0,
            Qt_ModelIcon(iRelation->GetToRelation()->GetIcon()));
        setNode(item, iRelation->GetFromClass(), false);
        if (!iRelation->GetFromClass()->GetFlag())
        {
            iRelation->GetFromClass()->SetFlag(1);
            fillVariableTree(iRelation->GetFromClass(), item);
            iRelation->GetFromClass()->SetFlag(0);
        }
    }

    Class::InheritIterator iInherit(pClass);
    while (++iInherit)
    {
        Class* pBase = dynamic_cast<Class*>(iInherit->GetBaseClass());
        if (pBase)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(parent);
            item->setText(0, toQ(pBase->GetName()));
            item->setIcon(0, Qt_ModelIcon(pBase->GetIcon()));
            setNode(item, nullptr, false);   // base header: no payload
            fillVariableTree(pBase, item);
        }
    }
}

// Scan `code` for `TypeName <var>` declarations (or `TypeName::XxxIterator
// <var>(` when `iterator`), adding each found variable as a top-level node.
void IteratorWizardDialog::findAndAddVariables(Class* pClass, QString code,
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
                    setNode(item, pClass, false);
                    fillVariableTree(pClass, item);
                }
                break;
            }
            if ((c == QLatin1Char('(') && !iterator) ||
                c == QLatin1Char(')') || c == QLatin1Char('[') ||
                c == QLatin1Char('.') || c == QLatin1Char(':'))
            {
                break;
            }
        }
    }
}

// Build the whole variable tree (the MFC FillVariableList).
void IteratorWizardDialog::fillVariableList()
{
    Class* pClass = dynamic_cast<Class*>(_pMethod->GetBaseClass());
    if (!pClass)
        return;

    // A non-static method gets the "this" variable first, selected by default.
    if (!_pMethod->GetStatic())
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(_ui->tree);
        item->setText(0, "this");
        item->setIcon(0, Qt_ModelIcon(ICON_ARGUMENT));
        setNode(item, pClass, false);
        _ui->tree->setCurrentItem(item);
        _variable = "this";
        fillVariableTree(pClass, item);
    }

    // Class-typed arguments.
    Method::ArgumentIterator argument(_pMethod);
    while (++argument)
    {
        Class* pArgClass = dynamic_cast<Class*>(argument->GetType());
        if (pArgClass)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(_ui->tree);
            item->setText(0, toQ(argument->GetName()));
            item->setIcon(0, Qt_ModelIcon(ICON_ARGUMENT));
            setNode(item, pArgClass, false);
            fillVariableTree(pArgClass, item);
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

    // A "Class::" entry for every class that owns a static relation.
    iClass.Reset();
    while (++iClass)
    {
        Class::FromRelationIterator rel(iClass, &Relation::GetStatic);
        if (++rel)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(_ui->tree);
            item->setText(0, toQ(iClass->GetName()) + "::");
            item->setIcon(0, Qt_ModelIcon(ICON_CLASS));
            setNode(item, rel->GetFromClass(), true);
        }
    }
}

// ---------------------------------------------------------------------------
// Relation / filter lists
// ---------------------------------------------------------------------------

// Recurse a class' multi non-static relations + base classes into listRelation.
void IteratorWizardDialog::fillRelationList(Class* pClass,
                                            const QString& prefix)
{
    Class::FromRelationIterator rel(pClass, &Relation::GetMulti);
    while (++rel)
    {
        if (!rel->GetStatic())
            addListItem(_ui->listRelation,
                        prefix + toQ(rel->GetFromRelation()->GetItemText()),
                        rel.Get());
    }

    Class::InheritIterator inherit(pClass);
    while (++inherit)
    {
        Class* pBase = dynamic_cast<Class*>(inherit->GetBaseClass());
        if (pBase)
            fillRelationList(pBase, toQ(pBase->GetName()) + "::");
    }
}

void IteratorWizardDialog::fillRelationList(Class* pClass)
{
    _ui->listRelation->clear();
    if (pClass)
        fillRelationList(pClass, QString());
    _ui->listFilter->clear();
}

// Filter candidates: bool- or int-returning, const, 0-argument, non-static
// methods of the relation's to-class (and bases) -- skipping IsClass methods
// that cannot apply to that class. (bool is the going-forward predicate return
// type -- IsClassMethod now generates bool; int stays accepted while the legacy
// IsX family is still mid-conversion. See project_classbuilder_int_isx_to_bool.)
void IteratorWizardDialog::fillFilterList(BaseClass* pBaseClass)
{
    BaseClass::MethodIterator method(pBaseClass);
    while (++method)
    {
        const CbString retName = method->GetType()->GetName();
        if (method->GetArgumentCount() == 0 &&
            (retName == "bool" || retName == "int") &&
            !method->GetStatic() &&
            method->GetConst())
        {
            bool present = false;
            for (int i = 0; i < _ui->listFilter->count() && !present; ++i)
                if (_ui->listFilter->item(i)->text() ==
                    toQ(method->GetName()))
                    present = true;
            if (present)
                continue;

            IsClassMethod* pIsClass =
                dynamic_cast<IsClassMethod*>((Method*)method);
            if (!pIsClass ||
                pIsClass->GetRefClass()->IsBaseClass(_pRelation->GetToClass()))
            {
                addListItem(_ui->listFilter, toQ(method->GetName()),
                            method.Get());
            }
        }
    }

    Class* pClass = dynamic_cast<Class*>(pBaseClass);
    if (pClass)
    {
        Class::InheritIterator inherit(pClass);
        while (++inherit)
            fillFilterList(inherit->GetBaseClass());
    }
}

void IteratorWizardDialog::fillFilterList()
{
    _ui->listFilter->clear();

    addListItem(_ui->listFilter, "(none)", nullptr);
    _ui->listFilter->setCurrentRow(0);
    _pFilterMethod = nullptr;

    if (_pRelation && _pRelation->GetFilter())
        fillFilterList(_pRelation->GetToClass());
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void IteratorWizardDialog::onVariableChanged()
{
    QList<QTreeWidgetItem*> selected = _ui->tree->selectedItems();
    if (selected.isEmpty())
        return;

    QTreeWidgetItem* item = selected.first();
    Class* pClass = nodeClass(item);

    if (nodeStatic(item) && pClass)
    {
        // A "Class::" node -- list that class's static relations directly.
        _ui->listFilter->clear();
        _ui->listRelation->clear();
        Class::FromRelationIterator rel(pClass, &Relation::GetStatic);
        while (++rel)
            addListItem(_ui->listRelation,
                        toQ(rel->GetFromRelation()->GetItemText()),
                        rel.Get());
    }
    else
    {
        fillRelationList(pClass);
    }

    // Compose the variable path: every ancestor node that carries a payload.
    _variable.clear();
    for (QTreeWidgetItem* it = item; it; it = it->parent())
        if (nodeClass(it))
            _variable = it->text(0) + _variable;

    if (_variable.startsWith("this->"))
        _variable = _variable.mid(6);
    if (_variable.endsWith("::"))
        _variable.clear();
}

void IteratorWizardDialog::onRelationChanged()
{
    QList<QListWidgetItem*> selected = _ui->listRelation->selectedItems();
    if (selected.isEmpty())
        return;

    _pRelation = listPtr<Relation>(selected.first());
    fillFilterList();
    _ui->editName->setText("i" + toQ(_pRelation->GetToName()));
}

void IteratorWizardDialog::onFilterChanged()
{
    QList<QListWidgetItem*> selected = _ui->listFilter->selectedItems();
    if (!selected.isEmpty())
        _pFilterMethod = listPtr<Method>(selected.first());
}

// OK -- validate, then build the iterator snippet (the MFC OnOK). The snippet
// uses bare CR (\r) line breaks, the code editor's convention.
void IteratorWizardDialog::accept()
{
    if (!_pRelation || _ui->listRelation->selectedItems().isEmpty())
    {
        QMessageBox::warning(this, "Iterator Wizard",
                             "Must select a relation");
        return;
    }

    const QString name = _ui->editName->text();
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Iterator Wizard",
                             "Must give iterator a name");
        return;
    }
    if (_pMethod->GetDataModelDoc()->HasNonCSymbols(toCb(name)))
    {
        QMessageBox::warning(this, "Iterator Wizard",
                             "Iterator name contains illegal characters");
        return;
    }
    if (!_ui->checkReset->isChecked() && findTreeItem(name))
    {
        QMessageBox::warning(this, "Iterator Wizard",
            "Iterator name already used, give other name or use reset");
        return;
    }

    _insertCode.clear();
    if (_ui->checkReset->isChecked())
    {
        _insertCode += name + ".Reset();\r";
    }
    else
    {
        if (_pRelation->GetFromClass() != _pMethod->GetBaseClass())
            _insertCode += toQ(_pRelation->GetFromClass()->GetName()) + "::";
        _insertCode += toQ(_pRelation->GetToName()) + "Iterator " + name;

        if (_variable.isEmpty() && !_pFilterMethod)
        {
            _insertCode += ";\r";
        }
        else
        {
            _insertCode += "(" + _variable;
            if (_pFilterMethod)
            {
                if (!_variable.isEmpty())
                    _insertCode += ", ";
                _insertCode += "&" +
                    toQ(_pRelation->GetToClass()->GetName()) + "::" +
                    toQ(_pFilterMethod->GetName());
            }
            _insertCode += ");\r";
        }
    }
    _insertCode += _ui->checkBackward->isChecked()
        ? "while (--" + name + ")\r{\r}"
        : "while (++" + name + ")\r{\r}";

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowIteratorWizardDialog(Method* pMethod, const char* code,
                                 CbString& insertCode, void* ownerHwnd)
{
    Qt_EnsureApplication();

    IteratorWizardDialog dlg(pMethod, QString::fromLocal8Bit(code));
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    // The snippet carries bare-CR line breaks for the code editor -- pass it
    // through verbatim (NOT toCb, which would rewrite the line endings).
    insertCode = CbString(dlg.insertCode().toLocal8Bit().constData());
    return true;
}
