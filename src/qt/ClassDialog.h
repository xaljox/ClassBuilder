// qt/ClassDialog.h -- the Qt Class attributes dialog.
//
// The form lives in ClassDialog.ui (Qt Designer). Ported from the MFC
// CClassDialog: name / source+include files / template / properties
// (replace, dll-export, serialize, struct, relation-macros-last) / member
// prefix / note. Non-live -- read on OK (applyFieldChanges, the MFC
// ::Update). Drives the model directly.
#pragma once

#include <QDialog>

class Class;

namespace Ui { class ClassDialog; }

class ClassDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClassDialog(Class* pClass, QWidget* parent = nullptr);
    ~ClassDialog();

    bool fieldsChanged() const { return _changed; }

private:
    void onChangeName();
    void onChangeCppFile();
    void onSerialize();
    void onTemplateCheck();
    void onChangeTemplateDecl();
    void accept() override;
    bool applyFieldChanges();      // the MFC CClassDialog::Update

    Ui::ClassDialog* _ui;
    Class* _pClass;
    bool   _serializeEnable = false;
    bool   _templateEnable  = true;
    bool   _changed = false;
};
