// qt/IteratorWizardDialog.h -- the Qt Iterator Wizard dialog.
//
// The form lives in IteratorWizardDialog.ui (Qt Designer). Ported from the MFC
// CIteratorWizardDialog: a variable QTreeWidget + a relation list + a filter
// list + name/reset/backward, producing a C++ iterator snippet for the code
// editor. Drives the model directly.
#pragma once

#include <QDialog>
#include <QString>

class Method;
class Relation;
class Class;
class BaseClass;
class QListWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace Ui { class IteratorWizardDialog; }

class IteratorWizardDialog : public QDialog
{
    Q_OBJECT
public:
    IteratorWizardDialog(Method* pMethod, const QString& code,
                         QWidget* parent = nullptr);
    ~IteratorWizardDialog();

    QString insertCode() const { return _insertCode; }

private:
    // Variable tree.
    void fillVariableList();
    void fillVariableTree(Class* pClass, QTreeWidgetItem* parent);
    void findAndAddVariables(Class* pClass, QString code,
                             const QString& typeName, bool iterator = false);
    QTreeWidgetItem* findTreeItem(const QString& text,
                                  QTreeWidgetItem* parent = nullptr);
    // Relation / filter lists.
    void fillRelationList(Class* pClass);
    void fillRelationList(Class* pClass, const QString& prefix);
    void fillFilterList();
    void fillFilterList(BaseClass* pBaseClass);

    void onVariableChanged();
    void onRelationChanged();
    void onFilterChanged();
    void accept() override;

    Ui::IteratorWizardDialog* _ui;
    Method*   _pMethod;
    QString   _code;
    QString   _insertCode;
    Relation* _pRelation     = nullptr;
    Method*   _pFilterMethod = nullptr;
    QString   _variable;
};
