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
#include "QtIteratorWizardDialog.h"
#include "QtTypeVariableDialog.h"
#include "QtVariableMethodDialog.h"
#include "QtSimilarLinesDialog.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QTextCursor>
#include <QTimer>
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

    // Spread the caret identifier's occurrence highlight to the signature
    // strip too -- it previews what an F2 rename would touch.
    connect(_ui->editCode, &CodeEditor::identifierUnderCursorChanged,
            this, [this](const QString& w) { updateHighlightWord(w); });

    _ui->editCode->setFocus();

    // Register as this method's open editor (modeless: one per method; reopen
    // refocuses; the model closes us via Qt_CloseCodeEditor if the method dies).
    _pMethod->SetOpenDialog(this);
}

MethodCodeDialog::~MethodCodeDialog()
{
    if (_pMethod)
        _pMethod->SetOpenDialog(nullptr);
    delete _ui;
}

void MethodCodeDialog::detachForDelete()
{
    _pMethod = nullptr;     // the method is being freed -- never touch it again
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

    Method::ArgumentIterator iArgument(_pMethod);
    while (++iArgument)
    {
        if (toQ(iArgument->GetName()) == name)
        {
            iArgument->SetName(toCb(newName));
            iArgument->Update();
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
    QAction* aRename = edit->addAction("Re&name identifier...",
        QKeySequence(Qt::Key_F2), this,
        [this] { renameIdentifierAtCursor(); });
    aRename->setEnabled(!fixed);

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

    _ui->mainLayout->setMenuBar(bar);
}

// The code editor's right-click menu -- the Edit / Add / Insert commands,
// reusing the menu-bar actions (a QAction can live in several menus).
void MethodCodeDialog::showEditorContextMenu(const QPoint& pos)
{
    QMenu menu;
    // Compact rows; colours derived from the live theme palette.
    menu.setStyleSheet(Qt_CompactMenuStyleSheet());
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

    // Reload so the editor matches the stored (normalised) text.
    _ui->editCode->setPlainText(toQ(_pMethod->GetCode()));
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

void MethodCodeDialog::closeEvent(QCloseEvent* event)
{
    if (!_pMethod)          // detached (the method was deleted) -- just go
    {
        event->accept();
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
}

// Esc -- route through closeEvent so the save-prompt still runs.
void MethodCodeDialog::reject()
{
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
        existing->showNormal();
        existing->raise();
        existing->activateWindow();
        return;
    }

    auto* dlg = new MethodCodeDialog(pMethod);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    Qt_ShowModeless(*dlg, ownerHwnd);
}
