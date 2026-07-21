// qt/ConstructorCodeDialog.h -- the Qt constructor-body code editor.
//
// The form lives in ConstructorCodeDialog.ui. Ported from the MFC
// CConstructorCodeDialog -- the modeless code editor for a constructor. A
// plain QWidget (dock content, not a dialog: no result codes, no Esc-reject,
// no default button). Two CodeEditors: a small init-list editor and the
// large body editor, each under a marker strip (signature / {//@CODE_<id>).
// A menu bar drives Save / Regenerate / the Edit commands / the Insert
// wizards. Drives the model directly.
#pragma once

#include <QWidget>

class CbString;
class Constructor;
class Method;
class CodeEditor;
class QKeyEvent;
class QMenu;
class QPoint;

namespace Ui { class ConstructorCodeDialog; }

class ConstructorCodeDialog : public QWidget
{
    Q_OBJECT
public:
    explicit ConstructorCodeDialog(Constructor* pConstructor,
                                   QWidget* parent = nullptr);
    ~ConstructorCodeDialog();

    // The model is destroying this editor's constructor: detach + close.
    void detachForDelete();

    // The model rewrote the stored code/init (argument / member / type
    // rename): apply the same whole-identifier replacement to both editors.
    // Called via Qt_ReplaceInOpenCodeEditor.
    void modelReplacedInCode(const CbString& oldStr, const CbString& newStr);

    // Undo/redo restored the constructor's state: reload each editor that was
    // unedited (its text matches pOldState's code/init); unsaved edits are
    // left alone. Called via Qt_UndoRedoOpenCodeEditor.
    void modelStateRestored(Method* pOldState);

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;  // Esc -> closeEvent
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
    void renameIdentifierAtCursor();
    void goToDefinition();
    void updateHighlightWord(const QString& word);
    void save();
    void regenerateCode();
    void regenerateInit();
    bool changed() const;

    Ui::ConstructorCodeDialog* _ui;
    Constructor* _pConstructor;
    CodeEditor*  _focusEdit = nullptr;   // last code editor that held focus
    bool         _splitterSized = false; // size the split only on the first show

    class ModelCompletionProvider* _completion = nullptr;      // body pane, owned
    class ModelCompletionProvider* _initCompletion = nullptr;  // init pane (init-list mode), owned

    // Menus reused to build the editors' right-click context menu.
    QMenu* _editMenu   = nullptr;
    QMenu* _addMenu    = nullptr;
    QMenu* _insertMenu = nullptr;
    // Anchor for the context menu: the Add / Insert submenus are inserted
    // BEFORE this action, so they land right after the cut/copy/paste/delete
    // block and above Select All (see showEditorContextMenu).
    QAction* _selectAllAction = nullptr;
    QList<QMenu*> _allMenus;       // for the key -> action event filter

    // Enabled only while an identifier is highlight-active (the yellow
    // occurrences ARE what F2 renames); the label names it.
    class QAction* _renameAction = nullptr;
};
