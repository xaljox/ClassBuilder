// qt/QtApp.h -- shared QApplication bootstrap for the Qt dialogs.
//
// Qt-header-free, so any translation unit (Qt or MFC) may include it.
#pragma once

// Ensures the process's single QApplication exists. Created lazily on first
// call and kept for the process lifetime. Safe to call repeatedly and from
// any thread context an MFC command handler runs in.
void Qt_EnsureApplication();

// Registers a process-wide "emergency save" callback invoked from the
// last-resort GUI-crash handler -- a Windows access violation caught at
// QApplication::notify (the QTBUG-58036 floating-dock reparent bug). On the
// crash a NATIVE dialog offers to run this before ExitProcess, so unsaved
// models can be rescued even though Qt's widget state is corrupt. The handler
// must touch only the MODEL (CbArchive save), never Qt's widget layout. Pass
// nullptr to clear.
void Cb_SetEmergencySaveHandler(void (*fn)());

class QDialog;
class QWidget;

// Runs `dlg` modally, owned by a Win32 window (the MFC HWND, passed as void*
// to keep this header MFC-free). It makes the Qt dialog a Win32-owned popup
// of that window AND disables the owner for the dialog's lifetime -- i.e.
// true modality over a non-Qt window, so the MFC app cannot be activated or
// closed underneath the dialog (a plain QDialog::exec() does not do this and
// leaves the MFC window live). Returns QDialog::exec()'s result code.
int Qt_ExecModal(QDialog& dlg, void* ownerHwnd);

// Shows `w` as a non-modal top-level window owned by a Win32 HWND. The
// caller is expected to set Qt::WA_DeleteOnClose so the widget self-destructs
// on close; nothing is disabled, and ownership is purely Z-order / activation
// (the popup stays above its owner and shares its taskbar entry, but the
// owner remains usable). Returns immediately.
void Qt_ShowModeless(QWidget& w, void* ownerHwnd);

// Hosts `view` (a diagram window) in a dockable QDockWidget on the application
// shell -- FLOATING by default, but the user can dock / tab it with the model
// trees and float it back, same as a tree. The dock is WA_DeleteOnClose and
// owns `view`; its title is view->windowTitle(). Returns false when there is no
// shell (e.g. headless), so the caller can fall back to Qt_ShowModeless.
bool Qt_HostDiagramDock(QWidget* view);

// Hosts a modeless code-editor dialog in a dockable/tabbable shell dock.
// Closing the dock routes through the dialog's closeEvent (save prompt,
// Cancel vetoes); a dialog that closes itself takes the dock with it. New
// editors tab onto an existing docked editor group; the first opens
// floating. `tabTitle` is the short tab caption ("Matrix::GetRow"), local
// 8-bit (this header stays Qt-free). Returns false when there is no shell --
// fall back to Qt_ShowModeless.
bool Qt_HostEditorDock(QWidget* dlg, const char* tabTitle);

// Raise + activate the dock hosting `dlg` (tab-activates a tabbed editor).
// Returns false when `dlg` is not dock-hosted.
bool Qt_RaiseEditorDock(QWidget* dlg);

// Select + reveal a model object in the tree of the document that owns it,
// raising that tree's dock/tab (F12 go-to-definition). Returns false when
// the document isn't open in the shell or the object has no tree row.
class DataModelDoc;
class Gti;
bool Qt_SelectInModelTree(DataModelDoc* pDoc, Gti* pGti);
