// qt/QtMessageBox.cpp -- Qt backend for CbMessageBox (see CbMessageBox.h).
//
// WHY a QMessageBox and not ::MessageBox: a native box is not a Qt window, so
// while it is up Qt's application state drops to Inactive; dismissing it
// re-fires applicationStateChanged(Active), which re-ran the shell's
// CheckUpdates activation hook -- the "read changed sources?" prompt looped. A
// QMessageBox keeps the application state Active throughout.
//
// Maps the windows.h-free CBMB_* flags to QMessageBox. No windows.h here.

#include "CbMessageBox.h"

#include "QtApp.h"   // Qt_EnsureApplication

#include <QApplication>
#include <QMessageBox>

int CbMessageBox(const char* text, unsigned int type)
{
    Qt_EnsureApplication();

    QMessageBox box(QApplication::activeWindow());
    box.setWindowTitle("ClassBuilder");
    box.setText(QString::fromLocal8Bit(text));

    switch (type & CBMB_ICONMASK)
    {
    case CBMB_ICONQUESTION:    box.setIcon(QMessageBox::Question); break;
    case CBMB_ICONEXCLAMATION: box.setIcon(QMessageBox::Warning);  break;
    case CBMB_ICONSTOP:        box.setIcon(QMessageBox::Critical); break;
    case CBMB_ICONINFORMATION: box.setIcon(QMessageBox::Information); break;
    default:                   box.setIcon(QMessageBox::NoIcon);   break;
    }

    switch (type & CBMB_TYPEMASK)
    {
    case CBMB_YESNO:
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setDefaultButton((type & CBMB_DEFMASK) == CBMB_DEFBUTTON2
                                 ? QMessageBox::No : QMessageBox::Yes);
        break;
    case CBMB_YESNOCANCEL:
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No |
                               QMessageBox::Cancel);
        box.setDefaultButton((type & CBMB_DEFMASK) == CBMB_DEFBUTTON2
                                 ? QMessageBox::No : QMessageBox::Yes);
        break;
    case CBMB_OKCANCEL:
        box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        box.setDefaultButton((type & CBMB_DEFMASK) == CBMB_DEFBUTTON2
                                 ? QMessageBox::Cancel : QMessageBox::Ok);
        break;
    default:
        box.setStandardButtons(QMessageBox::Ok);
        break;
    }

    switch (box.exec())
    {
    case QMessageBox::Yes:    return CBMB_IDYES;
    case QMessageBox::No:     return CBMB_IDNO;
    case QMessageBox::Cancel: return CBMB_IDCANCEL;
    default:                  return CBMB_IDOK;
    }
}
