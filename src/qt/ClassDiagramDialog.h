// qt/ClassDiagramDialog.h -- the Qt ClassDiagram properties dialog.
//
// The form lives in ClassDiagramDialog.ui (Qt Designer). Drives the model
// directly. The page width/height (set from a paper-size combo + orientation)
// is applied here, but re-zooming the views afterward is left to the MFC
// caller -- ClassDiagramView is an MFC-only class this TU cannot reach.
#pragma once

#include <QDialog>

class ClassDiagram;

namespace Ui { class ClassDiagramDialog; }

class ClassDiagramDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClassDiagramDialog(ClassDiagram* pClassDiagram,
                                QWidget* parent = nullptr);
    ~ClassDiagramDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }
    bool sizeChanged()  const { return _sizeChanged; }

private slots:
    void onPaperSizeChanged();       // combo -> _width / _height
    void updatePageIllustration();   // orientation -> portrait/landscape bitmap

private:
    void accept() override;          // apply edits to the model

    Ui::ClassDiagramDialog* _ui;
    ClassDiagram*           _pClassDiagram;
    unsigned short          _width  = 0;   // portrait orientation (w <= h)
    unsigned short          _height = 0;
    bool                    _modelChanged = false;
    bool                    _sizeChanged  = false;
};
