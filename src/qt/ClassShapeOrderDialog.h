// qt/ClassShapeOrderDialog.h -- the Qt Class Shape Order dialog.
//
// The form lives in ClassShapeOrderDialog.ui (Qt Designer). Two single-select
// QListWidgets -- the member and method shapes the diagram shape displays --
// with Move Up / Move Down buttons (the MFC original used spin controls).
// Drives the model directly (handed the live ClassShape*).
#pragma once

#include <QDialog>

class ClassShape;
class QListWidget;

namespace Ui { class ClassShapeOrderDialog; }

class ClassShapeOrderDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClassShapeOrderDialog(ClassShape* pClassShape,
                                   QWidget* parent = nullptr);
    ~ClassShapeOrderDialog();

private:
    void fillLists();                          // populate both lists
    void moveRow(QListWidget* list, int delta);// shift the selected row up/down
    void accept() override;                    // apply the new order to the model

    Ui::ClassShapeOrderDialog* _ui;
    ClassShape*                _pClassShape;
};
