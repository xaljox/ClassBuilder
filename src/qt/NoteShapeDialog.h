// qt/NoteShapeDialog.h -- the Qt Note properties dialog.
//
// The form lives in NoteShapeDialog.ui (Qt Designer). Serves both the
// class-diagram note (NoteShape) and the sequence-diagram note (SDNoteShape):
// it is constructed with whichever one applies and drives that model object
// directly.
#pragma once

#include <QDialog>

class NoteShape;
class SDNoteShape;

namespace Ui { class NoteShapeDialog; }

class NoteShapeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NoteShapeDialog(NoteShape* pNoteShape, QWidget* parent = nullptr);
    explicit NoteShapeDialog(SDNoteShape* pSDNoteShape,
                             QWidget* parent = nullptr);
    ~NoteShapeDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }

private:
    void init(const QString& note, int fontHeight);   // shared ctor setup
    void accept() override;                           // apply edits

    Ui::NoteShapeDialog* _ui;
    NoteShape*           _pNoteShape   = nullptr;
    SDNoteShape*         _pSDNoteShape = nullptr;
    bool                 _modelChanged = false;
};
