// qt/ExternClassDialog.h -- the Qt Extern Class attributes dialog.
//
// The form lives in ExternClassDialog.ui (Qt Designer). Ported from the MFC
// CExternClassDialog: name / struct / member-prefix / template / suppress-
// forward-declaration of an extern class. Non-live -- the widgets are read on
// OK (applyFieldChanges, the MFC ::Update). Drives the model directly.
#pragma once

#include <QDialog>

class ExternClass;

namespace Ui { class ExternClassDialog; }

class ExternClassDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExternClassDialog(ExternClass* pExternClass,
                               QWidget* parent = nullptr);
    ~ExternClassDialog();

    bool fieldsChanged() const { return _changed; }

private:
    void onTemplateToggled();
    void onTemplateDeclChanged();
    void accept() override;
    bool applyFieldChanges();      // the MFC CExternClassDialog::Update

    Ui::ExternClassDialog* _ui;
    ExternClass* _pExternClass;
    bool         _changed = false;
};
