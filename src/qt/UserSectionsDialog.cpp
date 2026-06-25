// qt/UserSectionsDialog.cpp -- the Qt "edit user sections" dialog.
//
// Ported from the MFC MfcWizardSupportDialog and renamed: despite the old
// name it is really a plain editor for a Class's six //@START_USER regions.
// Drives the model directly. The ClassWizard-support radios are an optional
// add-on shown only when the class has a qualifying MFC base class.

#include "UserSectionsDialog.h"
#include "ui_UserSectionsDialog.h"

#include "QtUserSectionsDialog.h"        // Qt_ShowUserSectionsDialog
#include "QtApp.h"                       // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"                 // toQ / toCb

#include <QDialogButtonBox>

#include "CodeEditor.h"


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

UserSectionsDialog::UserSectionsDialog(Class* pClass, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::UserSectionsDialog)
    , _pClass(pClass)
    , _pBaseClass(nullptr)
{
    _ui->setupUi(this);

    setWindowTitle(QString("User sections for class: '%1'")
                       .arg(toQ(_pClass->GetName())));

    // The two group boxes are titled with the file they edit.
    _ui->groupHFile->setTitle(toQ(_pClass->GetHFile()));
    _ui->groupCppFile->setTitle(toQ(_pClass->GetCppFile()));

    // The six section editors are CodeEditor widgets -- they carry the
    // monospace font and auto-indent themselves; just hand them the model's
    // indent step.
    const int indent = _pClass->GetDataModel()->GetIndentSize();
    CodeEditor* edits[] = {
        _ui->plainTextHUser1, _ui->plainTextHUser2, _ui->plainTextHUser3,
        _ui->plainTextCppUser1, _ui->plainTextCppUser2, _ui->plainTextCppUser3 };
    for (CodeEditor* e : edits)
        e->setIndentSize(indent);

    // Each section carries its //@START_USER / //@END_USER markers as fixed
    // bands inside its own frame -- consistent with the single-section
    // UserCodeDialog and the method/constructor code editors. The .cpp USER3
    // region runs to end-of-file and has no //@END_USER3: it gets no footer,
    // and that one missing band marks it as the open-ended section.
    _ui->plainTextHUser1->setHeaderText("//@START_USER1");
    _ui->plainTextHUser1->setFooterText("//@END_USER1");
    _ui->plainTextHUser2->setHeaderText("//@START_USER2");
    _ui->plainTextHUser2->setFooterText("//@END_USER2");
    _ui->plainTextHUser3->setHeaderText("//@START_USER3");
    _ui->plainTextHUser3->setFooterText("//@END_USER3");
    _ui->plainTextCppUser1->setHeaderText("//@START_USER1");
    _ui->plainTextCppUser1->setFooterText("//@END_USER1");
    _ui->plainTextCppUser2->setHeaderText("//@START_USER2");
    _ui->plainTextCppUser2->setFooterText("//@END_USER2");
    _ui->plainTextCppUser3->setHeaderText("//@START_USER3");
    // plainTextCppUser3: no footer -- open-ended (end of file).

    // The ClassWizard-support radios apply only when the class derives from a
    // single MFC base (StdAfx project, exactly one inherit).
    if (_pClass->GetDataModel()->GetStdAfx() &&
        _pClass->GetInheritCount() == 1 && _pClass->GetFirstInherit())
    {
        _pBaseClass = _pClass->GetFirstInherit()->GetBaseClass();
    }
    _ui->groupSupport->setVisible(_pBaseClass != nullptr);

    loadFromModel();
    _ui->radioNone->setChecked(true);

    connect(_ui->radioNone, &QRadioButton::clicked,
            this, &UserSectionsDialog::onNone);
    connect(_ui->radioGeneral, &QRadioButton::clicked,
            this, &UserSectionsDialog::onGeneral);
    connect(_ui->radioDocView, &QRadioButton::clicked,
            this, &UserSectionsDialog::onDocView);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &UserSectionsDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &UserSectionsDialog::reject);
}

UserSectionsDialog::~UserSectionsDialog()
{
    delete _ui;
}

void UserSectionsDialog::loadFromModel()
{
    _ui->plainTextHUser1->setPlainText(toQ(_pClass->GetHUser1()));
    _ui->plainTextHUser2->setPlainText(toQ(_pClass->GetHUser2()));
    _ui->plainTextHUser3->setPlainText(toQ(_pClass->GetHUser3()));
    _ui->plainTextCppUser1->setPlainText(toQ(_pClass->GetCppUser1()));
    _ui->plainTextCppUser2->setPlainText(toQ(_pClass->GetCppUser2()));
    _ui->plainTextCppUser3->setPlainText(toQ(_pClass->GetCppUser3()));
}

void UserSectionsDialog::onNone()
{
    loadFromModel();
}

// Generic CWnd: rebuild the sections from the model, then append the MFC
// message-map / virtual scaffolding.
void UserSectionsDialog::onGeneral()
{
    loadFromModel();

    const QString name = toQ(_pClass->GetName());

    QString hUser2 = _ui->plainTextHUser2->toPlainText();
    QString hUser3 = _ui->plainTextHUser3->toPlainText();
    QString cppUser2 = _ui->plainTextCppUser2->toPlainText();

    if (!hUser2.isEmpty())   hUser2 += '\n';
    if (!hUser3.isEmpty())   hUser2 += '\n';
    if (!cppUser2.isEmpty()) cppUser2 += '\n';

    hUser2 += "    //{{AFX_VIRTUAL(" + name + ")\n";
    hUser2 += "    //}}AFX_VIRTUAL\n";
    hUser2 += "\n";
    hUser2 += "protected:\n";
    hUser2 += "    //{{AFX_MSG(" + name + ")\n";
    hUser2 += "    //}}AFX_MSG\n";
    hUser2 += "    DECLARE_MESSAGE_MAP()\n";

    hUser3 += "//{{AFX_INSERT_LOCATION}}\n";

    cppUser2 += "BEGIN_MESSAGE_MAP(" + name + ", "
                + toQ(_pBaseClass->GetName()) + ")\n";
    cppUser2 += "    //{{AFX_MSG_MAP(" + name + ")\n";
    cppUser2 += "    //}}AFX_MSG_MAP\n";
    cppUser2 += "END_MESSAGE_MAP()\n";

    _ui->plainTextHUser2->setPlainText(hUser2);
    _ui->plainTextHUser3->setPlainText(hUser3);
    _ui->plainTextCppUser2->setPlainText(cppUser2);
}

// Doc/View: as Generic CWnd, plus DECLARE_DYNCREATE / IMPLEMENT_DYNCREATE for
// classes that are not serialized.
void UserSectionsDialog::onDocView()
{
    loadFromModel();

    const QString name = toQ(_pClass->GetName());

    QString hUser2 = _ui->plainTextHUser2->toPlainText();
    QString hUser3 = _ui->plainTextHUser3->toPlainText();
    QString cppUser2 = _ui->plainTextCppUser2->toPlainText();

    if (!hUser2.isEmpty())   hUser2 += '\n';
    if (!hUser3.isEmpty())   hUser2 += '\n';
    if (!cppUser2.isEmpty()) cppUser2 += '\n';

    if (!_pClass->GetSerialize())
    {
        hUser2 += "    DECLARE_DYNCREATE(" + name + ")\n";
        hUser2 += "\n";
    }
    hUser2 += "    //{{AFX_VIRTUAL(" + name + ")\n";
    hUser2 += "    //}}AFX_VIRTUAL\n";
    hUser2 += "\n";
    hUser2 += "protected:\n";
    hUser2 += "    //{{AFX_MSG(" + name + ")\n";
    hUser2 += "    //}}AFX_MSG\n";
    hUser2 += "    DECLARE_MESSAGE_MAP()\n";

    hUser3 += "//{{AFX_INSERT_LOCATION}}\n";

    if (!_pClass->GetSerialize())
    {
        cppUser2 += "IMPLEMENT_DYNCREATE(" + name + ", "
                    + toQ(_pBaseClass->GetName()) + ")\n";
        cppUser2 += "\n";
    }
    cppUser2 += "BEGIN_MESSAGE_MAP(" + name + ", "
                + toQ(_pBaseClass->GetName()) + ")\n";
    cppUser2 += "    //{{AFX_MSG_MAP(" + name + ")\n";
    cppUser2 += "    //}}AFX_MSG_MAP\n";
    cppUser2 += "END_MESSAGE_MAP()\n";

    _ui->plainTextHUser2->setPlainText(hUser2);
    _ui->plainTextHUser3->setPlainText(hUser3);
    _ui->plainTextCppUser2->setPlainText(cppUser2);
}

// OK -- apply every changed section to the model (the MFC OnOK()).
void UserSectionsDialog::accept()
{
    const QString hUser1   = _ui->plainTextHUser1->toPlainText();
    const QString hUser2   = _ui->plainTextHUser2->toPlainText();
    const QString hUser3   = _ui->plainTextHUser3->toPlainText();
    const QString cppUser1 = _ui->plainTextCppUser1->toPlainText();
    const QString cppUser2 = _ui->plainTextCppUser2->toPlainText();
    const QString cppUser3 = _ui->plainTextCppUser3->toPlainText();

    if (hUser1 != toQ(_pClass->GetHUser1()) ||
        hUser2 != toQ(_pClass->GetHUser2()) ||
        hUser3 != toQ(_pClass->GetHUser3()) ||
        cppUser1 != toQ(_pClass->GetCppUser1()) ||
        cppUser2 != toQ(_pClass->GetCppUser2()) ||
        cppUser3 != toQ(_pClass->GetCppUser3()))
    {
        _pClass->SaveState();
        _pClass->SetHUser1(toCb(hUser1));
        _pClass->SetHUser2(toCb(hUser2));
        _pClass->SetHUser3(toCb(hUser3));
        _pClass->SetCppUser1(toCb(cppUser1));
        _pClass->SetCppUser2(toCb(cppUser2));
        _pClass->SetCppUser3(toCb(cppUser3));
        _pClass->GetDataModelDoc()->MarkLastUndo();
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowUserSectionsDialog(Class* pClass, void* ownerHwnd)
{
    Qt_EnsureApplication();

    UserSectionsDialog dlg(pClass);
    Qt_ExecModal(dlg, ownerHwnd);
}
