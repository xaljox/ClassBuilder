// qt/SelectClassesDialog.h -- the Qt Select Classes dialog.
//
// The form lives in SelectClassesDialog.ui (Qt Designer). Ported from the MFC
// SelectClassesDialog: a checkbox tree of the model's classes (grouped by
// data model / meta groups / extern classes). On OK every class shape on the
// diagram is added or removed to match the ticked set. Drives the model
// directly.
#pragma once

#include <QDialog>

class ClassDiagram;
class Gti;
class QTreeWidgetItem;

namespace Ui { class SelectClassesDialog; }

class SelectClassesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectClassesDialog(ClassDiagram* pClassDiagram,
                                 QWidget* parent = nullptr);
    ~SelectClassesDialog();

private:
    QTreeWidgetItem* fillTreeItems(Gti* pGti, QTreeWidgetItem* parent);
    void checkItem(QTreeWidgetItem* item);
    void accept() override;

    Ui::SelectClassesDialog* _ui;
    ClassDiagram*            _pClassDiagram;
};
