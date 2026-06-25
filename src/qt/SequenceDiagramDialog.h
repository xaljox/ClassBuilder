// qt/SequenceDiagramDialog.h -- the Qt SequenceDiagram properties dialog.
//
// The form lives in SequenceDiagramDialog.ui (Qt Designer). Drives the model
// directly. The page width/height (set from a paper-size combo + orientation)
// is applied here, but re-zooming the views afterward is left to the MFC
// caller -- SequenceDiagramView is an MFC-only class this TU cannot reach.
#pragma once

#include <QDialog>

class SequenceDiagram;

namespace Ui { class SequenceDiagramDialog; }

class SequenceDiagramDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SequenceDiagramDialog(SequenceDiagram* pSequenceDiagram,
                                   QWidget* parent = nullptr);
    ~SequenceDiagramDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }
    bool sizeChanged()  const { return _sizeChanged; }

private slots:
    void onPaperSizeChanged();       // combo -> _width / _height
    void updatePageIllustration();   // orientation -> portrait/landscape bitmap

private:
    void accept() override;          // apply edits to the model

    Ui::SequenceDiagramDialog* _ui;
    SequenceDiagram*           _pSequenceDiagram;
    unsigned short             _width  = 0;   // portrait orientation (w <= h)
    unsigned short             _height = 0;
    bool                       _modelChanged = false;
    bool                       _sizeChanged  = false;
};
