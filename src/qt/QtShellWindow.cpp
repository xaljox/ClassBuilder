// qt/QtShellWindow.cpp -- the Qt application shell. See QtShellWindow.h.

#include "QtShellWindow.h"

#include "QtShell.h"
#include "QtApp.h"            // Qt_EnsureApplication
#include "QtModelText.h"      // toQ
#include "MainTreeQtView.h"
#include "QtDiagramZoom.h"
#include "QtAbout.h"          // Qt_ShowAboutDialog
#include "QtCommandServer.h"  // QTcpServer-based command transport
#include "QtToolBarIcons.h"   // CB_TOOLBAR_ICON_PX (shared toolbar icon size)

#include <QActionGroup>
#include <QApplication>
#include <QChildEvent>
#include <QCloseEvent>
#include <QDockWidget>
#include <QMetaObject>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QCursor>
#include <QElapsedTimer>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QProcess>
#include <QProxyStyle>
#include <QPushButton>
#include <QSettings>
#include <QStyleFactory>
#include <QStyleOption>
#include <QStatusBar>
#include <QTabBar>
#include <QToolBar>
#include <QWindow>

#include <functional>
#include <string>

// windows.h before the model headers (LONG_PTR / LONG / DWORD still used by model headers), matching the
// other Qt views. Also WM_CB_COMMAND / MSG for the pipe marshal. Guarded --
// CMake also defines both globally, so an unguarded #define warns C4005.
// macOS/Linux have no windows.h -- CbWinTypes.h supplies the same vocabulary.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include "CbWinTypes.h"
#endif

#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

#include "ClassBuilderDoc.h"
#include "CbShellHooks.h"
#include "CbPlatformCompat.h"      // Cb_SetMainHwnd
#include "CbCommandServer.h"
#include "MainFrm.h"          // the MFC-free facade: s_loadInProgress + status hook

namespace {

QtShellWindow* g_shell = nullptr;   // the one shell window (set in ctor)

// CMainFrame::s_statusTextFn target -- forwards the model-side status-bar
// coordinate calls onto the shell's QLabel panes.
void statusTextThunk(int pane, const char* text);

// The dock tab hosting one model's tree. Clicking its close button must run
// the model's save-prompt-and-close flow instead of just hiding the dock.
class ModelDockWidget : public QDockWidget
{
public:
    using QDockWidget::QDockWidget;
    std::function<void()> onCloseRequested;   // runs closeEntry (deletes us)
protected:
    void closeEvent(QCloseEvent* e) override
    {
        // Never let Qt hide-on-close: closeEntry tears the dock down (or the
        // user cancelled and it stays). Either way the event itself is moot.
        e->ignore();
        if (onCloseRequested)
            onCloseRequested();
    }
};

} // namespace

// ---------------------------------------------------------------------------
// Shell hooks (the CbShellHooks.h seam used by the pipe server)
// ---------------------------------------------------------------------------

namespace {
CClassBuilderDoc* g_activeDoc = nullptr;
}

CClassBuilderDoc* Cb_ActiveDoc()                      { return g_activeDoc; }
void              Cb_SetActiveDoc(CClassBuilderDoc* p){ g_activeDoc = p; }

CClassBuilderDoc* Cb_ShellNewDocument()
{
    return g_shell ? g_shell->newDocument() : nullptr;
}

CClassBuilderDoc* Cb_ShellOpenDocument(const char* path)
{
    return g_shell ? g_shell->openDocument(QString::fromLocal8Bit(path)) : nullptr;
}

// Open-document enumeration + headless close (pipe document-selection seam).
int               Cb_DocumentCount()                          { return g_shell ? g_shell->documentCount() : 0; }
CClassBuilderDoc* Cb_DocumentAt(int index)                    { return g_shell ? g_shell->documentAt(index) : nullptr; }
bool              Cb_IsDocumentOpen(CClassBuilderDoc* pDoc)   { return g_shell && g_shell->isDocumentOpen(pDoc); }
bool              Cb_CloseDocument(CClassBuilderDoc* pDoc, bool save)
{
    return g_shell && g_shell->closeDocumentHeadless(pDoc, save);
}

// Host a diagram window in a dockable QDockWidget on the shell -- floating by
// default, dockable/tabbable with the model trees (drag), floatable back. The
// dock owns `view` and is WA_DeleteOnClose, so closing it (X) deletes the view,
// which tears down its ViewModel (the model then knows the diagram is closed) --
// the same net effect as closing the old standalone window. See QtApp.h.
bool Qt_HostDiagramDock(QWidget* view)
{
    if (!g_shell || !view)
        return false;
    g_shell->hostDiagramDock(view);
    return true;
}

// ---------------------------------------------------------------------------
// Dock separators
// ---------------------------------------------------------------------------

// True when `r` is the immovable separator between a dock area and the
// zero-size central placeholder (the phantom "split bar" at the window edge).
// A REAL split separator has dock content on BOTH sides; the phantom has it
// on one side only (the other side is the central placeholder -- whose own
// geometry is useless as a reference: QMainWindowLayout parks the zero-size
// widget off-screen at (-500,-500)).
static bool isPhantomSeparatorRect(const QMainWindow* mw, const QRect& r)
{
    bool before = false, after = false;            // along the thickness axis
    const bool vertical = r.height() >= r.width(); // vertical bar: docks left/right
    const QList<QDockWidget*> docks = mw->findChildren<QDockWidget*>();
    for (const QDockWidget* d : docks)
    {
        if (!d->isVisible() || d->isFloating())
            continue;
        const QRect g = d->geometry();
        if (vertical)
        {
            if (g.top() > r.bottom() || g.bottom() < r.top())
                continue;                                       // no overlap
            if (qAbs(g.right() - r.left()) <= 2)  before = true;
            if (qAbs(g.left() - r.right()) <= 2)  after  = true;
        }
        else
        {
            if (g.left() > r.right() || g.right() < r.left())
                continue;
            if (qAbs(g.bottom() - r.top()) <= 2)  before = true;
            if (qAbs(g.top() - r.bottom()) <= 2)  after  = true;
        }
    }
    return !(before && after);
}

// True when `dock` currently shares a tab strip with a sibling (docked
// tab group OR a floating QDockWidgetGroupWindow's own tab bar) -- i.e. its
// name is ALREADY shown as a tab label, so its own title text would be a
// redundant duplicate. Mirrors the tabData()-holds-the-QDockWidget* Qt
// convention wireDockTabBars() already relies on. The title BAR itself is
// never hidden (see refreshDockTitleBarPolicy -- hiding it via
// setTitleBarWidget crashed, JV-confirmed 2026-06-19); this only decides
// whether to paint the text INSIDE it.
static bool isDockTabbed(const QDockWidget* dock)
{
    if (!dock || !dock->parentWidget())
        return false;
    const QList<QTabBar*> bars = dock->parentWidget()->findChildren<QTabBar*>();
    for (QTabBar* bar : bars)
    {
        if (!bar->isVisible())
            continue;
        for (int i = 0; i < bar->count(); ++i)
        {
            if (reinterpret_cast<QDockWidget*>(bar->tabData(i).toULongLong()) == dock)
                return true;
        }
    }
    return false;
}

// Paints the QMainWindow dock separators: real ones in an explicit visible
// colour (the platform default is a near-invisible hairline), the one being
// DRAGGED in the app selection accent (consistent with the diagrams'
// selection colour), and the phantom area/central boundary not at all. Set
// per-widget on the shell only, so no other widget's painting is affected.
class ShellSeparatorStyle : public QProxyStyle
{
public:
    ShellSeparatorStyle(QtShellWindow* shell, QStyle* base)
        : QProxyStyle(base)   // takes ownership of base
        , _shell(shell)
    {
    }

    void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt,
                       QPainter* p, const QWidget* w) const override
    {
        if (pe == PE_IndicatorDockWidgetResizeHandle && w == _shell)
        {
            if (!isPhantomSeparatorRect(_shell, opt->rect))
            {
                // During a separator drag the moving separator is the hovered
                // one (State_MouseOver follows the layout's hover position).
                // Accent from the APPLICATION palette -- a styled widget's
                // own palette() resolves QSS defaults, not the theme accent.
                const bool dragged = _shell->separatorDragging()
 && (opt->state & QStyle::State_MouseOver);
                p->fillRect(opt->rect,
                            dragged ? QApplication::palette().color(
                                          QPalette::Active, QPalette::Highlight)
                                    : QColor(0x8a, 0x8a, 0x8a));
            }
            return;
        }
        QProxyStyle::drawPrimitive(pe, opt, p, w);
    }

    int pixelMetric(PixelMetric m, const QStyleOption* opt,
                    const QWidget* w) const override
    {
        if (m == PM_DockWidgetSeparatorExtent)
            return 4;   // the old QSS separator width
        return QProxyStyle::pixelMetric(m, opt, w);
    }

    void drawControl(ControlElement el, const QStyleOption* opt,
                      QPainter* p, const QWidget* w) const override
    {
        // A STANDALONE (untabbed) dock's native title renders BOLD at the
        // style's own title font -- heavier/bigger than the SAME name shown as
        // a tab label the moment that dock gets tabbed with another. Keep the
        // native chrome (background, float/close buttons) but paint the text
        // ourselves at the app's plain tab-matching font, so a dock's title
        // reads the same whether it's standalone or in a tab.
        if (el == CE_DockWidgetTitle)
        {
            if (const auto* dwOpt = qstyleoption_cast<const QStyleOptionDockWidget*>(opt))
            {
                const bool tabbed = isDockTabbed(qobject_cast<const QDockWidget*>(w));
#ifdef _WIN32
                // Seamless notebook merge (JV 2026-07-04): the SELECTED tab
                // above this bar is painted in QPalette::Window with no
                // bottom edge (wireDockTabBars QSS); when this dock is
                // TABBED, paint its title bar as that same flat colour --
                // no native panel, so no edge line between tab and bar: the
                // tab flows seamlessly into the pane it belongs to. The
                // float/close buttons are child widgets and paint themselves
                // on top. Windows-only: the mac/linux native styles already
                // merge tab and title bar (and Linux verified the native
                // path below).
                if (tabbed)
                {
                    p->fillRect(dwOpt->rect, QApplication::palette().color(
                                    QPalette::Active, QPalette::Window));
                    return;
                }
#endif
                QStyleOptionDockWidget blank = *dwOpt;
                blank.title.clear();
                QProxyStyle::drawControl(el, &blank, p, w);

                // Tabbed: the tab label already shows this name right above --
                // leave the bar textless (still there, still draggable/closable,
                // just not a duplicate caption).
                if (!tabbed && !dwOpt->title.isEmpty())
                {
                    QFont f = QApplication::font();
                    f.setBold(false);
                    const int pad = 8;
                    const QRect textRect = dwOpt->rect.adjusted(pad, 0, -pad, 0);
                    p->save();
                    p->setFont(f);
                    p->setPen(dwOpt->palette.color(QPalette::WindowText));
                    p->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                        QFontMetrics(f).elidedText(dwOpt->title, Qt::ElideRight, textRect.width()));
                    p->restore();
                }
                return;
            }
        }
        QProxyStyle::drawControl(el, opt, p, w);
    }

    int styleHint(StyleHint h, const QStyleOption* opt, const QWidget* w,
                  QStyleHintReturn* ret) const override
    {
#ifdef __APPLE__
        // macOS' style centres tab-bar tabs (SH_TabBar_Alignment == AlignCenter);
        // Windows left-aligns. Force left so the model/dock tabs pack from the
        // left edge as on Windows. macOS-only; other platforms are unaffected.
        if (h == SH_TabBar_Alignment)
            return Qt::AlignLeft;
#endif
        return QProxyStyle::styleHint(h, opt, w, ret);
    }

private:
    QtShellWindow* _shell;
};

// ---------------------------------------------------------------------------
// QtShellWindow
// ---------------------------------------------------------------------------

// Bridge for the QtApp.cpp GUI-crash handler: rescue every open model when Qt
// faults. Guards on g_shell so it stays safe even after the shell is gone.
static void cbEmergencySaveAllDocs()
{
    if (g_shell)
        g_shell->emergencySaveAll();
}

QtShellWindow::QtShellWindow()
{
    g_shell = this;
    Cb_SetEmergencySaveHandler(&cbEmergencySaveAllDocs);

    setWindowTitle("ClassBuilder");
    resize(480, 720);

    // Model trees + diagrams live in dock tabs (native tabs, no per-tab
    // cross); every dock keeps its default title bar at all times (close +
    // float buttons -- refreshDockChrome never hides or swaps it), which is
    // also what closes a tabbed pane.
    // Qt 6.11.1: forming a FLOATING dock tab-GROUP (GroupedDragging) crashes
    // intermittently -- a use-after-free in QMainWindowLayout::animationFinished ->
    // QWidget::setParent() on a freed dock. It surfaces FAR more readily on
    // macOS/cocoa (native NSView reparent) but is confirmed on Linux too (xcb, not
    // cocoa) -- so it is a platform-independent timing/UAF. No safe subset (same- or
    // cross-model, first op or Nth) and no upstream fix yet. Disable GroupedDragging
    // on ALL platforms as a stopgap until the root cause is patched (or Qt >= 6.11.2
    // clears it): no floating tab-group can form, so the crash is unreachable.
    // Single-dock floating (drag clear of the main), tabs, and side-by-side splits
    // all still work; only combining several floats into one tabbed group is gone.
    // See crossplatform/KNOWN_ISSUES.md.
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks
                 | QMainWindow::AllowNestedDocks);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    // Make the split separator clearly visible: the default is a hairline that's
    // lost where two side-by-side docks meet, so a left/right split reads as one
    // joined pane. Painted by ShellSeparatorStyle (was QSS QMainWindow::separator,
    // but a stylesheet can't address INDIVIDUAL separators): the proxy style
    // paints real separators in the explicit colour and SKIPS the phantom one
    // QMainWindow always reserves between a dock area and the central widget --
    // this shell is dock-only (zero-size central placeholder), so that boundary
    // can never move, yet Qt still painted a dead "split bar" at the window edge.
    auto* sepStyle = new ShellSeparatorStyle(
        this, QStyleFactory::create(QApplication::style()->name()));
    sepStyle->setParent(this);   // setStyle() does not take ownership
    setStyle(sepStyle);

    // Dock-only QMainWindow: a zero-size central placeholder, so the dock
    // area (the model trees) takes the whole client space instead of leaving
    // a dead central region the docks collapse around.
    QWidget* placeholder = new QWidget(this);
    placeholder->setMaximumSize(0, 0);
    setCentralWidget(placeholder);

    // Restore the saved geometry (the MFC frame persisted its placement too).
    // restoreGeometry() returns false (or lands a degenerate rect) when the saved
    // state predates a screen-layout change -- common on multi-monitor -- which left
    // the window opening unusably tiny (0x0 central widget + not-yet-populated docks).
    // Guard it: fall back to a sane default if the restore failed or came up too small.
    QSettings settings("ClassBuilder", "ClassBuilder");
    const QByteArray geo = settings.value("shell/geometry").toByteArray();
    bool restored = !geo.isEmpty() && restoreGeometry(geo);
    if (!restored || width() < 700 || height() < 500)
        resize(1100, 750);

    buildMenus();
    buildToolBar();
    buildStatusBar();
    refreshDocIndicators();

    CMainFrame::s_statusTextFn = statusTextThunk;

    // The open-document source-freshness check the MFC frame ran on app
    // activation (prompts to re-read changed sources into the model).
    connect(qApp, &QApplication::applicationStateChanged,
            this, &QtShellWindow::onAppStateChanged);

    // Clicking into a floating tree (no tab raise involved) activates its
    // model -- the menus, toolbar, and pipe follow the focused tree.
    connect(qApp, &QApplication::focusChanged,
            this, [this](QWidget*, QWidget* now) {
        if (!now)
            return;
        for (DocEntry* entry : _entries)
        {
            if (entry->tree &&
                (now == entry->tree || entry->tree->isAncestorOf(now)))
            {
                setActiveEntry(entry);
                return;
            }
        }
    });
}

QtShellWindow::~QtShellWindow()
{
    CMainFrame::s_statusTextFn = nullptr;
    while (!_entries.isEmpty())
        destroyEntry(_entries.last());
    if (g_shell == this)
        g_shell = nullptr;
}

void QtShellWindow::emergencySaveAll()
{
    // Last-resort save from the GUI-crash handler (QtApp.cpp). Touch ONLY the
    // MODEL (CbArchive .cbz write) -- never Qt's widget layout, which is corrupt
    // past the access violation. Each modified model -> "<path>.recovered.cbz"
    // (or "<title>.recovered.cbz" when untitled), beside the original.
    for (DocEntry* e : _entries)
    {
        if (!e || !e->doc)
            continue;   // snapshot ALL open models (non-destructive, beside originals)
        CbString path = e->doc->GetPathName();
        CbString rec  = (path.IsEmpty() ? e->doc->GetTitle() : path) + ".recovered.cbz";
        e->doc->WriteRecoveryCbz(rec);
    }
}

// macOS: the native NSOpenPanel/NSSavePanel behind QFileDialog's static
// helpers does not appear in this app (the in-app Qt dialogs -- e.g. the File
// New flow -- show fine, but the native file panel shows nothing). Force Qt's
// own file dialog there. Windows/Linux keep the native panel.
#ifdef __APPLE__
static const QFileDialog::Options kCbFileDialogOpts = QFileDialog::DontUseNativeDialog;
#else
static const QFileDialog::Options kCbFileDialogOpts = QFileDialog::Options();
#endif

void QtShellWindow::buildMenus()
{
    // --- File ---------------------------------------------------------------
    QMenu* file = menuBar()->addMenu("&File");
    file->addAction("&New...", QKeySequence::New, this, [this] {
        newDocument();
    });
    file->addAction("&Open...", QKeySequence::Open, this, [this] {
        const QString path = QFileDialog::getOpenFileName(
            this, "Open Model", QString(), "ClassBuilder CBZ Files (*.cbz)",
            nullptr, kCbFileDialogOpts);
        if (!path.isEmpty())
            openDocument(path);
    });
    _actClose = file->addAction("&Close Model", QKeySequence("Ctrl+W"), this, [this] {
        if (_active)
            closeEntry(_active);
    });
    _actSave = file->addAction("&Save", QKeySequence::Save, this, [this] {
        saveDocument();
    });
    _actSaveAs = file->addAction("Save &As...", this, [this] {
        saveDocumentAs();
    });
    file->addSeparator();
    _actSaveSource = file->addAction("Save Sou&rce...", this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->FileSaveSource();
    });
    _actReadSource = file->addAction("Read Sour&ce...", this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->FileReadSource();
    });
    _actDeleteSource = file->addAction("De&lete Source", this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->FileDeleteSource();
    });
    file->addSeparator();
    file->addAction("E&xit", QKeySequence("Alt+F4"), this, [this] {
        close();
    });

    // --- Edit (Undo / Redo; the per-node commands live on the tree) ---------
    QMenu* edit = menuBar()->addMenu("&Edit");
    _actUndo = edit->addAction("&Undo", QKeySequence::Undo, this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->Undo();
    });
    _actRedo = edit->addAction("&Redo", QKeySequence::Redo, this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->Redo();
    });
    connect(edit, &QMenu::aboutToShow, this, [this] {
        CClassBuilderDoc* doc = activeDoc();
        _actUndo->setEnabled(doc && doc->CanUndo());
        _actRedo->setEnabled(doc && doc->CanRedo());
    });

    // --- Project -------------------------------------------------------------
    QMenu* project = menuBar()->addMenu("&Project");
    _actSettings = project->addAction("&Settings...", this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->ProjectSettings();
    });
    _actAddSerialize = project->addAction("Add Seriali&ze...", this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->ProjectAddSerialize();
    });
    _actRefreshIds = project->addAction("&Refresh Object IDs", this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->ProjectRefreshObjectIds();
    });
    connect(project, &QMenu::aboutToShow, this, [this] {
        CClassBuilderDoc* doc = activeDoc();
        _actSettings->setEnabled(doc != nullptr);
        _actAddSerialize->setEnabled(doc && doc->CanAddSerialize());
        _actRefreshIds->setEnabled(doc != nullptr);
    });

    // --- View (zoom routes to the active diagram window) --------------------
    QMenu* view = menuBar()->addMenu("&View");

    // New Window: open another FULL-tree view of the active model in its own window
    // (the MFC "Window -> New Window"; "New Sub Window" -- a node-scoped view -- lives
    // in the tree's context menu). A peer mirror: its own TreeViewModel registers with
    // the doc, so it live-refreshes and closes with the document.
    QAction* actNewWindow = view->addAction("&New Window", [this] {
        if (CClassBuilderDoc* doc = activeDoc())
        {
            // A second FULL-tree view, hosted as a DOCKABLE shell dock (the same path
            // the diagram windows use) so it can dock / tab / split with the others and
            // receive docks -- unlike the standalone "New Sub Window" peer. hostDiagram-
            // Dock tracks it, manages its title-bar chrome, and WA_DeleteOnClose-deletes
            // it on close (the MainTreeQtView dtor unregisters its TreeViewModel).
            auto* tree = new MainTreeQtView(&doc->GetDataModelDoc(), (void*)winId());
            if (_active && _active->dock)
                tree->setWindowTitle(_active->dock->windowTitle());   // model name as the dock title
            hostDiagramDock(tree);
        }
    });
    view->addSeparator();

    _actZoomIn   = view->addAction("Zoom &In",   QKeySequence("Ctrl++"), [] { Qt_DiagramZoom(+1); });
    _actZoomOut  = view->addAction("Zoom &Out",  QKeySequence("Ctrl+-"), [] { Qt_DiagramZoom(-1); });
    _actZoomFull = view->addAction("Zoom &Full", [] { Qt_DiagramZoom(0); });
    connect(view, &QMenu::aboutToShow, this, [this, actNewWindow] {
        actNewWindow->setEnabled(activeDoc() != nullptr);
        const bool on = Qt_DiagramZoomAvailable();
        _actZoomIn->setEnabled(on);
        _actZoomOut->setEnabled(on);
        _actZoomFull->setEnabled(on);
    });

    // UI Scale: whole-app Qt scale factor (QT_SCALE_FACTOR, applied in
    // QtApp.cpp's Qt_EnsureApplication -- BEFORE QApplication exists, so a
    // change here can only take effect on the next launch). Distinct from the
    // Zoom actions above, which scale just the active diagram canvas.
    view->addSeparator();
    QMenu* uiScale = view->addMenu("UI &Scale");
    auto* scaleGroup = new QActionGroup(uiScale);
    scaleGroup->setExclusive(true);
    const double currentScale =
        QSettings("ClassBuilder", "ClassBuilder").value("shell/uiScale", 1.0).toDouble();
    for (double s : { 1.0, 1.25, 1.5, 1.75, 2.0 })
    {
        QAction* act = uiScale->addAction(QString("%1%").arg(qRound(s * 100)));
        act->setCheckable(true);
        act->setChecked(s == currentScale);
        scaleGroup->addAction(act);
        connect(act, &QAction::triggered, this, [this, s] { applyUiScale(s); });
    }

    // --- Help ----------------------------------------------------------------
    QMenu* help = menuBar()->addMenu("&Help");
    help->addAction("&About ClassBuilder...", this, [this] {
        Qt_ShowAboutDialog((void*)winId());
    });

    // File-menu enabled state follows the active doc.
    connect(file, &QMenu::aboutToShow, this, [this] {
        CClassBuilderDoc* doc = activeDoc();
        const bool hasDoc  = doc != nullptr;
        const bool hasPath = hasDoc && !doc->GetPathName().IsEmpty();
        _actClose->setEnabled(hasDoc);
        _actSave->setEnabled(hasDoc && doc->IsModified());
        _actSaveAs->setEnabled(hasDoc);
        _actSaveSource->setEnabled(hasPath);
        _actReadSource->setEnabled(hasPath);
        _actDeleteSource->setEnabled(hasPath);
    });
}

void QtShellWindow::buildToolBar()
{
    // App-level buttons (doc lifecycle, source round-trip). The view-specific
    // buttons live on each view's own toolbar (tree: Add*/Filters/Find + Undo/
    // Redo; diagrams: zoom) -- per-view bars instead of the old MFC single bar
    // with per-view enabling. Undo/Redo deliberately live ONLY on the tree
    // bar: with several models open it must be unambiguous WHICH model an undo
    // hits (a stray Save is recoverable, an undo on the wrong model is not).
    // Theme icons (Qt's built-in fallback theme) for the standard operations;
    // text for the CB-specific ones until dedicated SVGs are drawn.
    QToolBar* tb = addToolBar("Main");
    tb->setObjectName("mainToolBar");
    tb->setMovable(false);
    // Match the view toolbars' icon size (CB_TOOLBAR_ICON_PX): without this the
    // toolbar uses the platform default, which on macOS is ~50% taller than the
    // diagram/tree bars. Windows' default already matched, so 20px is a no-op there.
    tb->setIconSize(QSize(CB_TOOLBAR_ICON_PX, CB_TOOLBAR_ICON_PX));

    // Prefer the native theme glyph; fall back to the legacy Toolbar.bmp tile so
    // the button always has an icon. macOS has no native "save" image, so without
    // the fallback the Save button renders blank/invisible on the icon-only bar.
    // TODO (toolbar SVG pass): when the toolbar goes all-SVG, draw
    // tb_file_{new,open,save}.svg and prefer them over fromTheme so New/Open/Save
    // look the same -- and nicer -- on every platform (macOS's native file icons
    // are plainer than Windows'). The theme+fallback below is the interim.
    _tbNew = tb->addAction(QIcon::fromTheme(QStringLiteral("document-new"), Qt_ToolBarIcon(TG_FILE_NEW)),
                           "New", this, [this] {
        newDocument();
    });
    _tbOpen = tb->addAction(QIcon::fromTheme(QStringLiteral("document-open"), Qt_ToolBarIcon(TG_FILE_OPEN)),
                            "Open", this, [this] {
        const QString path = QFileDialog::getOpenFileName(
            this, "Open Model", QString(), "ClassBuilder CBZ Files (*.cbz)",
            nullptr, kCbFileDialogOpts);
        if (!path.isEmpty())
            openDocument(path);
    });
    _tbSave = tb->addAction(QIcon::fromTheme(QStringLiteral("document-save"), Qt_ToolBarIcon(TG_FILE_SAVE)),
                            "Save", this, [this] {
        saveDocument();
        updateToolBarEnables();
    });
    tb->addSeparator();
    _tbReadSource = tb->addAction("Read Source", this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->FileReadSource();
    });
    _tbWriteSource = tb->addAction("Write Source", this, [this] {
        if (CClassBuilderDoc* doc = activeDoc()) doc->FileSaveSource();
    });

    _tbNew->setToolTip("New model (Ctrl+N)");
    _tbOpen->setToolTip("Open model (Ctrl+O)");
    _tbSave->setToolTip("Save model (Ctrl+S)");
    _tbReadSource->setToolTip("Read source files back into the model");
    _tbWriteSource->setToolTip("Generate source files from the model");

    updateToolBarEnables();
}

void QtShellWindow::updateToolBarEnables()
{
    CClassBuilderDoc* doc = activeDoc();
    const bool hasDoc  = doc != nullptr;
    const bool hasPath = hasDoc && !doc->GetPathName().IsEmpty();
    _tbSave->setEnabled(hasDoc && doc->IsModified());
    _tbReadSource->setEnabled(hasPath);
    _tbWriteSource->setEnabled(hasPath);
}

void QtShellWindow::buildStatusBar()
{
    // Four permanent panes: Width / Height / X / Y (the old MFC status bar's
    // coordinate fields, fed by the CMainFrame facade hook).
    for (int i = 0; i < 4; ++i)
    {
        _statusPane[i] = new QLabel(this);
        _statusPane[i]->setMinimumWidth(70);
        statusBar()->addPermanentWidget(_statusPane[i]);
    }
}

namespace {
void statusTextThunk(int pane, const char* text)
{
    if (g_shell)
        QMetaObject::invokeMethod(g_shell, [pane, t = QString::fromLocal8Bit(text)] {
            g_shell->setStatusPane(pane, t);
        }, Qt::QueuedConnection);
}
} // namespace

void QtShellWindow::setStatusPane(int pane, const QString& text)
{
    if (pane >= 0 && pane < 4 && _statusPane[pane])
        _statusPane[pane]->setText(text);
}

// ---------------------------------------------------------------------------
// Document lifecycle
// ---------------------------------------------------------------------------

CClassBuilderDoc* QtShellWindow::activeDoc() const
{
    return _active ? _active->doc : nullptr;
}

QtShellWindow::DocEntry* QtShellWindow::addDocument(CClassBuilderDoc* doc)
{
    DocEntry* entry = new DocEntry;
    entry->doc  = doc;
    entry->tree = new MainTreeQtView(&doc->GetDataModelDoc(), (void*)winId());

    ModelDockWidget* dock = new ModelDockWidget(QString(), this);
    dock->setWidget(entry->tree);
    dock->onCloseRequested = [this, entry] { closeEntry(entry); };
    entry->dock = dock;

    // Chrome is managed centrally by refreshDockChrome: every dock keeps its
    // default title bar at all times (never hidden/swapped -- see the
    // setTitleBarWidget crash note there); ShellSeparatorStyle blanks the
    // title TEXT while tabbed (the tab is the caption). NO force-tabify any
    // more -- a tree dropped on a left/right edge is ALLOWED to split
    // side-by-side (two views next to each other, to drag model items
    // between them). Deferred so the refresh runs after the layout
    // transition (a synchronous pass mid-signal raced the layout state).
    auto deferChrome = [this] {
        QMetaObject::invokeMethod(this, [this] {
            refreshDockChrome();
            wireDockTabBars();
        }, Qt::QueuedConnection);
    };
    connect(dock, &QDockWidget::topLevelChanged, this, [this, dock, deferChrome](bool floating) {
        if (auto* tree = qobject_cast<MainTreeQtView*>(dock->widget()))
            tree->setFloating(floating);   // refresh the tree bar's Undo/Redo enables
        deferChrome();
    });
    connect(dock, &QDockWidget::dockLocationChanged, this,
            [deferChrome](Qt::DockWidgetArea){ deferChrome(); });

    // TOP area, not LEFT: the layout always reserves one separator strip
    // (PM_DockWidgetSeparatorExtent) between a populated dock area and the
    // zero-size central placeholder, on the side where they meet. For a LEFT
    // dock that is a full-height vertical strip at the window's right edge --
    // very visible (9 physical px at 225% DPI). For a TOP dock it is a
    // horizontal strip directly above the status bar, where it reads as
    // status-bar separation. It also makes startup consistent with the state
    // after a tear-off + redock (which lands the dock top/bottom): same
    // structure, same (near-invisible) strip.
    addDockWidget(Qt::TopDockWidgetArea, dock);
    // Tab onto the newest IN-WINDOW model; tabifying onto a floating dock
    // would add the new model to that floating window instead of the shell.
    for (int i = _entries.size() - 1; i >= 0; --i)
    {
        if (!_entries[i]->dock->isFloating())
        {
            tabifyDockWidget(_entries[i]->dock, dock);
            break;
        }
    }
    dock->setFloating(false);
    dock->show();
    dock->raise();
    wireDockTabBars();
    refreshDockChrome();   // single model -> title-bar handle; tabbed -> hidden

    // Raising a tab activates its model (for floating trees, focusChanged
    // in the ctor covers activation). Move keyboard focus along: if it stayed
    // in the now-hidden tree, the focus tracker would flip activation right
    // back to the hidden model (window title vs raised tab mismatch).
    connect(dock, &QDockWidget::visibilityChanged, this, [this, entry](bool vis) {
        if (vis)
        {
            setActiveEntry(entry);
            if (entry->tree)
                entry->tree->setFocus();
        }
    });

    _entries.append(entry);
    setActiveEntry(entry);
    refreshDocIndicators();
    return entry;
}

void QtShellWindow::wireDockTabBars()
{
    // QMainWindow creates internal QTabBars for tabified docks lazily; give
    // every new one the shared dock-tab treatment (left-packed native tabs +
    // the proxy-style selected marker on Windows; see the loop body).
    //
    // Wire BOTH the main-window dock tab bars (direct children of the shell)
    // AND the ones inside floating dock groups (children of the top-level
    // QDockWidgetGroupWindows). QMainWindow POOLS and REUSES these bars
    // across both hosts, which is why float-group chrome used to be
    // inconsistent: a bar first wired while docked kept its cross + selected
    // style when reused in a float group, while a bar born inside a float
    // group stayed native (no cross on Windows, near-invisible selected tab).
    // (Float-group wiring was tried 2026-06-19 and reverted after a teardown
    // crash; that crash was the Qt 6.11.1 savedState bug family, since fixed
    // by the two local Qt patches -- see crossplatform/qt-patches/.)
    QList<QTabBar*> bars =
        findChildren<QTabBar*>(QString(), Qt::FindDirectChildrenOnly);
    const QList<QWidget*> kids =
        findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget* w : kids)
    {
        if (w->isWindow()
            && qstrcmp(w->metaObject()->className(), "QDockWidgetGroupWindow") == 0)
        {
            bars += w->findChildren<QTabBar*>();
            // A tab bar born directly inside this group window (e.g. created
            // when the drop animation finishes) never hits the shell's
            // childEvent -- watch the group window's own ChildAdded too.
            if (!w->property("cbGroupWatched").toBool())
            {
                w->setProperty("cbGroupWatched", true);
                w->installEventFilter(this);
            }
        }
    }
    for (QTabBar* bar : bars)
    {
        if (bar->property("cbDocTabsWired").toBool())
            continue;
        bar->setProperty("cbDocTabsWired", true);
        bar->setElideMode(Qt::ElideRight);
        // Pack tabs from the LEFT (macOS' native layout centres them).
        bar->setExpanding(false);
        // No per-tab close cross on any platform: a tabbed model/diagram
        // closes via its dock's TITLE-BAR close button, reliably present
        // everywhere since the always-painted title-bar pass (KNOWN_ISSUES
        // #4). macOS/Linux tabs render native: their styles already draw the
        // classic notebook look (selected tab merges with the title bar
        // below it; unselected tabs recessed behind it with a base line).
#ifdef _WIN32
        // Windows' style does NOT merge: it painted white bordered boxes on
        // the grey title-bar strip, with the selected tab barely marked
        // (text a shade dimmer). Rebuild the notebook look the other
        // platforms get natively (JV 2026-07-04): the SELECTED tab takes the
        // title bar's own colour and has no bottom edge, so tab and bar read
        // as one piece; UNSELECTED tabs are a step darker, sit 3px lower
        // ("behind"), and the pane edge line runs under them. Colours from
        // the APP palette -- palette(...) refs inside QSS resolve to QSS
        // defaults, not the theme.
        const QPalette appPal = QApplication::palette();
        const QColor  winCol  = appPal.color(QPalette::Active, QPalette::Window);
        const QString selBg   = winCol.name();               // == title-bar strip
        const QString unselBg = winCol.darker(110).name();   // recessed
        const QString mid     = appPal.color(QPalette::Active, QPalette::Mid).name();
        bar->setDrawBase(false);   // no style-drawn base line under the row
        bar->setStyleSheet(QString(
            "QTabBar::tab { padding: 4px 12px;"
            "  border: 1px solid %1; border-bottom: none;"
            "  border-top-left-radius: 4px; border-top-right-radius: 4px;"
            "  background: %2; margin-top: 3px; }"
            "QTabBar::tab:!selected { border-bottom: 1px solid %1; }"
            "QTabBar::tab:selected { background: %3; margin-top: 0px;"
            "  border-bottom: none; }")
            .arg(mid, unselBg, selBg));
#endif
        // Route the bar through the shell's ShellSeparatorStyle -- a thin
        // proxy over the platform style whose only tab-bar effect is
        // SH_TabBar_Alignment (AlignLeft on macOS), so tabs pack from the
        // left everywhere (a QSS otherwise re-centres them there).
        bar->setStyle(style());
        // No event filter: dragging a tab off the row floats that dock as a
        // single window. GroupedDragging is disabled (see setDockOptions), so
        // floats can't be combined into a tabbed group.
    }
}

void QtShellWindow::scheduleWireDockTabBars()
{
    if (_wireTabBarsPending)
        return;
    _wireTabBarsPending = true;
    QMetaObject::invokeMethod(this, [this] {
        _wireTabBarsPending = false;
        wireDockTabBars();
    }, Qt::QueuedConnection);
}

void QtShellWindow::childEvent(QChildEvent* e)
{
    QMainWindow::childEvent(e);
    // See the header comment: pooled dock tab bars are created lazily as
    // direct children of the shell (sometimes only at drop-animation finish,
    // after every dock-signal-driven chrome pass) -- their ChildAdded here is
    // the reliable wire trigger. Deferred because the child is still
    // mid-construction during the event; coalesced across the ChildAdded
    // bursts of a layout transition. The wire is idempotent (cbDocTabsWired).
    if (e->added())
        scheduleWireDockTabBars();
}

bool QtShellWindow::eventFilter(QObject* obj, QEvent* e)
{
    // Installed on every floating QDockWidgetGroupWindow (wireDockTabBars):
    // a tab bar born directly inside an existing group window never touches
    // the shell's childEvent, so watch the group's own ChildAdded too.
    if (e->type() == QEvent::ChildAdded)
        scheduleWireDockTabBars();
    return QMainWindow::eventFilter(obj, e);
}

void QtShellWindow::hostDiagramDock(QWidget* view)
{
    // The diagram view sets its own size in its ctor (e.g. 900x700); use that.
    // (The QDialog's sizeHint collapses to a thin bar -- the earlier "opens as
    // a bar".) Floor it so a stray tiny size still gives a usable window.
    const QSize wantSize = view->size().expandedTo(QSize(820, 620));

    QDockWidget* dock = new QDockWidget(view->windowTitle(), this);
    dock->setWidget(view);
    dock->setAttribute(Qt::WA_DeleteOnClose, true);   // close -> delete dock+view (tears down the ViewModel)
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    _diagramDocks.append(dock);

    // Keep diagram chrome consistent with the trees as the layout changes:
    // hidden title bar when tabbed (the tab is the handle), default bar when
    // alone or floating (so there's always something to grab -- splits stay
    // usable). Deferred so the title-bar swap runs AFTER the layout transition
    // (a synchronous swap mid-signal can leave a floating dock bald). wireDock-
    // TabBars styles + close-wires the (possibly new) tab to match the trees.
    auto onLayout = [this] {
        QMetaObject::invokeMethod(this, [this] {
            refreshDockChrome();
            wireDockTabBars();
            // refreshDockChrome deleteLater's the swapped-out title-bar widgets.
            // FLUSH them now (same fix as the model-close relayout below): a dock
            // chrome widget left pending-delete lingers in Qt's dock layout state
            // and becomes the garbage widget the GroupedDragging tear-off / exit
            // reparentWidgets dereferences (AV @ 0xFFFF...). Keep the state clean
            // at every layout transition so a later save/restore can't trip on it.
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        }, Qt::QueuedConnection);
    };
    connect(dock, &QDockWidget::topLevelChanged,    this, [onLayout](bool){ onLayout(); });
    connect(dock, &QDockWidget::dockLocationChanged, this, [onLayout](Qt::DockWidgetArea){ onLayout(); });
    // Closing one of two tabbed docks leaves the OTHER alone with no signal of
    // its own -> re-run the chrome so the survivor regains its title bar.
    connect(dock, &QObject::destroyed, this, [onLayout](QObject*){ onLayout(); });

    // Open floating with the default (draggable) title bar at a real size; the
    // user docks/tabs it by dragging, exactly like a tree window.
    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->setFloating(true);

    // Place it explicitly. Qt's default float position derives from the dock's
    // in-shell spot; after a monitor change, or when the content-sized view is
    // taller than the display, that can land the title bar above the screen
    // edge -- an unmovable, unclosable window. Center over the shell, cap the
    // size to the screen's available area (40px slack for the window frame),
    // and clamp so the title bar always stays reachable.
    const QRect avail = screen()->availableGeometry();
    const QSize sz = wantSize.boundedTo(
        QSize(avail.width() - 16, avail.height() - 40));
    dock->resize(sz);
    QRect place(QPoint(), sz);
    place.moveCenter(frameGeometry().center());
    place.moveLeft(qBound(avail.left(), place.left(),
                          avail.left() + avail.width() - sz.width()));
    place.moveTop (qBound(avail.top(),  place.top(),
                          avail.top() + avail.height() - sz.height()));
    dock->move(place.topLeft());

    dock->show();
    dock->raise();
    dock->activateWindow();

    // Open floating with a normal draggable title bar; the user docks/tabs it by
    // dragging. onLayout() runs refreshDockChrome for the tabbed/alone chrome.
    onLayout();
}

void QtShellWindow::refreshDockChrome()
{
    // Every managed dock -- model trees AND diagram windows -- follows the same
    // rule: hidden per-dock title bar when TABBED (the tab is the handle), the
    // default draggable title bar when ALONE in a split or FLOATING (so a lone
    // pane is still movable/closable). This is what lets trees and diagrams be
    // dropped left/right side-by-side: a split pane keeps a grab handle.
    _diagramDocks.removeAll(QPointer<QDockWidget>(nullptr));   // drop closed docks
    QList<QDockWidget*> all;
    for (DocEntry* e : _entries)
        if (e->dock) all.append(e->dock);
    for (const QPointer<QDockWidget>& p : _diagramDocks)
        if (p) all.append(p.data());

    for (QDockWidget* d : all)
    {
        // Never hide the title bar, and never call setTitleBarWidget() AT ALL. We used to
        // hide it for main-shell tabs (swap in an empty placeholder, restore with
        // setTitleBarWidget(nullptr) on tear-off); that swap could not survive a dock<->float
        // reparent -- the torn-off dock came back HEADLESS and a drop onto it AV'd in Qt's
        // reparentWidgets (JV-confirmed 2026-06-19). ANY setTitleBarWidget() call -- even
        // nullptr on an already-default bar -- can trigger that, so we make none; each dock
        // simply keeps its default (closable, draggable) title bar.
        //
        // We also do NOT try to force float-group member close buttons here: setTitleBarWidget
        // VANISHES the bar, and a setFeatures(Closable) toggle either does nothing (this
        // refresh isn't even triggered when an initial split forms) or renders STRAY,
        // non-functional buttons on tabbed groups (JV screenshots 2026-06-19). So float-group
        // member close-button layout AND the group window's own title-bar PRESENCE are left
        // entirely to Qt -- which is inconsistent (esp. the initial 2-window split until a tab
        // appears). True consistency needs the paused deeper work: a QDockWidgetGroupWindow
        // source patch or a custom group title bar. (Tear-crash stays fixed by the patched
        // reparentToMainWindow -- savedState.clear().)
        const QDockWidget::DockWidgetFeatures f = d->features();
        d->setFeatures(f | QDockWidget::DockWidgetFloatable);
    }
}

void QtShellWindow::setActiveEntry(DocEntry* entry)
{
    if (_active == entry)
        return;
    _active = entry;
    Cb_SetActiveDoc(entry ? entry->doc : nullptr);
    refreshDocIndicators();
}

bool QtShellWindow::closeEntry(DocEntry* entry)
{
    if (!entry || !maybeSave(entry))
        return false;
    destroyEntry(entry);
    return true;
}

// --- Open-document enumeration + headless close (pipe document-selection seam) ---

int QtShellWindow::documentCount() const
{
    return _entries.size();
}

CClassBuilderDoc* QtShellWindow::documentAt(int index) const
{
    return (index >= 0 && index < _entries.size()) ? _entries[index]->doc : nullptr;
}

bool QtShellWindow::isDocumentOpen(CClassBuilderDoc* pDoc) const
{
    for (DocEntry* e : _entries)
        if (e->doc == pDoc)
            return true;
    return false;
}

bool QtShellWindow::closeDocumentHeadless(CClassBuilderDoc* pDoc, bool save)
{
    for (DocEntry* e : _entries)
    {
        if (e->doc != pDoc)
            continue;
        // Headless: no save prompt. Save first only when asked AND the doc already
        // has a path (an untitled doc would otherwise pop a Save-As dialog and block
        // the pipe worker's SendMessage). Then tear down exactly like a GUI close.
        if (save && e->doc && !e->doc->GetPathName().IsEmpty())
            e->doc->DoSave();
        destroyEntry(e);
        return true;
    }
    return false;
}

void QtShellWindow::destroyEntry(DocEntry* entry)
{
    _entries.removeOne(entry);
    if (_active == entry)
        _active = nullptr;

    // Tree first (its TreeViewModel unregisters from the doc), then the doc
    // (DeleteContents tears down the diagram ViewModels, which closes any
    // floating Qt diagram windows). The dock dies via deleteLater, so cut its
    // signals first -- a late visibilityChanged must not touch the freed entry.
    if (entry->dock)
    {
        entry->dock->disconnect(this);
        static_cast<ModelDockWidget*>(entry->dock)->onCloseRequested = nullptr;
        entry->dock->setWidget(nullptr);
        entry->dock->deleteLater();
    }
    delete entry->tree;
    if (entry->doc)
    {
        if (Cb_ActiveDoc() == entry->doc)
            Cb_SetActiveDoc(nullptr);
        delete entry->doc;
    }
    delete entry;

    if (!_active && !_entries.isEmpty())
    {
        setActiveEntry(_entries.last());
        _entries.last()->dock->raise();
    }
    refreshDocIndicators();
    // Closing a model can leave a surviving tab-mate alone -> re-run the chrome
    // (deferred: the dock dies via deleteLater, so let the layout settle first).
    QMetaObject::invokeMethod(this, [this] {
        refreshDockChrome();
        wireDockTabBars();
        // The closed model's docks die via deleteLater; FLUSH those deferred
        // deletes so they're really gone first -- otherwise the relayout below
        // still sees them and leaves the orphaned separators (the lingering
        // "split bars") behind. Then force QMainWindow to recompute its dock
        // layout: a 1px resize-nudge mirrors the manual resize that clears them;
        // when maximized (resize is ignored) re-seat the central widget instead.
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        const QSize s = size();
        if (!isMaximized() && !isFullScreen() && s.width() > 2)
        {
            resize(s.width() - 1, s.height());
            resize(s);
        }
        else if (QWidget* central = takeCentralWidget())
        {
            setCentralWidget(central);
        }
    }, Qt::QueuedConnection);
}

CClassBuilderDoc* QtShellWindow::newDocument()
{
    CClassBuilderDoc* doc = new CClassBuilderDoc;
    doc->SetStateChangedHook(&QtShellWindow::docStateChangedThunk, this);
    doc->SetPostFlushHook(&QtShellWindow::docPostFlushThunk, this);
    if (!doc->OnNewDocument())
    {
        delete doc;
        return nullptr;
    }
    return addDocument(doc)->doc;
}

CClassBuilderDoc* QtShellWindow::openDocument(const QString& path)
{
    // Already open? Activate that tab instead of loading a second doc on the
    // same file (two models writing one .cbz would be a corruption hazard).
    const QString wanted = QFileInfo(path).absoluteFilePath();
    for (DocEntry* entry : _entries)
    {
        const QString open =
            QFileInfo(toQ(entry->doc->GetPathName())).absoluteFilePath();
        if (!open.isEmpty() && open.compare(wanted, Qt::CaseInsensitive) == 0)
        {
            entry->dock->raise();
            setActiveEntry(entry);
            return entry->doc;
        }
    }

    CClassBuilderDoc* doc = new CClassBuilderDoc;
    doc->SetStateChangedHook(&QtShellWindow::docStateChangedThunk, this);
    doc->SetPostFlushHook(&QtShellWindow::docPostFlushThunk, this);
    if (!doc->OnOpenDocument(path.toLocal8Bit().constData()))
    {
        delete doc;
        return nullptr;
    }
    DocEntry* entry = addDocument(doc);

    // Source-freshness check at OPEN (the MFC shell ran it in the view's
    // OnInitialUpdate): a freshly started app never re-activates before the
    // user starts working, so the activation-time check alone misses sources
    // edited while the model was closed. Deferred so the window is up first.
    // Funnels through checkSourceFreshness() (debounced) so this and the
    // reactivation check that the File-Open dialog triggers don't double-prompt.
    QMetaObject::invokeMethod(this, [this] { checkSourceFreshness(); },
                              Qt::QueuedConnection);

    return entry->doc;
}

bool QtShellWindow::saveDocument()
{
    CClassBuilderDoc* doc = activeDoc();
    if (!doc)
        return false;
    if (doc->GetPathName().IsEmpty())
        return saveDocumentAs();
    return doc->DoSave();
}

bool QtShellWindow::saveDocumentAs()
{
    CClassBuilderDoc* doc = activeDoc();
    if (!doc)
        return false;
    QString initial = toQ(doc->GetPathName());
    QString path = QFileDialog::getSaveFileName(
        this, "Save Model As", initial, "ClassBuilder CBZ Files (*.cbz)",
        nullptr, kCbFileDialogOpts);
    if (path.isEmpty())
        return false;
    if (!path.endsWith(".cbz", Qt::CaseInsensitive))
        path += ".cbz";

    const bool ok = doc->OnSaveDocument(path.toLocal8Bit().constData()) != 0;
    if (ok)
        doc->SetPathName(path.toLocal8Bit().constData());
    refreshDocIndicators();
    return ok;
}

bool QtShellWindow::maybeSave(DocEntry* entry)
{
    if (!entry->doc || !entry->doc->IsModified())
        return true;

    const QMessageBox::StandardButton ret = QMessageBox::warning(
        this, "ClassBuilder",
        QString("Save changes to %1?").arg(toQ(entry->doc->GetTitle())),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
    {
        // Save THIS model (Save As needs it active for the file dialog).
        setActiveEntry(entry);
        return saveDocument();
    }
    return ret == QMessageBox::Discard;
}

void QtShellWindow::refreshDocIndicators()
{
    // Dock tab titles carry the per-model dirty marker; the window title
    // follows the active model.
    for (DocEntry* entry : _entries)
    {
        QString docTitle = toQ(entry->doc->GetTitle());
        if (docTitle.isEmpty())
            docTitle = "Untitled";
        if (entry->doc->IsModified())
            docTitle += "*";
        entry->dock->setWindowTitle(docTitle);
    }

    QString title = "ClassBuilder";
    if (_active)
    {
        QString docTitle = toQ(_active->doc->GetTitle());
        if (docTitle.isEmpty())
            docTitle = "Untitled";
        title = docTitle + "[*] - ClassBuilder";
        setWindowModified(_active->doc->IsModified() != 0);
    }
    else
    {
        setWindowModified(false);
    }
    setWindowTitle(title);

    updateToolBarEnables();

    // Fan out: every open view re-evaluates its OWN undo/redo toolbar enables
    // (each reads its own doc's CanUndo/CanRedo). A per-diagram action only
    // refreshes the acting view, so this keeps the other views of the same
    // model in sync. invokeMethod by name no-ops on widgets without the slot.
    for (DocEntry* entry : _entries)
        if (entry->tree)
            QMetaObject::invokeMethod(entry->tree, "refreshUndoRedoEnables");
    for (const QPointer<QDockWidget>& d : _diagramDocks)
        if (d && d->widget())
            QMetaObject::invokeMethod(d->widget(), "refreshUndoRedoEnables");
}

void QtShellWindow::docStateChangedThunk(void* ctx)
{
    QtShellWindow* w = static_cast<QtShellWindow*>(ctx);
    w->refreshDocIndicators();   // dirty flags changed -> titles + Save/Undo/Redo
}

void QtShellWindow::docPostFlushThunk(void* ctx, CClassBuilderDoc* pDoc)
{
    QtShellWindow* w = static_cast<QtShellWindow*>(ctx);
    QMetaObject::invokeMethod(w, [w, pDoc] {
        // The doc may have died between queueing and landing -- only flush
        // documents the shell still owns.
        for (DocEntry* entry : w->_entries)
        {
            if (entry->doc == pDoc)
            {
                pDoc->FlushQueuedRefresh();
                return;
            }
        }
    }, Qt::QueuedConnection);
}

void QtShellWindow::onAppStateChanged(Qt::ApplicationState state)
{
    // Mirrors the MFC OnActivateApp CheckUpdates: when the app regains focus,
    // ask each model whether generated sources changed on disk (offer re-read).
    if (state == Qt::ApplicationActive)
        checkSourceFreshness();
}

void QtShellWindow::checkSourceFreshness()
{
    // The single funnel for the source-freshness prompt, hit by BOTH the
    // open-time check (openDocument) and the app-activation check. Opening a
    // model fires both (the File-Open dialog deactivates/reactivates the
    // window), which used to double-prompt. Guards:
    //   - s_loadInProgress: never mid-load.
    //   - checking: re-entry (the read dialog pumps the event loop, which can
    //     redeliver an activation signal while we're still inside).
    //   - timer debounce: collapse the open-time + reactivation pair (which
    //     arrive within a moment of each other) into ONE check; a genuine
    //     later re-activation is well past the window and still prompts.
    if (CMainFrame::s_loadInProgress)
        return;
    static bool checking = false;
    if (checking)
        return;
    static QElapsedTimer sinceLast;   // invalid until first run -> first call proceeds
    if (sinceLast.isValid() && sinceLast.elapsed() < 1000)
        return;

    checking = true;
    const QList<DocEntry*> entries = _entries;   // prompts can close models
    for (DocEntry* entry : entries)
    {
        if (!_entries.contains(entry))
            continue;
        if (entry->doc && entry->doc->GetDataModelDoc().GetDataModel())
            entry->doc->GetDataModelDoc().GetDataModel()->CheckUpdates();
    }
    sinceLast.restart();   // stamp after the (possibly modal) checks finish
    checking = false;
}

void QtShellWindow::closeEvent(QCloseEvent* e)
{
    for (DocEntry* entry : _entries)
    {
        if (!maybeSave(entry))
        {
            e->ignore();
            return;
        }
    }

    // GroupedDragging: a FLOATING dock-group window crashes Qt's teardown on exit --
    // QDockWidgetGroupWindow's close drives endDrag -> QMainWindowLayout::restore ->
    // reparentWidgets, which derefs an already-freed widget (AV reading 0xFFFF...).
    // Re-dock every floating dock NOW, while all widgets are still alive, so no group
    // window is left for the close cascade to tear down. The dock layout isn't
    // persisted (only the shell geometry is), so this is invisible to the user.
    for (QDockWidget* d : findChildren<QDockWidget*>())
        if (d->isFloating())
            d->setFloating(false);

    // Only persist a usable geometry -- never save a transient tiny/minimized state,
    // or it reloads tiny next launch (paired with the restore guard in the ctor).
    if (width() >= 700 && height() >= 500)
    {
        QSettings settings("ClassBuilder", "ClassBuilder");
        settings.setValue("shell/geometry", saveGeometry());
    }

    while (!_entries.isEmpty())                 // also closes the floating
        destroyEntry(_entries.last());          // diagram windows per doc
    e->accept();
}

void QtShellWindow::applyUiScale(double scale)
{
    QSettings settings("ClassBuilder", "ClassBuilder");
    if (scale == settings.value("shell/uiScale", 1.0).toDouble())
        return;   // unchanged -- e.g. re-clicking the already-checked entry
    settings.setValue("shell/uiScale", scale);

    const auto reply = QMessageBox::question(this, "Restart Required",
        QString("The new UI scale (%1%) takes effect after restarting ClassBuilder.\n\n"
                "Restart now?").arg(qRound(scale * 100)),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (reply != QMessageBox::Yes)
        return;

    // Relaunch only once the shell actually finishes closing -- closeEvent's
    // maybeSave() prompts can still cancel the shutdown (unsaved changes), in
    // which case aboutToQuit never fires and nothing relaunches.
    connect(qApp, &QCoreApplication::aboutToQuit, qApp, [] {
        QProcess::startDetached(QCoreApplication::applicationFilePath(), QStringList());
    });
    close();
}

// True when `pos` (shell coords) lies on a dock separator strip with dock
// content on only ONE side -- the immovable phantom separator against the
// zero-size central placeholder. Works edge-agnostically (the phantom sits on
// whichever window edge the layout parked the central boundary: right on
// initial open, bottom after a redock, ...). A candidate strip is
// reconstructed from the dock edge `pos` is adjacent to, then classified by
// the same both-sides test the painting suppression uses
// (isPhantomSeparatorRect), so hit-test and paint can't disagree. The final
// outside-the-docks'-union fallback catches corner pixels of the phantom
// strip that are adjacent to no dock edge -- a REAL separator always lies
// between two docks, hence inside the union.
bool QtShellWindow::phantomSeparatorHitTest(const QPoint& pos) const
{
    const int ext = style()->pixelMetric(
        QStyle::PM_DockWidgetSeparatorExtent, nullptr, this);

    int minL = INT_MAX, maxR = INT_MIN, minT = INT_MAX, maxB = INT_MIN;
    bool any = false;
    const QList<QDockWidget*> docks = findChildren<QDockWidget*>();
    for (const QDockWidget* d : docks)
    {
        if (!d->isVisible() || d->isFloating())
            continue;
        const QRect g = d->geometry();
        any = true;
        minL = qMin(minL, g.left());   maxR = qMax(maxR, g.right());
        minT = qMin(minT, g.top());    maxB = qMax(maxB, g.bottom());

        QRect cand;
        if      (pos.x() > g.right()  && pos.x() <= g.right() + ext
              && pos.y() >= g.top()   && pos.y() <= g.bottom())
            cand = QRect(g.right() + 1, g.top(), ext, g.height());
        else if (pos.x() <  g.left()  && pos.x() >= g.left() - ext
              && pos.y() >= g.top()   && pos.y() <= g.bottom())
            cand = QRect(g.left() - ext, g.top(), ext, g.height());
        else if (pos.y() > g.bottom() && pos.y() <= g.bottom() + ext
              && pos.x() >= g.left()  && pos.x() <= g.right())
            cand = QRect(g.left(), g.bottom() + 1, g.width(), ext);
        else if (pos.y() <  g.top()   && pos.y() >= g.top() - ext
              && pos.x() >= g.left()  && pos.x() <= g.right())
            cand = QRect(g.left(), g.top() - ext, g.width(), ext);
        else
            continue;
        return isPhantomSeparatorRect(this, cand);
    }

    return any && (pos.x() > maxR || pos.x() < minL ||
                   pos.y() > maxB || pos.y() < minT);
}

bool QtShellWindow::event(QEvent* e)
{
    switch (e->type())
    {
    case QEvent::CursorChange:
        // The HoverMove case below un-sets the phantom-separator cursor. That
        // un-set fires a synchronous CursorChange, which QMainWindow answers
        // by RE-SETTING its adjusted split cursor ("ensure our adjusted
        // cursor stays visible" in qmainwindow.cpp) -- silently defeating the
        // un-set. Swallow exactly that one event so the un-set sticks.
        if (_suppressCursorReadjust)
            return true;
        break;

    case QEvent::MouseButtonPress:
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        // A left press on the phantom separator would start a separator drag
        // that can never move anything (the central placeholder is pinned at
        // 0x0) -- the "you think you're resizing and nothing happens" trap.
        // Swallow it before QMainWindow can startSeparatorMove().
        if (me->button() == Qt::LeftButton
            && phantomSeparatorHitTest(me->position().toPoint()))
            return true;

        // Track "a REAL dock separator is being dragged": presses land on
        // the shell itself only over separators / empty dock space, and over
        // a separator the split cursor is showing (the phantom no longer
        // shows one, so it can't trigger this). ShellSeparatorStyle paints
        // the dragged separator in the selection accent (consistent with the
        // diagrams).
        if (me->button() == Qt::LeftButton && testAttribute(Qt::WA_SetCursor))
        {
            const Qt::CursorShape shape = cursor().shape();
            if (shape == Qt::SplitHCursor || shape == Qt::SplitVCursor ||
                shape == Qt::SizeHorCursor || shape == Qt::SizeVerCursor)
            {
                _separatorDragging = true;
                update();   // repaint separators: dragged one -> accent
            }
        }
        break;
    }

    case QEvent::MouseButtonRelease:
        if (_separatorDragging)
        {
            _separatorDragging = false;
            update();       // back to the normal separator colour
        }
        break;

    case QEvent::HoverEnter:
    case QEvent::HoverMove:
    {
        const QPoint pos = static_cast<QHoverEvent*>(e)->position().toPoint();
        const bool handled = QMainWindow::event(e);
        // QMainWindow just flipped a resize cursor if the hover is over ANY
        // dock separator -- including the immovable phantom one against the
        // central placeholder (whose painting ShellSeparatorStyle already
        // suppresses). Drop the cursor there: the strip must read as inert
        // window edge, not as a grabbable split.
        if (testAttribute(Qt::WA_SetCursor) && phantomSeparatorHitTest(pos))
        {
            _suppressCursorReadjust = true;
            unsetCursor();
            _suppressCursorReadjust = false;
        }
        return handled;
    }

    default:
        break;
    }
    return QMainWindow::event(e);
}

// ---------------------------------------------------------------------------
// Entry point (called by the EXE's WinMain via qt/QtShell.h)
// ---------------------------------------------------------------------------

int Cb_RunQtShell(const char* fileToOpen)
{
    Qt_EnsureApplication();

    QtShellWindow w;
    w.show();

    Cb_SetMainHwnd((void*)w.winId());

    // Cross-platform command transport: a QTcpServer on 127.0.0.1 driven by the
    // Qt event loop on this (GUI) thread -- replaces the old Win32 named-pipe
    // worker + SendMessage(WM_CB_COMMAND) hop. Dispatch runs inline on the GUI
    // thread, so command handlers touch the model exactly like a menu action.
    QtCommandServer cmdServer;
    cmdServer.start();   // 127.0.0.1:CB_CMD_PORT (default 51777)

    if (fileToOpen && *fileToOpen)
        w.openDocument(QString::fromLocal8Bit(fileToOpen));

    const int rc = qApp->exec();

    cmdServer.stop();
    Cb_SetMainHwnd(nullptr);
    return rc;
}
