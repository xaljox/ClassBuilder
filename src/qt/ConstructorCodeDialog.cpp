// qt/ConstructorCodeDialog.cpp -- the Qt constructor-body code editor.
//
// Ported from the MFC CConstructorCodeDialog. Modal (the MFC modeless
// behaviour waits for the full-Qt stage). Two CodeEditors -- a small init-list
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
#include "QtIteratorWizardDialog.h"
#include "QtTypeVariableDialog.h"
#include "QtVariableMethodDialog.h"
#include "QtSimilarLinesDialog.h"

#include <QCloseEvent>
#include <QShowEvent>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QInputDialog>
#include <QTimer>
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
    : QDialog(parent)
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
    }

    _ui->editCode->setFocus();

    // Register as this constructor's open editor (modeless: one per ctor; reopen
    // refocuses; the model closes us via Qt_CloseCodeEditor if it's deleted).
    // SetOpenDialog is Method's, inherited by Constructor.
    _pConstructor->SetOpenDialog(this);
}

ConstructorCodeDialog::~ConstructorCodeDialog()
{
    if (_pConstructor)
        _pConstructor->SetOpenDialog(nullptr);
    delete _ui;
}

void ConstructorCodeDialog::detachForDelete()
{
    _pConstructor = nullptr;   // the ctor is being freed -- never touch it again
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

    Method::ArgumentIterator iArgument(_pConstructor);
    while (++iArgument)
    {
        if (toQ(iArgument->GetName()) == name)
        {
            iArgument->SetName(toCb(newName));
            iArgument->Update();
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
    return QDialog::eventFilter(obj, event);
}

void ConstructorCodeDialog::buildMenu()
{
    QMenuBar* bar = new QMenuBar(this);

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
    edit->addAction("Re&name identifier...", QKeySequence(Qt::Key_F2),
                    this, [this] { renameIdentifierAtCursor(); });

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

    _ui->mainLayout->setMenuBar(bar);
}

// A code editor's right-click menu -- the Edit / Add / Insert commands,
// reusing the menu-bar actions (a QAction can live in several menus).
void ConstructorCodeDialog::showEditorContextMenu(CodeEditor* ed,
                                                  const QPoint& pos)
{
    _focusEdit = ed;          // route the Edit/Insert commands to this editor
    QMenu menu;
    // Compact rows; colours derived from the live theme palette.
    menu.setStyleSheet(Qt_CompactMenuStyleSheet());
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

    // Reload so the editors match the stored (normalised) text.
    _ui->editInit->setPlainText(toQ(_pConstructor->GetInit()));
    _ui->editCode->setPlainText(toQ(_pConstructor->GetCode()));
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
    QDialog::showEvent(event);
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

void ConstructorCodeDialog::closeEvent(QCloseEvent* event)
{
    if (!_pConstructor)     // detached (the ctor was deleted) -- just go
    {
        event->accept();
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
}

// Esc -- route through closeEvent so the save-prompt still runs.
void ConstructorCodeDialog::reject()
{
    close();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
// Modeless: one editor per constructor; reopen refocuses (see the ctor /
// Qt_CloseCodeEditor). Mirrors Qt_ShowMethodCodeDialog.
void Qt_ShowConstructorCodeDialog(Constructor* pConstructor, void* ownerHwnd)
{
    Qt_EnsureApplication();

    if (QDialog* existing = pConstructor->GetOpenDialog())
    {
        existing->showNormal();
        existing->raise();
        existing->activateWindow();
        return;
    }

    auto* dlg = new ConstructorCodeDialog(pConstructor);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    Qt_ShowModeless(*dlg, ownerHwnd);
}
