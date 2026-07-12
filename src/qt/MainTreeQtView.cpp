// qt/MainTreeQtView.cpp -- the Qt-side main class tree (read-only mirror). See
// MainTreeQtView.h for the M0 scope notes.

#include "MainTreeQtView.h"

#include "QtMainTreeView.h"   // Qt_ShowMainTreeView / Qt_MainTreeNavigate / ...
#include "QtApp.h"            // Qt_EnsureApplication / Qt_ShowModeless
#include "QtModelText.h"      // toQ
#include "QtModelIcons.h"     // Qt_ModelIcon (Actor has no toolbar-strip glyph)
#include "QtToolBarIcons.h"   // Qt_ToolBarIcon (the real MFC toolbar glyphs)
#include "CbTreeWidget.h"
#include "QtClassDiagramView.h"      // Qt_ShowClassDiagramView
#include "QtSequenceDiagramView.h"   // Qt_ShowSequenceDiagramView
#include "QtDiagramDropTarget.h"     // DiagramDropTarget (cross-view drop/ghost dispatch)
#include "QtInheritTreeDialog.h"     // Qt_ShowInheritsFromDialog / ...InheritedBy
#include "QtUserSectionsDialog.h"    // Qt_ShowUserSectionsDialog (all 6 regions)
#include "QtUserCodeDialog.h"        // Qt_ShowUserCodeDialog (one region)
#include "ClassBuilderDoc.h"         // CClassBuilderDoc Undo/Redo/CanUndo (floated-tree toolbar)
// TreeViewModel comes in via ClassBuilderInclude.h below (per-view filter state)

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidgetItem>
#include <QList>
#include <QVariant>
#include <QApplication>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QPalette>
#include <QColor>
#include <QCheckBox>
#include <QGroupBox>
#include <QDialog>
#include <QStyledItemDelegate>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QFontMetrics>

#include <vector>
#include <algorithm>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"
#include "ExternClasses.h"            // AddExternClass -> ExternClasses::OnAddExternClass

#include "CbMultiDeleteDialog.h"

namespace {

// Store/read the model Gti* on a tree row (for state preservation across a
// rebuild; M1 selection will reuse it).
constexpr int kGtiRole = Qt::UserRole;

void setItemGti(QTreeWidgetItem* item, Gti* pGti)
{
    item->setData(0, kGtiRole,
                  QVariant::fromValue<qulonglong>(reinterpret_cast<qulonglong>(pGti)));
}

quintptr itemGti(const QTreeWidgetItem* item)
{
    return static_cast<quintptr>(item->data(0, kGtiRole).toULongLong());
}


// M2 context-menu actions. Each maps 1:1 to a Gti OnXxx model method (the same
// the MFC tree's command handlers call). checkOnly==true returns the enable
// state (mirrors the OnUpdate* handlers); false performs the action (shows the
// ported Qt dialog, rolls back on cancel) -- the model's internal UpdateAllViews
// then refreshes both trees via CClassBuilderView::OnUpdate.
enum class TreeAction
{
    Open, EditAttributes, EditExceptionSpecification, EditContext, UserSections,
    UserSection1H, UserSection2H, UserSection3H,
    UserSection1Cpp, UserSection2Cpp, UserSection3Cpp,
    AddClassDiagram, AddSequenceDiagram,
    AddGroup, AddClass, AddExternClass, AddInherit, AddRelation, AddMember, AddMethod,
    AddConstructor, AddArgument, AddMetaGroup, AddActor,
    AddVirtuals, AddIsClass, AddType,
    Delete, Copy, Paste, SortOnName, SortOnPhase, BaseClasses, DerivedClasses,
    NewSubWindow, DeleteMultiple,
    PhaseAnalysis, PhaseDesign, PhaseImplementation, PhaseTest, PhaseComplete
};

// The OnXxx model-method actions (perform on false, enable on true). Special
// actions (dialogs / phase) return 0 here and are handled in treeActionEnabled
// + the onContextMenu dispatch.
int treeActionInvoke(Gti* pGti, TreeAction a, bool checkOnly)
{
    switch (a)
    {
    case TreeAction::Open:               return pGti->OnOpen(checkOnly);
    case TreeAction::EditAttributes:     return pGti->OnEditAttributes(checkOnly);
    case TreeAction::EditExceptionSpecification:
                                         return pGti->OnEditExceptionSpecification(checkOnly);
    case TreeAction::EditContext:        return pGti->OnEditContext(checkOnly);
    case TreeAction::Delete:             return pGti->OnDelete(checkOnly);
    case TreeAction::Copy:               return pGti->OnCopy(checkOnly);
    case TreeAction::Paste:              return Gti::GetGtiCopy()
                                             ? pGti->OnPaste(Gti::GetGtiCopy(), checkOnly) : 0;
    case TreeAction::SortOnName:         return pGti->SortOnName(checkOnly);
    case TreeAction::SortOnPhase:        return pGti->SortOnPhase(checkOnly);
    case TreeAction::AddClassDiagram:    return pGti->OnAddClassDiagram(checkOnly);
    case TreeAction::AddSequenceDiagram: return pGti->OnAddSequenceDiagram(checkOnly);
    case TreeAction::AddGroup:           return pGti->OnAddGroup(checkOnly);
    case TreeAction::AddClass:           return pGti->OnAddClass(checkOnly);
    case TreeAction::AddExternClass:
    {
        // Explicit "Add External Class" -> the model's OnAddExternClass on the
        // ExternClasses group, reachable from ANY selection. (OnAddClass is now
        // regular everywhere -- the extern branch was de-branched in the model.)
        ExternClasses* pEC = pGti ? pGti->GetDataModelDoc()->GetExternClasses() : nullptr;
        return pEC ? pEC->OnAddExternClass(checkOnly) : 0;
    }
    case TreeAction::AddInherit:         return pGti->OnAddInherit(checkOnly);
    case TreeAction::AddRelation:        return pGti->OnAddRelation(checkOnly);
    case TreeAction::AddMember:          return pGti->OnAddMember(checkOnly);
    case TreeAction::AddMethod:        return pGti->OnAddMethod(checkOnly);
    case TreeAction::AddConstructor:     return pGti->OnAddConstructor(checkOnly);
    case TreeAction::AddArgument:        return pGti->OnAddArgument(checkOnly);
    case TreeAction::AddMetaGroup:       return pGti->OnAddMetaGroup(checkOnly);
    case TreeAction::AddActor:           return pGti->OnAddActor(checkOnly);
    case TreeAction::AddVirtuals:        return pGti->OnAddVirtuals(checkOnly);
    case TreeAction::AddIsClass:         return pGti->OnAddIsClassMethods(checkOnly);
    case TreeAction::AddType:            return pGti->OnAddType(checkOnly);
    default:                             return 0;   // special (dialog/phase) actions
    }
}

// Accelerator for the Add* commands -- the agreed Ctrl+Shift+<letter> scheme
// (docs/Accelerator_Audit.md). Same binding wherever the command appears (here on
// the tree toolbar action that fires it, and shown on the tree context-menu item).
// Empty = no accelerator (menu/mnemonic only). Plain letters can't be used (the
// tree is a type-ahead widget); no Ctrl+Alt (AltGr). Methods are added more often
// than Members, so Method gets M and Member takes B ("memBer"); R/I match the CD
// drag-gestures.
QKeySequence treeAddShortcut(TreeAction a)
{
    switch (a)
    {
    case TreeAction::AddClass:       return QKeySequence(QStringLiteral("Ctrl+Shift+C"));
    case TreeAction::AddExternClass: return QKeySequence(QStringLiteral("Ctrl+Shift+E"));  // Extern (X is grabbed by a global OS hotkey -- unusable)
    case TreeAction::AddMethod:      return QKeySequence(QStringLiteral("Ctrl+Shift+M"));
    case TreeAction::AddMember:      return QKeySequence(QStringLiteral("Ctrl+Shift+B"));
    case TreeAction::AddRelation:    return QKeySequence(QStringLiteral("Ctrl+Shift+R"));
    case TreeAction::AddInherit:     return QKeySequence(QStringLiteral("Ctrl+Shift+I"));
    case TreeAction::AddArgument:    return QKeySequence(QStringLiteral("Ctrl+Shift+A"));  // Argument >> Actor in use
    case TreeAction::AddConstructor: return QKeySequence(QStringLiteral("Ctrl+Shift+U"));  // constrUctor -- S kept free for IsClass (better mnemonic there)
    case TreeAction::AddVirtuals:    return QKeySequence(QStringLiteral("Ctrl+Shift+V"));  // Virtual methods
    case TreeAction::AddIsClass:     return QKeySequence(QStringLiteral("Ctrl+Shift+S"));  // IsClaSs methods
    case TreeAction::AddActor:       return QKeySequence(QStringLiteral("Ctrl+Shift+T"));  // acTor (A went to Argument)
    case TreeAction::AddGroup:       return QKeySequence(QStringLiteral("Ctrl+Shift+G"));
    case TreeAction::AddMetaGroup:   return QKeySequence(QStringLiteral("Ctrl+Shift+P"));  // grouP (E reassigned to Extern)
    case TreeAction::AddType:        return QKeySequence(QStringLiteral("Ctrl+Shift+Y"));  // tYpe (P kept free -- more words want P)
    default:                         return QKeySequence();
    }
}

// section (1-3) + header(.h)/cpp for the per-region user-code editor actions.
bool userCodeArgs(TreeAction a, int& section, bool& header)
{
    switch (a)
    {
    case TreeAction::UserSection1H:   section = 1; header = true;  return true;
    case TreeAction::UserSection2H:   section = 2; header = true;  return true;
    case TreeAction::UserSection3H:   section = 3; header = true;  return true;
    case TreeAction::UserSection1Cpp: section = 1; header = false; return true;
    case TreeAction::UserSection2Cpp: section = 2; header = false; return true;
    case TreeAction::UserSection3Cpp: section = 3; header = false; return true;
    default:                          return false;
    }
}

Phase treeActionPhase(TreeAction a)
{
    switch (a)
    {
    case TreeAction::PhaseAnalysis:       return Analysis_Phase;
    case TreeAction::PhaseDesign:         return Design_Phase;
    case TreeAction::PhaseImplementation: return Implementation_Phase;
    case TreeAction::PhaseTest:           return Test_Phase;
    case TreeAction::PhaseComplete:       return Complete_Phase;
    default:                              return None_Phase;
    }
}

// Enable state, mirroring the matching MFC OnUpdate* handlers.
bool treeActionEnabled(Gti* pGti, TreeAction a)
{
    switch (a)
    {
    case TreeAction::UserSections:
    case TreeAction::UserSection1H:
    case TreeAction::UserSection2H:
    case TreeAction::UserSection3H:
    case TreeAction::UserSection1Cpp:
    case TreeAction::UserSection2Cpp:
    case TreeAction::UserSection3Cpp:
        return dynamic_cast<Class*>(pGti) != nullptr;
    case TreeAction::BaseClasses:
    {
        ExternClass* pExternClass = dynamic_cast<ExternClass*>(pGti);
        return pExternClass && pExternClass->GetInheritCount();
    }
    case TreeAction::DerivedClasses:
    {
        BaseClass* pBaseClass = dynamic_cast<BaseClass*>(pGti);
        return pBaseClass && pBaseClass->GetInheritCount();
    }
    case TreeAction::NewSubWindow:
        return true;   // any node can root a scoped sub-window
    case TreeAction::DeleteMultiple:
    {
        Gti::ChildIterator iChild(pGti);
        return ++iChild ? true : false;   // a subtree to bulk-delete from
    }
    case TreeAction::PhaseAnalysis:
    case TreeAction::PhaseDesign:
    case TreeAction::PhaseImplementation:
    case TreeAction::PhaseTest:
    case TreeAction::PhaseComplete:
        return pGti->GetDataModelDoc()->GetDataModel()->GetPhaseSupport()
 && pGti->GetPhase() != None_Phase
 && pGti->IsAllowedToEditPhase(treeActionPhase(a));
    default:
        return treeActionInvoke(pGti, a, true) != 0;
    }
}

// Children of pGti in CClassBuilderView display order, via the SAME comparator
// the MFC tree uses -- the model method Gti::CompareTreeOrder. stable_sort keeps
// insertion order for equal-rank ties, matching the MFC tree.
std::vector<Gti*> sortedChildren(Gti* pGti)
{
    std::vector<Gti*> children;
    Gti::ChildIterator iChild(pGti);
    while (++iChild)
        children.push_back(iChild.Get());

    std::stable_sort(children.begin(), children.end(),
                     [](Gti* a, Gti* b) { return Gti::CompareTreeOrder(a, b) < 0; });
    return children;
}

} // namespace

// Recolours two signature fragments WITHOUT tinting the rest of the row:
// a trailing " = delete" (deleted methods/constructors) and a leading
// "virtual" keyword -- both in the SAME magenta, so standout keywords on a
// tree line always carry one colour (JV 2026-07-12: the magenta reads
// better than the old dark-red delete suffix; virtual moved off the icon
// into the keyword). Anything without those fragments paints exactly as
// the default delegate.
class SignatureKeywordDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        static const QString kDelete  = QStringLiteral(" = delete");
        static const QString kVirtual = QStringLiteral("virtual ");
        static const QColor  kKeywordColor(196, 32, 186);
        const QString text = index.data(Qt::DisplayRole).toString();
        const bool hasDelete  = text.endsWith(kDelete);
        const bool hasVirtual = text.startsWith(kVirtual);

        // Let the default delegate paint the whole row first -- the correct
        // themed base-text colour, selection background and icon in EVERY state
        // (selected / inactive / hover). The tree's stylesheet makes those
        // colours unreliable to reproduce by hand (opt.palette and even
        // qApp's palette disagree with what the stylesheet actually draws --
        // white text on a light selection), so don't try. The fragments are
        // drawn in the normal colour too; we recolour just those next.
        QStyledItemDelegate::paint(painter, option, index);
        if (!hasDelete && !hasVirtual)
            return;

        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);
        QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();

        const QRect textRect =
            style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
        // Qt's item delegates inset the text by exactly this margin; match it so
        // the recoloured fragment lands precisely over the already-drawn one.
        const int textMargin =
            style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, opt.widget) + 1;
        const QFontMetrics fm(opt.font);

        // Repaint the row background over `rect` (text suppressed), then draw
        // `fragment` there in `color` -- readable on both the plain row and
        // the light selection highlight.
        auto recolour = [&](const QRect& rect, const QString& fragment,
                            const QColor& color) {
            painter->save();
            painter->setClipRect(rect);
            QStyleOptionViewItem bgOpt(opt);
            bgOpt.text.clear();
            style->drawControl(QStyle::CE_ItemViewItem, &bgOpt, painter, bgOpt.widget);
            painter->setFont(opt.font);
            painter->setPen(color);
            painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, fragment);
            painter->restore();
        };

        if (hasDelete)
        {
            const QString base = text.left(text.length() - kDelete.length());
            const int suffixLeft =
                textRect.left() + textMargin + fm.horizontalAdvance(base);
            if (suffixLeft < textRect.right())   // else elided -- leave as-is
                recolour(QRect(suffixLeft, textRect.top(),
                               textRect.right() - suffixLeft, textRect.height()),
                         kDelete, kKeywordColor);
        }

        if (hasVirtual)
        {
            const int kwLeft  = textRect.left() + textMargin;
            const int kwWidth = fm.horizontalAdvance(QStringLiteral("virtual"));
            if (kwLeft + kwWidth <= textRect.right())
                recolour(QRect(kwLeft, textRect.top(), kwWidth, textRect.height()),
                         QStringLiteral("virtual"), kKeywordColor);
        }
    }
};

// The main tree's widget: CbTreeWidget + a full-row drag drop-target highlight.
// A delegate can't paint the highlight across the whole row (it's clipped to the
// indented item cell), so it's overlaid in paintEvent: a translucent accent tint
// over the FULL row width + a solid accent bar pinned to the left edge (x=0), so
// it doesn't shift with the row's indentation. Accent = QPalette::Highlight (the
// green the diagrams + checkboxes use).
class MainTreeWidget : public CbTreeWidget
{
public:
    explicit MainTreeWidget(QWidget* parent = nullptr) : CbTreeWidget(parent) {}

    void setDropHighlight(QTreeWidgetItem* item)
    {
        if (item == _dropTarget)
            return;
        _dropTarget = item;
        viewport()->update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        CbTreeWidget::paintEvent(event);
        if (!_dropTarget)
            return;

        const QRect vr = visualItemRect(_dropTarget);
        if (vr.height() <= 0)
            return;

        QPainter p(viewport());
        const QColor accent =
            QApplication::palette().color(QPalette::Active, QPalette::Highlight);
        QColor tint = accent;
        tint.setAlpha(40);
        p.fillRect(QRect(0, vr.top(), viewport()->width(), vr.height()), tint);
        p.fillRect(QRect(0, vr.top(), 3, vr.height()), accent);
    }

private:
    QTreeWidgetItem* _dropTarget = nullptr;
};

MainTreeQtView::MainTreeQtView(DataModelDoc* pDataModelDoc, void* ownerHwnd,
                               Gti* subTree, QWidget* parent)
    : QDialog(parent)
    , _tree(new MainTreeWidget(this))
    , _pDoc(pDataModelDoc)
    , _ownerHwnd(ownerHwnd)
{
    _tree->setColumnCount(1);
    _tree->setHeaderHidden(true);
    _tree->setItemDelegate(new SignatureKeywordDelegate(_tree));
    // Icon size is set centrally in CbTreeWidget (shared by every tree); capture
    // it as the single-glyph base (rebuild() widens it to two when a phase
    // marker is shown).
    _baseIconSize = _tree->iconSize();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);          // no gap under the toolbar (matches the diagrams)
    layout->addWidget(_tree);

    // M4: this mirror's per-view filter state lives in its OWN TreeViewModel
    // (model-side, non-serialized -- views aren't stored). nodeVisible() reads
    // it through the shared model method Gti::ShownByFilter; the Filters dialog
    // flips the VM's bools. The VM also carries this view's refresh/close
    // callbacks (the SD/CD pattern): the model fires Refresh()
    // (DataModelDoc::UpdateTreeViews) to rebuild this tree, and cascade-deletes
    // the VM on doc close to close the window -- no openTrees registry needed.
    _vm = new TreeViewModel(_pDoc,
                            &MainTreeQtView::RefreshTree,
                            &MainTreeQtView::CloseTree,
                            this,
                            subTree);

    // A slim toolbar instead of a menu bar: embedded as the shell's central
    // widget a second menu row under the main menu read badly; compact buttons
    // cost no extra line visually and work the same when this view floats
    // (sub-windows). Shortcuts ride on the actions (window context).
    //
    // The Add-buttons reuse the model tree icons (Qt_ModelIcon -- the same
    // glyphs the rows show; each sharpens automatically once its SVG redraw
    // lands beside the legacy .ico). They act on the CURRENT selection with
    // the same gating as the context menu (checkOnly pass), re-evaluated on
    // every selection change.
    QToolBar* toolBar = new QToolBar(this);
    toolBar->setMovable(false);
    // Icon-only and trimmed, identical to the diagram bars so the height
    // matches (text labels made this bar taller and wider). Every action has
    // an icon so nothing blanks; tooltips carry the names.
    toolBar->setIconSize(QSize(CB_TOOLBAR_ICON_PX, CB_TOOLBAR_ICON_PX));
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setStyleSheet("QToolBar{border:0;spacing:1px;}"
                           "QToolButton{padding:1px;}");
    if (toolBar->layout())
        toolBar->layout()->setContentsMargins(0, 0, 0, 0);

    auto addTb = [this, toolBar](const QIcon& icon, const char* text,
                                 const char* tip, TreeAction a)
    {
        QAction* act = toolBar->addAction(icon, text);
        const QKeySequence sc = treeAddShortcut(a);
        act->setToolTip(sc.isEmpty()
            ? QString::fromUtf8(tip)
            : QString("%1 (%2)").arg(QString::fromUtf8(tip),
                                     sc.toString(QKeySequence::NativeText)));
        if (!sc.isEmpty())
        {
            act->setShortcut(sc);
            // Fire when the tree (a CHILD of the view) has focus, not just the
            // toolbar -- so register the action on the view too. WidgetWithChildren
            // keeps it scoped to THIS view, so CD/SD can reuse the same keys.
            act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            addAction(act);
        }
        connect(act, &QAction::triggered, this, [this, a] {
            QTreeWidgetItem* item = _tree->currentItem();
            Gti* pGti = item ? reinterpret_cast<Gti*>(itemGti(item))
                             : _pDoc->GetDataModel();
            if (treeActionInvoke(pGti, a, true))
            {
                treeActionInvoke(pGti, a, false);
                _pDoc->MarkLastUndo();
            }
        });
        _toolBarActions.append(qMakePair(int(a), act));
    };

    // Same MFC toolbar-strip glyphs the diagrams use (Qt_ToolBarIcon); Actor has
    // no strip glyph so it keeps its tree-node icon.
    addTb(Qt_ToolBarIcon(TG_ADD_CLASS),       "Class",       "Add a class",           TreeAction::AddClass);
    addTb(Qt_ToolBarIcon(TG_ADD_INHERIT),     "Inheritance", "Add an inheritance",    TreeAction::AddInherit);
    addTb(Qt_ToolBarIcon(TG_ADD_RELATION),    "Relation",    "Add a relation",        TreeAction::AddRelation);
    toolBar->addSeparator();
    addTb(Qt_ToolBarIcon(TG_ADD_MEMBER),      "Member",      "Add a member",          TreeAction::AddMember);
    addTb(Qt_ToolBarIcon(TG_ADD_FUNCTION),    "Method",      "Add a method",          TreeAction::AddMethod);
    addTb(Qt_ToolBarIcon(TG_ADD_CONSTRUCTOR), "Constructor", "Add a constructor",     TreeAction::AddConstructor);
    addTb(Qt_ToolBarIcon(TG_ADD_ARGUMENT),    "Argument",    "Add an argument",       TreeAction::AddArgument);
    addTb(Qt_ToolBarIcon(TG_ADD_VIRTUALS),    "Virtual Methods", "Add virtual methods", TreeAction::AddVirtuals);
    addTb(Qt_ToolBarIcon(TG_ADD_ISCLASS),     "IsClass Methods", "Add IsClass methods", TreeAction::AddIsClass);
    toolBar->addSeparator();
    addTb(Qt_ToolBarIcon(TG_ADD_TYPE),        "Type",        "Add a type",            TreeAction::AddType);
    addTb(Qt_ModelIcon(ICON_ACTOR),           "Actor",       "Add an actor",          TreeAction::AddActor);
    toolBar->addSeparator();
    addTb(Qt_ToolBarIcon(TG_ADD_CLASSDIAGRAM),    "Class Diagram",    "Add a class diagram",    TreeAction::AddClassDiagram);
    addTb(Qt_ToolBarIcon(TG_ADD_SEQUENCEDIAGRAM), "Sequence Diagram", "Add a sequence diagram", TreeAction::AddSequenceDiagram);
    toolBar->addSeparator();
    addTb(Qt_ToolBarIcon(TG_EDIT_DELETE),     "Delete",      "Delete the selected node", TreeAction::Delete);
    toolBar->addSeparator();

    // Group / Meta Group / External Class have an accelerator but NO toolbar button --
    // bind a QShortcut so the key FIRES (the context menu otherwise only shows a key
    // that does nothing). WidgetWithChildren scopes it to this tree so CD/SD can reuse
    // non-tree keys.
    auto addKeyOnly = [this](TreeAction a)
    {
        // QShortcut on _tree (the focus widget). A hidden QAction added via addAction()
        // did NOT fire without a visible host; the QShortcut does. The slot self-gates
        // via the checkOnly pass, so the menu item's own enable is the only gating.
        QShortcut* shortcut = new QShortcut(treeAddShortcut(a), _tree);  // _tree IS the focus widget
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(shortcut, &QShortcut::activated, this, [this, a] {
            QTreeWidgetItem* item = _tree->currentItem();
            Gti* pGti = item ? reinterpret_cast<Gti*>(itemGti(item))
                             : _pDoc->GetDataModel();
            if (treeActionInvoke(pGti, a, true))
            {
                treeActionInvoke(pGti, a, false);
                _pDoc->MarkLastUndo();
            }
        });
    };
    addKeyOnly(TreeAction::AddGroup);
    addKeyOnly(TreeAction::AddMetaGroup);
    addKeyOnly(TreeAction::AddExternClass);   // Ctrl+Shift+E -- X was grabbed by a global OS hotkey

    QAction* actFilters = toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("document-properties")), "Filters...",
        this, &MainTreeQtView::showFilterDialog);
    actFilters->setToolTip("Access / phase filters for this tree");
    QAction* actFind = toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("edit-find")), "Find...",
        this, &MainTreeQtView::onFindKey);
    actFind->setShortcut(QKeySequence::Find);
    actFind->setToolTip("Find a node by name (Ctrl+F)");
    QAction* actNext = toolBar->addAction(
        QIcon::fromTheme(QStringLiteral("go-next")), "Next",
        this, &MainTreeQtView::onRepeatKey);
    actNext->setShortcut(QKeySequence::FindNext);
    actNext->setToolTip("Find next match (F3)");

    // Undo/Redo live ONLY here (and on the diagram bars), NOT on the main
    // window toolbar: with several models open it must be unambiguous WHICH
    // model an undo hits -- so the button sits on the very view whose model it
    // mutates (_pDoc). Always visible, docked or floated. Same MFC glyphs / size
    // as the rest of the bar; enable tracks the doc's undo/redo stack
    // (refreshed in updateToolBarEnables).
    _undoRedoSep = toolBar->addSeparator();
    _undoAction = toolBar->addAction(Qt_ToolBarIcon(TG_EDIT_UNDO), "Undo",
        this, [this] { _pDoc->Undo(); });
    _undoAction->setToolTip("Undo (Ctrl+Z)");
    _redoAction = toolBar->addAction(Qt_ToolBarIcon(TG_EDIT_REDO), "Redo",
        this, [this] { _pDoc->Redo(); });
    _redoAction->setToolTip("Redo (Ctrl+Y)");

    layout->insertWidget(0, toolBar);

    CbString title = CbString("Qt tree -- ") +
        (subTree ? subTree->GetItemText() : _pDoc->GetDataModel()->GetItemText());
    setWindowTitle(toQ(title));
    resize(360, 640);

    // M1: user selection drives the MFC tree; double-click opens / edits.
    // Double-click is bound to open/edit, so suppress Qt's default expand/
    // collapse-on-double-click (the triangle/arrow + keyboard still expand).
    _tree->setExpandsOnDoubleClick(false);
    connect(_tree, &QTreeWidget::currentItemChanged,
            this, &MainTreeQtView::onCurrentItemChanged);
    connect(_tree, &QTreeWidget::itemDoubleClicked,
            this, &MainTreeQtView::onItemDoubleClicked);

    _tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_tree, &QWidget::customContextMenuRequested,
            this, &MainTreeQtView::onContextMenu);

    // M3: manual drag-drop on the viewport (Qt's own drag stays disabled). The
    // green drop-target highlight is painted by MainTreeWidget::paintEvent.
    _tree->viewport()->installEventFilter(this);

    // Del key deletes the current node (the MFC tree relies on the Del
    // accelerator -- its context menu has no Delete item either).
    QShortcut* del = new QShortcut(QKeySequence::Delete, _tree);
    del->setContext(Qt::WidgetWithChildrenShortcut);
    connect(del, &QShortcut::activated, this, &MainTreeQtView::onDeleteKey);

    // Ctrl+C / Ctrl+V -- the keyboard equivalents of the context-menu Copy/Paste
    // (the MFC tree relies on the same accelerators).
    QShortcut* copy = new QShortcut(QKeySequence::Copy, _tree);
    copy->setContext(Qt::WidgetWithChildrenShortcut);
    connect(copy, &QShortcut::activated, this, &MainTreeQtView::onCopyKey);

    QShortcut* paste = new QShortcut(QKeySequence::Paste, _tree);
    paste->setContext(Qt::WidgetWithChildrenShortcut);
    connect(paste, &QShortcut::activated, this, &MainTreeQtView::onPasteKey);

    rebuild();
    updateToolBarEnables();
}

MainTreeQtView::~MainTreeQtView()
{
    // Mirror the CD/SD canvas teardown: window-first -> _destructing makes the
    // VM's CloseTree no-op while we delete it; doc-first -> the cascade already
    // deleted the VM and nulled _vm via notifyModelGone().
    _destructing = true;
    if (_vm)
    {
        delete _vm;
        _vm = nullptr;
    }
}

void MainTreeQtView::RefreshTree(void* ctx)
{
    // Posted, never synchronous (refresh-audit step 2): this callback fires
    // from inside model mutations, so a synchronous full rebuild here can
    // re-enter the model mid-op (worst case from a draw-time relayout, i.e. a
    // tree rebuild inside a paintEvent). Queue one coalesced rebuild instead;
    // by the time it runs the mutation stack has unwound.
    if (auto* self = static_cast<MainTreeQtView*>(ctx))
        self->scheduleRebuild();
}

void MainTreeQtView::scheduleRebuild()
{
    if (_rebuildQueued)
        return;
    _rebuildQueued = true;
    QMetaObject::invokeMethod(this, [this] {
        _rebuildQueued = false;
        // The doc can die while the call is in flight (notifyModelGone nulls
        // _vm, the window close is itself deferred) -- nothing to rebuild then.
        if (_vm)
            rebuild();
    }, Qt::QueuedConnection);
}

void MainTreeQtView::CloseTree(void* ctx)
{
    if (auto* self = static_cast<MainTreeQtView*>(ctx))
        self->notifyModelGone();
}

void MainTreeQtView::notifyModelGone()
{
    if (_destructing)
        return;
    _vm = nullptr;   // already being cascade-deleted by the doc
    close();         // WA_DeleteOnClose -> this window is destroyed
}

void MainTreeQtView::showFilterDialog()
{
    if (_filterDialog)   // single instance; just raise it
    {
        _filterDialog->raise();
        _filterDialog->activateWindow();
        return;
    }

    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Tree filters");
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    _filterDialog = dlg;
    connect(dlg, &QObject::destroyed, this, [this]
            { _filterDialog = nullptr; _filterPhasesGroup = nullptr; });

    QVBoxLayout* outer = new QVBoxLayout(dlg);

    // One checkbox per VM flag, live: toggling sets the VM bool + rebuilds, so
    // the tree updates as you click. Non-modal so it sits beside the tree.
    auto addCheck = [this](QBoxLayout* lay, const QString& text,
                           bool (TreeViewModel::*get)() const,
                           void (TreeViewModel::*set)(bool))
    {
        QCheckBox* cb = new QCheckBox(text);
        cb->setChecked((_vm->*get)());
        connect(cb, &QCheckBox::toggled, this,
                [this, set](bool on) { (_vm->*set)(on); rebuild(); });
        lay->addWidget(cb);
    };

    QGroupBox* gClasses = new QGroupBox("Classes");
    QVBoxLayout* lClasses = new QVBoxLayout(gClasses);
    addCheck(lClasses, "Only without constructor",
             &TreeViewModel::GetShowOnlyClassesWithoutConstructor,
             &TreeViewModel::SetShowOnlyClassesWithoutConstructor);
    outer->addWidget(gClasses);

    QGroupBox* gMembers = new QGroupBox("Members");
    QVBoxLayout* lMembers = new QVBoxLayout(gMembers);
    QHBoxLayout* lMembersStatic = new QHBoxLayout();
    addCheck(lMembersStatic, "Non Static", &TreeViewModel::GetShowNonStaticMembers, &TreeViewModel::SetShowNonStaticMembers);
    addCheck(lMembersStatic, "Static",     &TreeViewModel::GetShowStaticMembers,    &TreeViewModel::SetShowStaticMembers);
    lMembersStatic->addStretch();
    lMembers->addLayout(lMembersStatic);
    addCheck(lMembers, "Public",    &TreeViewModel::GetShowPublicMembers,    &TreeViewModel::SetShowPublicMembers);
    addCheck(lMembers, "Protected", &TreeViewModel::GetShowProtectedMembers, &TreeViewModel::SetShowProtectedMembers);
    addCheck(lMembers, "Private",   &TreeViewModel::GetShowPrivateMembers,   &TreeViewModel::SetShowPrivateMembers);
    outer->addWidget(gMembers);

    QGroupBox* gMethods = new QGroupBox("Methods");
    QVBoxLayout* lMethods = new QVBoxLayout(gMethods);
    QHBoxLayout* lMethodsStatic = new QHBoxLayout();
    addCheck(lMethodsStatic, "Non Static", &TreeViewModel::GetShowNonStaticMethods, &TreeViewModel::SetShowNonStaticMethods);
    addCheck(lMethodsStatic, "Static",     &TreeViewModel::GetShowStaticMethods,    &TreeViewModel::SetShowStaticMethods);
    lMethodsStatic->addStretch();
    lMethods->addLayout(lMethodsStatic);
    addCheck(lMethods, "Public",    &TreeViewModel::GetShowPublicMethods,    &TreeViewModel::SetShowPublicMethods);
    addCheck(lMethods, "Protected", &TreeViewModel::GetShowProtectedMethods, &TreeViewModel::SetShowProtectedMethods);
    addCheck(lMethods, "Private",   &TreeViewModel::GetShowPrivateMethods,   &TreeViewModel::SetShowPrivateMethods);
    outer->addWidget(gMethods);

    QGroupBox* gRelations = new QGroupBox("Relations");
    QVBoxLayout* lRelations = new QVBoxLayout(gRelations);
    addCheck(lRelations, "Inheritance", &TreeViewModel::GetShowInheritance, &TreeViewModel::SetShowInheritance);
    QHBoxLayout* lRelCard = new QHBoxLayout();
    addCheck(lRelCard, "Multi",  &TreeViewModel::GetShowMultiRelations,  &TreeViewModel::SetShowMultiRelations);
    addCheck(lRelCard, "Single", &TreeViewModel::GetShowSingleRelations, &TreeViewModel::SetShowSingleRelations);
    lRelCard->addStretch();
    lRelations->addLayout(lRelCard);
    QHBoxLayout* lRelAgg = new QHBoxLayout();
    addCheck(lRelAgg, "Aggregation", &TreeViewModel::GetShowAggregationRelations,    &TreeViewModel::SetShowAggregationRelations);
    addCheck(lRelAgg, "Association", &TreeViewModel::GetShowNonAggregationRelations, &TreeViewModel::SetShowNonAggregationRelations);
    lRelAgg->addStretch();
    lRelations->addLayout(lRelAgg);
    // Static relations are intentionally always shown (Relation::ShownByFilter
    // bypasses the gate), so there's no checkbox for them.
    outer->addWidget(gRelations);

    QGroupBox* gPhases = new QGroupBox("Phases");
    QVBoxLayout* lPhases = new QVBoxLayout(gPhases);
    addCheck(lPhases, "Analysis",       &TreeViewModel::GetShowAnalysisPhase,       &TreeViewModel::SetShowAnalysisPhase);
    addCheck(lPhases, "Design",         &TreeViewModel::GetShowDesignPhase,         &TreeViewModel::SetShowDesignPhase);
    addCheck(lPhases, "Implementation", &TreeViewModel::GetShowImplementationPhase, &TreeViewModel::SetShowImplementationPhase);
    addCheck(lPhases, "Test",           &TreeViewModel::GetShowTestPhase,           &TreeViewModel::SetShowTestPhase);
    addCheck(lPhases, "Complete",       &TreeViewModel::GetShowCompletePhase,       &TreeViewModel::SetShowCompletePhase);
    // Phases only mean anything when the model has phase support -- grey them out
    // otherwise (matches ShownByFilter). Kept live: rebuild() re-syncs this when
    // phase support is toggled (e.g. in the DataModel dialog) while this stays open.
    _filterPhasesGroup = gPhases;
    gPhases->setEnabled(_pDoc->GetDataModel()->GetPhaseSupport());
    outer->addWidget(gPhases);

    dlg->show();
}

void MainTreeQtView::rebuild()
{
    // Remember expansion + selection (by Gti*, which survives the rebuild).
    QSet<quintptr> expanded;
    quintptr       selected = 0;
    for (int i = 0; i < _tree->topLevelItemCount(); ++i)
        captureState(_tree->topLevelItem(i), expanded, selected);

    _tree->setDropHighlight(nullptr);   // the target item is about to be deleted

    // Programmatic repopulate: the selection churn from clear()/restore must not
    // drive the MFC bridge.
    _applyingExternalSelection = true;

    // Phase markers on when the model has phase support -- widen the icon area to
    // two glyphs so the composited [phase][type] icon renders unsquished.
    _phaseSupport = _pDoc->GetDataModel()->GetPhaseSupport();
    _tree->setIconSize(_phaseSupport
        ? QSize(_baseIconSize.width() * 2, _baseIconSize.height())
        : _baseIconSize);

    // Keep an open Filters dialog's Phases group in sync with phase support.
    if (_filterPhasesGroup)
        _filterPhasesGroup->setEnabled(_phaseSupport);

    _tree->clear();
    _itemByGti.clear();

    // The top-level roots CClassBuilderView::RedrawTree inserts, then sorted by
    // the same comparator that orders them under TVI_ROOT (DataModel, then any
    // MetaGroups, then Actors, then the Extern Classes / Other Types containers).
    std::vector<Gti*> roots;
    if (Gti* sub = _vm->GetSubTree())
    {
        // Sub-window: the single scoped root + its descendants (mirrors the MFC
        // sub-window, which Show()s just m_pGtiSubTree).
        roots.push_back(sub);
    }
    else
    {
        DataModel* pDataModel = _pDoc->GetDataModel();
        roots.push_back(pDataModel);
        DataModel::MetaGroupIterator iMetaGroup(pDataModel);
        while (++iMetaGroup)
            roots.push_back(iMetaGroup.Get());
        roots.push_back(_pDoc->GetExternClasses());
        roots.push_back(_pDoc->GetOtherTypes());
        roots.push_back(_pDoc->GetActors());
    }

    std::stable_sort(roots.begin(), roots.end(),
                     [](Gti* a, Gti* b) { return Gti::CompareTreeOrder(a, b) < 0; });
    for (Gti* pRoot : roots)
        addGtiRecursive(pRoot, nullptr);

    // Restore expansion + selection.
    QTreeWidgetItem* selItem = nullptr;
    for (int i = 0; i < _tree->topLevelItemCount(); ++i)
        restoreState(_tree->topLevelItem(i), expanded, selected, selItem);
    if (selItem)
        _tree->setCurrentItem(selItem);

    _applyingExternalSelection = false;

    // rebuild() is the per-model-change refresh (RefreshTree callback), so it's
    // the reliable spot to re-evaluate the toolbar enables -- the Add/Delete
    // gates AND the floated-tree Undo/Redo (which track the undo stack, not the
    // selection, so they wouldn't otherwise refresh after a plain edit/undo).
    updateToolBarEnables();

    // This tree's model changed -> ask the shell to refresh the undo/redo
    // enables of every OTHER open view of this model (they don't observe this
    // tree's rebuild and would otherwise show stale undo/redo state). Routed
    // through DataModelDoc so only it knows the framework doc.
    _pDoc->NotifyStateChanged();
}

bool MainTreeQtView::nodeVisible(Gti* pGti) const
{
    // The visibility decision is a MODEL method (Gti::ShownByFilter, overridden
    // by Member/Method) reading this view's TreeViewModel -- the same one the
    // MFC tree's Show() will call. Nothing Qt-specific here anymore.
    return pGti->ShownByFilter(_vm);
}

void MainTreeQtView::addGtiRecursive(Gti* pGti, QTreeWidgetItem* parent)
{
    if (!pGti || !pGti->GetAdded())
        return;   // mirrors the _added gate in Gti::Show
    if (!nodeVisible(pGti))
        return;   // M4 view filters (access / phase)

    QTreeWidgetItem* item = parent ? new QTreeWidgetItem(parent)
                                   : new QTreeWidgetItem(_tree);
    item->setText(0, toQ(pGti->GetItemText()));

    const QIcon typeIcon = Qt_ModelIcon(pGti->GetIcon());
    if (_phaseSupport)
    {
        // [phase][type], matching the MFC tree's state-image + normal-image
        // layout. Phase 0 (None) leaves the left glyph blank so the type icons
        // (and text) stay column-aligned. Build at the real device-pixel-ratio
        // and request each glyph at that ratio, else the view upscales a dpr=1
        // pixmap and both icons blur (badly for the larger diagram glyphs).
        const int   w   = _baseIconSize.width();
        const int   h   = _baseIconSize.height();
        const qreal dpr = _tree->devicePixelRatioF();
        QPixmap pm(qRound(w * 2 * dpr), qRound(h * dpr));
        pm.setDevicePixelRatio(dpr);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        const QIcon phaseIcon = Qt_PhaseIcon(pGti->GetPhase());
        if (!phaseIcon.isNull())
        {
            // At full cell size the phase marker dominates the row (JV
            // 2026-07-12) -- draw it at 70% in its slot, centred.
            const int pw = qRound(w * 0.70);
            const int ph = qRound(h * 0.70);
            p.drawPixmap(QPoint((w - pw) / 2, (h - ph) / 2),
                         phaseIcon.pixmap(QSize(pw, ph), dpr));
        }
        p.drawPixmap(QPoint(w, 0), typeIcon.pixmap(QSize(w, h), dpr));
        p.end();
        item->setIcon(0, QIcon(pm));
    }
    else
    {
        item->setIcon(0, typeIcon);
    }
    setItemGti(item, pGti);
    _itemByGti.insert(reinterpret_cast<quintptr>(pGti), item);

    for (Gti* pChild : sortedChildren(pGti))
        addGtiRecursive(pChild, item);
}

void MainTreeQtView::captureState(QTreeWidgetItem* item, QSet<quintptr>& expanded,
                                  quintptr& selected) const
{
    quintptr gti = itemGti(item);
    if (gti && item->isExpanded())
        expanded.insert(gti);
    if (gti && item == _tree->currentItem())
        selected = gti;

    for (int i = 0; i < item->childCount(); ++i)
        captureState(item->child(i), expanded, selected);
}

void MainTreeQtView::restoreState(QTreeWidgetItem* item, const QSet<quintptr>& expanded,
                                  quintptr selected, QTreeWidgetItem*& selItemOut) const
{
    quintptr gti = itemGti(item);
    if (gti && expanded.contains(gti))
        item->setExpanded(true);
    if (gti && gti == selected)
        selItemOut = item;

    for (int i = 0; i < item->childCount(); ++i)
        restoreState(item->child(i), expanded, selected, selItemOut);
}

// ---------------------------------------------------------------------------
// Selection / navigation (the MFC selection-echo bridge is gone with the MFC
// tree -- this view's own currentItem IS the selection now)
// ---------------------------------------------------------------------------
void MainTreeQtView::onCurrentItemChanged(QTreeWidgetItem* /*current*/, QTreeWidgetItem*)
{
    updateToolBarEnables();
}

void MainTreeQtView::updateToolBarEnables()
{
    QTreeWidgetItem* item = _tree->currentItem();
    Gti* pGti = item ? reinterpret_cast<Gti*>(itemGti(item))
                     : _pDoc->GetDataModel();
    for (const auto& pair : _toolBarActions)
        pair.second->setEnabled(
            treeActionEnabled(pGti, static_cast<TreeAction>(pair.first)));

    // Undo/Redo track the doc's undo/redo STACK (not the selection).
    refreshUndoRedoEnables();
}

// Enable-only refresh of this tree's Undo/Redo buttons -- the shell invokes it
// by name on every open view when any view broadcasts a state change, so all
// views of a model agree on undo/redo availability.
void MainTreeQtView::refreshUndoRedoEnables()
{
    if (_undoAction) _undoAction->setEnabled(_pDoc->CanUndo());
    if (_redoAction) _redoAction->setEnabled(_pDoc->CanRedo());
}

void MainTreeQtView::setFloating(bool /*floating*/)
{
    // Undo/Redo are always visible on this bar now (no longer float-gated --
    // they're the model's only undo buttons). Re-evaluate enables across the
    // dock/float transition, since the stack state isn't selection-driven.
    updateToolBarEnables();
}

void MainTreeQtView::onItemDoubleClicked(QTreeWidgetItem* item, int)
{
    if (!item)
        return;

    Gti* pGti = reinterpret_cast<Gti*>(itemGti(item));
    // Ctrl or Alt -> Edit Attributes, plain -> Open (mirrors the MFC OnDblclk
    // VK_CONTROL/VK_MENU test). Ctrl+Alt is avoided (AltGr on EU layouts).
    //
    // A double-click ALWAYS acts. Only a METHOD distinguishes the two: plain
    // opens its code editor, Ctrl/Alt opens its attributes. For every other
    // node type Gti::OnOpen aliases to OnEditAttributes, so plain and Ctrl/Alt
    // do the same thing -- e.g. double-clicking a class edits it. (Diagrams are
    // the one node whose plain Open opens a view rather than a dialog.)
    //
    // M-C: drive the model directly (as the context menu does) instead of
    // round-tripping through the MFC view's posted ID_RETURN/ID_EDIT_ATTRIBUTES
    // -- the Qt tree is now self-sufficient for open/edit.
    const Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    const TreeAction action = (mods & (Qt::ControlModifier | Qt::AltModifier))
                                  ? TreeAction::EditAttributes
                                  : TreeAction::Open;
    // Plain open of a diagram opens its Qt view, not the MFC MDI child.
    if (action == TreeAction::Open && openDiagramViewIfDiagram(pGti))
        return;
    treeActionInvoke(pGti, action, false);
    _pDoc->MarkLastUndo();
}

bool MainTreeQtView::openDiagramViewIfDiagram(Gti* pGti)
{
    // Open the Qt diagram view rather than the MFC MDI child (Gti::OnOpen would
    // create the MFC view). Non-diagram opens still go through the normal path.
    if (pGti->IsClassDiagram())
    {
        Qt_ShowClassDiagramView(dynamic_cast<ClassDiagram*>(pGti), _ownerHwnd);
        return true;
    }
    if (pGti->IsSequenceDiagram())
    {
        Qt_ShowSequenceDiagramView(dynamic_cast<SequenceDiagram*>(pGti), _ownerHwnd);
        return true;
    }
    return false;
}

// One-click "delete the copy operations" on a class:
//     ClassName(const ClassName& rClassName)             = delete;
//     ClassName& operator =(const ClassName& rClassName) = delete;
// Both Declare-on / Implement-off / Delete-on (no body), created as a SINGLE
// undo step via the same new -> set flags -> Add -> NotifyAddMethod model path
// the GUI dialogs and the pipe use. pClass is a Class (so also a BaseClass and
// a Type -- reused as the self-type for the argument and the operator= return).
static void addDeletedCopyOps(Class* pClass)
{
    if (!pClass)
        return;

    pClass->GetDataModelDoc()->MarkLastUndo();

    const CbString cls     = pClass->GetName();
    const CbString argName = CbString("r") + cls;

    // Copy constructor -- one `const ClassName&` argument. The Constructor ctor
    // already names it after the class and sets public access; we do NOT call
    // CreateArguments (that seeds owned-relation parent pointers we don't want).
    Constructor* pCtor = new Constructor(pClass);
    Argument* pCtorArg = new Argument(pCtor, pClass);
    pCtorArg->SetConst(1);
    pCtorArg->SetReference(1);
    pCtorArg->SetName(argName);
    pCtor->SetDeclare(1);
    pCtor->SetImplement(0);
    pCtor->SetDelete(true);
    pCtor->SetCode("");
    pCtor->Add();
    pClass->NotifyAddMethod(pCtor);

    // Copy assignment -- `ClassName&` return, one `const ClassName&` argument.
    Method* pAssign = new Method(pClass, pClass);
    pAssign->SetAccess(PUBLIC);
    pAssign->SetReference(1);                 // ClassName& return
    pAssign->SetName("operator =");
    Argument* pAsgArg = new Argument(pAssign, pClass);
    pAsgArg->SetConst(1);
    pAsgArg->SetReference(1);
    pAsgArg->SetName(argName);
    pAssign->SetDeclare(1);
    pAssign->SetImplement(0);
    pAssign->SetPure(0);
    pAssign->SetDelete(true);
    pAssign->SetCode("");
    pAssign->Add();
    pClass->NotifyAddMethod(pAssign);
}

void MainTreeQtView::onContextMenu(const QPoint& pos)
{
    QTreeWidgetItem* item = _tree->itemAt(pos);
    if (item)
        _tree->setCurrentItem(item);   // right-click selects (syncs MFC m_current)

    // The right-clicked node, or the DataModel when over empty space -- the same
    // fallback the MFC handlers use (m_current ? m_current : GetDataModel()).
    Gti* pGti = item ? reinterpret_cast<Gti*>(itemGti(item)) : _pDoc->GetDataModel();
    if (!pGti)
        return;

    QMenu menu(this);
    auto add = [&](QMenu* m, const QString& text, TreeAction a)
    {
        QAction* act = m->addAction(text);
        act->setData(static_cast<int>(a));
        act->setEnabled(treeActionEnabled(pGti, a));
        // Show the accelerator hint right-aligned in the menu (discoverability).
        // The persistent toolbar action above does the actual firing; this menu
        // action only exists while the popup is open (when the toolbar action's
        // WidgetWithChildren shortcut is inactive), so they never both fire.
        const QKeySequence sc = treeAddShortcut(a);
        if (!sc.isEmpty())
            act->setShortcut(sc);
    };

    // Order mirrors the MFC IDR_SHORTCUT "View" popup.
    add(&menu, "Open", TreeAction::Open);
    add(&menu, "Edit Attributes", TreeAction::EditAttributes);
    // Combined editor kept at top level (fast to reach); the 6 individual
    // regions live in a submenu beside it.
    add(&menu, "Edit User Sections", TreeAction::UserSections);
    QMenu* sectionMenu = menu.addMenu("Edit Section");
    sectionMenu->menuAction()->setEnabled(treeActionEnabled(pGti, TreeAction::UserSections));
    add(sectionMenu, "Before class definition  *.h", TreeAction::UserSection1H);
    add(sectionMenu, "Inside class definition  *.h", TreeAction::UserSection2H);
    add(sectionMenu, "After class definition  *.h",  TreeAction::UserSection3H);
    sectionMenu->addSeparator();
    add(sectionMenu, "Before master include  *.cpp", TreeAction::UserSection1Cpp);
    add(sectionMenu, "After master include  *.cpp",  TreeAction::UserSection2Cpp);
    add(sectionMenu, "After generated code  *.cpp",  TreeAction::UserSection3Cpp);
    add(&menu, "Edit Exception Specification", TreeAction::EditExceptionSpecification);
    // Context label: "Edit" on the DataModel, "Assign" elsewhere (OnUpdateEditContext).
    {
        QAction* act = menu.addAction(
            pGti->IsDataModel() ? "Edit Context" : "Assign Context");
        act->setData(static_cast<int>(TreeAction::EditContext));
        act->setEnabled(treeActionEnabled(pGti, TreeAction::EditContext));
    }
    menu.addSeparator();
    add(&menu, "Copy", TreeAction::Copy);
    add(&menu, "Paste", TreeAction::Paste);
    add(&menu, "Sort on Name", TreeAction::SortOnName);
    add(&menu, "Sort on Phase", TreeAction::SortOnPhase);

    menu.addSeparator();

    QMenu* addMenu = menu.addMenu("Add");
    add(addMenu, "Class Diagram",    TreeAction::AddClassDiagram);
    add(addMenu, "Sequence Diagram", TreeAction::AddSequenceDiagram);
    addMenu->addSeparator();
    add(addMenu, "Group",           TreeAction::AddGroup);
    add(addMenu, "Class",           TreeAction::AddClass);
    add(addMenu, "External Class",  TreeAction::AddExternClass);
    add(addMenu, "Inheritance",     TreeAction::AddInherit);
    add(addMenu, "Relation",        TreeAction::AddRelation);
    add(addMenu, "Member",          TreeAction::AddMember);
    add(addMenu, "Method",          TreeAction::AddMethod);
    add(addMenu, "Constructor",     TreeAction::AddConstructor);
    add(addMenu, "Argument",        TreeAction::AddArgument);
    add(addMenu, "Meta Group",      TreeAction::AddMetaGroup);
    add(addMenu, "Actor",           TreeAction::AddActor);
    addMenu->addSeparator();
    add(addMenu, "Virtual Methods", TreeAction::AddVirtuals);
    add(addMenu, "IsClass Methods", TreeAction::AddIsClass);
    addMenu->addSeparator();
    add(addMenu, "Type",            TreeAction::AddType);

    // One-click deleted copy operations -- only meaningful on a real Class.
    QAction* actDeletedCopyOps = nullptr;
    if (pGti->IsClass())
    {
        addMenu->addSeparator();
        actDeletedCopyOps = addMenu->addAction("Deleted Copy Ctor + operator=");
    }

    menu.addSeparator();
    QMenu* phaseMenu = menu.addMenu("Phase");
    add(phaseMenu, "Analysis",       TreeAction::PhaseAnalysis);
    add(phaseMenu, "Design",         TreeAction::PhaseDesign);
    add(phaseMenu, "Implementation", TreeAction::PhaseImplementation);
    add(phaseMenu, "Test",           TreeAction::PhaseTest);
    add(phaseMenu, "Complete",       TreeAction::PhaseComplete);

    menu.addSeparator();
    add(&menu, "Base Classes",    TreeAction::BaseClasses);
    add(&menu, "Derived Classes", TreeAction::DerivedClasses);

    menu.addSeparator();
    add(&menu, "New Sub Window", TreeAction::NewSubWindow);
    add(&menu, "Delete Multiple...", TreeAction::DeleteMultiple);

    // Dispatch AFTER exec (not from a lambda mid-popup): a model action triggers
    // a tree rebuild via OnUpdate, which must not run while the popup is open.
    QAction* chosen = menu.exec(_tree->viewport()->mapToGlobal(pos));
    if (!chosen)
        return;

    // Special (non-TreeAction) item: create the deleted copy-ctor + operator=.
    if (chosen == actDeletedCopyOps)
    {
        addDeletedCopyOps(static_cast<Class*>(pGti));
        rebuild();
        updateToolBarEnables();
        return;
    }

    const TreeAction a = static_cast<TreeAction>(chosen->data().toInt());
    switch (a)
    {
    case TreeAction::Open:
        if (openDiagramViewIfDiagram(pGti))
            return;   // opened the Qt diagram view -- no model mutation
        treeActionInvoke(pGti, a, false);
        _pDoc->MarkLastUndo();
        break;

    // Dialog actions -- no model mutation, so no undo step (mirrors the MFC
    // handlers, which don't MarkLastUndo for these).
    case TreeAction::UserSections:
        Qt_ShowUserSectionsDialog(dynamic_cast<Class*>(pGti), _ownerHwnd);
        break;
    case TreeAction::UserSection1H:
    case TreeAction::UserSection2H:
    case TreeAction::UserSection3H:
    case TreeAction::UserSection1Cpp:
    case TreeAction::UserSection2Cpp:
    case TreeAction::UserSection3Cpp:
    {
        int section = 0; bool header = true;
        userCodeArgs(a, section, header);
        Qt_ShowUserCodeDialog(dynamic_cast<Class*>(pGti), section, header, _ownerHwnd);
        break;
    }
    case TreeAction::BaseClasses:
        Qt_ShowInheritsFromDialog(dynamic_cast<ExternClass*>(pGti), _ownerHwnd);
        break;
    case TreeAction::DerivedClasses:
        Qt_ShowInheritedByDialog(dynamic_cast<BaseClass*>(pGti), _ownerHwnd);
        break;

    case TreeAction::PhaseAnalysis:
    case TreeAction::PhaseDesign:
    case TreeAction::PhaseImplementation:
    case TreeAction::PhaseTest:
    case TreeAction::PhaseComplete:
        pGti->SetPhaseDownAndUpwards(treeActionPhase(a));
        _pDoc->MarkLastUndo();
        break;

    case TreeAction::Copy:
        // Just stashes the node in Gti's shared copy buffer -- no model mutation,
        // so no undo step (mirrors the MFC OnEditCopy).
        treeActionInvoke(pGti, a, false);
        break;

    case TreeAction::NewSubWindow:
    {
        // A scoped peer mirror -- its own TreeViewModel registers with the doc, so it
        // live-refreshes and closes with the document like any other mirror. Hosted as
        // a DOCKABLE shell dock (the same path as "New Window" / the diagram windows)
        // so it can dock / tab / split with the others; falls back to a standalone
        // modeless window only when there is no shell (headless).
        MainTreeQtView* w = new MainTreeQtView(_pDoc, _ownerHwnd, pGti);
        if (!Qt_HostDiagramDock(w))
        {
            w->setAttribute(Qt::WA_DeleteOnClose, true);
            w->setFloating(true);   // standalone top-level peer -- no main toolbar nearby
            Qt_ShowModeless(*w, _ownerHwnd);
        }
        break;
    }

    case TreeAction::DeleteMultiple:
    {
        // Scoped bulk-delete dialog; on OK it deletes as one undo step, so a
        // single rebuild + enable-refresh afterwards.
        CbMultiDeleteDialog dlg(pGti, this);
        if (dlg.exec() == QDialog::Accepted)
        {
            rebuild();
            updateToolBarEnables();
        }
        break;
    }

    default:   // OnXxx model methods (Edit Attributes, Exception Spec, Context, Add*, Paste)
        treeActionInvoke(pGti, a, false);
        _pDoc->MarkLastUndo();
        break;
    }
}

void MainTreeQtView::onDeleteKey()
{
    QTreeWidgetItem* item = _tree->currentItem();
    if (!item)
        return;

    Gti* pGti = reinterpret_cast<Gti*>(itemGti(item));
    if (pGti && pGti->OnDelete(true))   // enable gate (OnUpdateEditDelete)
    {
        pGti->OnDelete(false);
        _pDoc->MarkLastUndo();
    }
}

void MainTreeQtView::onCopyKey()
{
    QTreeWidgetItem* item = _tree->currentItem();
    if (!item)
        return;

    Gti* pGti = reinterpret_cast<Gti*>(itemGti(item));
    if (pGti && pGti->OnCopy(true))     // copyable? (OnUpdateEditCopy)
        pGti->OnCopy(false);            // stash in Gti::_pGtiCopy -- no undo step
}

void MainTreeQtView::onPasteKey()
{
    QTreeWidgetItem* item = _tree->currentItem();
    if (!item)
        return;

    Gti* pTarget = reinterpret_cast<Gti*>(itemGti(item));
    Gti* pSource = Gti::GetGtiCopy();
    if (pTarget && pSource && pTarget->OnPaste(pSource, true))   // OnUpdateEditPaste
    {
        pTarget->OnPaste(pSource, false);
        _pDoc->MarkLastUndo();
    }
}

// FindStringFiltered from `pStart`, then select + scroll to the hit (if it's a
// visible row). Mirrors CClassBuilderView's tree-select-on-find.
void MainTreeQtView::findSelectFrom(Gti* pStart)
{
    if (!pStart || _searchParams.text.empty())
        return;
    Gti* pGti = pStart->FindStringFiltered(
        CbString(_searchParams.text.c_str()),
        _searchParams.matchCase, _searchParams.wholeName,
        _searchParams.searchTypes, _searchParams.searchMethods,
        _searchParams.searchArguments, _searchParams.searchMembers);
    if (!pGti)
        return;
    QTreeWidgetItem* hit = _itemByGti.value(reinterpret_cast<quintptr>(pGti), nullptr);
    if (hit)
    {
        _tree->setCurrentItem(hit);   // selects + echoes to MFC via onCurrentItemChanged
        _tree->scrollToItem(hit);
    }
}

void MainTreeQtView::onFindKey()
{
    QTreeWidgetItem* item = _tree->currentItem();
    Gti* pStart = item ? reinterpret_cast<Gti*>(itemGti(item)) : _pDoc->GetDataModel();
    if (pStart && Qt_ShowSearchDialog(_searchParams, _ownerHwnd))
        findSelectFrom(pStart);
}

void MainTreeQtView::onRepeatKey()
{
    QTreeWidgetItem* item = _tree->currentItem();
    Gti* pStart = item ? reinterpret_cast<Gti*>(itemGti(item)) : _pDoc->GetDataModel();
    findSelectFrom(pStart);
}

// ---------------------------------------------------------------------------
// M3 drag-drop (manual, mirrors the MFC OnBegindrag/OnMouseMove/OnLButtonUp)
// ---------------------------------------------------------------------------
bool MainTreeQtView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj != _tree->viewport())
        return QDialog::eventFilter(obj, event);

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton)
        {
            QTreeWidgetItem* item = _tree->itemAt(me->pos());
            _dragArmed    = item && itemGti(item);
            _dragActive   = false;
            _dragPressPos = me->pos();
        }
        break;   // let the tree select the pressed row
    }
    case QEvent::MouseMove:
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (_dragActive)
        {
            updateDragTarget(me->pos());
            return true;
        }
        if (_dragArmed && (me->buttons() & Qt::LeftButton) &&
            (me->pos() - _dragPressPos).manhattanLength() >= QApplication::startDragDistance())
        {
            if (beginDrag())
                return true;
            _dragArmed = false;   // node refused the drag (Gti::Drag == false)
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton && _dragActive)
        {
            finishDrag();
            return true;
        }
        _dragArmed = false;
        break;
    }
    default:
        break;
    }
    return QDialog::eventFilter(obj, event);
}

bool MainTreeQtView::beginDrag()
{
    QTreeWidgetItem* item = _tree->itemAt(_dragPressPos);
    if (!item)
        return false;
    Gti* pGti = reinterpret_cast<Gti*>(itemGti(item));
    if (!pGti)
        return false;

    _dragCtrl = (QApplication::keyboardModifiers() & Qt::ControlModifier) != 0;

    // Build the icon cursor BEFORE Drag(): Drag() mutates the model and may
    // trigger a rebuild that deletes `item`.
    const QCursor cur = makeDragCursor(pGti, _dragCtrl);

    _pDoc->MarkLastUndo();          // open the drag transaction (mirrors OnBegindrag)
    _dragDefault = nullptr;
    if (!pGti->Drag(_dragCtrl ? true : false, _dragDefault))
        return false;               // not draggable

    _dragGti     = pGti;
    _dragTarget  = _dragDefault;
    _dragActive  = true;
    _tree->viewport()->setCursor(cur);

    // Take over selection for the drag: clear the tree's own selection so the
    // ONLY highlight is the drop target (updateDragTarget). A move also hides the
    // dragged row -- the model removed it -- showing it "lifted out"; the drop's
    // rebuild (via OnUpdate) restores the final state.
    _applyingExternalSelection = true;
    if (!_dragCtrl)
    {
        QTreeWidgetItem* dragItem =
            _itemByGti.value(reinterpret_cast<quintptr>(pGti), nullptr);
        if (dragItem)
            dragItem->setHidden(true);
    }
    _tree->setCurrentItem(nullptr);
    _tree->clearSelection();
    _applyingExternalSelection = false;
    return true;
}

static DiagramDropTarget* diagramDropTargetAt(QPoint globalPos);   // defined below

void MainTreeQtView::updateDragTarget(const QPoint& viewportPos)
{
    QTreeWidgetItem* item = _tree->itemAt(viewportPos);
    Gti* candidate = item ? reinterpret_cast<Gti*>(itemGti(item)) : nullptr;

    const bool valid = candidate && candidate != _dragGti &&
                       _dragGti->DropTarget(_dragCtrl ? true : false, candidate);
    _dragTarget = valid ? candidate : _dragDefault;   // invalid -> origin (drop = no-op)

    // Green full-row drop-target highlight; none when the hovered row can't
    // receive the drop.
    _tree->setDropHighlight(valid ? item : nullptr);

    // Cross-view: Ctrl+drag over a Qt ClassDiagram canvas shows a footprint ghost
    // at the cursor (only when the class can land there). Track the hovered canvas
    // so we can clear the ghost when the cursor moves to another / off the canvas.
    const QPoint global = _tree->viewport()->mapToGlobal(viewportPos);
    DiagramDropTarget* canvas = _dragCtrl ? diagramDropTargetAt(global) : nullptr;
    if (canvas != _ghostCanvas)
    {
        if (_ghostCanvas)
            _ghostCanvas->clearDropGhost();
        _ghostCanvas = canvas;
    }
    if (_ghostCanvas)
        _ghostCanvas->showDropGhost(_dragGti, global);
}

// The Qt diagram canvas (CD or SD) under a global point, if any -- found by
// dynamic_cast'ing the widget under the cursor up its parent chain to the
// DiagramDropTarget interface (the diagram is a different top-level window).
static DiagramDropTarget* diagramDropTargetAt(QPoint globalPos)
{
    for (QWidget* w = QApplication::widgetAt(globalPos); w; w = w->parentWidget())
        if (auto* t = dynamic_cast<DiagramDropTarget*>(w))
            return t;
    return nullptr;
}

// Ctrl-drag of a class/actor released over an open Qt diagram canvas: add it
// there. Returns true if handled -- the caller then skips the in-tree drop.
static bool tryDropOnDiagram(Gti* pGti, QPoint globalPos)
{
    DiagramDropTarget* t = diagramDropTargetAt(globalPos);
    return t && t->dropFromTree(pGti, globalPos);
}

void MainTreeQtView::finishDrag()
{
    Gti* pGti     = _dragGti;
    Gti* pTarget  = _dragTarget;
    Gti* pDefault = _dragDefault;
    bool ctrl     = _dragCtrl;

    _dragActive = false;
    _dragArmed  = false;
    _dragGti = _dragTarget = _dragDefault = nullptr;
    _tree->setDropHighlight(nullptr);   // about to be reconciled by the rebuild
    _tree->viewport()->unsetCursor();
    if (_ghostCanvas)                   // clear the drop ghost before the real drop
    {
        _ghostCanvas->clearDropGhost();
        _ghostCanvas = nullptr;
    }

    // Ctrl-drag onto a diagram adds the class there (the user-chosen gesture; a
    // plain move silently becoming a copy would be confusing). Under Ctrl, Drag()
    // didn't mutate the model, so there's nothing to roll back -- the canvas opens
    // and closes its own undo step. Falls through to the in-tree drop otherwise.
    if (ctrl && tryDropOnDiagram(pGti, QCursor::pos()))
        return;

    // Drop / RollBack + close the transaction, MFC-side (Lock/UnLock). The model
    // refresh flows back through CClassBuilderView::OnUpdate.
    Qt_MainTreeDrop(_pDoc, pGti, pTarget, ctrl, pDefault);
}

QCursor MainTreeQtView::makeDragCursor(Gti* pGti, bool copy) const
{
    // The dragged row's type icon as the cursor (+ a green "+" badge for copy),
    // mirroring the MFC tree's icon-as-cursor drag.
    const int   sz  = _baseIconSize.height();
    const qreal dpr = _tree->devicePixelRatioF();
    QPixmap pm(qRound(sz * dpr), qRound(sz * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.drawPixmap(QPoint(0, 0), Qt_ModelIcon(pGti->GetIcon()).pixmap(QSize(sz, sz), dpr));
    if (copy)
    {
        const qreal b = sz / 2.0;
        const QRectF badge(sz - b, sz - b, b, b);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0x1E, 0x9E, 0x1E));
        p.drawEllipse(badge);
        QPen pen(Qt::white);
        pen.setWidthF(qMax(1.0, b / 6.0));
        p.setPen(pen);
        const qreal cx = badge.center().x(), cy = badge.center().y(), arm = b * 0.28;
        p.drawLine(QPointF(cx - arm, cy), QPointF(cx + arm, cy));
        p.drawLine(QPointF(cx, cy - arm), QPointF(cx, cy + arm));
    }
    p.end();

    return QCursor(pm, 0, 0);   // hotspot top-left
}

// ---------------------------------------------------------------------------
// MFC entry points
// ---------------------------------------------------------------------------
void Qt_ShowMainTreeView(DataModelDoc* pDataModelDoc, void* ownerHwnd)
{
    Qt_EnsureApplication();

    MainTreeQtView* w = new MainTreeQtView(pDataModelDoc, ownerHwnd);
    w->setAttribute(Qt::WA_DeleteOnClose, true);
    Qt_ShowModeless(*w, ownerHwnd);
}

void Qt_EnsureMainTreeView(DataModelDoc* pDataModelDoc, void* ownerHwnd)
{
    if (!pDataModelDoc)
        return;

    // Already showing a Qt tree for this doc? Open MainTreeQtView windows
    // register a TreeViewModel whose RefreshCtx is the window; the MFC view's
    // VM registers a null ctx, so a non-null ctx means a Qt mirror is up.
    DataModelDoc::TreeViewModelIterator iVM(pDataModelDoc);
    while (++iVM)
        if (iVM->GetRefreshCtx())
            return;

    Qt_ShowMainTreeView(pDataModelDoc, ownerHwnd);
}

