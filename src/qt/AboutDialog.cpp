// qt/AboutDialog.cpp -- the Qt "About ClassBuilder" dialog + the MFC bridge.
//
// This translation unit is pure Qt: it includes no MFC / afxwin.h headers,
// so it compiles cleanly alongside the MFC ClassBuilder.cpp in the same EXE.

#include "AboutDialog.h"
#include "QtAbout.h"
#include "QtApp.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("About ClassBuilder"));
    setModal(true);

    const QIcon appIcon(QStringLiteral(":/classbuilder.png"));
    setWindowIcon(appIcon);

    // The app icon (IDI_CLASS in the MFC build), shown left of the title --
    // the MFC About box had it as a static control in the same position.
    auto* icon = new QLabel(this);
    icon->setPixmap(QPixmap(QStringLiteral(":/classbuilder.png")));

    auto* title = new QLabel(QStringLiteral("ClassBuilder"), this);
    {
        QFont f = title->font();
        f.setPointSize(f.pointSize() + 6);
        f.setBold(true);
        title->setFont(f);
    }

    auto* version = new QLabel(QStringLiteral("Version 3.0"), this);

    auto* blurb = new QLabel(
        QStringLiteral("A refreshed, multi-platform release "
                       "with a new storage format."),
        this);
    blurb->setWordWrap(true);

    auto* copyright = new QLabel(
        QStringLiteral("Copyright © 2002–2026  Jimmy Venema"), this);

    auto* email = new QLabel(
        QStringLiteral("<a href=\"mailto:jimmy.venema@gmail.com\">"
                       "jimmy.venema@gmail.com</a>"),
        this);
    email->setTextFormat(Qt::RichText);
    email->setTextInteractionFlags(Qt::TextBrowserInteraction);
    email->setOpenExternalLinks(true);

    // QT_VERSION_STR is the Qt version this EXE was built against -- so the
    // line stays correct automatically if the Qt kit is ever updated.
    auto* builtWith = new QLabel(
        QStringLiteral("Built with Qt %1").arg(QStringLiteral(QT_VERSION_STR)),
        this);
    {
        QFont f = builtWith->font();
        f.setPointSize(f.pointSize() - 1);
        builtWith->setFont(f);
        builtWith->setEnabled(false); // dimmed -- it is a footnote
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    // Header row: icon on the left, title + version stacked to its right.
    auto* titleCol = new QVBoxLayout;
    titleCol->setSpacing(2);
    titleCol->addWidget(title);
    titleCol->addWidget(version);
    titleCol->addStretch(1);

    auto* header = new QHBoxLayout;
    header->setSpacing(14);
    header->addWidget(icon, 0, Qt::AlignTop);
    header->addLayout(titleCol, 1);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 16);
    layout->setSpacing(6);
    layout->addLayout(header);
    layout->addSpacing(10);
    layout->addWidget(blurb);
    layout->addSpacing(10);
    layout->addWidget(copyright);
    layout->addWidget(email);
    layout->addSpacing(14);
    layout->addWidget(builtWith);
    layout->addSpacing(4);
    layout->addWidget(buttons);

    setFixedSize(sizeHint());
}

// --- MFC bridge (declared in QtAbout.h) ------------------------------------

void Qt_ShowAboutDialog(void* ownerHwnd)
{
    Qt_EnsureApplication();

    // resources.qrc is compiled into the ClassBuilderQt *static* lib. Its
    // resource auto-registration sits in its own object file, which the
    // linker discards from the final binary unless a symbol in it is
    // referenced -- so :/classbuilder.png silently fails to register.
    // Q_INIT_RESOURCE forces that reference. Safe to call repeatedly.
    Q_INIT_RESOURCE(resources);

    AboutDialog dlg;
    Qt_ExecModal(dlg, ownerHwnd); // modal over the MFC owner window
}
