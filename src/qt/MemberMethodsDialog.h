// qt/MemberMethodsDialog.h -- the Qt Member Methods dialog.
//
// The form lives in MemberMethodsDialog.ui (Qt Designer). First ported dialog
// built around a list widget: a multi-select QListWidget of the member-type
// class's public methods. Drives the model directly (handed the live
// Member*).
#pragma once

#include <QDialog>

class Member;

namespace Ui { class MemberMethodsDialog; }

class MemberMethodsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MemberMethodsDialog(Member* pMember, QWidget* parent = nullptr);
    ~MemberMethodsDialog();

private:
    void fillList();                 // populate from the member-type class
    void setAllChecked(bool checked);// tick / untick every row
    void accept() override;          // wrap each selected method onto the member

    Ui::MemberMethodsDialog* _ui;
    Member*                  _pMember;
};
