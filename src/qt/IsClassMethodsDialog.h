// qt/IsClassMethodsDialog.h -- the Qt IsClass Methods dialog.
//
// The form lives in IsClassMethodsDialog.ui (Qt Designer). A multi-select
// QListWidget of the candidate `Is<Base>()` methods, walked from the class's
// inherited base classes. Drives the model directly (handed the live Class*).
#pragma once

#include <QDialog>

class Class;
class MemberAndMethodGroup;

namespace Ui { class IsClassMethodsDialog; }

class IsClassMethodsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IsClassMethodsDialog(Class* pClass, MemberAndMethodGroup* pGroup,
                                  QWidget* parent = nullptr);
    ~IsClassMethodsDialog();

private:
    void fillList(Class* pRefClass);   // recurse a base class, add candidates
    void setAllChecked(bool checked);  // tick / untick every row
    void accept() override;            // add an IsClassMethod per ticked row

    Ui::IsClassMethodsDialog* _ui;
    Class*                    _pClass;
    MemberAndMethodGroup*     _pGroup;
};
