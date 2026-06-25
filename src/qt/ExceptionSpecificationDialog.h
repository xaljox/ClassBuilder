// qt/ExceptionSpecificationDialog.h -- the Qt Exception Specification dialog.
//
// The form lives in ExceptionSpecificationDialog.ui (Qt Designer). A transfer
// pair (thrown types <-> available types) plus a property panel for the
// selected thrown type. Each edit mutates the model immediately; the caller
// wraps the dialog in MarkLastUndo / RollBack so Cancel undoes it all.
#pragma once

#include <QDialog>

class Method;
class ExceptionSpecificationType;

namespace Ui { class ExceptionSpecificationDialog; }

class ExceptionSpecificationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExceptionSpecificationDialog(Method* pMethod,
                                          QWidget* parent = nullptr);
    ~ExceptionSpecificationDialog();

private:
    void onEnableToggled(bool enabled);
    void onAdd();
    void onRemove();
    void onThrowSelChanged();
    void updateGui();                 // load the property panel from _pESType
    void validateArraySize();         // the MFC OnKillfocusArraysize

    Ui::ExceptionSpecificationDialog* _ui;
    Method*                           _pMethod;
    ExceptionSpecificationType*       _pESType = nullptr;   // selected, or null
};
