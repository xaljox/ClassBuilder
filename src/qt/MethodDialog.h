// qt/MethodDialog.h -- the Qt Method attributes dialog.
//
// The form lives in MethodDialog.ui (Qt Designer). Ported from the MFC
// CMethodDialog: return type / name / template, the type-property and
// access flags, virtual / pure / inline / const / declare / implement, the
// calling convention and a note. Non-live -- read on OK (applyFieldChanges,
// the MFC ::Update, which also propagates to virtual overrides). Drives the
// model directly.
#pragma once

#include <QDialog>

class Method;

namespace Ui { class MethodDialog; }

class MethodDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MethodDialog(Method* pMethod, QWidget* parent = nullptr);
    ~MethodDialog();

    bool fieldsChanged() const { return _changed; }

private:
    void onArray();
    void onVirtual();
    void onPure();
    void onStatic();
    void onPointer();
    void onSelchangeType();
    void onImplement();
    void onDelete();
    void accept() override;
    bool applyFieldChanges();      // the MFC CMethodDialog::Update

    Ui::MethodDialog* _ui;
    Method* _pMethod;
    bool    _changed = false;
};
