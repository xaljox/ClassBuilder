// qt/QtDesktopTheme.cpp -- see QtDesktopTheme.h.

#include "QtDesktopTheme.h"

#ifdef __linux__
#include <QDir>
#include <QIcon>
#include <QProcess>
#include <QString>
#include <QStringList>
#endif

#if defined(__linux__) && defined(CB_HAVE_DBUS)
#define CB_PORTAL_ACCENT 1
#endif

#ifdef CB_PORTAL_ACCENT
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QLatin1String>
#include <QObject>
#include <QVariant>

namespace {

const char* const kService   = "org.freedesktop.portal.Desktop";
const char* const kPath      = "/org/freedesktop/portal/desktop";
const char* const kInterface = "org.freedesktop.portal.Settings";
const char* const kNamespace = "org.freedesktop.appearance";
const char* const kKey       = "accent-color";

// The portal returns the accent as a (ddd) struct of 0..1 doubles inside a
// variant -- and the older Read call wraps it one layer deeper than ReadOne, so
// peel variants until the struct shows up. (-1,-1,-1) is the documented "no
// accent set", which must read as invalid rather than as black.
QColor colourFromReply(const QVariant& value)
{
    QVariant inner = value;
    for (int depth = 0; depth < 3 && inner.canConvert<QDBusVariant>(); ++depth)
        inner = inner.value<QDBusVariant>().variant();
    if (!inner.canConvert<QDBusArgument>())
        return QColor();

    const QDBusArgument arg = inner.value<QDBusArgument>();
    if (arg.currentType() != QDBusArgument::StructureType)
        return QColor();

    double r = -1, g = -1, b = -1;
    arg.beginStructure();
    arg >> r >> g >> b;
    arg.endStructure();
    if (r < 0 || g < 0 || b < 0)
        return QColor();
    return QColor::fromRgbF(r, g, b);
}

// Receiver for the portal's SettingChanged signal. A QObject with a real slot,
// because QDBusConnection::connect takes a SLOT() signature -- there is no
// lambda overload.
class PortalWatcher : public QObject
{
    Q_OBJECT
public:
    explicit PortalWatcher(void (*cb)()) : _cb(cb) {}

public slots:
    void onSettingChanged(const QString& ns, const QString& key,
                          const QDBusVariant& value)
    {
        Q_UNUSED(value)
        if (_cb && ns == QLatin1String(kNamespace) && key == QLatin1String(kKey))
            _cb();
    }

private:
    void (*_cb)() = nullptr;
};

} // namespace
#endif   // CB_PORTAL_ACCENT

QColor Cb_PortalAccent()
{
#ifdef CB_PORTAL_ACCENT
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return QColor();

    // ReadOne is the current call; portals older than 0.36 only have Read. Same
    // reply shape bar one variant layer, which colourFromReply peels either way.
    // A short timeout: this runs at startup, and a hung portal must not hold the
    // app -- an invalid colour just falls back to the palette.
    for (const char* method : { "ReadOne", "Read" })
    {
        QDBusMessage msg = QDBusMessage::createMethodCall(
            QLatin1String(kService), QLatin1String(kPath),
            QLatin1String(kInterface), QLatin1String(method));
        msg << QLatin1String(kNamespace) << QLatin1String(kKey);

        const QDBusMessage reply = bus.call(msg, QDBus::Block, 500);
        if (reply.type() == QDBusMessage::ReplyMessage &&
            !reply.arguments().isEmpty())
        {
            const QColor c = colourFromReply(reply.arguments().constFirst());
            if (c.isValid())
                return c;
        }
    }
#endif
    return QColor();
}

void Cb_StartPortalAccentWatch(void (*onChanged)())
{
#ifdef CB_PORTAL_ACCENT
    if (!onChanged)
        return;
    static PortalWatcher* watcher = nullptr;
    if (watcher)                      // once per process
        return;
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return;
    watcher = new PortalWatcher(onChanged);
    bus.connect(QLatin1String(kService), QLatin1String(kPath),
                QLatin1String(kInterface), QLatin1String("SettingChanged"),
                watcher, SLOT(onSettingChanged(QString,QString,QDBusVariant)));
#else
    Q_UNUSED(onChanged)
#endif
}

void Cb_ApplyDesktopIconTheme()
{
#ifdef __linux__
    // gsettings is the only reader for this: GNOME keeps the icon theme in
    // dconf. /etc/gtk-3.0/settings.ini holds the DISTRO default (measured:
    // "Yaru" while the desktop was actually on "Yaru-prussiangreen") and
    // ~/.config/gtk-*/settings.ini does not exist under GNOME at all. One
    // short-lived process, run at startup and again just before a file dialog.
    QProcess gs;
    gs.start(QStringLiteral("gsettings"),
             { QStringLiteral("get"),
               QStringLiteral("org.gnome.desktop.interface"),
               QStringLiteral("icon-theme") });
    if (!gs.waitForFinished(1000) || gs.exitCode() != 0)
        return;                       // no gsettings / not GNOME -- keep Qt's own

    QString name = QString::fromUtf8(gs.readAllStandardOutput()).trimmed();
    name.remove(QLatin1Char('\''));   // gsettings quotes the value
    name.remove(QLatin1Char('"'));
    if (name.isEmpty() || name == QIcon::themeName())
        return;

    // Only switch to a theme that is actually installed: a name Qt cannot find
    // leaves it with NO icons at all, which is worse than the wrong ones.
    for (const QString& dir : QIcon::themeSearchPaths())
        if (QDir(dir).exists(name + QStringLiteral("/index.theme")))
        {
            QIcon::setThemeName(name);
            return;
        }
#endif
}


QFileDialog::Options Cb_FileDialogOptions()
{
    // Refresh FIRST, before the dialog is built: Qt's own dialog draws its
    // folders from the icon theme, and a static Qt has no platform theme to keep
    // that current. Reading it here rather than on the accent signal also dodges
    // a race -- GNOME writes the new icon theme to dconf a moment AFTER that
    // signal, which left CB one change behind.
    Cb_ApplyDesktopIconTheme();

#ifdef _WIN32
    // Windows keeps its NATIVE panel: it scales and filters correctly there.
    return QFileDialog::Options();
#else
    // macOS + Linux use Qt's OWN dialog. macOS: the native NSOpenPanel/NSSavePanel
    // behind QFileDialog's static helpers shows nothing in this app. Linux: the
    // native GTK/portal chooser ignores QT_SCALE_FACTOR (so it renders tiny under
    // a non-1.0 View > UI Scale) and shows ALL files with the non-matching ones
    // greyed out, so a full directory needs scrolling to reach the .cbz. Qt's own
    // dialog scales with the rest of CB and hides non-matching files.
    return QFileDialog::DontUseNativeDialog;
#endif
}

#ifdef CB_PORTAL_ACCENT
#include "QtDesktopTheme.moc"
#endif
