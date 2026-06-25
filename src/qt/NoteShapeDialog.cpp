// qt/NoteShapeDialog.cpp -- the Qt Note properties dialog.
//
// Ported from the MFC NoteShapeDialog. One dialog for both the class-diagram
// note (NoteShape) and the sequence-diagram note (SDNoteShape); it holds
// whichever applies and drives it directly.

#include "NoteShapeDialog.h"
#include "ui_NoteShapeDialog.h"

#include "QtNoteShapeDialog.h"   // Qt_ShowNoteShapeDialog
#include "QtApp.h"               // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"         // toQ / toCb (CRLF-aware)
#include "CbPainter_QFontMetrics.h" // off-view note-height recompute at the edit boundary

#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

NoteShapeDialog::NoteShapeDialog(NoteShape* pNoteShape, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::NoteShapeDialog)
    , _pNoteShape(pNoteShape)
{
    init(toQ(pNoteShape->GetNote()), pNoteShape->GetFontHeight());
}

NoteShapeDialog::NoteShapeDialog(SDNoteShape* pSDNoteShape, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::NoteShapeDialog)
    , _pSDNoteShape(pSDNoteShape)
{
    init(toQ(pSDNoteShape->GetNote()), pSDNoteShape->GetFontHeight());
}

NoteShapeDialog::~NoteShapeDialog()
{
    delete _ui;
}

// Shared constructor body -- the two shape types differ only in their getters.
void NoteShapeDialog::init(const QString& note, int fontHeight)
{
    _ui->setupUi(this);
    _ui->plainTextNote->setPlainText(note);
    _ui->spinFontHeight->setValue(fontHeight);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &NoteShapeDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &NoteShapeDialog::reject);
}

// OK -- apply edits to the note shape (the MFC OnOK).
void NoteShapeDialog::accept()
{
    const QString qNote      = _ui->plainTextNote->toPlainText();
    const int     fontHeight = _ui->spinFontHeight->value();
    const bool    force      = _ui->checkForceFontSize->isChecked();

    // Off-view measure for the height recompute that NoteShape/SDNoteShape::Draw no
    // longer does during paint. RecalcHeight re-derives rect.top from the wrapped
    // text (same QFontMetrics mapping as the on-screen painter); the SaveState above
    // already snapshots the old rect, so it lands in this edit's undo step.
    CbPainter_QFontMetrics m;

    if (_pNoteShape)
    {
        if (qNote != toQ(_pNoteShape->GetNote()) ||
            fontHeight != _pNoteShape->GetFontHeight())
        {
            _pNoteShape->SaveState();
            _pNoteShape->SetNote(toCb(qNote));
            _pNoteShape->SetFontHeight(fontHeight);
            _pNoteShape->RecalcHeight(m);
            _modelChanged = true;
        }

        if (force)
        {
            ClassDiagram::ClassDiagramShapeIterator
                iShape(_pNoteShape->GetClassDiagram());
            while (++iShape)
            {
                NoteShape* p = dynamic_cast<NoteShape*>(iShape.Get());
                if (p && p->GetFontHeight() != fontHeight)
                {
                    p->SaveState();
                    p->SetFontHeight(fontHeight);
                    p->RecalcHeight(m);
                    _modelChanged = true;
                }
            }
        }
    }

    if (_pSDNoteShape)
    {
        if (qNote != toQ(_pSDNoteShape->GetNote()) ||
            fontHeight != _pSDNoteShape->GetFontHeight())
        {
            _pSDNoteShape->SaveState();
            _pSDNoteShape->SetNote(toCb(qNote));
            _pSDNoteShape->SetFontHeight(fontHeight);
            _pSDNoteShape->RecalcHeight(m);
            _modelChanged = true;
        }

        if (force)
        {
            SequenceDiagram::SequenceDiagramShapeIterator
                iShape(_pSDNoteShape->GetSequenceDiagram());
            while (++iShape)
            {
                SDNoteShape* p = dynamic_cast<SDNoteShape*>(iShape.Get());
                if (p && p->GetFontHeight() != fontHeight)
                {
                    p->SaveState();
                    p->SetFontHeight(fontHeight);
                    p->RecalcHeight(m);
                    _modelChanged = true;
                }
            }
        }
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry points
// ---------------------------------------------------------------------------
bool Qt_ShowNoteShapeDialog(NoteShape* pNoteShape, bool& modelChangedOut,
                            void* ownerHwnd)
{
    Qt_EnsureApplication();

    NoteShapeDialog dlg(pNoteShape);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    return true;
}

bool Qt_ShowNoteShapeDialog(SDNoteShape* pSDNoteShape, bool& modelChangedOut,
                            void* ownerHwnd)
{
    Qt_EnsureApplication();

    NoteShapeDialog dlg(pSDNoteShape);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    modelChangedOut = dlg.modelChanged();
    return true;
}
