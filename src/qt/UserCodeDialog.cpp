// qt/UserCodeDialog.cpp -- the Qt User Section code editor dialog.
//
// Ported from the MFC UserCodeDialog. Edits one of a class's six user-code
// sections via the CodeEditor widget; applies on OK. Drives the model
// directly.

#include "UserCodeDialog.h"
#include "ui_UserCodeDialog.h"

#include "QtUserCodeDialog.h"        // Qt_ShowUserCodeDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ / toCb
#include "CodeEditor.h"

#include <QDialogButtonBox>
#include <QTextCursor>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// The class's user-code section, as stored in the model (CRLF line endings).
CbString sectionCode(Class* c, int section, bool header)
{
    if (header)
    {
        if (section == 1) return c->GetHUser1();
        if (section == 2) return c->GetHUser2();
        return c->GetHUser3();
    }
    if (section == 1) return c->GetCppUser1();
    if (section == 2) return c->GetCppUser2();
    return c->GetCppUser3();
}

void setSectionCode(Class* c, int section, bool header, const CbString& code)
{
    if (header)
    {
        if (section == 1)      c->SetHUser1(code);
        else if (section == 2) c->SetHUser2(code);
        else                   c->SetHUser3(code);
    }
    else
    {
        if (section == 1)      c->SetCppUser1(code);
        else if (section == 2) c->SetCppUser2(code);
        else                   c->SetCppUser3(code);
    }
}

// The MFC start/end offsets index the CRLF text; the editor holds LF text.
// Each CR dropped shifts the offset down by one.
int crlfToLfOffset(const CbString& crlf, int offset)
{
    QString raw = QString::fromLocal8Bit(static_cast<const char*>(crlf));
    if (offset > raw.length())
        offset = raw.length();
    if (offset < 0)
        offset = 0;
    return offset - int(raw.left(offset).count('\r'));
}

} // namespace

UserCodeDialog::UserCodeDialog(Class* pClass, int section, bool header,
                               int start, int end, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::UserCodeDialog)
    , _pClass(pClass)
    , _section(section)
    , _header(header)
{
    _ui->setupUi(this);

    const CbString code = sectionCode(_pClass, _section, _header);
    _ui->editCode->setIndentSize(_pClass->GetDataModel()->GetIndentSize());
    _ui->editCode->setPlainText(toQ(code));

    // Marker bands inside the editor frame: //@START_USER<section> pinned to
    // the top, //@END_USER<section> pinned to the bottom. The .cpp USER3
    // section runs to end-of-file and has NO //@END_USER marker -- it gets no
    // footer, and that missing band is itself the cue that it is open-ended.
    _ui->editCode->setHeaderText(
        QString("//@START_USER%1").arg(_section));
    const bool openEnded = !_header && _section == 3;
    if (!openEnded)
        _ui->editCode->setFooterText(
            QString("//@END_USER%1").arg(_section));

    const CbString file = _header ? _pClass->GetHFileWithoutPath()
                                   : _pClass->GetCppFileWithoutPath();
    setWindowTitle(QString("User section %1 '%2'")
                       .arg(section).arg(toQ(file)));

    // Optional initial selection -- offsets are translated from the model's
    // CRLF text to the editor's LF text.
    if (start != 0 || end != 0)
    {
        QTextCursor cur = _ui->editCode->textCursor();
        cur.setPosition(crlfToLfOffset(code, start));
        cur.setPosition(crlfToLfOffset(code, end), QTextCursor::KeepAnchor);
        _ui->editCode->setTextCursor(cur);
        _ui->editCode->ensureCursorVisible();
    }
    _ui->editCode->setFocus();

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &UserCodeDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &UserCodeDialog::reject);
}

UserCodeDialog::~UserCodeDialog()
{
    delete _ui;
}

// OK -- apply the edited code to the model if it changed (the MFC OnOK).
void UserCodeDialog::accept()
{
    const CbString code = toCb(_ui->editCode->toPlainText());

    if (code != sectionCode(_pClass, _section, _header))
    {
        _pClass->SaveState(1);
        setSectionCode(_pClass, _section, _header, code);
        _pClass->GetDataModelDoc()->MarkLastUndo();
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowUserCodeDialog(Class* pClass, int section, bool header,
                           void* ownerHwnd, int start, int end)
{
    Qt_EnsureApplication();

    UserCodeDialog dlg(pClass, section, header, start, end);
    Qt_ExecModal(dlg, ownerHwnd);
}
