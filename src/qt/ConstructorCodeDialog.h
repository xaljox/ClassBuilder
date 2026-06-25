// qt/ConstructorCodeDialog.h -- the Qt constructor-body code editor.
//
// The form lives in ConstructorCodeDialog.ui. Ported from the MFC
// CConstructorCodeDialog -- the modeless code editor for a constructor. This
// Qt port is MODAL (the MFC host still owns the message loop; modeless waits
// for the full-Qt stage). Two CodeEditors: a small init-list editor and the
// large body editor, each under a marker strip (signature / {//@CODE_<id>).
// A menu bar drives Save / Regenerate / the Edit commands / the Insert
// wizards. Drives the model directly.
#pragma once

#include <QDialog>

class Constructor;
class CodeEditor;
class QMenu;
class QPoint;

namespace Ui { class ConstructorCodeDialog; }

class ConstructorCodeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConstructorCodeDialog(Constructor* pConstructor,
                                   QWidget* parent = nullptr);
    ~ConstructorCodeDialog();

    // The model is destroying this editor's constructor: detach + close.
    void detachForDelete();

protected:
    void closeEvent(QCloseEvent* event) override;
    void reject() override;        // Esc -> route through closeEvent
    bool eventFilter(QObject* obj, QEvent* event) override;
    void showEvent(QShowEvent* event) override;   // first show -> size the splitter

private:
    void buildMenu();
    // Size the init/code split to the init's content: fit the init lines (+1
    // spare), capped, and give the body the remainder (so slack lands on the
    // code side). Needs the splitter laid out, so driven from showEvent.
    void sizeSplitterToContent();
    void refreshSignature();       // re-read the marker strip from the model
    void showEditorContextMenu(CodeEditor* ed, const QPoint& pos);
    void save();
    void regenerateCode();
    void regenerateInit();
    bool changed() const;

    Ui::ConstructorCodeDialog* _ui;
    Constructor* _pConstructor;
    CodeEditor*  _focusEdit = nullptr;   // last code editor that held focus
    bool         _splitterSized = false; // size the split only on the first show

    // Menus reused to build the editors' right-click context menu.
    QMenu* _editMenu   = nullptr;
    QMenu* _addMenu    = nullptr;
    QMenu* _insertMenu = nullptr;
};
