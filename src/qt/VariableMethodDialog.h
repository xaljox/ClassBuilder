// qt/VariableMethodDialog.h -- the Qt Variable->Method() Wizard dialog.
//
// The form lives in VariableMethodDialog.ui (Qt Designer). Ported from the MFC
// VariableMethodDialog: a single CbTreeWidget of variables -> their reachable
// relations / methods / members, producing an access-path string for the
// code editor. Drives the model directly.
#pragma once

#include <QDialog>
#include <QString>

class Method;
class Class;
class BaseClass;
class QTreeWidget;
class QTreeWidgetItem;

namespace Ui { class VariableMethodDialog; }

class VariableMethodDialog : public QDialog
{
    Q_OBJECT
public:
    VariableMethodDialog(Method* pMethod, const QString& code,
                         QWidget* parent = nullptr);
    ~VariableMethodDialog();

    QString insertCode() const { return _insertCode; }

private:
    void fillVariableList();
    void fillVariableTree(BaseClass* pBaseClass, QTreeWidgetItem* parent);
    void findAndAddVariables(Class* pClass, QString code,
                             const QString& typeName, bool iterator = false);
    QTreeWidgetItem* findTreeItem(const QString& text);

    void onSelectionChanged();
    void accept() override;

    Ui::VariableMethodDialog* _ui;
    Method* _pMethod;
    QString _code;
    QString _insertCode;
};
