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
class MainTreeQtView;
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

    // Drops the resize cursor QMainWindow flips over the PHANTOM dock
    // separator (the immovable boundary against the zero-size central
    // placeholder; its painting is suppressed by ShellSeparatorStyle).
    bool event(QEvent* e) override;

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

    QList<DocEntry*> _entries;                   // open models, oldest first
    DocEntry*        _active = nullptr;          // menus/toolbar/pipe target
    QList<QPointer<QDockWidget>> _diagramDocks;  // open CD/SD diagram docks
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

    // Left button is down after a press on a dock separator (see
    // separatorDragging() / ShellSeparatorStyle).
    bool _separatorDragging = false;
};
