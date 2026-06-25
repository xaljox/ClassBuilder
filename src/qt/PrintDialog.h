// qt/PrintDialog.h -- the Qt Print dialog.
//
// The form lives in PrintDialog.ui (Qt Designer). Ported from the MFC
// PrintDialog: pick how many pages a diagram is printed across (1/2/4/8/16)
// and show a printer-orientation reminder. Pure UI -- the model is not
// touched; the chosen page count is returned to the caller.
#pragma once

#include <QDialog>

namespace Ui { class PrintDialog; }

class PrintDialog : public QDialog
{
    Q_OBJECT
public:
    // `portraitDiagram` is true when the diagram is taller than it is wide;
    // `initialIndex` is the diagram's stored multi-page index (0..4).
    PrintDialog(bool portraitDiagram, bool sequenceDiagram,
                int initialIndex, QWidget* parent = nullptr);
    ~PrintDialog();

    // The chosen page count: 1 << selectedIndex (1, 2, 4, 8 or 16).
    int numberOfPages() const;

private:
    void updateOrientation();
    int  selectedIndex() const;

    Ui::PrintDialog* _ui;
    bool _portraitDiagram;
};
