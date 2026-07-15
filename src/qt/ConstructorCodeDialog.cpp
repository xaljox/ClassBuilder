// qt/ConstructorCodeDialog.cpp -- the Qt constructor-body code editor.
//
// Ported from the MFC CConstructorCodeDialog. A modeless QWidget hosted in a
// shell dock (or standalone as fallback). Two CodeEditors -- a small init-list
// editor and the large body editor -- each under a marker strip, driven by a
// menu bar: Save / Regenerate Code / Regenerate Init, the Edit commands, and
// the Insert wizards. Drives the model directly.

#include "ConstructorCodeDialog.h"
#include "ui_ConstructorCodeDialog.h"

#include "QtConstructorCodeDialog.h"  // Qt_ShowConstructorCodeDialog
#include "QtApp.h"                    // Qt_EnsureApplication / Qt_ExecModal
#include "QtMenuStyle.h"              // Qt_CompactMenuStyleSheet
#include "QtModelText.h"              // toQ / toCb
#include "CodeEditor.h"
#include "ModelCompletionProvider.h"
#include "QtIteratorWizardDialog.h"
#include "QtTypeVariableDialog.h"
#include "QtWhoCallsMe.h"
#include "QtVariableMethodDialog.h"
#include "QtSimilarLinesDialog.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QShowEvent>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QInputDialog>
#include <QTimer>
#include <QToolTip>
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QTextCursor>
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

ConstructorCodeDialog::ConstructorCodeDialog(Constructor* pConstructor,
                                             QWidget* parent)
    : QWidget(parent)
    , _ui(new Ui::ConstructorCodeDialog)
    , _pConstructor(pConstructor)
{
    _ui->setupUi(this);

    setWindowTitle(toQ(CbString("Code of constructor ") +
        _pConstructor->GetBaseClass()->GetName() + "::" +
        _pConstructor->GetName()));

    // Marker bands inside the editor frames: the signature pinned atop the
    // init-list editor; {//@CODE_<id> pinned atop and }//@CODE_<id> below the
    // body editor.
    const int id = _pConstructor->GetId();
    refreshSignature();
    _ui->editCode->setHeaderText(QString("{//@CODE_%1").arg(id));
    _ui->editCode->setFooterText(QString("}//@CODE_%1").arg(id));

    const int indent =
        _pConstructor->GetDataModelDoc()->GetDataModel()->GetIndentSize();
    _ui->editInit->setIndentSize(indent);
    _ui->editCode->setIndentSize(indent);
    _ui->editInit->setPlainText(toQ(_pConstructor->GetInit()));
    _ui->editCode->setPlainText(toQ(_pConstructor->GetCode()));

    // The init/code split is sized to the init content on first show (the
    // splitter must be laid out for setSizes to take, so it's done in showEvent).

    // Track which code editor last held focus, for the Edit / Insert routing.
    _ui->editInit->installEventFilter(this);
    _ui->editCode->installEventFilter(this);
    _focusEdit = _ui->editCode;

    buildMenu();

    // Drop the dialog's content margins -- the menu bar already separates the
    // marker strip; the default top margin left a wide empty band.
    _ui->mainLayout->setContentsMargins(0, 0, 0, 0);

    // Restore each editor's custom right-click menu (the MFC CCodeEdit had
    // one) -- the Edit / Add / Insert commands, not Qt's generic edit menu.
    for (CodeEditor* ed : { _ui->editInit, _ui->editCode })
    {
        ed->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ed, &QWidget::customContextMenuRequested,
                this, [this, ed](const QPoint& p)
                { showEditorContextMenu(ed, p); });

        // Spread the caret identifier's occurrence highlight across BOTH
        // editors and the signature strip -- together the set of places an
        // F2 rename would touch.
        connect(ed, &CodeEditor::identifierUnderCursorChanged,
                this, [this](const QString& w) { updateHighlightWord(w); });

        // Cmd+Click (mac) / Ctrl+Click on an identifier: go to definition.
        connect(ed, &CodeEditor::definitionRequested,
                this, [this] { goToDefinition(); });

        // Cmd+Click (mac) / Ctrl+Click on a marker strip: who calls me.
        connect(ed, &CodeEditor::whoCallsMeRequested,
                this, [this, ed] { Qt_ShowWhoCallsMe(ed, _pConstructor); });
    }

    // Model-aware completion -- one provider serves both editors (the
    // context comes from the text each editor hands in per request).
    _completion = new ModelCompletionProvider(_pConstructor);
    _ui->editInit->setCompletionProvider(_completion);
    _ui->editCode->setCompletionProvider(_completion);

    // Focus the body editor now AND whenever the host dock focuses the
    // dialog (tab activation routes focus through the dialog's focus proxy).
    setFocusProxy(_ui->editCode);
    _ui->editCode->setFocus();

    // Register as this constructor's open editor (modeless: one per ctor; reopen
    // refocuses; the model closes us via Qt_CloseCodeEditor if it's deleted).
    // SetOpenWidget is Method's, inherited by Constructor.
    _pConstructor->SetOpenWidget(this);
}

ConstructorCodeDialog::~ConstructorCodeDialog()
{
    if (_pConstructor)
        _pConstructor->SetOpenWidget(nullptr);
    delete _completion;
    delete _ui;
}

void ConstructorCodeDialog::detachForDelete()
{
    _pConstructor = nullptr;   // the ctor is being freed -- never touch it again
    _ui->editInit->setCompletionProvider(nullptr);  // provider held the ctor
    _ui->editCode->setCompletionProvider(nullptr);
    close();                   // WA_DeleteOnClose deletes us; closeEvent sees null
}

void ConstructorCodeDialog::modelReplacedInCode(const CbString& oldStr,
                                                const CbString& newStr)
{
    replaceInEditor(_ui->editInit, _pConstructor, oldStr, newStr);
    replaceInEditor(_ui->editCode, _pConstructor, oldStr, newStr);

    // The hook fires mid-mutation (Argument::SetName replaces in code BEFORE
    // renaming the argument itself) -- re-read the signature strip once the
    // whole model change has settled.
    QTimer::singleShot(0, this, [this]
        { if (_pConstructor) refreshSignature(); });
}

// The identifier at the caret (in whichever editor is focused) changed:
// highlight its occurrences in the init editor, the body editor AND the
// signature strip -- together the set of places an F2 rename would touch.
// A single hit total is noise, not information -- highlight nothing then.
void ConstructorCodeDialog::updateHighlightWord(const QString& word)
{
    QString w = word;
    if (!w.isEmpty() &&
        CodeEditor::identifierCount(
            _ui->editInit->headerPlainText() + '\n' +
            _ui->editInit->toPlainText() + '\n' +
            _ui->editCode->toPlainText(), w) < 2)
        w.clear();

    _ui->editInit->setHighlightWord(w);
    _ui->editCode->setHighlightWord(w);
    _ui->editInit->setHeaderHighlightWord(w);

    // F2 renames exactly the yellow set -- gate and label the action by it.
    if (_renameAction)
    {
        _renameAction->setEnabled(!w.isEmpty());
        _renameAction->setText(w.isEmpty()
            ? QString("Re&name identifier...")
            : QString("Re&name '%1'...").arg(w));
    }
}

// F12: resolve the method named at the caret in the focused editor and go to
// its definition -- ALWAYS select it in the model tree; a relation MACRO
// method has no body, so the tree IS its definition -- for a real method
// also open its editor (OnOpen routes method vs constructor and refocuses).
void ConstructorCodeDialog::goToDefinition()
{
    if (!_completion)
        return;
    Gti* pTarget = _completion->definitionAtCursor(
        _focusEdit->toPlainText(),
        _focusEdit->textCursor().position());
    if (!pTarget)
    {
        // Silent no-ops read as "F12 is broken" -- say why nothing happened
        // (typically: the caret is not on an identifier the model knows).
        QToolTip::showText(
            _focusEdit->mapToGlobal(_focusEdit->cursorRect().bottomRight()),
            "No definition at the cursor", _focusEdit);
        return;
    }

    Qt_SelectInModelTree(pTarget->GetDataModelDoc(), pTarget);
    if (pTarget->IsMethod())
    {
        Method* pMethod = (Method*)pTarget;
        if (pMethod->IsNonMacroMethod())
            pMethod->OnOpen();
    }
}

// Rename the identifier at the caret everywhere. An argument goes through
// the model (Argument::SetName rewrites the stored code/init and mirrors
// into both editors via Qt_ReplaceInOpenCodeEditor, and the signature
// follows); anything else -- a local variable -- is a whole-identifier
// replace across both editors' text only.
void ConstructorCodeDialog::renameIdentifierAtCursor()
{
    const QString name = _focusEdit->identifierUnderCursor();
    if (name.isEmpty())
        return;

    const QString newName = promptRename(this, name);
    if (newName.isEmpty())
        return;

    Method::ArgumentIterator iCollision(_pConstructor);
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

    // A model-involved rename saves the editors FIRST: with no unsaved
    // edits in play, a later model undo can cleanly restore the editors too
    // (the undo mirror never overwrites unsaved edits, so an unsaved editor
    // would be left desynced on undo -- first undo reverts the rename,
    // second one this save).
    Method::ArgumentIterator iArgument(_pConstructor);
    while (++iArgument)
    {
        if (toQ(iArgument->GetName()) == name)
        {
            save();
            iArgument->SetName(toCb(newName));
            iArgument->Update();
            _pConstructor->GetDataModelDoc()->MarkLastUndo();
            return;
        }
    }

    // A member of the owning class goes through the model too --
    // Member::SetName fans the rename out through ALL methods of the class
    // (and non-private members through derived classes), which mirrors into
    // every open editor via Qt_ReplaceInOpenCodeEditor. SetName takes the
    // UNPREFIXED name; strip the member prefix if the user typed it.
    BaseClass::MemberIterator iMember(_pConstructor->GetBaseClass());
    while (++iMember)
    {
        if (toQ(iMember->GetPrefixedName()) == name)
        {
            save();                    // see the argument path above
            CbString raw = toCb(newName);
            const CbString prefix =
                _pConstructor->GetBaseClass()->GetMemberPrefix();
            if (prefix.GetLength() > 0 && raw.Find(prefix) == 0)
                raw = raw.Mid(prefix.GetLength());
            iMember->SetName(raw);
            iMember->Update();
            _pConstructor->GetDataModelDoc()->MarkLastUndo();
            return;
        }
    }

    _ui->editInit->renameIdentifier(name, newName);
    _ui->editCode->renameIdentifier(name, newName);
}

void ConstructorCodeDialog::modelStateRestored(Method* pOldState)
{
    // The pre-restore state of a Constructor is itself a Constructor.
    Constructor* pOld = (Constructor*)pOldState;
    undoRedoEditor(_ui->editInit, pOld->GetInit(), _pConstructor->GetInit());
    undoRedoEditor(_ui->editCode, pOld->GetCode(), _pConstructor->GetCode());

    // Other objects in the undo transaction (arguments, types) may not be
    // restored yet -- re-read the signature strip after it settles.
    QTimer::singleShot(0, this, [this]
        { if (_pConstructor) refreshSignature(); });
}

bool ConstructorCodeDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::FocusIn)
    {
        if (obj == _ui->editInit || obj == _ui->editCode)
            _focusEdit = static_cast<CodeEditor*>(obj);
    }

    // Menu-key dispatch: a key press in either editor triggers the matching
    // menu action directly (shortcuts are NOT registered with Qt's shortcut
    // map -- see buildMenu).
    // A HIDDEN editor (background tab that still held focus) must not act
    // on keys at all -- swallow them, so nothing invisible ever mutates.
    if (event->type() == QEvent::KeyPress &&
        !static_cast<QWidget*>(obj)->isVisible())
        return true;

    if (event->type() == QEvent::KeyPress &&
        (obj == _ui->editInit || obj == _ui->editCode))
        if (triggerMenuKey(_allMenus, static_cast<QKeyEvent*>(event)))
            return true;

    return QWidget::eventFilter(obj, event);
}

void ConstructorCodeDialog::buildMenu()
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

    // --- File ----------------------------------------------------------
    QMenu* file = bar->addMenu("&File");
    file->addAction("&Save", QKeySequence::Save, this, [this] { save(); });
    file->addAction("&Close", this, [this] { close(); });
    file->addSeparator();
    file->addAction("&Regenerate Code", this, [this] { regenerateCode(); });
    file->addAction("Regenerate &Init", this, [this] { regenerateInit(); });

    // --- Edit (operates on whichever code editor last held focus) ------
    QMenu* edit = bar->addMenu("&Edit");
    edit->addAction("&Undo", QKeySequence::Undo,
                    this, [this] { _focusEdit->undo(); });
    edit->addAction("&Redo", QKeySequence::Redo,
                    this, [this] { _focusEdit->redo(); });
    edit->addSeparator();
    edit->addAction("Cu&t", QKeySequence::Cut,
                    this, [this] { _focusEdit->cut(); });
    edit->addAction("&Copy", QKeySequence::Copy,
                    this, [this] { _focusEdit->copy(); });
    edit->addAction("&Paste", QKeySequence::Paste,
                    this, [this] { _focusEdit->paste(); });
    edit->addAction("&Delete", this, [this]
        { QTextCursor c = _focusEdit->textCursor(); c.deleteChar();
          _focusEdit->setTextCursor(c); });
    edit->addSeparator();
    edit->addAction("Select &All", QKeySequence::SelectAll,
                    this, [this] { _focusEdit->selectAll(); });
    edit->addSeparator();
    edit->addAction("Move lines &up", QKeySequence("Alt+Up"),
                    this, [this] { _focusEdit->moveSelectedLines(true); });
    edit->addAction("Move lines dow&n", QKeySequence("Alt+Down"),
                    this, [this] { _focusEdit->moveSelectedLines(false); });
    edit->addAction("Re&format code", QKeySequence("Ctrl+Shift+R"),
                    this, [this] { _focusEdit->reformatCode(); });
    _renameAction = edit->addAction("Re&name identifier...",
        QKeySequence(Qt::Key_F2),
        this, [this] { renameIdentifierAtCursor(); });
    _renameAction->setEnabled(false);   // enabled with the yellow highlight
    // F12 again: its earlier flakiness was the native-menubar hijack (the
    // system bar's key equivalents intercepted it for a hidden editor); with
    // in-widget menu bars it arrives as a plain key press. Ctrl/Cmd+J stays
    // as a silent alias (set below), Cmd+Click is the mouse path.
    QAction* aGotoDef = edit->addAction("&Go to definition",
        QKeySequence(Qt::Key_F12), this, [this] { goToDefinition(); });
    edit->addAction("Who calls &me", QKeySequence("Shift+F12"),
        this, [this] { Qt_ShowWhoCallsMe(_ui->editInit, _pConstructor); });

    // --- Add -----------------------------------------------------------
    // OnAddArgument / OnEditExceptionSpecification open their own sub-dialogs
    // owned by the MFC main window, which steals the Z-order on close --
    // re-raise this dialog afterwards so it stays in front.
    QMenu* add = bar->addMenu("&Add");
    QAction* aArg = add->addAction("&Argument",
        QKeySequence("Ctrl+Shift+A"), this, [this]
        { _pConstructor->OnAddArgument(false);
          refreshSignature(); raise(); activateWindow(); });
    aArg->setEnabled(_pConstructor->OnAddArgument(true) != 0);
    QAction* aThrow = add->addAction("&Throw list", this, [this]
        { _pConstructor->OnEditExceptionSpecification(false);
          refreshSignature(); raise(); activateWindow(); });
    aThrow->setEnabled(
        _pConstructor->OnEditExceptionSpecification(true) != 0);

    // --- Insert --------------------------------------------------------
    QMenu* ins = bar->addMenu("&Insert");
    CodeEditor* code = _ui->editCode;

    ins->addAction("&Iterator", QKeySequence("Ctrl+Shift+L"), this, [this, code]
    {
        CbString insertCode;
        if (Qt_ShowIteratorWizardDialog(_pConstructor,
                code->strippedCode().toLocal8Bit().constData(),
                insertCode, reinterpret_cast<void*>(winId())))
            code->insertWizardSnippet(
                QString::fromLocal8Bit((const char*)insertCode));
    });
    ins->addAction("&Type variable;", QKeySequence("Ctrl+Shift+T"),
                   this, [this, code]
    {
        std::string insertCode;
        if (Qt_ShowTypeVariableDialog(_pConstructor->GetDataModelDoc(),
                insertCode, reinterpret_cast<void*>(winId())))
            code->insertWizardSnippet(
                QString::fromLocal8Bit(insertCode.c_str()));
    });
    ins->addAction("&Variable->Method()", QKeySequence("Ctrl+Shift+V"),
                   this, [this]
    {
        // Targets whichever editor holds focus (init or body).
        CbString insertCode;
        if (Qt_ShowVariableMethodDialog(_pConstructor,
                _focusEdit->strippedCode().toLocal8Bit().constData(),
                insertCode, reinterpret_cast<void*>(winId())))
        {
            _focusEdit->insertWizardSnippet(
                QString::fromLocal8Bit((const char*)insertCode));
            if (_focusEdit == _ui->editCode)
            {
                QTextCursor c = _focusEdit->textCursor();
                if (c.atBlockEnd())
                    _focusEdit->insertPlainText(";");
            }
        }
    });
    ins->addAction("&Similar lines", QKeySequence("Ctrl+Shift+S"),
                   this, [this, code]
    {
        CbString lines;
        if (Qt_ShowSimilarLinesDialog(_pConstructor->GetBaseClass(), lines,
                reinterpret_cast<void*>(winId())))
            code->insertWizardSnippet(
                QString::fromLocal8Bit((const char*)lines));
    });
    ins->addSeparator();
    ins->addAction("i&f () {}", QKeySequence("Ctrl+Shift+I"), this, [code]
        { insertControl(code, "if ()\n{\n}", 4); });
    ins->addAction("if () {} e&lse {}", QKeySequence("Ctrl+Shift+E"),
                   this, [code]
        { insertControl(code, "if ()\n{\n}\nelse\n{\n}", 4); });
    ins->addAction("s&witch () {}", QKeySequence("Ctrl+Shift+C"), this, [code]
        { insertControl(code, "switch ()\n{\ncase :\nbreak;\n\ncase :\n"
                               "break;\n\ndefault:\nbreak;\n}", 8); });
    ins->addAction("w&hile () {}", QKeySequence("Ctrl+Shift+W"), this, [code]
        { insertControl(code, "while ()\n{\n}", 7); });
    ins->addAction("&do {} while ();", QKeySequence("Ctrl+Shift+D"),
                   this, [code]
        { insertControl(code, "do\n{\n}\nwhile ();", 2, true); });
    ins->addAction("f&or (; ; ) {}", QKeySequence("Ctrl+Shift+F"), this, [code]
        { insertControl(code, "for (; ; )\n{\n}", 5); });

    _editMenu   = edit;
    _addMenu    = add;
    _insertMenu = ins;

    // NO shortcut is registered with Qt's shortcut map (unreliable across
    // docked editors sharing the shell window -- see MethodCodeDialog::
    // buildMenu). The sequences move into a property + the visible "\t"
    // hint; eventFilter() dispatches keys from either editor directly.
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

// A code editor's right-click menu -- the Edit / Add / Insert commands,
// reusing the menu-bar actions (a QAction can live in several menus).
void ConstructorCodeDialog::showEditorContextMenu(CodeEditor* ed,
                                                  const QPoint& pos)
{
    _focusEdit = ed;          // route the Edit/Insert commands to this editor

    // IDE convention: right-click OUTSIDE the selection moves the caret to
    // the click point (inside it, the selection is preserved) -- so Go to
    // definition / Rename from this menu act on what was clicked, not on a
    // stale caret (a fresh editor's caret sits at position 0).
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
    menu.addSeparator();
    menu.addActions(_addMenu->actions());
    menu.addSeparator();
    menu.addActions(_insertMenu->actions());
    menu.exec(ed->mapToGlobal(pos));
}

// Re-read the signature marker from the model -- after Add Argument / Throw
// list the interface changes, so the strip must follow.
void ConstructorCodeDialog::refreshSignature()
{
    _ui->editInit->setHeaderText(toQ(_pConstructor->GetInterfaceCpp()));

    // Re-feed the highlighter's model-known names (types + this constructor's
    // arguments), which change on the same events as the signature.
    QSet<QString> typeNames;
    DataModelDoc::TypeIterator iType(_pConstructor->GetDataModelDoc());
    while (++iType)
    {
        if (iType->GetName() != "")
            typeNames.insert(toQ(iType->GetName()));
    }

    // The relation-generated iterator types (RowIterator etc.) -- nested in
    // the from-class, named <ToName>Iterator (see the Iterator wizard).
    DataModel::ClassIterator iClass(
        _pConstructor->GetDataModelDoc()->GetDataModel());
    while (++iClass)
    {
        Class::FromRelationIterator iRelation(iClass, &Relation::GetMulti);
        while (++iRelation)
            typeNames.insert(toQ(iRelation->GetToName()) + "Iterator");
    }

    QSet<QString> argNames;
    Method::ArgumentIterator iArgument(_pConstructor);
    while (++iArgument)
        argNames.insert(toQ(iArgument->GetName()));

    for (CodeEditor* ed : { _ui->editInit, _ui->editCode })
    {
        ed->setModelTypes(typeNames);
        ed->setArgumentNames(argNames);
    }
}

bool ConstructorCodeDialog::changed() const
{
    return toCb(_ui->editInit->toPlainText()) != _pConstructor->GetInit()
 || toCb(_ui->editCode->toPlainText()) != _pConstructor->GetCode();
}

// Write the edited init list + body back to the model (the MFC OnSave).
void ConstructorCodeDialog::save()
{
    if (!changed())
        return;

    const CbString init = toCb(_ui->editInit->toPlainText());
    const CbString code = toCb(_ui->editCode->toPlainText());

    _pConstructor->SaveState();
    _pConstructor->SetVersion(
        _pConstructor->GetDataModelDoc()->GetVersion() + 1);
    _pConstructor->SetInit(init);
    _pConstructor->SetCode(code);
    _pConstructor->GetDataModelDoc()->MarkLastUndo();

    // Reload so the editors match the stored (normalised) text -- keeping
    // the caret and the editor undo history (F2 saves mid-flow).
    setEditorText(_ui->editInit, toQ(_pConstructor->GetInit()));
    setEditorText(_ui->editCode, toQ(_pConstructor->GetCode()));
}

// Show the freshly regenerated body without committing it (the MFC
// OnRegeneratecode -- the model is restored, only the editor shows the regen).
void ConstructorCodeDialog::regenerateCode()
{
    if (QMessageBox::question(this, "Regenerate Code",
            "Are you sure -- 'Regenerate Code' destroys the current code")
 != QMessageBox::Yes)
        return;

    const CbString oldCode = _pConstructor->GetCode();
    _pConstructor->InitCode();
    _ui->editCode->setPlainText(toQ(_pConstructor->GetCode()));
    _pConstructor->SetCode(oldCode);
}

// Show the freshly regenerated init list without committing it.
void ConstructorCodeDialog::regenerateInit()
{
    if (QMessageBox::question(this, "Regenerate Init",
            "Are you sure -- 'Regenerate Init' destroys the current "
            "initialize code") != QMessageBox::Yes)
        return;

    const CbString oldInit = _pConstructor->GetInit();
    _pConstructor->InitInit();
    _ui->editInit->setPlainText(toQ(_pConstructor->GetInit()));
    _pConstructor->SetInit(oldInit);
    sizeSplitterToContent();   // the init list just changed size -- re-fit
}

void ConstructorCodeDialog::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (_splitterSized)
        return;
    _splitterSized = true;
    // Defer one cycle so the splitter has its laid-out height before we size it.
    QTimer::singleShot(0, this, [this]{ sizeSplitterToContent(); });
}

void ConstructorCodeDialog::sizeSplitterToContent()
{
    const int total = _ui->editSplitter->height();
    if (total <= 0)
        return;   // not laid out yet (showEvent defers until it is)

    const int lineH = qMax(1, QFontMetrics(_ui->editInit->font()).lineSpacing());
    const int initLines = qMax(1, _ui->editInit->document()->blockCount());

    // Fit the init to its content plus one spare line; never below ~3 lines, never
    // above 70% of the height (so the body keeps the bulk for a big init list).
    // The body editor takes the remainder, so slack lands on the code side.
    int initH = (initLines + 1) * lineH + 8;
    initH = qBound(3 * lineH, initH, total * 7 / 10);
    _ui->editSplitter->setSizes({ initH, total - initH });
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

void ConstructorCodeDialog::closeEvent(QCloseEvent* event)
{
    if (!_pConstructor)     // detached (the ctor was deleted) -- just go
    {
        event->accept();
        closeHostDockDeferred(this);
        return;
    }
    if (changed())
    {
        const auto r = QMessageBox::warning(this, "Constructor code",
            toQ(CbString("Save changes to constructor '") +
                _pConstructor->GetBaseClass()->GetName() + "::" +
                _pConstructor->GetName() + "()'"),
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

// Esc (QDialog's Esc-reject is gone -- this is a plain QWidget now). As a
// standalone window: close (through closeEvent, so the save prompt runs).
// DOCKED: do nothing -- a tab must not vanish on a stray Esc; the dock's
// close button / File > Close are the deliberate ways out.
void ConstructorCodeDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        if (!qobject_cast<QDockWidget*>(parentWidget()))
            close();
        return;
    }
    QWidget::keyPressEvent(event);
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
// Modeless: one editor per constructor; reopen refocuses (see the ctor /
// Qt_CloseCodeEditor). Mirrors Qt_ShowMethodCodeDialog.
void Qt_ShowConstructorCodeDialog(Constructor* pConstructor, void* ownerHwnd)
{
    Qt_EnsureApplication();

    if (QWidget* existing = pConstructor->GetOpenWidget())
    {
        if (!Qt_RaiseEditorDock(existing))   // tab-activates a docked editor
        {
            existing->showNormal();
            existing->raise();
            existing->activateWindow();
        }
        return;
    }

    auto* dlg = new ConstructorCodeDialog(pConstructor);

    // Prefer a dockable shell dock: floating at first, tab/dock like a tree;
    // further editors tab onto a docked one. The dock owns the dialog. Fall
    // back to a standalone window only if there's no shell.
    const CbString tab = pConstructor->GetBaseClass()->GetName() + "::" +
                         pConstructor->GetName();
    if (!Qt_HostEditorDock(dlg, tab.c_str()))
    {
        dlg->setAttribute(Qt::WA_DeleteOnClose, true);
        Qt_ShowModeless(*dlg, ownerHwnd);
    }
}
