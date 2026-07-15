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
#include <QByteArray>
#include <QDialog>
#include <QEvent>
#include <QIcon>
#include <QScreen>
#include <QSettings>
#include <QString>
#include <QWidget>
#ifndef _WIN32   // macOS + Linux: file-open event + button-font filter use these
#include <QFileOpenEvent>
#include <QPushButton>
#include <QFont>
#include "CbShellHooks.h"   // Cb_ShellOpenDocument (Finder file association)
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>   // Cb_MacDisableFontSmoothing
#endif

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
// The platform integration + native style plugins are platform-specific.
#ifdef _WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QModernWindowsStylePlugin)
#elif defined(__APPLE__)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#endif
// The SVG image-format + icon-engine plugins -- only when this Qt has the Svg
// module (CB_HAVE_SVG); a static Qt does not self-load plugins. Lets
// QtModelIcons render redrawn .svg model icons.
#ifdef CB_HAVE_SVG
Q_IMPORT_PLUGIN(QSvgPlugin)
Q_IMPORT_PLUGIN(QSvgIconPlugin)
#endif
#endif

#ifdef _WIN32
// windows.h last, after the Qt headers, with the macro guards -- otherwise its
// min/max macros collide with Qt headers. Guarded -- CMake also defines both
// globally, so an unguarded #define warns C4005 (redefinition). The Win32 GUI
// crash net + native dialog parenting below need it; macOS/Linux use the
// Qt-native fallbacks.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// The one knob for the Qt dialog UI font size, in points. The platform
// default is ~9pt; this is deliberately larger for readability.
//
// macOS renders a given point size smaller than Windows (different default
// system font + DPI basis), so the whole UI -- dialogs, tree rows, and the
// model icons (sized from fontMetrics().height()) -- came out smaller than the
// Windows build. Bump the point size on macOS so EVERYTHING scales up together
// to match; Windows keeps 11. One knob -- tune to taste.
#ifdef __APPLE__
static const int CB_UI_FONT_PT = 15;   // macOS renders points smaller than Windows; matches density
#else
static const int CB_UI_FONT_PT = 11;
#endif

// UI font weight: a bit heavier than Normal(400) for crisper text, short of
// Bold(700). 500 == QFont::Medium. Weight is stroke thickness, not glyph
// height, so this does NOT change fontMetrics().height() -> row heights / line
// spacing are unaffected. One knob, applied both ways like the size below.
//
// Windows renders the UI font too light, so it's bumped to Medium(500) there.
// macOS renders heavier (Core Text), so it stays at Normal(400) -- 500 looks
// half-bold. (The old 350 was a no-op: the system font snaps it to 400 and the
// tree QSS reset it to 400 anyway; it now reaches the tree explicitly, see
// CbTreeWidget.) The remaining Core Text stem-darkening -- what made mac text
// read heavier than the other platforms -- is removed separately by disabling
// font smoothing at startup (Cb_MacDisableFontSmoothing), which is what brings
// mac into line with Windows/Linux.
#ifdef __APPLE__
static const int CB_UI_FONT_WEIGHT = 400;   // Normal
#else
static const int CB_UI_FONT_WEIGHT = 500;
#endif

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

#ifdef _WIN32

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

#else  // macOS / Linux: no Win32 SEH. The dock-reparent AV net is a Windows-only
       // safety measure; elsewhere use a plain QApplication. (If the same crash
       // surfaces on another platform, add a POSIX signal-based net here.)

namespace {
class CbApplication : public QApplication
{
public:
    CbApplication(int& argc, char** argv) : QApplication(argc, argv) {}

    // macOS hands a double-clicked / "Open With" / Finder-associated file to the
    // app as a QFileOpenEvent -- NOT as an argv path (that's Windows/Linux). So a
    // .cbz double-click launches CB but the file never loads unless we handle it
    // here. The shell window exists by the time this is delivered (created before
    // exec()), so route straight to the open-document flow.
    bool event(QEvent* e) override
    {
        if (e->type() == QEvent::FileOpen)
        {
            const QString file = static_cast<QFileOpenEvent*>(e)->file();
            if (!file.isEmpty())
                Cb_ShellOpenDocument(file.toLocal8Bit().constData());
            return true;
        }
        return QApplication::event(e);
    }
};

// With NO app stylesheet (so edit fields + combos keep the native focus ring),
// the app's 15pt font makes QPushButtons taller than macOS' rounded "Aqua"
// bezel can take, so QMacStyle renders them square. Shrink JUST the button font
// to the macOS system size so they fit the rounded bezel -- rounded buttons +
// native focus rings, no stylesheet needed.
class CbButtonFontFilter : public QObject
{
public:
    using QObject::QObject;
protected:
    bool eventFilter(QObject* o, QEvent* e) override
    {
        if (e->type() == QEvent::Polish)
        {
            if (auto* b = qobject_cast<QPushButton*>(o))
            {
                QFont f = b->font();
                if (f.pointSize() > 13)
                {
                    f.setPointSize(13);
                    b->setFont(f);
                }
            }
        }
        return QObject::eventFilter(o, e);
    }
};
} // namespace

#endif // _WIN32

#ifdef __APPLE__
// Turn OFF Core Text font smoothing (stem-darkening) for THIS app. On macOS the
// same glyphs render a touch heavier than on Windows/Linux; writing our own
// AppleFontSmoothing default to 0 -- exactly what `defaults write <bundle-id>
// AppleFontSmoothing 0` does, but baked in so it ships and needs no per-machine
// setup -- lightens the whole UI to match. CoreGraphics reads this from the
// app's own preference domain; set it BEFORE the QApplication so it is in place
// before the first glyph is painted. CoreFoundation only; no Obj-C.
static void Cb_MacDisableFontSmoothing()
{
    const int zero = 0;
    CFNumberRef v = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &zero);
    CFPreferencesSetAppValue(CFSTR("AppleFontSmoothing"), v, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    CFRelease(v);
}
#endif

void Qt_EnsureApplication()
{
    if (qApp)
        return;

#ifdef __APPLE__
    Cb_MacDisableFontSmoothing();   // lighten Core Text stem-darkening (see above)
#endif

    // Whole-app UI scale (QtShellWindow's View > UI Scale menu): Qt only reads
    // QT_SCALE_FACTOR at QApplication construction, so the persisted choice has
    // to land in the environment HERE -- there is no later, in-process way to
    // rescale an already-built widget tree. QSettings works fine with no
    // QApplication yet (explicit org/app name form).
    const double uiScale =
        QSettings("ClassBuilder", "ClassBuilder").value("shell/uiScale", 1.0).toDouble();
    if (uiScale != 1.0)
        qputenv("QT_SCALE_FACTOR", QByteArray::number(uiScale, 'g', 3));

    static int   argc    = 1;
    static char  argv0[] = "ClassBuilder";
    static char* argv[]  = { argv0, nullptr };
    new CbApplication(argc, argv);

    // QT_SCALE_FACTOR only rescales what Qt itself paints -- the X11/Xcursor
    // pointer is a separate resource (XCURSOR_SIZE) that libXcursor reads
    // independently of Qt, so without this the pointer keeps the DESKTOP's size
    // and mismatches CB's uiScale-d content the moment it enters a CB window.
    //
    // XCURSOR_SIZE is in DEVICE pixels, but the desktop's cursor-size (24) is a
    // LOGICAL size -- the device size the desktop actually paints is 24 * DPR
    // (48 on a 2x screen). The old code used the logical 24 as its device base
    // and so emitted 24*1.25 = 30 at uiScale 1.25 -- BELOW the desktop's 48, so
    // the pointer visibly SHRANK on entering CB on a HiDPI screen. It only ever
    // looked right back when XWayland forced DPR 1 (device == logical).
    //
    // Set it AFTER the QApplication: devicePixelRatio() then already folds in
    // QT_SCALE_FACTOR (2 * 1.25 = 2.5), so 24 * dpr IS the wanted device size
    // (60). libXcursor reads the variable lazily, when a cursor is first
    // materialised, which is after this point. An explicit XCURSOR_SIZE from the
    // environment is already a device size, so that path only applies uiScale.
    // This touches OUR process's environment only, not the rest of the desktop.
    // No-op on Windows/macOS -- neither reads XCURSOR_SIZE.
    if (uiScale != 1.0)
    {
        const int    envSize = qEnvironmentVariableIntValue("XCURSOR_SIZE");
        const QScreen* scr   = QGuiApplication::primaryScreen();
        const qreal  dpr     = scr ? scr->devicePixelRatio() : 1.0;
        const int    size    = (envSize > 0) ? qRound(envSize * uiScale)
                                             : qRound(24.0 * dpr);
        qputenv("XCURSOR_SIZE", QByteArray::number(size));
    }

    // Show shortcut text (e.g. "Ctrl+Shift+M") next to CONTEXT-menu items, as on
    // Windows. macOS defaults this attribute ON (shortcuts hidden in context
    // menus); the diagram/tree right-click menus carry shortcuts worth showing,
    // so turn it off. (Top menu-bar items already show their shortcut.) Harmless
    // on Windows -- it already shows them.
    QCoreApplication::setAttribute(Qt::AA_DontShowShortcutsInContextMenus, false);

    // App-wide window/taskbar icon. The EXE's Win32 resource icon
    // (ClassBuilder_app.rc) is what the shell shows for the file and a pinned
    // taskbar button; the RUNNING window's titlebar + taskbar icon is a
    // separate Qt notion that defaults to nothing, so set it explicitly. Use
    // the vector app icon :/icons/class.svg (the class-glyph, also the base for
    // the derived platform icons -- .icns / hicolor PNGs); the About box uses it
    // too. SVG keeps the titlebar/taskbar icon crisp at any DPR.
    // setWindowIcon on the QApplication applies to every top-level window.
    qApp->setWindowIcon(QIcon(QStringLiteral(":/icons/class.svg")));

    // Wayland (GNOME/Ubuntu) ignores setWindowIcon for the dock/taskbar -- it
    // binds a running window to a .desktop file by app-id, not the window's own
    // icon. Set the desktop file name so the compositor resolves our installed
    // launcher (classbuilder.desktop) and shows ITS icon + groups the window
    // under it. Must match the .desktop basename (and its StartupWMClass). No
    // effect on Windows/macOS, harmless on X11.
    QGuiApplication::setDesktopFileName(QStringLiteral("classbuilder"));

    // Backstop for an AV that escapes the notify() SEH (a fault outside event
    // dispatch). Installed AFTER the app so neither Qt nor the CRT overrides it.
    // Windows-only (SEH); macOS/Linux run without this net.
#ifdef _WIN32
    ::SetUnhandledExceptionFilter(cbUnhandledExceptionFilter);
#endif

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

#ifdef __APPLE__
    // Accent colour. The issue isn't the hue -- it's contrast: macOS' light-blue
    // system accent leaves the tree's expand/collapse triangles and connector
    // lines (custom-drawn from QPalette::Highlight in CbTreeWidget::drawBranches)
    // barely visible on the white tree. Pin a DARKER blue -- still squarely in
    // the macOS style, just enough contrast to read on white -- as the app-wide
    // Highlight (drives tree chrome + diagram selection, both keyed off it).
    // Windows keeps its own (OS) accent -- this block is __APPLE__-only.
    // One knob: the `accent` RGB below, tune to taste.
    {
        const QColor accent(0x0A, 0x4D, 0xA8);   // deep blue (Mac-style, high contrast)
        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Active,   QPalette::Highlight, accent);
        pal.setColor(QPalette::Inactive, QPalette::Highlight, accent);
        pal.setColor(QPalette::Active,   QPalette::HighlightedText, Qt::white);
        pal.setColor(QPalette::Inactive, QPalette::HighlightedText, Qt::white);

        // Dialog background: a light grey Window vs a white Base, like Windows --
        // so white edit boxes stand out against the dialog by contrast (the real
        // reason they "popped" on Windows; no edit-box border needed).
        const QColor dialogGrey(0xEC, 0xEC, 0xEC);
        pal.setColor(QPalette::Active,   QPalette::Window, dialogGrey);
        pal.setColor(QPalette::Inactive, QPalette::Window, dialogGrey);
        pal.setColor(QPalette::Active,   QPalette::Base,   Qt::white);
        pal.setColor(QPalette::Inactive, QPalette::Base,   Qt::white);
        qApp->setPalette(pal);
    }
#endif

    // App-wide stylesheet: the font size (see above) + soften the QGroupBox
    // frame -- the modern Windows style draws a hard, near-black 1px box that
    // is too "in your face"; a soft theme-grey (palette `mid`) reads better.
    //
    // Tree connector lines are NOT done here -- a stylesheet cannot pick a
    // branch glyph per node (no notion of depth). CbTreeWidget::drawBranches
    // does that; see CbTreeWidget.cpp.
    QString sheet;
    // The FONT via stylesheet is a Windows-only need (its default UI font is
    // pixel-sized, so setFont's pointSize doesn't render). It must NOT go on
    // macOS: a `QWidget` QSS rule there routes EVERY widget -- including
    // QPushButton -- through Qt's stylesheet renderer in a way that squares the
    // native rounded button.
    // macOS gets NO app stylesheet (so edit fields AND combos keep the native
    // focus ring); the square-button side effect of that is fixed separately by
    // shrinking just the button font to fit the native rounded bezel (see
    // CbButtonFontFilter). Windows keeps its QSS: the font (its default UI font
    // is pixel-sized so setFont's pointSize doesn't render) + QGroupBox softening.
#ifndef __APPLE__
    sheet += QString("QWidget { font-size: %1pt; font-weight: %2; }")
                 .arg(CB_UI_FONT_PT).arg(CB_UI_FONT_WEIGHT);
    sheet +=
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
        "}";
#else
    // macOS gets NO blanket QWidget QSS (it would square the native buttons and
    // strip the native focus ring -- see above). But QMacStyle draws the
    // QGroupBox title in a small system font that reads tiny and faint next to
    // the 15pt UI everywhere else. Scope a QSS rule to JUST QGroupBox (buttons /
    // edits / combos stay native) to give the title the app font size + weight,
    // and match the Windows/Linux soft-border group look at the same time.
    sheet += QString("QGroupBox {"
                     "  font-size: %1pt;"
                     "  font-weight: %2;"
                     "  border: 1px solid palette(mid);"
                     "  border-radius: 4px;"
                     "  margin-top: 1.4ex;"
                     "}")
                 .arg(CB_UI_FONT_PT).arg(CB_UI_FONT_WEIGHT);
    sheet +=
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  left: 8px;"
        "  padding: 0 3px;"
        "}";
#endif
    // Tooltips in the classic soft info-yellow (the Win32 look) on every
    // platform -- Qt's own tooltip colour is white-ish. Scoped to QToolTip,
    // so nothing else is touched.
    sheet +=
        "QToolTip {"
        "  background-color: #FFFFE1;"
        "  color: black;"
        "  border: 1px solid #767676;"
        "}";
    if (!sheet.isEmpty())
        qApp->setStyleSheet(sheet);

#ifdef __APPLE__
    // Keep push buttons rounded under the native (stylesheet-free) style.
    qApp->installEventFilter(new CbButtonFontFilter(qApp));
#endif

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
#ifdef _WIN32
    HWND owner = static_cast<HWND>(ownerHwnd);
    if (!owner || !::IsWindow(owner))
        return dlg.exec();

    // Own the Qt dialog to the shell window: winId() forces the native window
    // into existence, then GWLP_HWNDPARENT makes it a Win32-owned popup --
    // it stays above the shell frame and shares its taskbar entry.
    ::SetWindowLongPtr(reinterpret_cast<HWND>(dlg.winId()),
                       GWLP_HWNDPARENT,
                       reinterpret_cast<LONG_PTR>(owner));

    // Disable the owner for the dialog's lifetime -- exactly what a native
    // modal dialog does. Without this the shell window stays clickable and can
    // be closed, orphaning the Qt dialog.
    const bool wasEnabled = ::IsWindowEnabled(owner) != FALSE;
    if (wasEnabled)
        ::EnableWindow(owner, FALSE);

    const int rc = dlg.exec();

    if (wasEnabled)
        ::EnableWindow(owner, TRUE);
    ::SetForegroundWindow(owner);   // restore activation to the shell window
    return rc;
#else
    // macOS/Linux: no native HWND re-parenting. exec() is application-modal,
    // which gives the same "blocks the rest of the app" behaviour; Qt manages
    // stacking above the main window. (ownerHwnd is the main window's winId,
    // not a QWidget we can setParent on, so it is unused here.)
    (void)ownerHwnd;
    return dlg.exec();
#endif
}

void Qt_ShowModeless(QWidget& w, void* ownerHwnd)
{
    // Make it a top-level window (not embedded as a child).
    w.setWindowFlag(Qt::Window, true);

#ifdef _WIN32
    // Force the native HWND into existence so GWLP_HWNDPARENT can be set BEFORE
    // show(), owning the Qt window to the shell frame.
    HWND owner = static_cast<HWND>(ownerHwnd);
    if (owner && ::IsWindow(owner))
    {
        ::SetWindowLongPtr(reinterpret_cast<HWND>(w.winId()),
                           GWLP_HWNDPARENT,
                           reinterpret_cast<LONG_PTR>(owner));
    }
#else
    (void)ownerHwnd;   // no native parenting on macOS/Linux
#endif

    // No EnableWindow toggle here -- non-modal: the owner stays usable.
    w.show();
    w.raise();
    w.activateWindow();
}
