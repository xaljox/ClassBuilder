// qt/InheritDialog.h -- the Qt Inherit attributes dialog.
//
// The form lives in InheritDialog.ui (Qt Designer). Ported from the MFC
// CInheritDialog: pick the base class an extern class inherits from, its
// access (public/protected/private), virtual flag, template reference and
// a note. Non-live -- the widgets are read on OK (applyFieldChanges, the
// MFC ::Update). Drives the model directly.
#pragma once

#include <QDialog>

class Inherit;

namespace Ui { class InheritDialog; }

class InheritDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InheritDialog(Inherit* pInherit, QWidget* parent = nullptr);
    ~InheritDialog();

    bool fieldsChanged() const { return _changed; }

private:
    void onSelchangeInherit();
    void accept() override;
    bool applyFieldChanges();      // the MFC CInheritDialog::Update

    Ui::InheritDialog* _ui;
    Inherit* _pInherit;
    bool     _changed = false;
};
