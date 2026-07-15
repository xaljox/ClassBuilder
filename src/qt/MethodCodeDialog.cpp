// qt/MethodCodeDialog.cpp -- the Qt method-body code editor.
//
// Ported from the MFC CMethodCodeDialog. Modal (the MFC modeless behaviour
// waits for the full-Qt stage). A CodeEditor under a marker strip (signature
// + {//@CODE_<id>), driven by a menu bar: Save / Regenerate, the Edit
// commands, and the Insert wizards. Drives the model directly.

#include "MethodCodeDialog.h"
#include "ui_MethodCodeDialog.h"

#include "QtMethodCodeDialog.h"      // Qt_ShowMethodCodeDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtMenuStyle.h"             // Qt_CompactMenuStyleSheet
#include "QtModelText.h"             // toQ / toCb
#include "CodeEditor.h"
#include "ModelCompletionProvider.h"
#include "QtIteratorWizardDialog.h"
#include "QtTypeVariableDialog.h"
#include "QtVariableMethodDialog.h"
#include "QtSimilarLinesDialog.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QTextCursor>
#include <QTimer>
#include <QToolTip>
#include <string>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {
// Insert a control snippet and place the caret `caretOffset` chars past the
// insertion start (or, when fromEnd, that many back from the end).
void insertControl(CodeEditor* ed, const QString& snippet,
                   int caretOffset, bool fromEnd = false)
{
    QTextCursor cur = ed->textCursor();
    const int start = cur.position();
    ed->insertSnippet(snippet);
    cur = ed->textCursor();
    cur.setPosition(fromEnd ? cur.position() - caretOffset
                            : start + caretOffset);
    ed->setTextCursor(cur);
}

// Replace the whole editor text as one editor-undo step, keeping the caret's
// character offset (setPlainText would wipe the editor's undo history).
void setEditorText(CodeEditor* ed, const QString& text)
{
    QTextCursor cur = ed->textCursor();
    const int pos = cur.position();
    cur.select(QTextCursor::Document);
    cur.insertText(text);
    cur.setPosition(qMin(pos, cur.position()));
    ed->setTextCursor(cur);
}

// Mirror a model-side rename into an editor: run the model's own ReplaceInStr
// (same whole-identifier matching) on the editor text, as one undo step,
// keeping the caret's character offset. saveState=false -- the editor text is
// not model state, the undo entry for the rename is pushed by the model side.
void replaceInEditor(CodeEditor* ed, Method* pMethod,
                     const CbString& oldStr, const CbString& newStr)
{
    CbString text = toCb(ed->toPlainText());
    if (pMethod->ReplaceInStr(text, oldStr, newStr, false))
        setEditorText(ed, toQ(text));
}

// Undo/redo swapped the stored code under the editor. Reload only when the
// editor was unedited (its text is the pre-restore code) -- unsaved edits are
// never overwritten; the close prompt then reports a real difference.
void undoRedoEditor(CodeEditor* ed, const CbString& oldCode,
                    const CbString& newCode)
{
    if (newCode != oldCode && toCb(ed->toPlainText()) == oldCode)
        setEditorText(ed, toQ(newCode));
}

// Trigger the menu action whose stored key sequence ("cbMenuKey" -- shortcuts
// are NOT registered with Qt's shortcut map, see buildMenu) matches the key
// event. Returns true when one fired.
bool triggerMenuKey(const QList<QMenu*>& menus, QKeyEvent* ke)
{
    // Strip KeypadModifier: macOS sets it on ARROW keys, so Alt+Up would
    // arrive as Alt+Keypad+Up and never match the stored "Alt+Up" (Qt's own
    // shortcut map applies the same forgiveness).
    const QKeySequence pressed(QKeyCombination(
        ke->modifiers() & ~Qt::KeypadModifier, Qt::Key(ke->key())));
    if (pressed.isEmpty())
        return false;
    for (QMenu* m : menus)
        for (QAction* a : m->actions())
        {
            for (const char* prop : { "cbMenuKey", "cbMenuKey2" })
            {
                const QVariant v = a->property(prop);
                if (v.isValid() &&
                    v.value<QKeySequence>().matches(pressed) ==
                        QKeySequence::ExactMatch)
                {
                    a->trigger();      // no-ops when the action is disabled
                    return true;
                }
            }
        }
    return false;
}

// Prompt for a new name for `name`; empty result means cancelled or invalid.
QString promptRename(QWidget* parent, const QString& name)
{
    bool ok = false;
    const QString newName = QInputDialog::getText(parent, "Rename",
        QString("Rename '%1' to:").arg(name), QLineEdit::Normal, name, &ok)
        .trimmed();
    if (!ok || newName.isEmpty() || newName == name)
        return QString();

    bool valid = newName[0].isLetter() || newName[0] == '_';
    for (QChar c : newName)
        if (!c.isLetterOrNumber() && c != '_')
            valid = false;
    if (!valid)
    {
        QMessageBox::warning(parent, "Rename",
            QString("'%1' is not a valid identifier").arg(newName));
        return QString();
    }
    return newName;
}
}

MethodCodeDialog::MethodCodeDialog(Method* pMethod, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::MethodCodeDialog)
    , _pMethod(pMethod)
{
    _ui->setupUi(this);

    setWindowTitle(toQ(CbString("Code of method ") +
        _pMethod->GetBaseClass()->GetName() + "::" +
        _pMethod->GetName()));

    // Marker bands inside the editor frame: signature + {//@CODE_<id> pinned
    // to the top, the matching }//@CODE_<id> pinned to the bottom (for a
    // fixed method, plain { } -- no @CODE id).
    const QString id = _pMethod->IsFixed()
        ? QString() : QString("//@CODE_%1").arg(_pMethod->GetId());
    _ui->editCode->setFooterText("}" + id);
    refreshSignature();

    _ui->editCode->setIndentSize(
        _pMethod->GetDataModelDoc()->GetDataModel()->GetIndentSize());
    _ui->editCode->setPlainText(toQ(_pMethod->GetCode()));
    if (_pMethod->IsFixed())
        _ui->editCode->setReadOnly(true);

    buildMenu();

    // Drop the dialog's content margins -- the menu bar already separates the
    // marker strip; the default top margin left a wide empty band.
    _ui->mainLayout->setContentsMargins(0, 0, 0, 0);

    // Restore the code editor's custom right-click menu (the MFC CCodeEdit had
    // one) -- the Edit / Add / Insert commands, not Qt's generic edit menu.
    _ui->editCode->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_ui->editCode, &QWidget::customContextMenuRequested,
            this, [this](const QPoint& p) { showEditorContextMenu(p); });

    // Menu keys are dispatched HERE (event filter), not via Qt's shortcut
    // map -- see buildMenu.
    _ui->editCode->installEventFilter(this);

    // Cmd+Click (mac) / Ctrl+Click on an identifier: go to definition.
    connect(_ui->editCode, &CodeEditor::definitionRequested,
            this, [this] { goToDefinition(); });

    // Spread the caret identifier's occurrence highlight to the signature
    // strip too -- it previews what an F2 rename would touch.
    connect(_ui->editCode, &CodeEditor::identifierUnderCursorChanged,
            this, [this](const QString& w) { updateHighlightWord(w); });

    // Model-aware completion (not for a fixed method -- read-only editor).
    if (!_pMethod->IsFixed())
    {
        _completion = new ModelCompletionProvider(_pMethod);
        _ui->editCode->setCompletionProvider(_completion);
    }

    // Focus the editor now AND whenever the host dock focuses the dialog
    // (tab activation routes focus through the dialog's focus proxy).
    setFocusProxy(_ui->editCode);
    _ui->editCode->setFocus();

    // Register as this method's open editor (modeless: one per method; reopen
    // refocuses; the model closes us via Qt_CloseCodeEditor if the method dies).
    _pMethod->SetOpenDialog(this);
}

MethodCodeDialog::~MethodCodeDialog()
{
    if (_pMethod)
        _pMethod->SetOpenDialog(nullptr);
    delete _completion;
    delete _ui;
}

// Menu-key dispatch: a key press in the editor triggers the matching menu
// action directly (none of the menu keys collide with the completion
// popup's navigation keys, so dispatching before the editor is safe).
bool MethodCodeDialog::eventFilter(QObject* obj, QEvent* event)
{
    // A HIDDEN editor (background tab that still held focus) must not act
    // on keys at all -- swallow them, so nothing invisible ever mutates.
    if (event->type() == QEvent::KeyPress &&
        !static_cast<QWidget*>(obj)->isVisible())
        return true;

    if (obj == _ui->editCode && event->type() == QEvent::KeyPress)
        if (triggerMenuKey(_allMenus, static_cast<QKeyEvent*>(event)))
            return true;
    return QDialog::eventFilter(obj, event);
}

void MethodCodeDialog::detachForDelete()
{
    _pMethod = nullptr;     // the method is being freed -- never touch it again
    _ui->editCode->setCompletionProvider(nullptr);  // provider held the method
    _closing = true;
    close();                // WA_DeleteOnClose deletes us; closeEvent sees null
}

void MethodCodeDialog::modelReplacedInCode(const CbString& oldStr,
                                           const CbString& newStr)
{
    replaceInEditor(_ui->editCode, _pMethod, oldStr, newStr);

    // The hook fires mid-mutation (Argument::SetName replaces in code BEFORE
    // renaming the argument itself) -- re-read the signature strip once the
    // whole model change has settled.
    QTimer::singleShot(0, this, [this] { if (_pMethod) refreshSignature(); });
}

// The identifier at the caret changed: highlight its occurrences in the
// editor AND the signature strip -- together the set of places an F2 rename
// would touch. A single hit total (just the word under the caret itself) is
// noise, not information -- highlight nothing then.
void MethodCodeDialog::updateHighlightWord(const QString& word)
{
    QString w = word;
    if (!w.isEmpty() &&
        CodeEditor::identifierCount(
            _ui->editCode->headerPlainText() + '\n' +
            _ui->editCode->toPlainText(), w) < 2)
        w.clear();

    _ui->editCode->setHighlightWord(w);
    _ui->editCode->setHeaderHighlightWord(w);

    // F2 renames exactly the yellow set -- gate and label the action by it.
    if (_renameAction)
    {
        _renameAction->setEnabled(!w.isEmpty() && !_pMethod->IsFixed());
        _renameAction->setText(w.isEmpty()
            ? QString("Re&name identifier...")
            : QString("Re&name '%1'...").arg(w));
    }
}

// F12: resolve the method named at the caret (receiver-aware -- var->Name,
// chain()->Name, Class::Name, else the own class) and go to its definition:
// ALWAYS select it in the model tree; a relation MACRO method has no body,
// so the tree IS its definition -- for a real method also open its editor
// (OnOpen routes method vs constructor and refocuses an already-open one).
void MethodCodeDialog::goToDefinition()
{
    if (!_completion)
        return;
    Gti* pTarget = _completion->definitionAtCursor(
        _ui->editCode->toPlainText(),
        _ui->editCode->textCursor().position());
    if (!pTarget)
    {
        // Silent no-ops read as "F12 is broken" -- say why nothing happened
        // (typically: the caret is not on an identifier the model knows).
        QToolTip::showText(
            _ui->editCode->mapToGlobal(_ui->editCode->cursorRect().bottomRight()),
            "No definition at the cursor", _ui->editCode);
        return;
    }

    const bool inTree = Qt_SelectInModelTree(pTarget->GetDataModelDoc(),
                                             pTarget);
    if (pTarget->IsMethod())
    {
        Method* pMethod = (Method*)pTarget;
        if (pMethod->IsNonMacroMethod())
            pMethod->OnOpen();
    }
}

// Rename the identifier at the caret everywhere. An argument goes through
// the model (Argument::SetName rewrites the stored code and mirrors into
// this editor via Qt_ReplaceInOpenCodeEditor, and the signature follows);
// anything else -- a local variable -- is a plain whole-identifier replace
// in the editor text only.
void MethodCodeDialog::renameIdentifierAtCursor()
{
    CodeEditor* ed = _ui->editCode;
    const QString name = ed->identifierUnderCursor();
    if (name.isEmpty())
        return;

    const QString newName = promptRename(this, name);
    if (newName.isEmpty())
        return;

    Method::ArgumentIterator iCollision(_pMethod);
    while (++iCollision)
    {
        if (toQ(iCollision->GetName()) == newName)
        {
            QMessageBox::warning(this, "Rename",
                QString("There is already an argument named '%1'")
                    .arg(newName));
            return;
        }
    }

    // A model-involved rename saves the editor FIRST: with no unsaved edits
    // in play, a later model undo can cleanly restore the editor too (the
    // undo mirror never overwrites unsaved edits, so an unsaved editor
    // would be left desynced on undo -- first undo reverts the rename,
    // second one this save).
    Method::ArgumentIterator iArgument(_pMethod);
    while (++iArgument)
    {
        if (toQ(iArgument->GetName()) == name)
        {
            save();
            iArgument->SetName(toCb(newName));
            iArgument->Update();
            _pMethod->GetDataModelDoc()->MarkLastUndo();
            return;
        }
    }

    // A member of the owning class goes through the model too --
    // Member::SetName fans the rename out through ALL methods of the class
    // (and non-private members through derived classes), which mirrors into
    // every open editor via Qt_ReplaceInOpenCodeEditor. SetName takes the
    // UNPREFIXED name; strip the member prefix if the user typed it.
    BaseClass::MemberIterator iMember(_pMethod->GetBaseClass());
    while (++iMember)
    {
        if (toQ(iMember->GetPrefixedName()) == name)
        {
            save();                    // see the argument path above
            CbString raw = toCb(newName);
            const CbString prefix =
                _pMethod->GetBaseClass()->GetMemberPrefix();
            if (prefix.GetLength() > 0 && raw.Find(prefix) == 0)
                raw = raw.Mid(prefix.GetLength());
            iMember->SetName(raw);
            iMember->Update();
            _pMethod->GetDataModelDoc()->MarkLastUndo();
            return;
        }
    }

    ed->renameIdentifier(name, newName);
}

void MethodCodeDialog::modelStateRestored(Method* pOldState)
{
    undoRedoEditor(_ui->editCode, pOldState->GetCode(), _pMethod->GetCode());

    // Other objects in the undo transaction (arguments, types) may not be
    // restored yet -- re-read the signature strip after it settles.
    QTimer::singleShot(0, this, [this] { if (_pMethod) refreshSignature(); });
}

void MethodCodeDialog::buildMenu()
{
    QMenuBar* bar = new QMenuBar(this);
    // NEVER let this menu bar go native on macOS: Qt promotes dialog menu
    // bars to the ONE system menu bar, where every open editor fights over
    // it -- its items (and key equivalents) then belong to whichever
    // (possibly hidden) editor grabbed it last. In-widget, each editor
    // keeps its own visible menu strip, same as on Windows.
    bar->setNativeMenuBar(false);
    // Compact rows: the mac style gives a non-native QMenuBar tall padding
    // (lots of empty space under the text) -- trim it to the text height.
    bar->setStyleSheet(
        "QMenuBar { padding: 0px; margin: 0px; }"
        "QMenuBar::item { padding: 2px 10px; }");
    const bool fixed = _pMethod->IsFixed();
    CodeEditor* ed = _ui->editCode;

    // --- File ----------------------------------------------------------
    QMenu* file = bar->addMenu("&File");
    QAction* aSave = file->addAction("&Save", QKeySequence::Save,
                                     this, [this] { save(); });
    aSave->setEnabled(!fixed);
    file->addAction("&Close", this, [this] { close(); });
    file->addSeparator();
    QAction* aRegen = file->addAction("&Regenerate Code",
                                      this, [this] { regenerateCode(); });
    aRegen->setEnabled(!fixed);

    // --- Edit ----------------------------------------------------------
    QMenu* edit = bar->addMenu("&Edit");
    edit->addAction("&Undo", QKeySequence::Undo, ed, &QPlainTextEdit::undo);
    edit->addAction("&Redo", QKeySequence::Redo, ed, &QPlainTextEdit::redo);
    edit->addSeparator();
    edit->addAction("Cu&t", QKeySequence::Cut, ed, &QPlainTextEdit::cut);
    edit->addAction("&Copy", QKeySequence::Copy, ed, &QPlainTextEdit::copy);
    edit->addAction("&Paste", QKeySequence::Paste, ed, &QPlainTextEdit::paste);
    edit->addAction("&Delete", this, [ed]
        { QTextCursor c = ed->textCursor(); c.deleteChar();
          ed->setTextCursor(c); });
    edit->addSeparator();
    edit->addAction("Select &All", QKeySequence::SelectAll,
                    ed, &QPlainTextEdit::selectAll);
    edit->addSeparator();
    QAction* aMoveUp = edit->addAction("Move lines &up",
        QKeySequence("Alt+Up"), ed, [ed] { ed->moveSelectedLines(true); });
    aMoveUp->setEnabled(!fixed);
    QAction* aMoveDown = edit->addAction("Move lines dow&n",
        QKeySequence("Alt+Down"), ed, [ed] { ed->moveSelectedLines(false); });
    aMoveDown->setEnabled(!fixed);
    QAction* aReformat = edit->addAction("Re&format code",
        QKeySequence("Ctrl+Shift+R"), ed, &CodeEditor::reformatCode);
    aReformat->setEnabled(!fixed);
    _renameAction = edit->addAction("Re&name identifier...",
        QKeySequence(Qt::Key_F2), this,
        [this] { renameIdentifierAtCursor(); });
    _renameAction->setEnabled(false);   // enabled with the yellow highlight
    // F12 again: its earlier flakiness was the native-menubar hijack (the
    // system bar's key equivalents intercepted it for a hidden editor); with
    // in-widget menu bars it arrives as a plain key press. Ctrl/Cmd+J stays
    // as a silent alias (set below), Cmd+Click is the mouse path.
    QAction* aGotoDef = edit->addAction("&Go to definition",
        QKeySequence(Qt::Key_F12), this, [this] { goToDefinition(); });

    // --- Add -----------------------------------------------------------
    // OnAddArgument / OnEditExceptionSpecification open their own sub-dialogs
    // owned by the MFC main window, which steals the Z-order on close --
    // re-raise this dialog afterwards so it stays in front.
    QMenu* add = bar->addMenu("&Add");
    QAction* aArg = add->addAction("&Argument",
        QKeySequence("Ctrl+Shift+A"), this, [this]
        { _pMethod->OnAddArgument(false);
          refreshSignature(); raise(); activateWindow(); });
    aArg->setEnabled(_pMethod->OnAddArgument(true) != 0);
    QAction* aThrow = add->addAction("&Throw list", this, [this]
        { _pMethod->OnEditExceptionSpecification(false);
          refreshSignature(); raise(); activateWindow(); });
    aThrow->setEnabled(_pMethod->OnEditExceptionSpecification(true) != 0);

    // --- Insert (disabled wholesale for a fixed/read-only method) ------
    QMenu* ins = bar->addMenu("&Insert");
    ins->setEnabled(!fixed);

    ins->addAction("&Iterator", QKeySequence("Ctrl+Shift+L"), this, [this, ed]
    {
        CbString insertCode;
        if (Qt_ShowIteratorWizardDialog(_pMethod,
                ed->strippedCode().toLocal8Bit().constData(),
                insertCode, reinterpret_cast<void*>(winId())))
            ed->insertWizardSnippet(
                QString::fromLocal8Bit((const char*)insertCode));
    });
    ins->addAction("&Type variable;", QKeySequence("Ctrl+Shift+T"),
                   this, [this, ed]
    {
        std::string insertCode;
        if (Qt_ShowTypeVariableDialog(_pMethod->GetDataModelDoc(),
                insertCode, reinterpret_cast<void*>(winId())))
            ed->insertWizardSnippet(
                QString::fromLocal8Bit(insertCode.c_str()));
    });
    ins->addAction("&Variable->Method()", QKeySequence("Ctrl+Shift+V"),
                   this, [this, ed]
    {
        CbString insertCode;
        if (Qt_ShowVariableMethodDialog(_pMethod,
                ed->strippedCode().toLocal8Bit().constData(),
                insertCode, reinterpret_cast<void*>(winId())))
        {
            ed->insertWizardSnippet(
                QString::fromLocal8Bit((const char*)insertCode));
            QTextCursor c = ed->textCursor();
            if (c.atBlockEnd())
                ed->insertPlainText(";");
        }
    });
    ins->addAction("&Similar lines", QKeySequence("Ctrl+Shift+S"),
                   this, [this, ed]
    {
        CbString code;
        if (Qt_ShowSimilarLinesDialog(_pMethod->GetBaseClass(), code,
                reinterpret_cast<void*>(winId())))
            ed->insertWizardSnippet(
                QString::fromLocal8Bit((const char*)code));
    });
    ins->addSeparator();
    ins->addAction("i&f () {}", QKeySequence("Ctrl+Shift+I"), this, [ed]
        { insertControl(ed, "if ()\n{\n}", 4); });
    ins->addAction("if () {} e&lse {}", QKeySequence("Ctrl+Shift+E"), this, [ed]
        { insertControl(ed, "if ()\n{\n}\nelse\n{\n}", 4); });
    ins->addAction("s&witch () {}", QKeySequence("Ctrl+Shift+C"), this, [ed]
        { insertControl(ed, "switch ()\n{\ncase :\nbreak;\n\ncase :\n"
                             "break;\n\ndefault:\nbreak;\n}", 8); });
    ins->addAction("w&hile () {}", QKeySequence("Ctrl+Shift+W"), this, [ed]
        { insertControl(ed, "while ()\n{\n}", 7); });
    ins->addAction("&do {} while ();", QKeySequence("Ctrl+Shift+D"), this, [ed]
        { insertControl(ed, "do\n{\n}\nwhile ();", 2, true); });
    ins->addAction("f&or (; ; ) {}", QKeySequence("Ctrl+Shift+F"), this, [ed]
        { insertControl(ed, "for (; ; )\n{\n}", 5); });

    _editMenu   = edit;
    _addMenu    = add;
    _insertMenu = ins;

    // Docked editors share the shell's top-level window, and Qt's shortcut
    // map proved unreliable there (window scope: two editors = ambiguous =
    // fires neither; widget scopes misdelivered to the wrong dialog). So NO
    // shortcut is registered at all: the key sequences move into a property
    // + the visible "\t" hint, and the dialog's event filter on its editor
    // triggers the matching action DIRECTLY -- keys are handled exactly
    // where typing lands, deterministically. See eventFilter().
    _allMenus = { file, edit, add, ins };
    for (QMenu* m : _allMenus)
        for (QAction* a : m->actions())
        {
            const QKeySequence ks = a->shortcut();
            if (ks.isEmpty())
                continue;
            a->setProperty("cbMenuKey", ks);
            a->setShortcut(QKeySequence());
            a->setText(a->text() + "\t" +
                       ks.toString(QKeySequence::NativeText));
        }

    aGotoDef->setProperty("cbMenuKey2", QKeySequence("Ctrl+J"));

    _ui->mainLayout->setMenuBar(bar);
}

// The code editor's right-click menu -- the Edit / Add / Insert commands,
// reusing the menu-bar actions (a QAction can live in several menus).
void MethodCodeDialog::showEditorContextMenu(const QPoint& pos)
{
    // IDE convention: right-click OUTSIDE the selection moves the caret to
    // the click point (inside it, the selection is preserved) -- so Go to
    // definition / Rename from this menu act on what was clicked, not on a
    // stale caret (a fresh editor's caret sits at position 0).
    CodeEditor* ed = _ui->editCode;
    QTextCursor clicked = ed->cursorForPosition(pos);
    QTextCursor cur = ed->textCursor();
    if (!cur.hasSelection() ||
        clicked.position() < cur.selectionStart() ||
        clicked.position() > cur.selectionEnd())
        ed->setTextCursor(clicked);

    QMenu menu;
    // Compact rows; colours derived from the live theme palette.
    Qt_ApplyCompactMenuStyle(&menu);
    menu.addActions(_editMenu->actions());
    if (!_pMethod->IsFixed())
    {
        menu.addSeparator();
        menu.addActions(_addMenu->actions());
        menu.addSeparator();
        menu.addActions(_insertMenu->actions());
    }
    menu.exec(_ui->editCode->mapToGlobal(pos));
}

// Re-read the signature marker from the model -- after Add Argument / Throw
// list the interface changes, so the strip must follow. Also re-feeds the
// highlighter's model-known names (types + this method's arguments), which
// change on the same events.
void MethodCodeDialog::refreshSignature()
{
    const QString id = _pMethod->IsFixed()
        ? QString() : QString("//@CODE_%1").arg(_pMethod->GetId());
    _ui->editCode->setHeaderText(
        toQ(_pMethod->GetInterfaceCpp()) + "\n{" + id);

    QSet<QString> typeNames;
    DataModelDoc::TypeIterator iType(_pMethod->GetDataModelDoc());
    while (++iType)
    {
        if (iType->GetName() != "")
            typeNames.insert(toQ(iType->GetName()));
    }

    // The relation-generated iterator types (RowIterator etc.) -- nested in
    // the from-class, named <ToName>Iterator (see the Iterator wizard).
    DataModel::ClassIterator iClass(
        _pMethod->GetDataModelDoc()->GetDataModel());
    while (++iClass)
    {
        Class::FromRelationIterator iRelation(iClass, &Relation::GetMulti);
        while (++iRelation)
            typeNames.insert(toQ(iRelation->GetToName()) + "Iterator");
    }
    _ui->editCode->setModelTypes(typeNames);

    QSet<QString> argNames;
    Method::ArgumentIterator iArgument(_pMethod);
    while (++iArgument)
        argNames.insert(toQ(iArgument->GetName()));
    _ui->editCode->setArgumentNames(argNames);
}

bool MethodCodeDialog::codeChanged() const
{
    return toCb(_ui->editCode->toPlainText()) != _pMethod->GetCode();
}

// Write the edited body back to the model (the MFC OnSave).
void MethodCodeDialog::save()
{
    const CbString code = toCb(_ui->editCode->toPlainText());
    if (_pMethod->GetCode() == code)
        return;

    _pMethod->SaveState();
    _pMethod->SetVersion(_pMethod->GetDataModelDoc()->GetVersion() + 1);
    _pMethod->InitCode();          // resets the untouched flag
    _pMethod->SetCode(code);
    _pMethod->GetDataModelDoc()->MarkLastUndo();

    // Reload so the editor matches the stored (normalised) text -- keeping
    // the caret and the editor undo history (F2 saves mid-flow).
    setEditorText(_ui->editCode, toQ(_pMethod->GetCode()));
}

// Show the freshly regenerated code without committing it (the MFC
// OnRegeneratecode -- the model is restored, only the editor shows the regen).
void MethodCodeDialog::regenerateCode()
{
    if (QMessageBox::question(this, "Regenerate Code",
            "Are you sure -- 'Regenerate Code' destroys the current code")
 != QMessageBox::Yes)
        return;

    const CbString oldCode = _pMethod->GetCode();
    _pMethod->InitCode();
    _ui->editCode->setPlainText(toQ(_pMethod->GetCode()));
    _pMethod->SetCode(oldCode);
}

namespace {
// Dock-hosted editor closed itself (Esc / File > Close / model delete): take
// the host dock with it -- QUEUED, we are inside the dialog's own closeEvent
// (the dock's re-run of close() finds the dialog hidden and just proceeds;
// dropped automatically if the dock is already being destroyed).
void closeHostDockDeferred(QWidget* dlg)
{
    if (auto* dock = qobject_cast<QDockWidget*>(dlg->parentWidget()))
        QMetaObject::invokeMethod(dock, [dock] { dock->close(); },
                                  Qt::QueuedConnection);
}
}

void MethodCodeDialog::closeEvent(QCloseEvent* event)
{
    if (!_pMethod)          // detached (the method was deleted) -- just go
    {
        event->accept();
        closeHostDockDeferred(this);
        return;
    }
    if (codeChanged())
    {
        const auto r = QMessageBox::warning(this, "Method code",
            toQ(CbString("Save changes to method '") +
                _pMethod->GetBaseClass()->GetName() + "::" +
                _pMethod->GetName() + "()'"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (r == QMessageBox::Cancel)
        {
            event->ignore();
            return;
        }
        if (r == QMessageBox::Yes)
            save();
    }
    event->accept();
    closeHostDockDeferred(this);
}

// Esc. As a standalone window: close (through closeEvent, so the save
// prompt runs). DOCKED: do nothing -- a tab must not vanish on a stray Esc
// (e.g. a habit-press for the completion popup when it isn't showing); the
// dock's close button / File > Close are the deliberate ways out.
void MethodCodeDialog::reject()
{
    if (qobject_cast<QDockWidget*>(parentWidget()))
        return;
    close();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
// Modeless: one editor per method. If one is already open for pMethod, just
// refocus it; otherwise create a non-blocking editor that self-destructs on
// close and registers itself on the method (see the ctor / Qt_CloseCodeEditor).
void Qt_ShowMethodCodeDialog(Method* pMethod, void* ownerHwnd)
{
    Qt_EnsureApplication();

    if (QDialog* existing = pMethod->GetOpenDialog())
    {
        if (!Qt_RaiseEditorDock(existing))   // tab-activates a docked editor
        {
            existing->showNormal();
            existing->raise();
            existing->activateWindow();
        }
        return;
    }

    auto* dlg = new MethodCodeDialog(pMethod);

    // Prefer a dockable shell dock: floating at first, tab/dock like a tree;
    // further editors tab onto a docked one. The dock owns the dialog. Fall
    // back to a standalone window only if there's no shell.
    const CbString tab = pMethod->GetBaseClass()->GetName() + "::" +
                         pMethod->GetName();
    if (!Qt_HostEditorDock(dlg, tab.c_str()))
    {
        dlg->setAttribute(Qt::WA_DeleteOnClose, true);
        Qt_ShowModeless(*dlg, ownerHwnd);
    }
}
