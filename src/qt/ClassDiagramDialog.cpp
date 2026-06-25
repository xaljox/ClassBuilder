// qt/ClassDiagramDialog.cpp -- the Qt ClassDiagram properties dialog.
//
// Ported from the MFC CClassDiagramDialog. Drives the model directly.
//
// As with SequenceDiagramDialog: on a page-size change the MFC Update()
// re-zoomed the ClassDiagramView objects. ClassDiagramView is MFC-only
// (FORWARD_ONLY here), so that step is reported via sizeChanged() and done
// by the MFC caller.

#include "ClassDiagramDialog.h"
#include "ui_ClassDiagramDialog.h"

#include "QtClassDiagramDialog.h"      // Qt_ShowClassDiagramDialog
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

ClassDiagramDialog::ClassDiagramDialog(ClassDiagram* pClassDiagram,
                                       QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::ClassDiagramDialog)
    , _pClassDiagram(pClassDiagram)
{
    _ui->setupUi(this);

    _ui->lineEditName->setText(toQ(_pClassDiagram->GetName()));
    _ui->plainTextNote->setPlainText(toQ(_pClassDiagram->GetNote()));
    _ui->lineEditCaption->setText(toQ(_pClassDiagram->GetCaption()));
    _ui->spinScale->setValue(_pClassDiagram->GetScale());

    _ui->checkPrivateMembers->setChecked(
        _pClassDiagram->GetPrivateMembers());
    _ui->checkPrivateMethods->setChecked(
        _pClassDiagram->GetPrivateMethods());
    _ui->checkProtectedMembers->setChecked(
        _pClassDiagram->GetProtectedMembers());
    _ui->checkProtectedMethods->setChecked(
        _pClassDiagram->GetProtectedMethods());
    _ui->checkPublicMembers->setChecked(
        _pClassDiagram->GetPublicMembers());
    _ui->checkPublicMethods->setChecked(
        _pClassDiagram->GetPublicMethods());
    _ui->checkGetSetMethods->setChecked(
        _pClassDiagram->GetGetSetMethods());

    // Multi-page: 0..4 (1/2/4/8/16 pages).
    QRadioButton* multiPage[] = {
        _ui->radioPage1, _ui->radioPage2, _ui->radioPage4,
        _ui->radioPage8, _ui->radioPage16 };
    int mp = _pClassDiagram->GetMultiPage();
    if (mp < 0 || mp > 4) mp = 0;
    multiPage[mp]->setChecked(true);

    // Orientation: keep _width <= _height (portrait), the radio says if the
    // diagram is actually rotated.
    const unsigned short w = _pClassDiagram->GetWidth();
    const unsigned short h = _pClassDiagram->GetHeight();
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
            this, &ClassDiagramDialog::onPaperSizeChanged);
    connect(_ui->radioLandscape, &QRadioButton::toggled,
            this, &ClassDiagramDialog::updatePageIllustration);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ClassDiagramDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ClassDiagramDialog::reject);

    updatePageIllustration();
}

// The multi-page group shows a page-layout illustration -- portrait or
// landscape depending on the orientation radios (res/print*.bmp).
void ClassDiagramDialog::updatePageIllustration()
{
    _ui->labelPageIllustration->setPixmap(QPixmap(
        _ui->radioLandscape->isChecked() ? ":/printlandscape.bmp"
                                         : ":/printportrait.bmp"));
}

ClassDiagramDialog::~ClassDiagramDialog()
{
    delete _ui;
}

void ClassDiagramDialog::onPaperSizeChanged()
{
    const int data = _ui->comboPageSize->currentData().toInt();
    if (data != 0)
    {
        _width  = static_cast<unsigned short>((data >> 16) & 0xFFFF);
        _height = static_cast<unsigned short>(data & 0xFFFF);
    }
}

// OK -- apply every changed field to the model (the MFC Update()).
void ClassDiagramDialog::accept()
{
    const QString qName    = _ui->lineEditName->text();
    const QString qNote    = _ui->plainTextNote->toPlainText();
    const QString qCaption = _ui->lineEditCaption->text();
    const int scale        = _ui->spinScale->value();
    const bool privMembers = _ui->checkPrivateMembers->isChecked();
    const bool privMethods = _ui->checkPrivateMethods->isChecked();
    const bool protMembers = _ui->checkProtectedMembers->isChecked();
    const bool protMethods = _ui->checkProtectedMethods->isChecked();
    const bool pubMembers  = _ui->checkPublicMembers->isChecked();
    const bool pubMethods  = _ui->checkPublicMethods->isChecked();
    const bool getSet      = _ui->checkGetSetMethods->isChecked();

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
        qName != toQ(_pClassDiagram->GetName()) ||
        qNote != toQ(_pClassDiagram->GetNote()) ||
        qCaption != toQ(_pClassDiagram->GetCaption()) ||
        multiPage != _pClassDiagram->GetMultiPage() ||
        scale != _pClassDiagram->GetScale() ||
        privMembers != _pClassDiagram->GetPrivateMembers() ||
        privMethods != _pClassDiagram->GetPrivateMethods() ||
        protMembers != _pClassDiagram->GetProtectedMembers() ||
        protMethods != _pClassDiagram->GetProtectedMethods() ||
        pubMembers != _pClassDiagram->GetPublicMembers() ||
        pubMethods != _pClassDiagram->GetPublicMethods() ||
        getSet != _pClassDiagram->GetGetSetMethods();

    const bool sizeDiff = width != _pClassDiagram->GetWidth() ||
                          height != _pClassDiagram->GetHeight();

    if (fieldsChanged || sizeDiff)
        _pClassDiagram->SaveState();

    if (fieldsChanged)
    {
        _pClassDiagram->SetName(toCb(qName));
        _pClassDiagram->SetNote(toCb(qNote));
        _pClassDiagram->SetCaption(toCb(qCaption));
        _pClassDiagram->SetMultiPage(multiPage);
        _pClassDiagram->SetScale(scale);
        _pClassDiagram->SetPrivateMembers(privMembers);
        _pClassDiagram->SetPrivateMethods(privMethods);
        _pClassDiagram->SetProtectedMembers(protMembers);
        _pClassDiagram->SetProtectedMethods(protMethods);
        _pClassDiagram->SetPublicMembers(pubMembers);
        _pClassDiagram->SetPublicMethods(pubMethods);
        _pClassDiagram->SetGetSetMethods(getSet);
        _modelChanged = true;
    }

    if (sizeDiff)
    {
        _pClassDiagram->SetWidth(width);
        _pClassDiagram->SetHeight(height);
        _modelChanged = true;
        _sizeChanged  = true;   // caller re-zooms the views
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowClassDiagramDialog(ClassDiagram* pClassDiagram,
                               bool& modelChangedOut, bool& sizeChangedOut,
                               void* ownerHwnd)
{
    Qt_EnsureApplication();

    ClassDiagramDialog dlg(pClassDiagram);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    sizeChangedOut  = dlg.sizeChanged();
    return true;
}
