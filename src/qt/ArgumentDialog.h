// qt/ArgumentDialog.h -- the Qt Argument attributes dialog.
//
// The form lives in ArgumentDialog.ui (Qt Designer). Ported from the MFC
// CArgumentDialog: type / name / const / pointer / reference / array / default
// / note / template-reference of a method argument. Like the MFC original it
// is non-live -- the widgets are read on OK (applyFieldChanges, the MFC
// ::Update). Drives the model directly.
#pragma once

#include <QDialog>
#include <QString>

class Argument;

namespace Ui { class ArgumentDialog; }

class ArgumentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ArgumentDialog(Argument* pArgument, QWidget* parent = nullptr);
    ~ArgumentDialog();

    bool fieldsChanged() const { return _changed; }

private:
    void onTypeChanged();
    void onArrayToggled();
    void onPointerToggled();
    void onUpdateName();
    void accept() override;
    bool applyFieldChanges();      // the MFC CArgumentDialog::Update

    Ui::ArgumentDialog* _ui;
    Argument* _pArgument;
    QString   _prevType;           // type as of the last onUpdateName
    bool      _changed = false;
};
