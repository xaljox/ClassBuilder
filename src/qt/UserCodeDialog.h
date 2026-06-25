// qt/UserCodeDialog.h -- the Qt User Section code editor dialog.
//
// The form lives in UserCodeDialog.ui (Qt Designer). Ported from the MFC
// UserCodeDialog: edits one of a class's six fully-editable user-code
// sections (the //@START_USER blocks) -- header/cpp x section 1..3.
// Embeds the CodeEditor widget; applies on OK. Drives the model directly.
#pragma once

#include <QDialog>

class Class;

namespace Ui { class UserCodeDialog; }

class UserCodeDialog : public QDialog
{
    Q_OBJECT
public:
    UserCodeDialog(Class* pClass, int section, bool header,
                   int start, int end, QWidget* parent = nullptr);
    ~UserCodeDialog();

private:
    void accept() override;

    Ui::UserCodeDialog* _ui;
    Class* _pClass;
    int    _section;
    bool   _header;
};
