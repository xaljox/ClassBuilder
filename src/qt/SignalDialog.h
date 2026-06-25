// qt/SignalDialog.h -- the Qt Signal (Message) dialog.
//
// The form lives in SignalDialog.ui (Qt Designer). Ported from the MFC
// SignalDialog: edits a sequence-diagram signal -- the linked method (or a
// free message name), clause, label, return, note, and the various display
// flags. Drives the model directly; applies on OK.
#pragma once

#include <QDialog>

class SignalShape;
class BaseClass;
class Method;
class QComboBox;

namespace Ui { class SignalDialog; }

class SignalDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SignalDialog(SignalShape* pSignalShape, QWidget* parent = nullptr);
    ~SignalDialog();

private:
    void fillMethodList();
    void fillMethodList(BaseClass* pBaseClass, int access);
    void setMethod(Method* pMethod);
    Method* methodForText(const QString& text) const;
    void onSelchangeMethod();
    void onEditchangeMethod();
    void onEnableReturn();
    void onAsync();
    void onNewMethod();
    void accept() override;

    Ui::SignalDialog* _ui;
    SignalShape* _pSignalShape;
    BaseClass*   _pBaseClass;
    BaseClass*   _pSenderBaseClass;
    Method*      _pMethod;

    // Method-list filter flags -- persist across invocations (the MFC
    // statics).
    static bool _classBuilderMethods;
    static bool _relationMethods;
    static bool _getSetMethods;
};
