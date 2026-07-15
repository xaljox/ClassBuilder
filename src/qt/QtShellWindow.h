// qt/QtShellWindow.h -- the Qt application main window (the MFC CMainFrame +
// CWinApp + MDI shell replacement, 2026-06-09; multi-document 2026-06-10).
//
// MULTI-DOC: each open model is a DocEntry -- a CClassBuilderDoc plus its
// MainTreeQtView, hosted in a QDockWidget. The docks are tabified by default
// (a tab per model); dragging a tab out floats that model's tree next to the
// others, which is how cross-model drag/copy is done. Closing a dock closes
// that model (with save prompt). The ACTIVE model (menus, toolbar, pipe
// Cb_ActiveDoc) follows tab raise / focus. Class/sequence diagrams float as
// owned top-level Qt windows per document (as before).
#pragma once

#include <QList>
#include <QMainWindow>
#include <QPointer>

class CClassBuilderDoc;
class DataModelDoc;
class Gti;
class MainTreeQtView;
class QDialog;
class QDockWidget;
class QLabel;

class QtShellWindow : public QMainWindow
{
    Q_OBJECT
public:
    QtShellWindow();
    ~QtShellWindow();

    // Document actions (also reached by the pipe via the CbShellHooks seam).
    // New/Open ADD a model; existing ones stay open. Save* act on the active.
    CClassBuilderDoc* newDocument();             // wizard or pending pipe params
    CClassBuilderDoc* openDocument(const QString& path);
    bool saveDocument();                         // false on cancel/failure
    bool saveDocumentAs();

    // Status-bar coordinate panes (0=width 1=height 2=X 3=Y), fed by the
    // CMainFrame facade hook.
    void setStatusPane(int pane, const QString& text);

    // True while a dock separator is being dragged (left button held after a
    // press on one). ShellSeparatorStyle paints the dragged separator in the
    // app selection accent -- consistent with the diagrams' selection colour.
    bool separatorDragging() const { return _separatorDragging; }

    // Wrap a diagram window (CD/SD view) in a dock: floating by default, but
    // dockable/tabbable like the trees (Qt_HostDiagramDock entry point).
    void hostDiagramDock(QWidget* view);

    // Wrap a modeless code-editor dialog (method/constructor) in a dock
    // (Qt_HostEditorDock entry point). Closing the dock routes through the
    // dialog's own closeEvent, so the save prompt runs and Cancel vetoes.
    // A new editor tabs onto an existing docked editor group (the "all
    // editors tabbed next to the tree" workflow); with none, it re-opens
    // docked at the REMEMBERED editor spot (see below), floating only when
    // no editor has been docked yet this session. `tabTitle` is the SHORT
    // caption for the tab / dock title bar ("Matrix::GetRow" -- the dialog
    // keeps its long descriptive windowTitle for the no-shell fallback).
    void hostEditorDock(QWidget* dlg, const QString& tabTitle);

    // Record the current placement (area + size) of the docked editor group.
    // Called on editor-dock layout changes and just before an editor dock
    // closes: when the LAST editor tab closes and its split disappears, the
    // memory survives, so the next editor opens docked there again instead
    // of floating (public: EditorDockWidget::closeEvent calls it).
    void rememberEditorDockPlacement();

    // Select + reveal a model object in the tree of the document that owns
    // it, raising that tree's dock/tab (Qt_SelectInModelTree entry point --
    // F12 go-to-definition). False when the doc isn't open here or the
    // object has no row.
    bool selectGtiInTree(DataModelDoc* pDoc, Gti* pGti);

    // Open-document enumeration + headless close, for the pipe's document-selection
    // commands (reached via the CbShellHooks Cb_Document* seam).
    int               documentCount() const;
    CClassBuilderDoc* documentAt(int index) const;            // null if out of range
    bool              isDocumentOpen(CClassBuilderDoc* pDoc) const;
    bool              closeDocumentHeadless(CClassBuilderDoc* pDoc, bool save);

    // Save every modified open model to "<path>.recovered.cbz" (or
    // "<title>.recovered.cbz" when untitled). Called ONLY from the last-resort
    // GUI-crash handler -- touches the model (CbArchive), never the corrupt Qt
    // widget layout.
    void emergencySaveAll();

protected:
    void closeEvent(QCloseEvent* e) override;

    // Neutralizes the PHANTOM dock separator (the immovable boundary against
    // the zero-size central placeholder; its painting is suppressed by
    // ShellSeparatorStyle): drops the resize cursor QMainWindow flips over it,
    // swallows the CursorChange re-adjust QMainWindow answers that with, and
    // eats the left-press that would start a drag that can never move
    // anything. Real separators (docks on both sides) are untouched.
    bool event(QEvent* e) override;

    // Re-wires the dock tab bars whenever a new direct child appears.
    // QMainWindowLayout pools its dock QTabBars and creates them lazily as
    // direct children of the main window -- sometimes only when the dock-drop
    // ANIMATION finishes, after the deferChrome pass triggered by the dock
    // signals. A float group's fresh tab bar was born in that gap and stayed
    // native (no close cross, near-invisible selected tab). ChildAdded on the
    // shell is the creation signal for the pooled bars AND for the float-group
    // windows themselves; a bar born directly inside an existing group window
    // is caught by the eventFilter wireDockTabBars installs on each group
    // window. The wire itself is deferred (the object is still constructing
    // during the event) and coalesced.
    void childEvent(QChildEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* e) override;

    // (Tab tear-off is Qt-native: GroupedDragging in dockOptions lets a tab be
    // dragged off the row, floating that dock and continuing the drag with
    // drop zones -- no event filter needed.)
    //
    // (The old nativeEvent() WM_CB_COMMAND hop is gone -- the command server is
    // now a QTcpServer dispatched on the event loop; see qt/QtCommandServer.)

private:
    // One open model: the doc, its tree view, and the dock tab hosting it.
    struct DocEntry
    {
        CClassBuilderDoc* doc  = nullptr;
        MainTreeQtView*   tree = nullptr;
        QDockWidget*      dock = nullptr;
    };

    void buildMenus();
    void buildToolBar();
    void updateToolBarEnables();
    void buildStatusBar();
    void applyUiScale(double scale);             // persist + offer restart-now

    CClassBuilderDoc* activeDoc() const;         // null when no model open
    DocEntry* addDocument(CClassBuilderDoc* doc);// wrap a loaded doc in dock+tree
    void setActiveEntry(DocEntry* entry);
    bool closeEntry(DocEntry* entry);            // save prompt; false = cancelled
    void destroyEntry(DocEntry* entry);          // tree first, then doc
    bool maybeSave(DocEntry* entry);             // prompt when dirty; false = cancel
    void refreshDocIndicators();                 // window/dock titles + toolbar
    void wireDockTabBars();                      // close crosses on dock tabs
    void refreshDockChrome();                    // every managed dock: hide title bar when tabbed, show when alone/floating; float-group members non-floatable
    void onAppStateChanged(Qt::ApplicationState state);
    void checkSourceFreshness();                 // CheckUpdates on all docs, debounced

    static void docStateChangedThunk(void* ctx);
    // Queued half of the doc's refresh flusher (derived-refresh design): the
    // doc is Qt-quarantined, so IT asks US to post FlushQueuedRefresh onto the
    // event loop. Re-validated against _entries when the call lands -- a doc
    // closed with a flush still queued is silently dropped.
    static void docPostFlushThunk(void* ctx, CClassBuilderDoc* pDoc);

    // Shared floating-dock placement: size (clamped to the screen), centred
    // over the shell, title bar kept reachable. Used by diagram + editor docks.
    void placeFloatingDock(QDockWidget* dock, const QSize& wantSize);

    QList<DocEntry*> _entries;                   // open models, oldest first
    DocEntry*        _active = nullptr;          // menus/toolbar/pipe target
    QList<QPointer<QDockWidget>> _diagramDocks;  // open CD/SD diagram docks
    QList<QPointer<QDockWidget>> _editorDocks;   // open method/ctor code editors

    // Last known docked-editor placement (rememberEditorDockPlacement).
    bool               _editorPlaceKnown = false;
    Qt::DockWidgetArea _editorPlaceArea  = Qt::RightDockWidgetArea;
    QSize              _editorPlaceSize;
    QLabel*          _statusPane[4] = {};        // width / height / X / Y

    QAction* _actSave         = nullptr;
    QAction* _actSaveAs       = nullptr;
    QAction* _actClose        = nullptr;
    QAction* _actSaveSource   = nullptr;
    QAction* _actReadSource   = nullptr;
    QAction* _actDeleteSource = nullptr;
    QAction* _actUndo         = nullptr;
    QAction* _actRedo         = nullptr;
    QAction* _actSettings     = nullptr;
    QAction* _actAddSerialize = nullptr;
    QAction* _actRefreshIds   = nullptr;
    QAction* _actZoomIn       = nullptr;
    QAction* _actZoomOut      = nullptr;
    QAction* _actZoomFull     = nullptr;

    // App toolbar twins (enable-managed in updateToolBarEnables).
    QAction* _tbNew         = nullptr;
    QAction* _tbOpen        = nullptr;
    QAction* _tbSave        = nullptr;
    QAction* _tbReadSource  = nullptr;
    QAction* _tbWriteSource = nullptr;

    // True when `pos` (shell coords) lies on a dock separator strip with dock
    // content on only ONE side -- the immovable phantom separator against the
    // zero-size central placeholder. Real separators (docks both sides) and
    // all other positions return false. See event().
    bool phantomSeparatorHitTest(const QPoint& pos) const;

    // Left button is down after a press on a dock separator (see
    // separatorDragging() / ShellSeparatorStyle).
    bool _separatorDragging = false;

    // event() is un-setting the phantom-separator cursor: swallow the
    // CursorChange QMainWindow would answer with (it re-sets its "adjusted"
    // split cursor from that handler, defeating the un-set).
    bool _suppressCursorReadjust = false;

    // Coalesced deferred wireDockTabBars (childEvent / group-window filter).
    void scheduleWireDockTabBars();
    bool _wireTabBarsPending = false;
};
