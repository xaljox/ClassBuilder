// qt/ClassShapeDialog.h -- the Qt Class Shape dialog.
//
// The form lives in ClassShapeDialog.ui (Qt Designer). Two multi-select
// QListWidgets -- the class's members and methods -- picking which ones the
// diagram shape displays. Drives the model directly (handed the live
// ClassShape*).
#pragma once

#include <QDialog>

class ClassShape;
class QListWidget;

namespace Ui { class ClassShapeDialog; }

class ClassShapeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClassShapeDialog(ClassShape* pClassShape, QWidget* parent = nullptr);
    ~ClassShapeDialog();

private:
    void fillLists();                                  // populate both lists
    void setAllChecked(QListWidget* list, bool checked);
    void accept() override;        // rebuild the shape's member/method shapes

    Ui::ClassShapeDialog* _ui;
    ClassShape*           _pClassShape;
};
