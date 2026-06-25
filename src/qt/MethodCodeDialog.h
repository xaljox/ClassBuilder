// qt/MethodCodeDialog.h -- the Qt method-body code editor.
//
// The form lives in MethodCodeDialog.ui. Ported from the MFC
// CMethodCodeDialog -- the modeless code editor for a method body. This Qt
// port is MODAL (the MFC host still owns the message loop; modeless waits
// for the full-Qt stage). A CodeEditor under a marker strip showing the
// signature + {//@CODE_<id>; a menu bar drives Save / Regenerate / the Edit
// commands / the Insert wizards. Drives the model directly.
#pragma once

#include <QDialog>

class Method;
class QMenu;
class QPoint;

namespace Ui { class MethodCodeDialog; }

class MethodCodeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MethodCodeDialog(Method* pMethod, QWidget* parent = nullptr);
    ~MethodCodeDialog();

    // The model is destroying this editor's method: detach (forget the method,
    // skip the save prompt) and close. Called via Qt_CloseCodeEditor.
    void detachForDelete();

protected:
    void closeEvent(QCloseEvent* event) override;
    void reject() override;        // Esc -> route through closeEvent

private:
    void buildMenu();
    void refreshSignature();       // re-read the marker strip from the model
    void showEditorContextMenu(const QPoint& pos);
    void save();
    void regenerateCode();
    bool codeChanged() const;

    Ui::MethodCodeDialog* _ui;
    Method* _pMethod;
    bool    _closing = false;

    // Menus reused to build the editor's right-click context menu.
    QMenu* _editMenu   = nullptr;
    QMenu* _addMenu    = nullptr;
    QMenu* _insertMenu = nullptr;
};
