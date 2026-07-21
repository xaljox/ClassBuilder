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
#include <QAbstractItemView>
#include <QColor>
#include <QList>
#include <QPalette>
#include "CbTreeWidget.h"      // live accent re-derive (Cb_OnAppPaletteChanged)
#include "CodeEditor.h"
#include "QtSoftSelection.h"   // Qt_ApplySoftSelection (popup re-tint)
#include "QtMenuStyle.h"       // Qt_CompactMenuStyleSheet (the one menu sheet)
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

namespace {

// The accent CB last wrote into the palette, so the fetch below can tell a
// fresh system value apart from CB's own write coming back at it.
QColor g_writtenAccent;
QColor g_systemAccent;

// THE one platform-specific step in the whole colour system: fetch the accent
// the DESKTOP was set to. Nothing is decided here -- no contrast, no darkening,
// no per-OS look -- only "which colour did the user pick". Everything after this
// point is shared code, so the same chosen accent gives the same result on every
// platform, and every platform can be set to its own.
//
// Where that colour lives differs per platform, and the difference is not
// cosmetic -- each OS hands Qt a DERIVED shade, and a different derivation:
//   * Windows -- DWM's AccentColor: the swatch itself, from the Personalization
//     page. Both QPalette::Highlight AND ::Accent come back as a darker rung of
//     the shade ladder Windows generates around that swatch (measured on a
//     #258292 accent: Qt reports #1D6978), so using them would mean the same
//     chosen colour renders darker on Windows than on the other platforms.
//   * macOS -- QPalette::Accent (NSColor controlAccentColor), the colour the
//     native menus highlight with. NOT Highlight: on macOS that role is the pale
//     text-selection wash (~#A5CDFF), far too light to be the app accent.
//     (macOS fills Accent; Linux does not -- see below.)
//   * Linux -- QPalette::Highlight. The theme puts the chosen desktop accent
//     there; QPalette::Accent is left at Qt's built-in default blue, and since
//     that is a valid colour it cannot be told apart by an isValid() test.
// So the fetch is per-platform on purpose; everything after it is not.
QColor Cb_SystemAccent()
{
    const QPalette pal = qApp->palette();
    QColor accent;
#ifdef _WIN32
    const QSettings dwm("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\DWM",
                        QSettings::NativeFormat);
    const QVariant v = dwm.value("AccentColor");
    if (v.isValid())
    {
        const uint abgr = v.toUInt();          // 0xAABBGGRR
        accent = QColor(int(abgr & 0xFF), int((abgr >> 8) & 0xFF),
                        int((abgr >> 16) & 0xFF));
    }
#elif defined(__APPLE__)
    accent = pal.color(QPalette::Active, QPalette::Accent);
#else
    // Linux: the desktop accent lands in Highlight, NOT in Accent. Measured on
    // Ubuntu 26.04 / GNOME (Fusion style): with the desktop accent set to teal,
    // Highlight = #308280 (the chosen colour) while Accent stayed at Qt's
    // built-in default #308cc6 blue. That default is a VALID colour, so the
    // isValid() fallback below never fired and every desktop accent rendered as
    // that blue. Read Highlight directly here; CB's own write-back is handled by
    // the g_writtenAccent check below (which keeps the last real system value).
    accent = pal.color(QPalette::Active, QPalette::Highlight);
#endif

    // Reading back CB's own write is not a fetch: keep the last real system
    // value in that case (Accent is untouched by CB, but Highlight -- the
    // fallback below -- is not).
    if (!accent.isValid() || accent == g_writtenAccent)
        accent = pal.color(QPalette::Active, QPalette::Highlight);
    if (accent.isValid() && accent != g_writtenAccent)
        g_systemAccent = accent;
    return g_systemAccent.isValid() ? g_systemAccent : accent;
}

// The dialog/panel background, derived from the input-field background.
//
// A dialog reads as a surface with edit boxes, lists and trees ON it, and that
// only works while the two backgrounds differ: where QPalette::Window sits too
// close to QPalette::Base the fields dissolve into the dialog and only their
// (thin, styled-away) frames still mark them. macOS hands Qt a near-white
// Window, which is exactly that case -- CB used to pin a grey there, per-OS.
//
// Shared rule instead: measure the contrast ratio between Window and Base and,
// when it is below the target, step Window further AWAY from Base -- darker
// under a light theme, lighter under a dark one -- keeping the theme's own hue.
// A theme that already separates them is left alone. Same target everywhere, so
// the platforms land on the same relationship (and, since all three use a white
// Base in a light theme, on the same grey) instead of three different ones.
// One knob: the ratio (JV 2026-07-21).
QColor Cb_PanelColour(const QColor& base, const QColor& window)
{
    const auto lum = [](const QColor& c) {
        const auto lin = [](double v) {
            return v <= 0.03928 ? v / 12.92
                                : std::pow((v + 0.055) / 1.055, 2.4);
        };
        return 0.2126 * lin(c.redF()) + 0.7152 * lin(c.greenF())
             + 0.0722 * lin(c.blueF());
    };
    const auto contrast = [&lum](const QColor& a, const QColor& b) {
        const double la = lum(a), lb = lum(b);
        return (qMax(la, lb) + 0.05) / (qMin(la, lb) + 0.05);
    };
    const double kTarget = 1.18;      // the separation the macOS pin had
    if (contrast(window, base) >= kTarget)
        return window;

    const float step = (base.lightnessF() > 0.5f) ? -0.01f : 0.01f;
    float h = 0, s = 0, l = 0, a = 0;
    window.getHslF(&h, &s, &l, &a);
    QColor c = window;
    for (int i = 0; i < 60 && l > 0.0f && l < 1.0f; ++i)
    {
        l = qBound(0.0f, l + step, 1.0f);
        c = QColor::fromHslF(qMax(0.0f, h), s, l, a);
        if (contrast(c, base) >= kTarget)
            break;
    }
    return c;
}

// THE single point that decides the app-wide accent -- and from here on it is
// platform-INDEPENDENT: the fetched accent lands in QPalette::Highlight with a
// derived HighlightedText beside it, and everything theme-derived downstream --
// tree chrome/triangles, the selection & hover tint, the editor text selection,
// diagram selection, Qt's own widget selections -- keys off that one colour.
// The only per-element derivation is the tree glyphs' lightness clamp
// (Qt_ChromeAccent, QtSoftSelection.h), which is shared code too.
// Only writes the palette when it actually differs, so calling this from the
// ApplicationPaletteChange handler (below) can't loop.
void Cb_ApplyAccentPalette()
{
    // Dev hook, every platform: CB_FORCE_ACCENT=#rrggbb stands in for the
    // desktop's accent for this run, so a LIGHT accent (where the tree glyphs'
    // clamp kicks in) can be exercised on a machine whose own accent is dark,
    // and the same value can be compared across platforms. Unset -> the real
    // system accent, no effect at all.
    const QString forced = qEnvironmentVariable("CB_FORCE_ACCENT");
    const QColor accent = (!forced.isEmpty() && QColor::isValidColorName(forced))
                              ? QColor(forced) : Cb_SystemAccent();

    QPalette want = qApp->palette();

    // Dialog/panel background: derived from the theme's own Base + Window on
    // every platform (Cb_PanelColour), replacing the macOS-only grey pin -- the
    // last per-OS colour. Base itself is left to the theme.
    const QColor panel = Cb_PanelColour(
        want.color(QPalette::Active, QPalette::Base),
        want.color(QPalette::Active, QPalette::Window));
    want.setColor(QPalette::Active,   QPalette::Window, panel);
    want.setColor(QPalette::Inactive, QPalette::Window, panel);

    // The accent goes in AS CHOSEN -- no correction here. A light accent is
    // perfectly readable as a row-sized tint or a filled selection; only the
    // small solid tree glyphs wash out at that size, and they deepen it
    // themselves (Qt_ChromeAccent).
    // Text ON the accent: whichever of black/white it carries better -- derived,
    // not assumed white (a light accent needs black).
    const QColor onAccent =
        (accent.lightnessF() < 0.6f) ? QColor(Qt::white) : QColor(Qt::black);
    want.setColor(QPalette::Active,   QPalette::Highlight, accent);
    want.setColor(QPalette::Inactive, QPalette::Highlight, accent);
    want.setColor(QPalette::Active,   QPalette::HighlightedText, onAccent);
    want.setColor(QPalette::Inactive, QPalette::HighlightedText, onAccent);
    if (want != qApp->palette())
        qApp->setPalette(want);
    g_writtenAccent = accent;
}

// The desktop accent/theme changed while CB is open: re-derive EVERY widget
// that depends on the accent from the (possibly new) value, so nothing sits in
// a stale colour and there is no mixed look -- no restart needed. Cheap and
// rare (fires only on an accent/theme change). The tree branch triangles/lines
// read the live accent every paint, so their widgets just repaint.
void Cb_OnAppPaletteChanged()
{
    Cb_ApplyAccentPalette();   // keep the macOS override; no-op on Win/Linux
    const QList<QWidget*> widgets = QApplication::allWidgets();
    for (QWidget* w : widgets)
    {
        if (auto* tree = qobject_cast<CbTreeWidget*>(w))
            tree->reapplyThemeAccent();
        else if (auto* editor = qobject_cast<CodeEditor*>(w))
            editor->reapplyThemeAccent();
        else if (auto* view = qobject_cast<QAbstractItemView*>(w))
        {
            // Completion / who-calls-me popups tag themselves in
            // Qt_ApplySoftSelection, so QtApp need not know their concrete type.
            if (view->property("cbSoftSelection").toBool())
                Qt_ApplySoftSelection(view);
        }
    }
}

// Watches the one event that announces a desktop accent/theme change --
// QEvent::ApplicationPaletteChange, delivered to qApp. Installed on qApp so it
// works on every platform without touching the per-platform CbApplication.
class CbAccentWatcher : public QObject
{
public:
    using QObject::QObject;
protected:
    bool eventFilter(QObject* obj, QEvent* e) override
    {
        if (e->type() == QEvent::ApplicationPaletteChange)
            Cb_OnAppPaletteChanged();
        return QObject::eventFilter(obj, e);
    }
};

} // namespace

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

    // The app-wide accent: filled per platform by the ONE chokepoint (macOS pins
    // a high-contrast deep blue + the light dialog palette; Windows/Linux follow
    // the system accent). Everything theme-derived keys off QPalette::Highlight.
    // See Cb_ApplyAccentPalette() above; re-run live on accent change by
    // CbAccentWatcher (installed below).
    Cb_ApplyAccentPalette();

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
    // Menus: every QMenu CB shows -- menu-bar dropdowns, tool-button menus, and
    // any context menu that is not explicitly styled -- takes the accent on the
    // selected item, the same compact metrics, on EVERY platform. It was Windows
    // -only, which made the menus one of the places CB behaved differently per
    // OS (JV 2026-07-21: after the accent choice the behaviour has to match).
    // The rule has to own the WHOLE QMenu, not just ::item:selected: the modern
    // Windows 11 style paints the selected item a flat darker GREY and ignores a
    // QSS background-color on the item alone (only the text colour obeyed, so it
    // came out white-on-grey). palette() refs follow the LIVE accent without
    // rebuilding the sheet. macOS keeps its NATIVE menu BAR regardless -- that
    // is an NSMenu owned by the OS, not a QMenu -- so this reaches its popup
    // menus only.
    // ONE source for every menu in CB: the same sheet the explicitly-styled
    // context menus get (Qt_ApplyCompactMenuStyle), so a menu-bar dropdown and a
    // right-click menu are identical on every platform. It also fixes the
    // invisible separator/border this block used to have: `palette(mid)` is
    // #ffffff on Ubuntu/GNOME while the menu background is #fcfcfc, so both were
    // painted white on near-white. QtMenuStyle derives those greys from the
    // background instead; the accent parts stay live palette() refs.
    sheet += Qt_CompactMenuStyleSheet();
    if (!sheet.isEmpty())
        qApp->setStyleSheet(sheet);

#ifdef __APPLE__
    // Keep push buttons rounded under the native (stylesheet-free) style.
    qApp->installEventFilter(new CbButtonFontFilter(qApp));
#endif

    // Live accent: re-derive tree/editor/popup tints when the desktop accent
    // changes while CB is open (no restart, no mixed colours). Cross-platform;
    // a no-op in practice on macOS (its accent is pinned) but harmless there.
    qApp->installEventFilter(new CbAccentWatcher(qApp));

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
