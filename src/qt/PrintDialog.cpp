// qt/PrintDialog.cpp -- the Qt Print dialog.
//
// Ported from the MFC PrintDialog. Picks the page count a diagram prints
// across and shows a printer-orientation reminder. Pure UI -- the bridge
// reads the diagram, the dialog itself touches no model.

#include "PrintDialog.h"
#include "ui_PrintDialog.h"

#include "QtPrintDialog.h"           // Qt_ShowPrintDialogClass / ...Sequence
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal

#include <QRadioButton>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

PrintDialog::PrintDialog(bool portraitDiagram, bool sequenceDiagram,
                         int initialIndex, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::PrintDialog)
    , _portraitDiagram(portraitDiagram)
{
    _ui->setupUi(this);

    if (sequenceDiagram)
        setWindowTitle("Print SequenceDiagram");

    QRadioButton* radios[5] = {
        _ui->radioPages1, _ui->radioPages2, _ui->radioPages4,
        _ui->radioPages8, _ui->radioPages16 };

    if (initialIndex < 0 || initialIndex > 4)
        initialIndex = 0;
    radios[initialIndex]->setChecked(true);

    for (QRadioButton* r : radios)
        connect(r, &QRadioButton::toggled, this,
                [this] { updateOrientation(); });

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    updateOrientation();
}

PrintDialog::~PrintDialog()
{
    delete _ui;
}

int PrintDialog::selectedIndex() const
{
    if (_ui->radioPages2->isChecked())  return 1;
    if (_ui->radioPages4->isChecked())  return 2;
    if (_ui->radioPages8->isChecked())  return 3;
    if (_ui->radioPages16->isChecked()) return 4;
    return 0;
}

int PrintDialog::numberOfPages() const
{
    return 1 << selectedIndex();
}

// Even page-counts (1/4/16) keep the diagram's own orientation; odd ones
// (2/8) rotate it -- mirrors the MFC OnNumberofpagesN handlers.
void PrintDialog::updateOrientation()
{
    const bool printerPortrait =
        (selectedIndex() % 2) == 0 ? _portraitDiagram : !_portraitDiagram;
    _ui->labelOrientation->setText(printerPortrait
        ? "Don't forget to set printer in portrait !!"
        : "Don't forget to set printer in landscape !!");
}

// ---------------------------------------------------------------------------
// MFC entry points
// ---------------------------------------------------------------------------
int Qt_ShowPrintDialogClass(ClassDiagram* pClassDiagram, void* ownerHwnd)
{
    Qt_EnsureApplication();

    PrintDialog dlg(pClassDiagram->GetWidth() < pClassDiagram->GetHeight(),
                    false, pClassDiagram->GetMultiPage());
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return 0;
    return dlg.numberOfPages();
}

int Qt_ShowPrintDialogSequence(SequenceDiagram* pSequenceDiagram,
                               void* ownerHwnd)
{
    Qt_EnsureApplication();

    PrintDialog dlg(
        pSequenceDiagram->GetWidth() < pSequenceDiagram->GetHeight(),
        true, pSequenceDiagram->GetMultiPage());
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return 0;
    return dlg.numberOfPages();
}
