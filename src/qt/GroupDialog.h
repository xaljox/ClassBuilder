// qt/GroupDialog.h -- the Qt Group properties dialog.
//
// The form lives in GroupDialog.ui (Qt Designer). Drives the model directly:
// handed the live Group*, reads it into the controls, writes back on OK.
#pragma once

#include <QDialog>

class Group;

namespace Ui { class GroupDialog; }

class GroupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GroupDialog(Group* pGroup, QWidget* parent = nullptr);
    ~GroupDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }

private:
    void accept() override;          // validate, then write the model

    Ui::GroupDialog* _ui;
    Group*           _pGroup;
    bool             _modelChanged = false;
};
