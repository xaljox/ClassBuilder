// qt/VirtualMethodsDialog.h -- the Qt Virtual Methods dialog.
//
// The form lives in VirtualMethodsDialog.ui (Qt Designer). A multi-select
// checkbox QListWidget. Two modes (see QtVirtualMethodsDialog.h): extern-class
// -- list inherited virtual methods to override; method -- list base classes
// to override one method into. Drives the model directly.
#pragma once

#include <QDialog>
#include <QSet>
#include <QString>

class ExternClass;
class MemberAndMethodGroup;
class Method;
class BaseClass;

namespace Ui { class VirtualMethodsDialog; }

class VirtualMethodsDialog : public QDialog
{
    Q_OBJECT
public:
    // Extern-class mode.
    VirtualMethodsDialog(ExternClass* pExternClass,
                         MemberAndMethodGroup* pGroup,
                         QWidget* parent = nullptr);
    // Method mode.
    explicit VirtualMethodsDialog(Method* pMethod, QWidget* parent = nullptr);
    ~VirtualMethodsDialog();

private:
    void initExternClass();
    void fillExternClass(BaseClass* pBaseClass, QSet<QString>& seen);
    void addExternRow(Method* pMethod);

    void initMethod();
    void fillMethod(BaseClass* pBaseClass);

    void setAllChecked(bool checked);
    void accept() override;

    Ui::VirtualMethodsDialog* _ui;
    ExternClass*              _pExternClass = nullptr;
    MemberAndMethodGroup*     _pGroup       = nullptr;
    Method*                   _pMethod      = nullptr;  // set => method mode
};
