// qt/InheritTreeDialog.h -- the Qt inheritance-tree dialog.
//
// The form lives in InheritTreeDialog.ui (Qt Designer). Ported from the MFC
// InheritByTree + InheritFromTree -- two near-mirror tree windows merged into
// one class with a mode. Read-only: a CbTreeWidget showing either a class's
// base-class chain or the classes derived from it. No buttons -- closed via
// the window frame. Drives the model directly.
#pragma once

#include <QDialog>

class BaseClass;
class QTreeWidgetItem;

namespace Ui { class InheritTreeDialog; }

class InheritTreeDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode { InheritsFrom, InheritedBy };

    InheritTreeDialog(BaseClass* pClass, Mode mode, QWidget* parent = nullptr);
    ~InheritTreeDialog();

private:
    void fillInheritedBy(BaseClass* pBaseClass, QTreeWidgetItem* parent);
    void fillInheritsFrom(BaseClass* pClass, QTreeWidgetItem* parent);

    Ui::InheritTreeDialog* _ui;
};
