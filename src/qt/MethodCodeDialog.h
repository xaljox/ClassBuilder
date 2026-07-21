// qt/MethodCodeDialog.h -- the Qt method-body code editor.
//
// The form lives in MethodCodeDialog.ui. Ported from the MFC
// CMethodCodeDialog -- the modeless code editor for a method body. A plain
// QWidget (dock content, not a dialog: no result codes, no Esc-reject, no
// default button): a CodeEditor under a marker strip showing the signature
// + {//@CODE_<id>; a menu bar drives Save / Regenerate / the Edit commands /
// the Insert wizards. Drives the model directly.
#pragma once

#include <QWidget>

class CbString;
class Method;
class QKeyEvent;
class QMenu;
class QPoint;

namespace Ui { class MethodCodeDialog; }

class MethodCodeDialog : public QWidget
{
    Q_OBJECT
public:
    explicit MethodCodeDialog(Method* pMethod, QWidget* parent = nullptr);
    ~MethodCodeDialog();

    // The model is destroying this editor's method: detach (forget the method,
    // skip the save prompt) and close. Called via Qt_CloseCodeEditor.
    void detachForDelete();

    // The model rewrote the stored code (argument / member / type rename):
    // apply the same whole-identifier replacement to the editor text, so an
    // unedited editor stays identical to the stored code and unsaved edits
    // get the rename too. Called via Qt_ReplaceInOpenCodeEditor.
    void modelReplacedInCode(const CbString& oldStr, const CbString& newStr);

    // Undo/redo restored the method's state: reload the editor if it was
    // unedited (its text matches pOldState's code); unsaved edits are left
    // alone. Called via Qt_UndoRedoOpenCodeEditor.
    void modelStateRestored(Method* pOldState);

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;  // Esc -> closeEvent
    bool eventFilter(QObject* obj, QEvent* event) override;  // menu keys

private:
    void buildMenu();
    void refreshSignature();       // re-read the marker strip from the model
    void showEditorContextMenu(const QPoint& pos);
    void renameIdentifierAtCursor();
    void goToDefinition();
    void updateHighlightWord(const QString& word);
    void save();
    void regenerateCode();
    bool codeChanged() const;

    Ui::MethodCodeDialog* _ui;
    Method* _pMethod;
    bool    _closing = false;

    class ModelCompletionProvider* _completion = nullptr;  // owned

    // Menus reused to build the editor's right-click context menu.
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
