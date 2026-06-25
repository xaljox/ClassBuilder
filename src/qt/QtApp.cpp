// qt/QtApp.cpp -- the process's single QApplication.
//
// Qt requires exactly one QApplication, created before any QWidget and
// outliving every widget. The MFC app has none of its own, so we create one
// lazily on the first Qt-dialog invocation and keep it for the rest of the
// process lifetime (a process-lifetime singleton, intentionally never
// deleted -- not a leak to chase). QApplication takes argc by reference and
// retains the argv pointers, so both have static storage duration.

#include "QtApp.h"

#include "CbPainter.h"                 // app-wide measure-painter install
#include "CbPainter_QFontMetrics.h"    // the headless measure backend

#include <QApplication>
#include <QDialog>
#include <QEvent>
#include <QIcon>
#include <QString>
#include <QWidget>

// Static Qt: the platform plugin (qwindows) and the native style are baked
// into the EXE instead of loaded as DLLs at runtime, so they must be
// registered explicitly. CB_STATIC_QT is defined by CMake only when the
// located Qt is a static build; with a shared Qt these plugins load
// themselves and Q_IMPORT_PLUGIN would reference symbols that don't exist.
// This lives in QtApp.cpp (not its own TU) because QtApp.cpp is always
// pulled out of the ClassBuilderQt static lib -- a standalone TU could be
// dropped by the linker, taking the plugin registration with it.
#ifdef CB_STATIC_QT
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QModernWindowsStylePlugin)
// The SVG image-format + icon-engine plugins -- only when this Qt has the Svg
// module (CB_HAVE_SVG); a static Qt does not self-load plugins. Lets
// QtModelIcons render redrawn .svg model icons.
#ifdef CB_HAVE_SVG
Q_IMPORT_PLUGIN(QSvgPlugin)
Q_IMPORT_PLUGIN(QSvgIconPlugin)
#endif
#endif

// windows.h last, after the Qt headers, with the macro guards -- otherwise its
// min/max macros collide with Qt headers. Guarded -- CMake also defines both
// globally, so an unguarded #define warns C4005 (redefinition).
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// The one knob for the Qt dialog UI font size, in points. The platform
// default is ~9pt; this is deliberately larger for readability.
static const int CB_UI_FONT_PT = 11;

// UI font weight: a bit heavier than Normal(400) for crisper text, short of
// Bold(700). 500 == QFont::Medium. Weight is stroke thickness, not glyph
// height, so this does NOT change fontMetrics().height() -> row heights / line
// spacing are unaffected. One knob, applied both ways like the size below.
static const int CB_UI_FONT_WEIGHT = 500;

// --------------------------------------------------------------------------
// Last-resort GUI-crash safety net. Rearranging/tearing tabs in a FLOATING dock
// group can reparent through Qt dock code that dereferences a freed pointer (an
// access violation). The root cause is NOT yet pinned down -- and it is NOT
// QTBUG-58036 (that was a hide-on-redock bug, not a crash, fixed back in Qt
// 5.6.3 / 5.9.0 and long present in our 6.x). Until we find it, we CATCH the AV
// at the event-dispatch boundary (QApplication::notify) with Win32 SEH and,
// instead of dying, give the user a native dialog + a chance to save before a
// clean exit. We do NOT resume -- Qt's widget state is corrupt past the AV, so
// continuing would just fault again.
// --------------------------------------------------------------------------
static void (*g_cbEmergencySave)() = nullptr;

void Cb_SetEmergencySaveHandler(void (*fn)()) { g_cbEmergencySave = fn; }

static int cbGuiCrashFilter(DWORD code)
{
    // Only an access violation (the dock-reparent fault). Everything else --
    // C++ exceptions (0xE06D7363), breakpoints, stack overflow -- propagates.
    return (code == EXCEPTION_ACCESS_VIOLATION)
         ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

// The crash dialog runs on its OWN thread (see cbHandleFatalGuiCrash): pumping a
// MessageBox on the GUI thread re-enters the corrupt Qt layout and faults again.
static DWORD WINAPI cbDialogThread(LPVOID)
{
    ::MessageBoxW(nullptr,
        L"ClassBuilder hit a fatal Qt dock error and must close.\n\n"
        L"Your open models were saved next to their files as\n"
        L"\"<name>.recovered.cbz\" -- reopen those to continue.",
        L"ClassBuilder - work saved",
        MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
    return 0;
}

static void cbHandleFatalGuiCrash()
{
    // Re-entrancy guard: if the save or the dialog itself faults, bail hard.
    static volatile LONG s_inHandler = 0;
    if (InterlockedExchange(&s_inHandler, 1) != 0)
        ::TerminateProcess(::GetCurrentProcess(), 3);

    // SAVE FIRST, before any dialog. A MessageBox pumps messages -> re-enters the
    // corrupt Qt event loop -> a SECOND access violation that would terminate
    // before the save ran. The save is model-only (CbArchive), pumps nothing, and
    // writes BESIDE the originals (non-destructive), so doing it unconditionally
    // here guarantees the work is rescued.
    if (g_cbEmergencySave)
        g_cbEmergencySave();

    // Inform on a SEPARATE thread -- a MessageBox pumped on THIS (GUI) thread
    // re-dispatches into the corrupt Qt layout (a second AV). A worker thread
    // pumps only its own empty queue, so the box shows cleanly while this thread
    // just blocks on it. The save above already ran regardless.
    HANDLE th = ::CreateThread(nullptr, 0, cbDialogThread, nullptr, 0, nullptr);
    if (th)
    {
        ::WaitForSingleObject(th, INFINITE);
        ::CloseHandle(th);
    }

    // Do NOT return into Qt or run its teardown -- the widget/layout state is
    // corrupt. Exit the process directly.
    ::ExitProcess(2);
}

// Top-level backstop: an access violation that ESCAPES the notify() SEH wrapper
// (a fault outside event dispatch) lands here as the process's unhandled filter.
// CAVEATS: a debugger (F5) intercepts the fault FIRST-chance and breaks, so this
// only fires when running WITHOUT a debugger; and the Debug heap can turn a
// use-after-free into an uncatchable fast-fail -- the shipping Release build
// raises a clean, catchable AV.
static LONG WINAPI cbUnhandledExceptionFilter(EXCEPTION_POINTERS* info)
{
    if (info && info->ExceptionRecord &&
        info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
        cbHandleFatalGuiCrash();   // save + dialog + ExitProcess -- never returns
    return EXCEPTION_EXECUTE_HANDLER;
}

namespace {
// SEH must live in a leaf with no C++ object unwinding; notify() has none, but
// keeping the __try here makes that obviously true regardless of later edits.
bool cbGuardedNotify(QApplication* app, QObject* receiver, QEvent* event)
{
    __try
    {
        return app->QApplication::notify(receiver, event);   // base, not the override
    }
    __except (cbGuiCrashFilter(GetExceptionCode()))
    {
        cbHandleFatalGuiCrash();   // native dialog + save + ExitProcess -- never returns
        return false;              // unreachable; satisfies the compiler
    }
}

// QApplication subclass whose only job is to route notify() through the SEH
// guard above.
class CbApplication : public QApplication
{
public:
    CbApplication(int& argc, char** argv) : QApplication(argc, argv) {}
    bool notify(QObject* receiver, QEvent* event) override
    {
        return cbGuardedNotify(this, receiver, event);
    }
};
} // namespace

void Qt_EnsureApplication()
{
    if (qApp)
        return;

    static int   argc    = 1;
    static char  argv0[] = "ClassBuilder";
    static char* argv[]  = { argv0, nullptr };
    new CbApplication(argc, argv);

    // App-wide window/taskbar icon. The EXE's Win32 resource icon
    // (ClassBuilder_app.rc) is what the shell shows for the file and a pinned
    // taskbar button; the RUNNING window's titlebar + taskbar icon is a
    // separate Qt notion that defaults to nothing, so set it explicitly. Reuse
    // the already-embedded :/classbuilder.png (the About box uses it too).
    // setWindowIcon on the QApplication applies to every top-level window.
    qApp->setWindowIcon(QIcon(QStringLiteral(":/classbuilder.png")));

    // Backstop for an AV that escapes the notify() SEH (a fault outside event
    // dispatch). Installed AFTER the app so neither Qt nor the CRT overrides it.
    ::SetUnhandledExceptionFilter(cbUnhandledExceptionFilter);

    // The UI font. The platform default (~9pt) is too small for the dense
    // class / member / method text in the ported dialogs. Set an ABSOLUTE
    // size (CB_UI_FONT_PT) -- not "default + N", because the Windows default
    // font is pixel-defined so pointSizeF() returns -1 and "+N" arithmetic
    // silently produces nothing.
    //
    // Set it BOTH ways: QApplication::setFont (so a widget's fontMetrics() is
    // already correct at construction time -- the lists' compactItemSize and
    // CbTreeWidget's row height depend on that), AND a stylesheet font-size
    // rule (authoritative for rendering). Both carry the same value, so they
    // agree. One knob: CB_UI_FONT_PT.
    {
        QFont appFont = qApp->font();
        appFont.setPointSize(CB_UI_FONT_PT);
        appFont.setWeight(static_cast<QFont::Weight>(CB_UI_FONT_WEIGHT));
        qApp->setFont(appFont);
    }

    // App-wide stylesheet: the font size (see above) + soften the QGroupBox
    // frame -- the modern Windows style draws a hard, near-black 1px box that
    // is too "in your face"; a soft theme-grey (palette `mid`) reads better.
    //
    // Tree connector lines are NOT done here -- a stylesheet cannot pick a
    // branch glyph per node (no notion of depth). CbTreeWidget::drawBranches
    // does that; see CbTreeWidget.cpp.
    qApp->setStyleSheet(
        QString("QWidget { font-size: %1pt; font-weight: %2; }")
            .arg(CB_UI_FONT_PT).arg(CB_UI_FONT_WEIGHT) +
        "QGroupBox {"
        "  border: 1px solid palette(mid);"
        "  border-radius: 4px;"
        "  margin-top: 1.4ex;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  left: 8px;"
        "  padding: 0 3px;"
        "}");

    // Install the process-wide headless text-measure painter. Model-side layout
    // methods (lifeline auto-width, class auto-size, OptimizePlacement, the
    // signal-text hit-test) measure through CbPainter::GetMeasurePainter()
    // instead of a view DC -- which is how they worked before the MFC views
    // were removed. Needs QApplication alive (QFontMetrics), hence here.
    // Process-lifetime singleton, like the QApplication itself.
    static CbPainter_QFontMetrics s_measurePainter;
    CbPainter::SetMeasurePainter(&s_measurePainter);
}

int Qt_ExecModal(QDialog& dlg, void* ownerHwnd)
{
    HWND owner = static_cast<HWND>(ownerHwnd);
    if (!owner || !::IsWindow(owner))
        return dlg.exec();

    // Own the Qt dialog to the MFC window: winId() forces the native window
    // into existence, then GWLP_HWNDPARENT makes it a Win32-owned popup --
    // it stays above the MFC frame and shares its taskbar entry.
    ::SetWindowLongPtr(reinterpret_cast<HWND>(dlg.winId()),
                       GWLP_HWNDPARENT,
                       reinterpret_cast<LONG_PTR>(owner));

    // Disable the owner for the dialog's lifetime -- exactly what a native
    // modal dialog does. Without this the MFC window stays clickable and can
    // be closed, orphaning the Qt dialog.
    const bool wasEnabled = ::IsWindowEnabled(owner) != FALSE;
    if (wasEnabled)
        ::EnableWindow(owner, FALSE);

    const int rc = dlg.exec();

    if (wasEnabled)
        ::EnableWindow(owner, TRUE);
    ::SetForegroundWindow(owner);   // restore activation to the MFC window
    return rc;
}

void Qt_ShowModeless(QWidget& w, void* ownerHwnd)
{
    // Make it a top-level window (not embedded as a child) and force the
    // native HWND into existence so GWLP_HWNDPARENT can be set BEFORE show().
    w.setWindowFlag(Qt::Window, true);

    HWND owner = static_cast<HWND>(ownerHwnd);
    if (owner && ::IsWindow(owner))
    {
        ::SetWindowLongPtr(reinterpret_cast<HWND>(w.winId()),
                           GWLP_HWNDPARENT,
                           reinterpret_cast<LONG_PTR>(owner));
    }

    // No EnableWindow toggle here -- non-modal: the owner stays usable.
    w.show();
    w.raise();
    w.activateWindow();
}
