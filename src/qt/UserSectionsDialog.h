// qt/UserSectionsDialog.h -- the Qt "edit user sections" dialog.
//
// The form lives in UserSectionsDialog.ui (Qt Designer). It edits a Class's
// six //@START_USER regions -- three in the .h (HUser1..3) and three in the
// .cpp (CppUser1..3) -- in six monospace text panes.
//
// (Ported from the MFC MfcWizardSupportDialog and renamed: it is really just
// a six-section editor.) The "Visual Studio ClassWizard Support" radios are
// an optional extra: when the class derives from an MFC base they rebuild the
// six sections with the MFC scaffolding (DECLARE_MESSAGE_MAP / AFX_VIRTUAL /
// ...). The radio group is hidden when the class has no qualifying MFC base.
#pragma once

#include <QDialog>

class Class;
class BaseClass;

namespace Ui { class UserSectionsDialog; }

class UserSectionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserSectionsDialog(Class* pClass, QWidget* parent = nullptr);
    ~UserSectionsDialog();

private slots:
    void onNone();       // restore the six sections from the model
    void onGeneral();    // restore + append generic-CWnd MFC scaffolding
    void onDocView();    // restore + append Doc/View MFC scaffolding

private:
    void accept() override;          // apply edits to the model
    void loadFromModel();            // model -> the six text panes

    Ui::UserSectionsDialog* _ui;
    Class*                  _pClass;
    BaseClass*              _pBaseClass;   // non-null => support radios shown
};
