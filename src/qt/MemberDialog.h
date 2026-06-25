// qt/MemberDialog.h -- the Qt Member attributes dialog.
//
// The form lives in MemberDialog.ui (Qt Designer). Ported from the MFC
// CMemberDialog: type / name / template, the type-property and access
// flags, get/set-method access, bit-field, initial value and a note. Non-
// live -- read on OK (applyFieldChanges, the MFC ::Update). Drives the model
// directly.
#pragma once

#include <QDialog>

class Member;

namespace Ui { class MemberDialog; }

class MemberDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MemberDialog(Member* pMember, QWidget* parent = nullptr);
    ~MemberDialog();

    bool fieldsChanged() const { return _changed; }

private:
    // The Handle* enable/disable helpers -- the MFC namesakes.
    void handleInitialization();
    void handleBitField();
    void handleBitFieldSize();
    void handleArraySize();
    void handleDelete();
    void handleConstPointer();
    void handlePointerPointer();
    void handleSet();
    void handleGet();
    void handleConst();
    void handleMutable();
    void handleStatic();
    void handleTemplate();

    void onSelchangeType();
    void onUpdateName();
    void onArray();
    void onPointer();
    void onStatic();
    void onBitField();
    void onConst();
    void onMutable();
    void onConstPointer();

    int  getAccess() const;        // get-method access, NONE when off
    int  setAccess() const;        // set-method access, NONE when off
    void accept() override;
    bool applyFieldChanges();      // the MFC CMemberDialog::Update

    Ui::MemberDialog* _ui;
    Member* _pMember;
    bool    _changed = false;
};
