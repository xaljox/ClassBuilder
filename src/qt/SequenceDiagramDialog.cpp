// qt/SequenceDiagramDialog.cpp -- the Qt SequenceDiagram properties dialog.
//
// Ported from the MFC CSequenceDiagramDialog. Drives the model directly.
//
// Two notes on the port:
//  - The MFC dialog had a SS_BITMAP "portrait/landscape" illustration in the
//    multi-page group, but the .rc wired it to method-icon resources
//    (placeholders) -- it was never a real illustration, so it is dropped.
//    The Portrait/Landscape radios carry the function.
//  - On a page-size change the MFC Update() re-zoomed the SequenceDiagramView
//    objects. SequenceDiagramView is MFC-only (FORWARD_ONLY here), so that
//    step is reported via sizeChanged() and done by the MFC caller.

#include "SequenceDiagramDialog.h"
#include "ui_SequenceDiagramDialog.h"

#include "QtSequenceDiagramDialog.h"   // Qt_ShowSequenceDiagramDialog
#include "QtApp.h"                     // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"               // toQ / toCb
#include "QtCompact.h"                 // compactCombo

#include <QComboBox>
#include <QDialogButtonBox>
#include <QPixmap>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

struct PaperSize { const char* name; unsigned short width; unsigned short height; };

const PaperSize kPaperSizes[] = {
    { "A0 (841x1189 mm)",   8410, 11890 },
    { "A1 (594x841 mm)",    5940,  8410 },
    { "A2 (420x594 mm)",    4200,  5940 },
    { "A3 (297x420 mm)",    2970,  4200 },
    { "A4 (210x297 mm)",    2100,  2970 },
    { "B0 (1000x1414 mm)", 10000, 14140 },
    { "B1 (707x1000 mm)",   7070, 10000 },
    { "B2 (500x707 mm)",    5000,  7070 },
    { "B3 (353x500 mm)",    3530,  5000 },
    { "B4 (250x353 mm)",    2500,  3530 },
    { "B5 (176x250 mm)",    1760,  2500 },
    { "Legal (8.5x14 in)",  2159,  3556 },
    { "Letter (8.5x11 in)", 2159,  2794 },
    { "Quarto (215x275 mm)",2150,  2750 },
    { "Tabloid (11x17 in)", 2794,  4318 },
};

// width/height packed into one int -- the combo item's user data.
int packSize(unsigned short w, unsigned short h) { return (int(w) << 16) | h; }

} // namespace

SequenceDiagramDialog::SequenceDiagramDialog(SequenceDiagram* pSequenceDiagram,
                                             QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::SequenceDiagramDialog)
    , _pSequenceDiagram(pSequenceDiagram)
{
    _ui->setupUi(this);

    _ui->lineEditName->setText(toQ(_pSequenceDiagram->GetName()));
    _ui->plainTextNote->setPlainText(toQ(_pSequenceDiagram->GetNote()));
    _ui->lineEditCaption->setText(toQ(_pSequenceDiagram->GetCaption()));
    _ui->spinScale->setValue(_pSequenceDiagram->GetScale());

    _ui->checkArgumentTypes->setChecked(_pSequenceDiagram->GetArguments());
    _ui->checkArgumentNames->setChecked(
        _pSequenceDiagram->GetArgumentNames());
    _ui->checkScope->setChecked(_pSequenceDiagram->GetScope());

    // Message numbering: 0..6.
    QRadioButton* numbering[] = {
        _ui->radioNumNone, _ui->radioNum1, _ui->radioNum111, _ui->radioNumALo,
        _ui->radioNumAaa, _ui->radioNumAUp, _ui->radioNumAAA };
    int n = _pSequenceDiagram->GetNumbering();
    if (n < 0 || n > 6) n = 0;
    numbering[n]->setChecked(true);

    // Multi-page: 0..4 (1/2/4/8/16 pages).
    QRadioButton* multiPage[] = {
        _ui->radioPage1, _ui->radioPage2, _ui->radioPage4,
        _ui->radioPage8, _ui->radioPage16 };
    int mp = _pSequenceDiagram->GetMultiPage();
    if (mp < 0 || mp > 4) mp = 0;
    multiPage[mp]->setChecked(true);

    // Orientation: keep _width <= _height (portrait), the radio says if the
    // diagram is actually rotated.
    const unsigned short w = _pSequenceDiagram->GetWidth();
    const unsigned short h = _pSequenceDiagram->GetHeight();
    if (w <= h)
    {
        _width = w; _height = h;
        _ui->radioPortrait->setChecked(true);
    }
    else
    {
        _width = h; _height = w;
        _ui->radioLandscape->setChecked(true);
    }

    // Page-size combo.
    for (const PaperSize& ps : kPaperSizes)
        _ui->comboPageSize->addItem(ps.name,
                                    packSize(ps.width, ps.height));
    _ui->comboPageSize->model()->sort(0);   // CBS_SORT
    compactCombo(_ui->comboPageSize);
    for (int i = 0; i < _ui->comboPageSize->count(); ++i)
    {
        if (_ui->comboPageSize->itemData(i).toInt() == packSize(_width, _height))
        {
            _ui->comboPageSize->setCurrentIndex(i);
            break;
        }
    }

    connect(_ui->comboPageSize, &QComboBox::currentIndexChanged,
            this, &SequenceDiagramDialog::onPaperSizeChanged);
    connect(_ui->radioLandscape, &QRadioButton::toggled,
            this, &SequenceDiagramDialog::updatePageIllustration);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SequenceDiagramDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &SequenceDiagramDialog::reject);

    updatePageIllustration();
}

// The multi-page group shows a page-layout illustration -- portrait or
// landscape depending on the orientation radios (res/print*.bmp).
void SequenceDiagramDialog::updatePageIllustration()
{
    _ui->labelPageIllustration->setPixmap(QPixmap(
        _ui->radioLandscape->isChecked() ? ":/printlandscape.bmp"
                                         : ":/printportrait.bmp"));
}

SequenceDiagramDialog::~SequenceDiagramDialog()
{
    delete _ui;
}

void SequenceDiagramDialog::onPaperSizeChanged()
{
    const int data = _ui->comboPageSize->currentData().toInt();
    if (data != 0)
    {
        _width  = static_cast<unsigned short>((data >> 16) & 0xFFFF);
        _height = static_cast<unsigned short>(data & 0xFFFF);
    }
}

// OK -- apply every changed field to the model (the MFC Update()).
void SequenceDiagramDialog::accept()
{
    const QString qName    = _ui->lineEditName->text();
    const QString qNote    = _ui->plainTextNote->toPlainText();
    const QString qCaption = _ui->lineEditCaption->text();
    const int scale        = _ui->spinScale->value();
    const bool argTypes    = _ui->checkArgumentTypes->isChecked();
    const bool argNames    = _ui->checkArgumentNames->isChecked();
    const bool scope       = _ui->checkScope->isChecked();

    int numbering = 0;
    QRadioButton* numButtons[] = {
        _ui->radioNumNone, _ui->radioNum1, _ui->radioNum111, _ui->radioNumALo,
        _ui->radioNumAaa, _ui->radioNumAUp, _ui->radioNumAAA };
    for (int i = 0; i < 7; ++i)
        if (numButtons[i]->isChecked()) numbering = i;

    int multiPage = 0;
    QRadioButton* mpButtons[] = {
        _ui->radioPage1, _ui->radioPage2, _ui->radioPage4,
        _ui->radioPage8, _ui->radioPage16 };
    for (int i = 0; i < 5; ++i)
        if (mpButtons[i]->isChecked()) multiPage = i;

    // The on-disk width/height are rotated for landscape.
    unsigned short width = _width, height = _height;
    if (_ui->radioLandscape->isChecked())
    {
        width = _height;
        height = _width;
    }

    const bool fieldsChanged =
        qName != toQ(_pSequenceDiagram->GetName()) ||
        qNote != toQ(_pSequenceDiagram->GetNote()) ||
        qCaption != toQ(_pSequenceDiagram->GetCaption()) ||
        multiPage != _pSequenceDiagram->GetMultiPage() ||
        scale != _pSequenceDiagram->GetScale() ||
        argTypes != _pSequenceDiagram->GetArguments() ||
        argNames != _pSequenceDiagram->GetArgumentNames() ||
        scope != _pSequenceDiagram->GetScope() ||
        numbering != _pSequenceDiagram->GetNumbering();

    const bool sizeDiff = width != _pSequenceDiagram->GetWidth() ||
                          height != _pSequenceDiagram->GetHeight();

    if (fieldsChanged || sizeDiff)
        _pSequenceDiagram->SaveState();

    if (fieldsChanged)
    {
        _pSequenceDiagram->SetName(toCb(qName));
        _pSequenceDiagram->SetNote(toCb(qNote));
        _pSequenceDiagram->SetCaption(toCb(qCaption));
        _pSequenceDiagram->SetMultiPage(multiPage);
        _pSequenceDiagram->SetScale(scale);
        _pSequenceDiagram->SetArguments(argTypes);
        _pSequenceDiagram->SetArgumentNames(argNames);
        _pSequenceDiagram->SetScope(scope);
        _pSequenceDiagram->SetNumbering(SeqType(numbering));
        _modelChanged = true;
    }

    if (sizeDiff)
    {
        _pSequenceDiagram->SetWidth(width);
        _pSequenceDiagram->SetHeight(height);
        _modelChanged = true;
        _sizeChanged  = true;   // caller re-zooms the views
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowSequenceDiagramDialog(SequenceDiagram* pSequenceDiagram,
                                  bool& modelChangedOut, bool& sizeChangedOut,
                                  void* ownerHwnd)
{
    Qt_EnsureApplication();

    SequenceDiagramDialog dlg(pSequenceDiagram);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    sizeChangedOut  = dlg.sizeChanged();
    return true;
}
